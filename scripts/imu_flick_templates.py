"""Turn flick recordings from imu_record.py into the firmware's flick tables.

    python scripts/imu_record.py --out recordings/flicks
    python scripts/imu_flick_templates.py recordings/flicks

Each direction's take is split into its separate flicks, the flicks are lined up on the main
gyro axis and averaged, and the average (minus the resting pose) is written to
library/HOJA-LIB-RP2040/src/input/motion_gesture_flicks.h as accel (mg) and gyro (dps) samples
every MOTION_FLICK_STEP_US. Values are what SInput hosts received, in HOJA's IMU frame.
"""

import argparse
import csv
import os
import sys

import numpy as np

ACCEL_MG_PER_LSB = 1000.0 / 4096.0
GYRO_DPS_PER_LSB = 0.07

STEP_MS = 5
PRE_MS = 25      # Kept before the flick's onset
TAPER_MS = 40    # Faded to rest at the end
QUIET_MS = 400   # Gap that separates two flicks
ONSET_FRAC = 0.3 # Onset: main gyro axis passes this share of the flick's peak

# Direction, main gyro axis (0 = X, 2 = Z) and its sign during the flick, table length
FLICKS = [
    ("FLICK_UP", "flick_up", 0, -1, 220),
    ("FLICK_DOWN", "flick_down", 0, 1, 300),
    ("FLICK_LEFT", "flick_left", 2, 1, 300),
    ("FLICK_RIGHT", "flick_right", 2, -1, 300),
]

# Directions built by mirroring another one left-to-right instead of from their own take. The
# recorded left flicks whipped back almost as hard as they went out (games read that as left and
# right at once), while the right flicks had a clean outward stroke.
MIRRORED = {"flick_left": "flick_right"}

# Mirroring left-to-right (X -> -X) flips accel X; the gyro is a turn, so Y and Z flip instead
MIRROR_ACCEL = np.array([-1, 1, 1])
MIRROR_GYRO = np.array([1, -1, -1])

# Directions built by turning another one upside down: the same stroke, pitching the other way.
# Recorded down flicks pushed forward far more than the up flicks and ran longer, so Flick Down
# felt weaker than Flick Up.
INVERTED = {"flick_down": "flick_up"}

# Inverting top-to-bottom (Z -> -Z) flips the hand's motion along Z and, being a turn, the gyro's
# X and Y. Gravity can't be flipped the same way (it moves the same whichever way the controller
# pitches), so it is taken out first and worked out again for the inverted turn.
INVERT_MOTION = np.array([1, 1, -1])
INVERT_GYRO = np.array([-1, -1, 1])

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_H = os.path.join(HERE, "..", "library", "HOJA-LIB-RP2040", "src", "input", "motion_gesture_flicks.h")


def load(path):
    with open(path) as f:
        rows = list(csv.reader(f))[1:]
    d = np.array([[float(x) for x in r] for r in rows])
    t_ms = (d[:, 1] - d[0, 1]) / 1000.0
    return t_ms, d[:, 2:8]


def template(folder, take, axis, sign, length_ms, rest):
    t, raw = load(os.path.join(folder, take + ".csv"))
    accel = (raw[:, 0:3] - rest[0:3]) * ACCEL_MG_PER_LSB
    gyro = (raw[:, 3:6] - rest[3:6]) * GYRO_DPS_PER_LSB

    main = gyro[:, axis] * sign
    active = np.where(np.linalg.norm(gyro, axis=1) > 150)[0]

    starts, prev = [], -1e9
    for i in active:
        if t[i] - prev > QUIET_MS:
            starts.append(i)
        prev = t[i]

    grid = np.arange(-PRE_MS, length_ms, STEP_MS)
    accel_sets, gyro_sets = [], []
    for s in starts:
        # Peak of the main axis within the flick, then onset where it passes ONSET_FRAC of it
        win = (t >= t[s]) & (t <= t[s] + 150)
        idx = np.where(win)[0]
        peak = idx[np.argmax(main[idx])]
        before = np.where((t <= t[peak]) & (t >= t[s] - 50) & (main < ONSET_FRAC * main[peak]))[0]
        t0 = t[before[-1]] if len(before) else t[s]
        accel_sets.append(np.stack([np.interp(t0 + grid, t, accel[:, k]) for k in range(3)], 1))
        gyro_sets.append(np.stack([np.interp(t0 + grid, t, gyro[:, k]) for k in range(3)], 1))

    if not starts:
        sys.exit(f"{take}: no flicks found")

    a = np.mean(accel_sets, 0)
    g = np.mean(gyro_sets, 0)

    # Fade in over the lead-in and out over the tail so the flick starts and ends at rest
    fade = np.ones(len(grid))
    lead = grid < 0
    fade[lead] = np.linspace(0, 1, lead.sum() + 1)[:-1]
    n_tail = TAPER_MS // STEP_MS
    fade[-n_tail:] = np.linspace(1, 0, n_tail)
    a *= fade[:, None]
    g *= fade[:, None]

    # The wrist's return can run past the table; finish it in the second half so the gyro adds
    # up to no net turn and gyro aim does not drift with every flick
    w = np.zeros(len(grid))
    half = len(grid) // 2
    w[half:] = np.hanning(len(grid) - half + 2)[1:-1]
    g -= np.outer(w / w.sum(), g.sum(0))
    return len(starts), np.round(a).astype(int), np.round(g).astype(int)


