"""RFCOMM vs HID from a GCU-2 test build (HOJA_BT_RFCOMM_TEST) over the PC's own Bluetooth.

Listens on RFCOMM channel 5 and reads the SInput HID device at the same time, timestamping every
report. Each report carries the gamepad's stamp at byte 47 (seq, send time, interval, role, path,
stage, skipped sends, flags, RFCOMM credits). Prints, per stage / role / path / interval: reports
per second, lost and skipped reports, the PC's arrival intervals and the delay spread (arrival minus
send time, drift removed).

    python -I scripts/rfcomm_test.py [seconds] [--save capture.json]
    python -I scripts/rfcomm_test.py --load capture.json

Windows only (Winsock Bluetooth via Python's socket module, hid.dll via hid_rate.py).
"""
import ctypes
import json
import os
import socket
import sys
import threading
import time
from ctypes import wintypes

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

CHANNEL = 5
FRAME = 64
OFF = 47
SETTLE_US = 300000
STALL_MS = 10.0

rows = []  # (t_pc_ns, source, frame bytes)
lock = threading.Lock()
stop = False
events = []


def log(msg):
    events.append((time.perf_counter_ns(), msg))
    print("%8.1f s  %s" % ((time.perf_counter_ns() - T0) / 1e9, msg), flush=True)


def rfcomm_thread():
    srv = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    srv.bind(("00:00:00:00:00:00", CHANNEL))
    srv.listen(1)
    srv.settimeout(1.0)
    log("RFCOMM listening on %s channel %d" % srv.getsockname())
    while not stop:
        try:
            c, addr = srv.accept()
        except socket.timeout:
            continue
        log("RFCOMM connected from %s" % (addr,))
        c.settimeout(1.0)
        buf = b""
        while not stop:
            try:
                d = c.recv(4096)
            except socket.timeout:
                continue
            except OSError as e:
                log("RFCOMM error %s" % e)
                break
            t = time.perf_counter_ns()
            if not d:
                log("RFCOMM closed")
                break
            buf += d
            while len(buf) >= FRAME:
                f, buf = buf[:FRAME], buf[FRAME:]
                if f[0] != 1:
                    # Out of step: resync on the next report ID
                    i = buf.find(b"\x01")
                    log("RFCOMM framing lost, dropping %d bytes" % (FRAME + (i if i >= 0 else len(buf))))
                    buf = buf[i:] if i >= 0 else b""
                    continue
                with lock:
                    rows.append((t, "rf", f))
        c.close()
    srv.close()


def hid_thread():
    import hid_rate as hr
    hr.boost_priority()
    while not stop:
        path = next((p[0] for p in hr.enumerate_hid() if p[1] == 0x2E8A and p[2] == 0x10DD), None)
        if not path:
            time.sleep(0.5)
            continue
        h = hr.open_path(path, overlapped=True)
        if not h:
            time.sleep(0.5)
            continue
        log("HID SInput device open")
        ev = hr.kernel32.CreateEventW(None, True, False, None)
        buf = ctypes.create_string_buffer(64)
        n = wintypes.DWORD()
        fails = 0
        while not stop and fails < 3:
            ov = hr.OVERLAPPED()
            ov.hEvent = ev
            hr.kernel32.ResetEvent(ev)
            if not hr.kernel32.ReadFile(h, buf, 64, ctypes.byref(n), ctypes.byref(ov)):
                if ctypes.get_last_error() != hr.ERROR_IO_PENDING:
                    fails += 1
                    time.sleep(0.2)
                    continue
                if hr.kernel32.WaitForSingleObject(ev, 1000) == hr.WAIT_TIMEOUT:
                    hr.kernel32.CancelIo(h)
                    continue
                if not hr.kernel32.GetOverlappedResult(h, ctypes.byref(ov), ctypes.byref(n), False):
                    fails += 1
                    continue
            t = time.perf_counter_ns()
            fails = 0
            if buf.raw[0] == 1:
                with lock:
                    rows.append((t, "hid", buf.raw[:64]))
        hr.kernel32.CloseHandle(h)
        log("HID device gone")


def decode(t, src, f):
    u = lambda o, k: int.from_bytes(f[OFF + o:OFF + o + k], "little")
    return {
        "t": t / 1000.0, "src": src, "seq": u(0, 2), "pad": u(2, 4), "interval": u(6, 2), "role": f[OFF + 8],
        "path": f[OFF + 9] & 0x0F, "stage": f[OFF + 9] >> 4, "busy": u(10, 2), "flags": f[OFF + 12],
        "credits": f[OFF + 13], "open_status": f[OFF + 14], "mtu": f[OFF + 15],
        "log": list(f[1:46]),
    }


def pct(v, q):
    v = sorted(v)
    return v[min(len(v) - 1, int(q * len(v)))] if v else float("nan")


