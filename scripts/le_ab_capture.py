"""Capture a GCU-2 bench build (HOJA_BT_LE_AB) over direct SInput Bluetooth and compare its four
states: report interval 4 ms or 3.75 ms, LE advertising off or on. Reports carry [seq u16]
[sent_us u32][state u8] at byte 47.

    python -I scripts/le_ab_capture.py [seconds] [--save run.json]

Windows only (hid_rate.py).
"""
import ctypes
import json
import os
import struct
import sys
import time
from ctypes import wintypes

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hid_rate as hr  # noqa: E402

NAMES = {0: "no adv", 1: "1 s, 3 channels", 2: "1 s, 1 channel", 3: "4 s, 3 channels"}
NOMINAL = {0: 3.75, 1: 3.75, 2: 3.75, 3: 3.75}


def pct(v, q):
    v = sorted(v)
    return v[min(len(v) - 1, int(q * len(v)))] if v else float("nan")


def main():
    a = sys.argv[1:]
    save = a[a.index("--save") + 1] if "--save" in a else None
    nums = [x for x in a if x.replace(".", "").isdigit()]
    seconds = float(nums[0]) if nums else 250
    path = None
    end = time.time() + 60
    while not path and time.time() < end:
        path = next((p[0] for p in hr.enumerate_hid() if p[1] == 0x2E8A and p[2] == 0x10DD and p[4] == 64), None)
        if not path:
            time.sleep(0.5)
    if not path:
        sys.exit("no SInput Bluetooth device")
    hr.boost_priority()
    h = hr.open_path(path, overlapped=True)
    ev = hr.kernel32.CreateEventW(None, True, False, None)
    buf = ctypes.create_string_buffer(64)
    n = wintypes.DWORD()
    rows = []
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < seconds:
        ov = hr.OVERLAPPED()
        ov.hEvent = ev
        hr.kernel32.ResetEvent(ev)
        if not hr.kernel32.ReadFile(h, buf, 64, ctypes.byref(n), ctypes.byref(ov)):
            if hr.kernel32.WaitForSingleObject(ev, 2000) == hr.WAIT_TIMEOUT:
                hr.kernel32.CancelIo(h)
                if rows:
                    break  # the bench build has finished (it reboots onto USB)
                continue
            if not hr.kernel32.GetOverlappedResult(h, ctypes.byref(ov), ctypes.byref(n), False):
                break
        r = buf.raw
        if r[0] == 1:
            seq, sent = struct.unpack_from("<HI", r, 47)
            rows.append((time.perf_counter() * 1e6, seq, sent, r[53]))
    hr.kernel32.CloseHandle(h)
    if save:
        json.dump(rows, open(save, "w"))
    analyse(rows)


def analyse(rows):
    # Drift: line through per-second minimums of (pc - sent) over the whole capture
    base, last, unwrapped = 0, None, []
    for t, seq, sent, st in rows:
        if last is not None and sent < last and last - sent > 1 << 31:
            base += 1 << 32
        last = sent
        unwrapped.append((t, seq, sent + base, st))
    by = {}
    for t, _, s, _ in unwrapped:
        k = s // 1_000_000
        by[k] = min(by.get(k, t - s), t - s)
    xs = sorted(by)
    mx = sum(xs) / len(xs)
    my = sum(by[x] for x in xs) / len(xs)
    slope = sum((x - mx) * (by[x] - my) for x in xs) / max(1e-9, sum((x - mx) ** 2 for x in xs)) / 1e6
    delays = [(t - s) - slope * s for t, _, s, _ in unwrapped]
    floor = min(delays)
    print("state             seconds  rx/s  lost | PC interval p50  p90  p99  max  even(+-0.3ms) | gaps>=8ms | delay p50  p90  p99  max")
    for st in sorted(NAMES):
        idx = [i for i, r in enumerate(unwrapped) if r[3] == st]
        # Contiguous runs, the first 0.3 s of each dropped
        runs, cur = [], []
        for i in idx:
            if cur and unwrapped[i][0] - unwrapped[cur[-1]][0] > 1e6:
                runs.append(cur)
                cur = []
            cur.append(i)
        if cur:
            runs.append(cur)
        dur = n = lost = 0
        gaps, dl = [], []
        for run in runs:
            run = [i for i in run if unwrapped[i][0] - unwrapped[run[0]][0] > 3e5]
            if len(run) < 10:
                continue
            dur += (unwrapped[run[-1]][0] - unwrapped[run[0]][0]) / 1e6
            n += len(run)
            seqs = [unwrapped[i][1] for i in run]
            lost += max(0, ((seqs[-1] - seqs[0]) & 0xFFFF) + 1 - len(set(seqs)))
            gaps += [(unwrapped[b][0] - unwrapped[a][0]) / 1000 for a, b in zip(run, run[1:])]
            dl += [(delays[i] - floor) / 1000 for i in run]
        if not dur:
            continue
        even = sum(1 for g in gaps if abs(g - NOMINAL[st]) <= 0.3) / len(gaps) * 100
        print("%-17s %6.0f %6.0f %5d | %15.2f %5.2f %5.2f %5.1f %9.1f%%     | %8d  | %9.2f %5.2f %5.2f %5.1f" % (
            NAMES[st], dur, n / dur, lost, pct(gaps, .5), pct(gaps, .9), pct(gaps, .99), max(gaps), even,
            sum(1 for g in gaps if g >= 8), pct(dl, .5), pct(dl, .9), pct(dl, .99), max(dl)))


if __name__ == "__main__":
    if len(sys.argv) > 2 and sys.argv[1] == "--load":
        analyse(json.load(open(sys.argv[2])))
    else:
        main()
