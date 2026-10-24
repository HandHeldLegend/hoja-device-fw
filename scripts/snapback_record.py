"""Record stick releases and flicks from a HOJA gamepad for snapback tuning (Windows, WebUSB).

The firmware captures a stick each time it comes back from a push (out past 800, then back under 500):
both axes, as the snapback filter sees them (-2048..2048), every 500 us for 128 ms, starting 24 ms
before it fell back under 500.

    python -I scripts/snapback_record.py --stick left --label release --out releases.jsonl
    python -I scripts/snapback_record.py --stick left --label flick --out flicks.jsonl

Each capture is one JSON line: {"label", "stick", "number", "period_us", "pre", "x": [...], "y": [...]}.
Stop with Ctrl+C.
"""

import argparse
import json
import math
import os
import struct
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hoja_bench as hb  # noqa: E402

WEBUSB_ID_INPUT_COMMAND = 5
INPUT_CMD_SNAPBACK_CAPTURE = 2
WEBUSB_SNAPBACK_CAPTURE = 249
CAPTURE_OFF = 0xFF
PERIOD_US = 500
PRE = 48


def _summary(x, y):
    d = [math.hypot(a, b) for a, b in zip(x, y)]
    peak = max(d[:PRE + 1]) if d else 0
    # The deepest swing past centre, measured against the direction the stick left from
    ex, ey = x[0], y[0]
    n = math.hypot(ex, ey) or 1.0
    proj = [(a * ex + b * ey) / n for a, b in zip(x, y)]
    overshoot = -min(proj[PRE:]) if len(proj) > PRE else 0
    return peak, overshoot


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--stick", choices=["left", "right"], default="left")
    ap.add_argument("--label", required=True, help="what you are doing, e.g. release or flick")
    ap.add_argument("--out", required=True, help="JSON lines file to append captures to")
    ap.add_argument("--count", type=int, default=0, help="stop after this many captures (0 = until Ctrl+C)")
    a = ap.parse_args()

    stick = 0 if a.stick == "left" else 1
    path = hb.find_webusb("pad")
    if not path:
        sys.exit("no gamepad on its cable")
    dev = hb.WebUsb(path)
    parts = {}
    saved = 0
    try:
        dev.write([WEBUSB_ID_INPUT_COMMAND, INPUT_CMD_SNAPBACK_CAPTURE, stick])
        print("armed the %s stick: push it to the edge and %s it (Ctrl+C to stop)" % (a.stick, a.label))
        with open(a.out, "a", encoding="utf-8") as f:
            while not a.count or saved < a.count:
                r = dev.read()
                if not r or r[0] != WEBUSB_SNAPBACK_CAPTURE or len(r) < 6:
                    continue
                number, idx, total, count = r[2], r[3], r[4], r[5]
                samples = struct.unpack_from("<%dh" % (count * 2), r, 6)
                cap = parts.setdefault(number, {})
                cap[idx] = samples
                if len(cap) < total:
                    continue
                flat = [v for i in range(total) for v in cap[i]]
                del parts[number]
                x, y = flat[0::2], flat[1::2]
                rec = {"label": a.label, "stick": a.stick, "number": number, "period_us": PERIOD_US,
                       "pre": PRE, "time": time.time(), "x": x, "y": y}
                f.write(json.dumps(rec) + "\n")
                f.flush()
                saved += 1
                peak, over = _summary(x, y)
                print("  #%d  edge %4d  swing past centre %4d (%3.0f%%)" % (saved, peak, over,
                                                                            100.0 * over / (peak or 1)))
                # A capture that missed packets never completes; drop stale ones
                for k in [k for k in parts if (number - k) % 256 > 4]:
                    del parts[k]
    except KeyboardInterrupt:
        pass
    finally:
        dev.write([WEBUSB_ID_INPUT_COMMAND, INPUT_CMD_SNAPBACK_CAPTURE, CAPTURE_OFF])
        dev.close()
    print("saved %d captures to %s" % (saved, a.out))


if __name__ == "__main__":
    main()
