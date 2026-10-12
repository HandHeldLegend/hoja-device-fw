"""Check the config app's traffic to a HOJA gamepad, on its cable or through a dongle.

dump: read every static and config block over WebUSB (as the config app does), time it, measure the
live input stream, and optionally write a block back unchanged and read it again. compare: diff two
dumps (the same gamepad wired and through the dongle should give the same blocks, apart from live
values such as the battery).

    python -I scripts/hoja_config_check.py dump --via dongle --out dongle.json [--write-back]
    python -I scripts/hoja_config_check.py dump --via pad --out wired.json
    python -I scripts/hoja_config_check.py compare wired.json dongle.json

Windows, ctypes only (uses hoja_bench.py next to it).
"""
import argparse
import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hoja_bench as hb  # noqa: E402

ID_READ_CONFIG = 0x01
ID_WRITE_CONFIG = 0x02
ID_READ_STATIC = 0x03
ID_INPUT_COMMAND = 0x05
INPUT_RAW = 0xFF
CFG_BLOCKS = ["gamepad", "hover", "analog", "rgb", "trigger", "imu", "haptic", "user", "input"]
STATIC_BLOCKS = ["device", "input", "analog", "haptic", "imu", "battery", "bluetooth", "rgb"]
CHUNK_DATA_MAX = 32


def open_dev(via):
    path = hb.find_webusb(via)
    if not path:
        sys.exit("no %s WebUSB interface" % via)
    if via == "dongle" and not hb.wait_dongle_gamepad(10):
        sys.exit("no gamepad connected through the dongle")
    return hb.WebUsb(path)


def read_block(d, cmd, block, timeout=5.0):
    """Send [cmd, block], collect its chunks up to the closing one. Returns (data, seconds, others)."""
    t0 = time.perf_counter()
    if not d.write([cmd, block]):
        return None, 0.0, 0
    parts, others = {}, 0
    while time.perf_counter() - t0 < timeout:
        r = d.read()
        if not r:
            continue
        if len(r) >= 4 and r[0] == cmd and r[1] == block:
            size, part = r[2], r[3]
            if part == 0xFF:
                data = b"".join(parts[i] for i in sorted(parts))
                return data, time.perf_counter() - t0, others
            parts[part] = bytes(r[4:4 + size])
        else:
            others += 1
    return None, time.perf_counter() - t0, others


def write_block(d, block, data):
    for i in range(0, len(data), CHUNK_DATA_MAX):
        chunk = data[i:i + CHUNK_DATA_MAX]
        if not d.write([ID_WRITE_CONFIG, block, len(chunk), i // CHUNK_DATA_MAX] + list(chunk)):
            return False
    return d.write([ID_WRITE_CONFIG, block, 0, 0xFF])


def input_rate(d, seconds=5.0):
    d.write([ID_INPUT_COMMAND, 0, INPUT_RAW])
    t0 = time.perf_counter()
    n, last = 0, None
    gaps = []
    while time.perf_counter() - t0 < seconds:
        r = d.read()
        if r and r[0] == INPUT_RAW:
            now = time.perf_counter()
            if last is not None:
                gaps.append((now - last) * 1000.0)
            last = now
            n += 1
    gaps.sort()
    return {"per_second": n / seconds,
            "gap_ms_p50": gaps[len(gaps) // 2] if gaps else None,
            "gap_ms_max": gaps[-1] if gaps else None}


def dump(via, out, write_back):
    d = open_dev(via)
    res = {"via": via, "static": {}, "config": {}, "timing": {}}
    t_all = time.perf_counter()
    try:
        for i, name in enumerate(STATIC_BLOCKS):
            data, s, _ = read_block(d, ID_READ_STATIC, i)
            res["static"][name] = data.hex() if data is not None else None
            res["timing"]["static." + name] = round(s * 1000, 1)
        for i, name in enumerate(CFG_BLOCKS):
            data, s, _ = read_block(d, ID_READ_CONFIG, i)
            res["config"][name] = data.hex() if data is not None else None
            res["timing"]["config." + name] = round(s * 1000, 1)
        res["timing"]["all_blocks_ms"] = round((time.perf_counter() - t_all) * 1000, 1)
        res["input"] = input_rate(d)
        if write_back:
            name = "gamepad"
            before = res["config"][name]
            ok = before is not None and write_block(d, CFG_BLOCKS.index(name), bytes.fromhex(before))
            time.sleep(0.2)
            after, _, _ = read_block(d, ID_READ_CONFIG, CFG_BLOCKS.index(name))
            res["write_back"] = {"block": name, "sent": bool(ok),
                                 "read_back_equal": after is not None and after.hex() == before}
    finally:
        d.close()
    with open(out, "w") as f:
        json.dump(res, f, indent=1)
    missing = [k for k in ("static", "config") for n, v in res[k].items() if v is None]
    print("via %s: %d static + %d config blocks in %.0f ms, missing: %s" % (
        via, len(STATIC_BLOCKS), len(CFG_BLOCKS), res["timing"]["all_blocks_ms"], ", ".join(missing) or "none"))
    print("  slowest block: %s %.0f ms" % max(((k, v) for k, v in res["timing"].items() if k != "all_blocks_ms"),
                                            key=lambda kv: kv[1]))
    print("  live input: %(per_second).1f/s, gap p50 %(gap_ms_p50)s ms, max %(gap_ms_max)s ms" % res["input"])
    if "write_back" in res:
        print("  write back (%(block)s, unchanged): sent %(sent)s, read back equal %(read_back_equal)s" % res["write_back"])


def compare(a_path, b_path):
    a, b = json.load(open(a_path)), json.load(open(b_path))
    same = True
    for kind in ("static", "config"):
        for name in a[kind]:
            x, y = a[kind].get(name), b[kind].get(name)
            if x == y:
                continue
            same = False
            if x is None or y is None:
                print("%s.%s: missing in %s" % (kind, name, a["via"] if x is None else b["via"]))
                continue
            xb, yb = bytes.fromhex(x), bytes.fromhex(y)
            diff = [i for i in range(max(len(xb), len(yb))) if (xb[i:i + 1] or b"") != (yb[i:i + 1] or b"")]
            print("%s.%s: %d of %d bytes differ, at %s" % (kind, name, len(diff), max(len(xb), len(yb)),
                                                        ", ".join(str(i) for i in diff[:16])))
    if same:
        print("identical: every static and config block")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("dump")
    p.add_argument("--via", choices=["dongle", "pad"], required=True)
    p.add_argument("--out", required=True)
    p.add_argument("--write-back", action="store_true")
    c = sub.add_parser("compare")
    c.add_argument("a")
    c.add_argument("b")
    a = ap.parse_args()
    if a.cmd == "dump":
        dump(a.via, a.out, a.write_back)
    else:
        compare(a.a, a.b)


if __name__ == "__main__":
    main()