def analyse(recs):
    recs = sorted(recs, key=lambda r: r["t"])
    # Unwrap the gamepad clock (32-bit us)
    base, last = 0, None
    for r in recs:
        if last is not None and r["pad"] < last and last - r["pad"] > 1 << 31:
            base += 1 << 32
        last = r["pad"]
        r["padu"] = r["pad"] + base
    # Settled: same (stage, role, path, interval, hid flag) for SETTLE_US of gamepad time
    key = lambda r: (r["stage"], r["role"], r["path"], r["interval"], r["flags"] & 1)
    groups, since, prev = {}, None, None
    for r in sorted(recs, key=lambda r: r["padu"]):
        k = key(r)
        if k != prev:
            since, prev = r["padu"], k
        if r["padu"] - since >= SETTLE_US:
            groups.setdefault(k, []).append(r)

    # Drift between the clocks: slope of the per-second minimum of (pc - pad) over the capture
    mins = {}
    for r in recs:
        s = int(r["padu"] // 1e6)
        d = r["t"] - r["padu"]
        mins[s] = min(mins.get(s, d), d)
    xs = sorted(mins)
    if len(xs) > 2:
        mx = sum(xs) / len(xs)
        my = sum(mins[x] for x in xs) / len(xs)
        slope = sum((x - mx) * (mins[x] - my) for x in xs) / max(1e-9, sum((x - mx) ** 2 for x in xs))
    else:
        slope = 0.0
    for r in recs:
        r["delay"] = r["t"] - r["padu"] - slope * r["padu"] / 1e6

    print()
    print("stage role  path  int_us  HID  seconds  rx/s  of_sent  lost  skipped | PC interval ms p50  p90   p99    max  stalls>%dms | delay ms p50  p90   p99   max" % STALL_MS)
    floor = min((r["delay"] for g in groups.values() for r in g), default=0.0)
    out = []
    for k in sorted(groups):
        stage, role, path, interval, hid = k
        # Contiguous runs (the same settings come back every cycle)
        g = sorted(groups[k], key=lambda r: r["padu"])
        runs, cur = [], [g[0]]
        for a, b in zip(g, g[1:]):
            if b["padu"] - a["padu"] > 1000000:
                runs.append(cur)
                cur = []
            cur.append(b)
        runs.append(cur)
        dur = n_rx = sent = lost = skipped = 0
        gaps, dl = [], []
        for run in runs:
            dur += (run[-1]["padu"] - run[0]["padu"]) / 1e6
            seqs = {r["seq"] for r in run}
            span = ((run[-1]["seq"] - run[0]["seq"]) & 0xFFFF) + 1
            sk = (run[-1]["busy"] - run[0]["busy"]) & 0xFFFF
            n_rx += len(seqs)
            sent += span - sk
            skipped += sk
            lost += max(0, span - sk - len(seqs))
            arr = sorted(r["t"] for r in run)
            gaps += [(b - a) / 1000.0 for a, b in zip(arr, arr[1:])]
            dl += [(r["delay"] - floor) / 1000.0 for r in run]
        if dur < 1.0:
            continue
        stalls = sum(1 for x in gaps if x >= STALL_MS)
        line = "%5s %-5s %-5s %6d  %-3s %7.1f %6.0f %8d %5d %8d | %17.2f %5.2f %5.2f %6.1f %8d   | %12.2f %5.2f %5.2f %5.1f" % (
            {1: "A", 3: "B", 4: "fb"}.get(stage, str(stage)), {0: "pad", 1: "PC"}.get(role, "?"), "rf" if path else "hid",
            interval, "up" if hid else "-", dur, n_rx / dur, sent, lost, skipped,
            pct(gaps, .5), pct(gaps, .9), pct(gaps, .99), max(gaps) if gaps else 0, stalls,
            pct(dl, .5), pct(dl, .9), pct(dl, .99), max(dl) if dl else 0)
        print(line)
        out.append(line)
    print("(role: who is master; delay: arrival minus send time over the quickest, clock drift %.1f ppm removed)" % slope)
    fb = [r for r in recs if r["stage"] == 4]
    if fb:
        names = {1: "READY", 2: "CONNECTED", 3: "CONNECT_FAILED", 4: "DISCONNECTED", 0xE0: "gap_connect()",
                 0xE1: "rfcomm_connect()", 0xE2: "rfcomm on_open", 0xE3: "HID up", 0xE4: "fallback", 0xE5: "timer"}
        print("stage B log (from the HID fallback; GAP events are floss_gap_evt_type_t values):")
        for i in range(15):
            ev, a, b = fb[-1]["log"][i * 3:i * 3 + 3]
            if ev == 0 and a == 0 and b == 0:
                break
            print("  %-18s %3d %3d" % (names.get(ev, "GAP evt %d" % ev), a, b))
    first = recs[0] if recs else None
    if first:
        rf = [r for r in recs if r["src"] == "rf"]
        print("RFCOMM: open status %s, mtu %s, credits seen min %s max %s" % (
            first["open_status"], max((r["mtu"] for r in recs), default=0),
            min((r["credits"] for r in rf), default="-"), max((r["credits"] for r in rf), default="-")))
    return out


def main():
    global stop, T0
    a = sys.argv[1:]
    if "--load" in a:
        data = json.load(open(a[a.index("--load") + 1]))
        analyse([decode(t, s, bytes.fromhex(f)) for t, s, f in data["rows"]])
        return
    save = None
    if "--save" in a:
        i = a.index("--save")
        save = a[i + 1]
        del a[i:i + 2]
    seconds = float(a[0]) if a else 330.0
    T0 = time.perf_counter_ns()
    ts = [threading.Thread(target=rfcomm_thread, daemon=True), threading.Thread(target=hid_thread, daemon=True)]
    for t in ts:
        t.start()
    last_n, t_end = 0, time.time() + seconds
    while time.time() < t_end:
        time.sleep(5)
        with lock:
            n = len(rows)
            tail = rows[-1] if rows else None
        if tail:
            d = decode(*tail)
            log("%d reports (+%d)  last: stage %d role %d path %s interval %d flags %d credits %d" % (
                n, n - last_n, d["stage"], d["role"], "rf" if d["path"] else "hid", d["interval"], d["flags"],
                d["credits"]))
        last_n = n
    stop = True
    with lock:
        data = list(rows)
    if save:
        with open(save, "w") as f:
            json.dump({"rows": [(t, s, fr.hex()) for t, s, fr in data],
                       "events": events}, f)
    analyse([decode(*r) for r in data])


if __name__ == "__main__":
    main()
