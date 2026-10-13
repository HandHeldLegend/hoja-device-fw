"""Stand in for HHL Gamepad Link over HID and test a gamepad's Link mode (dongle_link.h, "Link mode").

Waits for the gamepad's Link HID collection (usage page 0xFF48, usage 0x4C) to appear, opens it with
hid.dll as the app does, sends the app's host info, then runs a script: SInput at 4 ms and 2 ms, a
config read, a switch to XInput and back. Prints what arrived and the timing of the live reports
(arrival minus the gamepad's sample time, drift removed). --ab N: just SInput at 4 ms for N s.

    python -I scripts/link_hid_peer.py [--wait 120] [--ab 120] [--save run.json]

Windows only (hid.dll via hid_rate.py). No admin needed.
"""
import ctypes
import json
import os
import struct
import sys
import threading
import time
from ctypes import wintypes

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hid_rate as hr  # noqa: E402

USAGE_PAGE, USAGE = 0xFF48, 0x4C
REPORT_LIVE, REPORT_MSG, REPORT_OUT = 1, 2, 3
LANE_BULK, LANE_MSG, LANE_STATUS = 2, 3, 4
MSG_HOST, MSG_WAKE, MSG_REPORT, MSG_CONFIG, MSG_TOKEN = 1, 2, 3, 5, 7
MODE_NAMES = {0: "Switch", 1: "SInput", 2: "XInput", 3: "Slippi", 15: "unknown"}
GENERIC_WRITE = 0x40000000
TOKEN_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".link_token")
T0 = time.perf_counter()


def log(msg):
    print("%7.2f s  %s" % (time.perf_counter() - T0, msg), flush=True)


def pct(values, q):
    values = sorted(values)
    return values[min(len(values) - 1, int(q * len(values)))] if values else float("nan")


