"""Input report timing on the USB wire, from a hardware USB sniffer: what a host really gets.

Captures with ataradov's usb-sniffer (VID 6666, PID 6620; usb_sniffer.exe built from
github.com/ataradov/usb-sniffer) placed between the host and the device, decodes the capture
with tshark, and reports for every interrupt IN endpoint that sent data:

  - the interval between reports, in USB frames and in microseconds (hardware timestamps:
    no operating system in the measurement)
  - new reports vs repeats (the same bytes again)
  - polls answered with NAK (nothing to send)
  - where in the frame the report went out

    python -I scripts/usb_wire_timing.py --seconds 20
    python -I scripts/usb_wire_timing.py --seconds 20 --speed hs --keep capture.pcapng
    python -I scripts/usb_wire_timing.py --pcap capture.pcapng       # analyse an earlier capture

Windows: needs Wireshark (tshark) and usb_sniffer.exe (--sniffer, or USB_SNIFFER in the
environment). Packets are captured by count, about 5000 per second of full-speed traffic.
"""
import argparse
import os
import subprocess
import sys
import tempfile
from collections import Counter, defaultdict

TSHARK_DEFAULT = r"C:\Program Files\Wireshark\tshark.exe"
PID_SOF, PID_IN = 0xA5, 0x69
PID_DATA0, PID_DATA1 = 0xC3, 0x4B
PID_ACK, PID_NAK, PID_STALL = 0xD2, 0x5A, 0x1E
PACKETS_PER_SECOND = 5000


def capture(sniffer, speed, seconds, out):
    n = int(seconds * PACKETS_PER_SECOND)
    r = subprocess.run([sniffer, "-s", speed, "-n", str(n), "-c", "-f", out],
                       capture_output=True, text=True)
    if r.returncode != 0 or not os.path.exists(out):
        sys.exit("capture failed:\n%s%s" % (r.stdout, r.stderr))


def packets(tshark, pcap):
    fields = ["frame.time_epoch", "usbll.pid", "usbll.device_addr", "usbll.endp",
              "usbll.frame_num", "usbll.data"]
    cmd = [tshark, "-r", pcap, "-Y", "usbll", "-T", "fields", "-E", "separator=;"]
    for f in fields:
        cmd += ["-e", f]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit("tshark failed:\n" + r.stderr)
    for line in r.stdout.splitlines():
        t, pid, addr, ep, fn, data = (line.split(";") + [""] * 6)[:6]
        if not pid:
            continue
        yield (float(t) * 1e6, int(pid, 16), addr, ep, fn, data)


def pct(v, q):
    v = sorted(v)
    return v[min(len(v) - 1, int(q * len(v)))] if v else float("nan")


def analyse(rows):
    frame_t = None      # this frame's SOF time
    frame_n = None      # its number (11 bits)
    frame_count = 0     # frames since the start (no wrap)
    token = None        # the last IN token: (addr, ep)
    reps = defaultdict(list)   # (addr, ep) -> [(time, frame index, offset in frame, data)]
    naks = Counter()
    for t, pid, addr, ep, fn, data in rows:
        if pid == PID_SOF:
            if fn:
                n = int(fn)
                if frame_n is not None:
                    frame_count += (n - frame_n) & 0x7FF
                frame_n = n
            frame_t = t
            token = None
        elif pid == PID_IN:
            token = (addr, ep)
        elif pid in (PID_DATA0, PID_DATA1) and token is not None:
            reps[token].append((t, frame_count, (t - frame_t) if frame_t is not None else float("nan"), data))
            token = None
        elif pid == PID_NAK and token is not None:
            naks[token] += 1
            token = None
    return reps, naks


def report(reps, naks):
    for key, v in sorted(reps.items(), key=lambda kv: -len(kv[1])):
        if len(v) < 20:
            continue
        span = (v[-1][0] - v[0][0]) / 1e6
        iv_us = [b[0] - a[0] for a, b in zip(v, v[1:])]
        iv_fr = Counter(b[1] - a[1] for a, b in zip(v, v[1:]))
        new = [v[0]] + [b for a, b in zip(v, v[1:]) if b[3] != a[3]]
        new_fr = Counter(b[1] - a[1] for a, b in zip(new, new[1:]))
        mode = iv_fr.most_common(1)[0][0]
        off = [x[2] for x in v]
        print("device %s endpoint %s: %d reports in %.1f s (%.1f/s), %d new (%.1f/s), %d NAKed polls" % (
            key[0], key[1], len(v), span, len(v) / span, len(new), len(new) / span, naks[key]))
        print("  interval in frames:      " + ", ".join(
            "%d: %.2f%%" % (k, 100.0 * c / (len(v) - 1)) for k, c in sorted(iv_fr.items())))
        print("  every %d frames exactly:  %.2f%% of intervals" % (mode, 100.0 * iv_fr[mode] / (len(v) - 1)))
        print("  interval (hardware):     p0.1 %.1f  p1 %.1f  p50 %.1f  p99 %.1f  p99.9 %.1f  max %.1f us" % (
            pct(iv_us, .001), pct(iv_us, .01), pct(iv_us, .5), pct(iv_us, .99), pct(iv_us, .999), max(iv_us)))
        if len(new) > 1:
            print("  new data every (frames): " + ", ".join(
                "%d: %.2f%%" % (k, 100.0 * c / (len(new) - 1)) for k, c in sorted(new_fr.items())))
            print("  repeats:                 %.2f%% of reports" % (100.0 * (len(v) - len(new)) / len(v)))
        print("  sent at (after SOF):     p1 %.1f  p50 %.1f  p99 %.1f us" % (pct(off, .01), pct(off, .5), pct(off, .99)))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--seconds", type=float, default=20.0)
    ap.add_argument("--speed", default="fs", choices=["ls", "fs", "hs"])
    ap.add_argument("--sniffer", default=os.environ.get("USB_SNIFFER", "usb_sniffer.exe"))
    ap.add_argument("--tshark", default=TSHARK_DEFAULT)
    ap.add_argument("--keep", help="keep the capture in this file")
    ap.add_argument("--pcap", help="analyse this capture instead of capturing")
    a = ap.parse_args()

    if a.pcap:
        pcap = a.pcap
    else:
        pcap = a.keep or os.path.join(tempfile.gettempdir(), "usb_wire_timing.pcapng")
        capture(a.sniffer, a.speed, a.seconds, pcap)
    reps, naks = analyse(packets(a.tshark, pcap))
    if not reps:
        sys.exit("no IN data in the capture (is the device behind the sniffer, at this speed?)")
    report(reps, naks)


if __name__ == "__main__":
    main()
