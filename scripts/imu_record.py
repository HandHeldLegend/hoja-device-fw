"""Record a HOJA gamepad's IMU over SInput (USB) while you perform motions on cue.

Boot the gamepad in SInput mode over USB, then run:

    pip install hidapi
    python scripts/imu_record.py --out recordings/flicks

Each take is saved as <out>/<name>.csv with columns:
    host_s, imu_us, ax, ay, az, gx, gy, gz
Raw device units, exactly what SInput hosts receive (HOJA IMU frame, after the IMU sensitivity
settings): accel 4096 LSB/g (8 g range), gyro 0.07 dps/LSB (2000 dps range).
"""

import argparse
import csv
import os
import struct
import sys
import time

try:
    import hid
except ImportError:
    sys.exit("Needs hidapi: pip install hidapi")

SINPUT_VID = 0x2E8A
SINPUT_PID = 0x10C6
REPORT_ID_INPUT = 0x01
IMU_OFFSET = 19  # uint32 timestamp_us, then int16 ax, ay, az, gx, gy, gz

TAKES = [
    ("rest", "Hold the controller still, the way you hold it to play.", 3),
    ("flick_up", "Flick UP: a quick wrist flick upward, back to rest. 5 flicks, about 1 s apart.", 8),
    ("flick_down", "Flick DOWN: a quick wrist flick downward, back to rest. 5 flicks, about 1 s apart.", 8),
    ("flick_left", "Flick LEFT: a quick wrist flick to the left, back to rest. 5 flicks, about 1 s apart.", 8),
    ("flick_right", "Flick RIGHT: a quick wrist flick to the right, back to rest. 5 flicks, about 1 s apart.", 8),
    ("shake", "SHAKE: shake the controller the way a game asks you to, for the whole take.", 5),
]


def record(dev, seconds):
    rows = []
    last_ts = None
    t0 = time.perf_counter()
    while time.perf_counter() - t0 < seconds:
        r = dev.read(64, 50)
        if not r or r[0] != REPORT_ID_INPUT:
            continue
        ts, ax, ay, az, gx, gy, gz = struct.unpack_from("<I6h", bytes(r), IMU_OFFSET)
        if ts == last_ts:
            continue  # Same IMU sample repeated in the next report
        last_ts = ts
        rows.append((round(time.perf_counter() - t0, 4), ts, ax, ay, az, gx, gy, gz))
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default="recordings/flicks", help="output folder")
    ap.add_argument("--only", nargs="*", help="record only these takes (e.g. flick_up flick_left)")
    args = ap.parse_args()

    os.makedirs(args.out, exist_ok=True)

    dev = hid.device()
    try:
        dev.open(SINPUT_VID, SINPUT_PID)
    except OSError:
        sys.exit("No SInput gamepad found. Boot it in SInput mode over USB and try again.")
    print(f"Connected: {dev.get_product_string()}")

    takes = [t for t in TAKES if not args.only or t[0] in args.only]
    for name, prompt, seconds in takes:
        print()
        print(f"[{name}] {prompt}")
        input(f"  Press Enter to start ({seconds} s)...")
        for n in (3, 2, 1):
            print(f"  {n}...", end=" ", flush=True)
            time.sleep(0.6)
        print("GO")
        dev.read(64, 0)
        rows = record(dev, seconds)
        path = os.path.join(args.out, f"{name}.csv")
        with open(path, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["host_s", "imu_us", "ax", "ay", "az", "gx", "gy", "gz"])
            w.writerows(rows)
        print(f"  Saved {len(rows)} samples to {path}")

    dev.close()
    print()
    print("Done.")


if __name__ == "__main__":
    main()