def timing(inputs, label):
    """Rate, losses, PC arrival gaps and delay spread of one phase's live reports."""
    rows = [r for r in inputs if r[4] == label]
    if len(rows) < 10:
        print("  %-16s %d reports" % (label, len(rows)))
        return
    rows = rows[len(rows) // 10:]  # Let it settle
    duration = (rows[-1][0] - rows[0][0]) / 1e6
    seqs = [r[1] for r in rows]
    span = ((seqs[-1] - seqs[0]) & 0xFFFF) + 1

    # Delay: arrival minus sampled, the clocks' drift removed by a line through per-second minimums
    points = [(r[2], r[0] - r[2]) for r in rows]
    floor = {}
    for sampled, delay in points:
        second = sampled // 1_000_000
        floor[second] = min(floor.get(second, delay), delay)
    xs = sorted(floor)
    slope = 0
    if len(xs) > 2:
        mx = sum(xs) / len(xs)
        my = sum(floor[x] for x in xs) / len(xs)
        slope = sum((x - mx) * (floor[x] - my) for x in xs) / sum((x - mx) ** 2 for x in xs) / 1e6
    delays = [delay - slope * sampled for sampled, delay in points]
    lowest = min(delays)
    delays = [(d - lowest) / 1000 for d in delays]

    gaps = [(b[0] - a[0]) / 1000 for a, b in zip(rows, rows[1:])]
    print("  %-16s %6.0f/s  sent-seq span %5d got %5d | PC gap p50 %.2f p99 %.2f max %.1f ms"
          " | delay p50 %.2f p90 %.2f p99 %.2f max %.1f ms" % (
              label, len(rows) / duration, span, len(set(seqs)),
              pct(gaps, .5), pct(gaps, .99), max(gaps),
              pct(delays, .5), pct(delays, .9), pct(delays, .99), max(delays)))


def find_link():
    for info in hr.enumerate_hid():
        h = hr.open_path(info[0])
        if not h:
            continue
        try:
            pre = ctypes.c_void_p()
            if hr.hid.HidD_GetPreparsedData(h, ctypes.byref(pre)):
                caps = hr.HIDP_CAPS()
                ok = hr.hid.HidP_GetCaps(pre, ctypes.byref(caps)) == hr.HIDP_STATUS_SUCCESS
                hr.hid.HidD_FreePreparsedData(pre)
                if ok and caps.UsagePage == USAGE_PAGE and caps.Usage == USAGE:
                    return info[0], caps.InputReportByteLength, caps.OutputReportByteLength
        finally:
            hr.kernel32.CloseHandle(h)
    return None


class Link:
    """One open Link HID device: a reader thread, and message writes."""

    def __init__(self, path, in_len, out_len):
        self.h = hr.kernel32.CreateFileW(path, hr.GENERIC_READ | GENERIC_WRITE,
                                         hr.FILE_SHARE_READ | hr.FILE_SHARE_WRITE, None,
                                         hr.OPEN_EXISTING, hr.FILE_FLAG_OVERLAPPED, None)
        self.in_len, self.out_len = in_len, out_len
        self.inputs, self.wakes, self.configs, self.reports = [], [], [], []
        self.bulk = 0
        self.token = None
        self.label = ""
        self.stop = False
        self.closed = False
        self.write_lock = threading.Lock()
        self.write_event = hr.kernel32.CreateEventW(None, True, False, None)
        self.thread = threading.Thread(target=self.reader, daemon=True)
        self.thread.start()

    def write_msg(self, lane, payload):
        r = bytes([REPORT_OUT, lane, len(payload)]) + bytes(payload)
        r = r + b"\0" * (self.out_len - len(r))
        with self.write_lock:
            ov = hr.OVERLAPPED()
            ov.hEvent = self.write_event
            hr.kernel32.ResetEvent(self.write_event)
            n = wintypes.DWORD()
            buf = ctypes.create_string_buffer(r, len(r))
            if not hr.kernel32.WriteFile(self.h, buf, len(r), ctypes.byref(n), ctypes.byref(ov)):
                if ctypes.get_last_error() != hr.ERROR_IO_PENDING:
                    return False
                hr.kernel32.WaitForSingleObject(self.write_event, 1000)
                if not hr.kernel32.GetOverlappedResult(self.h, ctypes.byref(ov), ctypes.byref(n), False):
                    return False
        return True

    def host(self, mode, interval_us):
        info = struct.pack("<BBBBIH", 3, 4, 1, mode, 0x000200, interval_us)
        return self.write_msg(LANE_MSG, bytes([MSG_HOST]) + info)

    def reader(self):
        hr.boost_priority()
        event = hr.kernel32.CreateEventW(None, True, False, None)
        buf = ctypes.create_string_buffer(self.in_len)
        n = wintypes.DWORD()
        while not self.stop:
            ov = hr.OVERLAPPED()
            ov.hEvent = event
            hr.kernel32.ResetEvent(event)
            if not hr.kernel32.ReadFile(self.h, buf, self.in_len, ctypes.byref(n), ctypes.byref(ov)):
                if ctypes.get_last_error() != hr.ERROR_IO_PENDING:
                    self.closed = True
                    return
                if hr.kernel32.WaitForSingleObject(event, 500) == hr.WAIT_TIMEOUT:
                    hr.kernel32.CancelIo(self.h)
                    hr.kernel32.GetOverlappedResult(self.h, ctypes.byref(ov), ctypes.byref(n), True)
                    continue
                if not hr.kernel32.GetOverlappedResult(self.h, ctypes.byref(ov), ctypes.byref(n), False):
                    self.closed = True
                    return
            self.on_report(buf.raw[:n.value], time.perf_counter() * 1e6)

    def on_report(self, r, t):
        if not r:
            return
        if r[0] == REPORT_LIVE:
            seq, sampled = struct.unpack_from("<HI", r, 1)
            self.inputs.append((t, seq, sampled, r[7], self.label))
            return
        if r[0] != REPORT_MSG:
            return
        lane, n = r[1], r[2]
        p = r[3:3 + n]
        if lane == LANE_BULK:
            self.bulk += 1
        elif lane == LANE_MSG and p:
            self.on_message(p)

    def on_message(self, p):
        if p[0] == MSG_WAKE:
            session, vid, pid, protocol, flags, fw = struct.unpack_from("<HHHBBI", p, 1)
            name = p[13:45].split(b"\0")[0].decode(errors="replace")
            wake = dict(mode=MODE_NAMES.get(session & 15, session & 15), vid=hex(vid), pid=hex(pid),
                        protocol=protocol, forced=bool(flags & 1), fw=fw, name=name)
            self.wakes.append(wake)
            log("WAKE %s" % wake)
            self.write_msg(LANE_STATUS, [1, 1, 0, 0, 0, 0])  # Adopted, player 1
        elif p[0] == MSG_TOKEN:
            self.token = bytes(p[1:17])
            log("call token %s" % self.token.hex())
            with open(TOKEN_FILE, "w") as f:
                f.write(self.token.hex())
        elif p[0] == MSG_CONFIG:
            self.configs.append(bytes(p[1:]))
        elif p[0] == MSG_REPORT:
            self.reports.append(bytes(p[1:]))

    def close(self):
        self.stop = True
        self.thread.join(1.5)
        hr.kernel32.CloseHandle(self.h)


def config_read_static(link, block=0):
    """Read one static config block through the link: (data, ms, closed by an end marker)."""
    def ours(c):
        return len(c) >= 4 and c[:2] == bytes([0x03, block])

    link.configs.clear()
    start = time.perf_counter()
    # A full 64-byte WebUSB packet, as the config app sends: the largest message (65 bytes)
    link.write_msg(LANE_MSG, bytes([MSG_CONFIG, 0x03, block]) + bytes(62))
    while time.perf_counter() < start + 5:
        time.sleep(0.01)
        if any(ours(c) and c[3] == 0xFF for c in list(link.configs)):
            break
    chunks = [c for c in list(link.configs) if ours(c)]
    data = b"".join(c[4:4 + c[2]] for c in chunks if c[3] != 0xFF)
    return data, (time.perf_counter() - start) * 1000, any(c[3] == 0xFF for c in chunks)


class Session:
    """The app's side: keep the Link device open (reopen when Windows drops it), and send the host
    info every second (Windows keeps a paired HID device present while it is disconnected, so an
    open device doesn't mean a connected gamepad, and the gamepad can't see the app quit)."""

    def __init__(self):
        self.link = None
        self.mode, self.interval = 1, 4000
        self.inputs, self.wakes, self.configs = [], [], []
        self.label = ""
        self.stop = False
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()

    def run(self):
        while not self.stop:
            if self.link is None or self.link.closed:
                if self.link is not None:
                    log("Link device closed")
                    self.link.close()
                    self.link = None
                found = find_link()
                if not found:
                    time.sleep(0.5)
                    continue
                path, in_len, out_len = found
                self.link = Link(path, in_len, out_len)
                self.link.inputs, self.link.wakes, self.link.configs = self.inputs, self.wakes, self.configs
                log("Link device open (in %d, out %d)" % (in_len, out_len))
            self.link.label = self.label
            # Every second: it brings a reconnected gamepad up, and tells a running one the app is
            # still there (silent for 3 s, the gamepad goes back to looking for a dongle)
            self.link.host(self.mode, self.interval)
            time.sleep(1.0)

    def host(self, mode, interval):
        self.mode, self.interval = mode, interval
        if self.link:
            self.link.host(mode, interval)

    def set_label(self, label):
        self.label = label
        if self.link:
            self.link.label = label

    def phase(self, label, mode, interval, seconds):
        self.set_label(label)
        self.host(mode, interval)
        time.sleep(seconds)

    def close(self):
        self.stop = True
        self.thread.join(2)
        if self.link:
            self.link.close()


def mode_switch_result(wakes, before):
    return "WAKE in " + wakes[-1]["mode"] if len(wakes) > before else "no new WAKE"


def main():
    a = sys.argv[1:]
    wait = float(a[a.index("--wait") + 1]) if "--wait" in a else 120.0
    ab = float(a[a.index("--ab") + 1]) if "--ab" in a else 0
    save = a[a.index("--save") + 1] if "--save" in a else None
    log("waiting for the gamepad on its Link HID collection (usage page %04X)" % USAGE_PAGE)
    ses = Session()
    end = time.time() + wait
    while time.time() < end and not ses.wakes:
        time.sleep(0.2)
    if not ses.wakes:
        log("no gamepad in %.0f s" % wait)
        ses.close()
        return 1
    try:
        if ab:
            ses.phase("SInput 4 ms", 1, 4000, ab)
        else:
            ses.phase("SInput 4 ms", 1, 4000, 15)
            ses.phase("SInput 2 ms", 1, 2000, 15)
            data, ms, closed = config_read_static(ses.link, 0)
            log("config: static block 0 %d bytes in %.0f ms, closed %s: %s"
                % (len(data), ms, closed, data[:40].split(b"\0")[0]))
            before = len(ses.wakes)
            ses.phase("XInput 2 ms", 2, 2000, 15)
            log("mode switch to XInput: %s" % mode_switch_result(ses.wakes, before))
            before = len(ses.wakes)
            ses.phase("SInput 2 ms again", 1, 2000, 10)
            log("mode switch back to SInput: %s" % mode_switch_result(ses.wakes, before))
    finally:
        ses.close()
    if save:
        with open(save, "w") as f:
            json.dump(ses.inputs, f)
    print()
    print("live reports (first 10% of each phase dropped):")
    for label in ("SInput 4 ms", "SInput 2 ms", "XInput 2 ms", "SInput 2 ms again"):
        timing(ses.inputs, label)
    gaps = [(b[0] - a_[0]) / 1000 for a_, b in zip(ses.inputs, ses.inputs[1:])]
    print("gaps >= 20 ms: %d over the run; WAKEs: %d" % (sum(1 for g in gaps if g >= 20), len(ses.wakes)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
