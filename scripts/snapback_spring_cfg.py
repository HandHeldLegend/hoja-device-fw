"""Read or write a HOJA gamepad's Spring snapback tuning (analog settings, l_spring / r_spring).

    python -I scripts/snapback_spring_cfg.py show
    python -I scripts/snapback_spring_cfg.py set --stick left --decide-step 60 [--a1 ... --a2 ...] [--tuned] [--save]
    python -I scripts/snapback_spring_cfg.py raw --stick left --hex <74 bytes> [--save]
    python -I scripts/snapback_spring_cfg.py clear --stick left [--save]   # back to the built-in defaults
    python -I scripts/snapback_spring_cfg.py type --stick left spring [--save]  # lpf, auto, off, spring

Writes go to the live settings at once (the filter reloads); --save also stores them in flash.
"""

import argparse
import os
import struct
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hoja_bench as hb  # noqa: E402

READ_BLOCK, WRITE_BLOCK, CONFIG_COMMAND = 1, 2, 4
CFG_BLOCK_GAMEPAD, CFG_BLOCK_ANALOG = 0, 2
GAMEPAD_CMD_SAVE_ALL = 0xFF
CHUNK = 32
ANALOG_SIZE = 1024
# analogConfig_s (settings_shared_types.h): the spring blocks follow r_exp_scaler
SPRING_AT = {"left": 698, "right": 772}
SPRING_FMT = "<BB8i8iHHHH"
SPRING_SIZE = struct.calcsize(SPRING_FMT)
assert SPRING_SIZE == 74
SPRING_VERSION, FLAG_TUNED = 1, 0x01
# l/r_snapback_type, as snapback.c dispatches it
TYPE_AT = {"left": 686, "right": 687}
TYPES = {"lpf": 0, "auto": 1, "off": 2, "spring": 3}


def read_block(dev, block):
    dev.write([READ_BLOCK, block])
    parts = {}
    end = time.time() + 3
    while time.time() < end:
        r = dev.read()
        if not r or r[0] != READ_BLOCK or r[1] != block:
            continue
        size, part = r[2], r[3]
        if part == 0xFF:
            break
        parts[part] = bytes(r[4:4 + size])
    data = b"".join(parts[i] for i in sorted(parts))
    if len(data) != ANALOG_SIZE:
        raise RuntimeError("analog block came back %d bytes, not %d" % (len(data), ANALOG_SIZE))
    return bytearray(data)


def write_block(dev, block, data):
    for part in range(0, len(data), CHUNK):
        chunk = data[part:part + CHUNK]
        dev.write([WRITE_BLOCK, block, len(chunk), part // CHUNK] + list(chunk))
    dev.write([WRITE_BLOCK, block, 0, 0xFF])


def unpack(data, stick):
    v = struct.unpack_from(SPRING_FMT, data, SPRING_AT[stick])
    return {"version": v[0], "flags": v[1], "a1": list(v[2:10]), "a2": list(v[10:18]),
            "decide_step": v[18], "grab_err": v[19], "stiff_den": v[20], "cross_speed": v[21]}


def pack(cfg):
    return struct.pack(SPRING_FMT, cfg["version"], cfg["flags"], *cfg["a1"], *cfg["a2"],
                       cfg["decide_step"], cfg["grab_err"], cfg["stiff_den"], cfg["cross_speed"])


def show(cfg, stick):
    print("%s stick: version %d, %s" % (stick, cfg["version"],
                                       "tuned" if cfg["flags"] & FLAG_TUNED else "built-in defaults"))
    print("  a1 (Q16): %s" % cfg["a1"])
    print("  a2 (Q16): %s" % cfg["a2"])
    print("  decide_step %d, grab_err %d, stiff_den %d, cross_speed %d" % (
        cfg["decide_step"], cfg["grab_err"], cfg["stiff_den"], cfg["cross_speed"]))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("action", choices=["show", "set", "raw", "clear", "type"])
    ap.add_argument("type", nargs="?", choices=sorted(TYPES), help="type: the filter to use")
    ap.add_argument("--stick", choices=["left", "right"], default="left")
    ap.add_argument("--a1", type=int, nargs=8)
    ap.add_argument("--a2", type=int, nargs=8)
    for name in ("decide-step", "grab-err", "stiff-den", "cross-speed"):
        ap.add_argument("--" + name, type=int)
    ap.add_argument("--tuned", action="store_true", help="mark it as fitted to this stick")
    ap.add_argument("--hex", help="raw: the whole 74-byte block as hex")
    ap.add_argument("--save", action="store_true", help="also store the settings in flash")
    a = ap.parse_args()

    path = hb.find_webusb("pad")
    if not path:
        sys.exit("no gamepad on its cable")
    dev = hb.WebUsb(path)
    try:
        data = read_block(dev, CFG_BLOCK_ANALOG)
        names = {v: k for k, v in TYPES.items()}
        if a.action == "show":
            for stick in ("left", "right"):
                print("%s stick filter: %s" % (stick, names.get(data[TYPE_AT[stick]], data[TYPE_AT[stick]])))
                show(unpack(data, stick), stick)
            return

        if a.action == "type":
            if not a.type:
                sys.exit("type needs one of: %s" % ", ".join(sorted(TYPES)))
            data[TYPE_AT[a.stick]] = TYPES[a.type]

        elif a.action == "raw":
            raw = bytes.fromhex(a.hex)
            if len(raw) != SPRING_SIZE:
                sys.exit("--hex must be %d bytes" % SPRING_SIZE)
            data[SPRING_AT[a.stick]:SPRING_AT[a.stick] + SPRING_SIZE] = raw
        elif a.action == "clear":
            # Version 0: nothing stored, so the firmware puts its defaults back
            data[SPRING_AT[a.stick]:SPRING_AT[a.stick] + SPRING_SIZE] = bytes(SPRING_SIZE)
        else:
            cfg = unpack(data, a.stick)
            cfg["version"] = SPRING_VERSION
            if a.a1:
                cfg["a1"] = a.a1
            if a.a2:
                cfg["a2"] = a.a2
            for name in ("decide_step", "grab_err", "stiff_den", "cross_speed"):
                v = getattr(a, name)
                if v is not None:
                    cfg[name] = v
            if a.tuned:
                cfg["flags"] |= FLAG_TUNED
            data[SPRING_AT[a.stick]:SPRING_AT[a.stick] + SPRING_SIZE] = pack(cfg)

        write_block(dev, CFG_BLOCK_ANALOG, data)
        if a.save:
            dev.write([CONFIG_COMMAND, CFG_BLOCK_GAMEPAD, GAMEPAD_CMD_SAVE_ALL])
        time.sleep(0.2)
        # What the pad now holds (it replaces anything it can't use with its defaults)
        show(unpack(read_block(dev, CFG_BLOCK_ANALOG), a.stick), a.stick)
    finally:
        dev.close()


if __name__ == "__main__":
    main()