def gravity(gyro_dps, up_mg):
    """How the resting reading changes as the controller turns by gyro_dps from rest (mg)."""
    u = np.array(up_mg, dtype=float)
    out = [np.zeros(3)]
    for i in range(len(gyro_dps) - 1):
        # HOJA's axes are left-handed, so up, seen from the controller, turns by the gyro as read
        # (the opposite of the right-handed rule). Checked against the recordings: only this way
        # round does the pull toward the wrist track the spin squared.
        w = np.radians((gyro_dps[i] + gyro_dps[i + 1]) / 2.0) * STEP_MS / 1000.0
        angle = np.linalg.norm(w)
        if angle > 0:
            k = w / angle
            u = u * np.cos(angle) + np.cross(k, u) * np.sin(angle) + k * np.dot(k, u) * (1 - np.cos(angle))
        out.append(u - up_mg)
    return np.array(out)


def invert(a, g, up_mg):
    # Summing the turn doesn't quite land back at rest, so fade gravity out with the table's tail
    fade = np.ones(len(g))
    n_tail = TAPER_MS // STEP_MS
    fade[-n_tail:] = np.linspace(1, 0, n_tail)

    g_inv = g * INVERT_GYRO
    motion = a - gravity(g, up_mg) * fade[:, None]
    a_inv = motion * INVERT_MOTION + gravity(g_inv, up_mg) * fade[:, None]
    return np.round(a_inv).astype(int), g_inv


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("folder", help="folder with rest.csv and flick_*.csv")
    args = ap.parse_args()

    _, rest_raw = load(os.path.join(args.folder, "rest.csv"))
    rest = rest_raw.mean(0)

    lines = [
        "// Generated by scripts/imu_flick_templates.py from recorded flicks; do not edit by hand.",
        "// Averaged real flicks minus the resting pose, in HOJA's IMU frame as SInput hosts receive it:",
        "// accel in mg, gyro in dps, one sample every MOTION_FLICK_STEP_US.",
        "#ifndef INPUT_MOTION_GESTURE_FLICKS_H",
        "#define INPUT_MOTION_GESTURE_FLICKS_H",
        "",
        "#include <stdint.h>",
        "",
        f"#define MOTION_FLICK_STEP_US {STEP_MS * 1000}u",
        "",
        "// Which way was up (unit vector, HOJA frame) while the flicks were recorded; flicks are",
        "// turned from this pose to the controller's current one so they stay relative to the ground",
        "#define MOTION_FLICK_REF_UP {{{:.4f}f, {:.4f}f, {:.4f}f}}".format(*(rest[0:3] / np.linalg.norm(rest[0:3]))),
        "",
        "typedef struct",
        "{",
        "    int16_t accel_mg[3];",
        "    int16_t gyro_dps[3];",
        "} motion_flick_sample_s;",
        "",
    ]
    for name, take, axis, sign, length in FLICKS:
        if take in INVERTED:
            src = INVERTED[take]
            src_axis, src_sign, src_length = next((f[2], f[3], f[4]) for f in FLICKS if f[1] == src)
            count, a, g = template(args.folder, src, src_axis, src_sign, src_length, rest)
            a, g = invert(a.astype(float), g, rest[0:3] * ACCEL_MG_PER_LSB)
            print(f"{take}: inverted from {src} ({count} flicks averaged), {len(a)} samples")
            lines.append(f"// {take}: {src} turned upside down ({count} flicks averaged)")
        elif take in MIRRORED:
            src = MIRRORED[take]
            src_axis, src_sign, src_length = next((f[2], f[3], f[4]) for f in FLICKS if f[1] == src)
            count, a, g = template(args.folder, src, src_axis, src_sign, src_length, rest)
            a, g = a * MIRROR_ACCEL, g * MIRROR_GYRO
            print(f"{take}: mirrored from {src} ({count} flicks averaged), {len(a)} samples")
            lines.append(f"// {take}: {src} mirrored left-to-right ({count} flicks averaged)")
        else:
            count, a, g = template(args.folder, take, axis, sign, length, rest)
            print(f"{take}: {count} flicks averaged, {len(a)} samples")
            lines.append(f"// {take}: {count} flicks averaged")
        lines.append(f"static const motion_flick_sample_s _flick_{take[6:]}[] = {{")
        for i in range(len(a)):
            lines.append("    {{{{{:6d},{:6d},{:6d}}}, {{{:6d},{:6d},{:6d}}}}},".format(*a[i], *g[i]))
        lines.append("};")
        lines.append("")
    lines.append("#endif")

    with open(OUT_H, "w", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    print(f"Wrote {os.path.normpath(OUT_H)}")


if __name__ == "__main__":
    main()
