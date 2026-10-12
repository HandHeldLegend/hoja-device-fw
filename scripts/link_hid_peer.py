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
from app_link_peer import pct, MODE_NAMES, timing  # noqa: E402

USAGE_PAGE, USAGE = 0xFF48, 0x4C
REPORT_LIVE, REPORT_MSG, REPORT_OUT = 1, 2, 3
LANE_BULK, LANE_MSG, LANE_STATUS = 2, 3, 4
MSG_HOST, MSG_WAKE, MSG_REPORT, MSG_CONFIG = 1, 2, 3, 5
GENERIC_WRITE = 0x40000000
T0 = time.perf_counter()


def log(msg):
    print("%7.2f s  %s" % (time.perf_counter() - T0, msg), flush=True)


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
                    return info[0], caps.InputReportByteLength, caps.OutputReportByteLength, info
        finally:
            hr.kernel32.CloseHandle(h)
    return None


class Link:
    def __init__(self, path, in_len, out_len):
        self.h = hr.kernel32.CreateFileW(path, hr.GENERIC_READ | GENERIC_WRITE,
                                         hr.FILE_SHARE_READ | hr.FILE_SHARE_WRITE, None, hr.OPEN_EXISTING,
                                         hr.FILE_FLAG_OVERLAPPED, None)
        self.in_len, self.out_len = in_len, out_len
        self.inputs, self.wakes, self.configs, self.reports = [], [], [], []
        self.bulk = 0
        self.label = ""
        self.stop = False
        self.closed = False
        self.wlock = threading.Lock()
        self.wev = hr.kernel32.CreateEventW(None, True, False, None)
        self.thread = threading.Thread(target=self.reader, daemon=True)
        self.thread.start()

    def write_msg(self, lane, payload):
        r = bytes([REPORT_OUT, lane, len(payload)]) + bytes(payload)
        r = r + b"\0" * (self.out_len - len(r))
        with self.wlock:
            ov = hr.OVERLAPPED()
            ov.hEvent = self.wev
            hr.kernel32.ResetEvent(self.wev)
            n = wintypes.DWORD()
            buf = ctypes.create_string_buffer(r, len(r))
            if not hr.kernel32.WriteFile(self.h, buf, len(r), ctypes.byref(n), ctypes.byref(ov)):
                if ctypes.get_last_error() != hr.ERROR_IO_PENDING:
                    return False
                hr.kernel32.WaitForSingleObject(self.wev, 1000)
                if not hr.kernel32.GetOverlappedResult(self.h, ctypes.byref(ov), ctypes.byref(n), False):
                    return False
        return True

    def host(self, mode, interval_us):
        return self.write_msg(LANE_MSG, bytes([MSG_HOST]) + struct.pack("<BBBBIH", 3, 4, 1, mode, 0x000200, interval_us))

    def reader(self):
        hr.boost_priority()
        ev = hr.kernel32.CreateEventW(None, True, False, None)
        buf = ctypes.create_string_buffer(self.in_len)
        n = wintypes.DWORD()
        while not self.stop:
            ov = hr.OVERLAPPED()
            ov.hEvent = ev
            hr.kernel32.ResetEvent(ev)
            if not hr.kernel32.ReadFile(self.h, buf, self.in_len, ctypes.byref(n), ctypes.byref(ov)):
                if ctypes.get_last_error() != hr.ERROR_IO_PENDING:
                    self.closed = True
                    return
                if hr.kernel32.WaitForSingleObject(ev, 500) == hr.WAIT_TIMEOUT:
                    hr.kernel32.CancelIo(self.h)
                    hr.kernel32.GetOverlappedResult(self.h, ctypes.byref(ov), ctypes.byref(n), True)
                    continue
                if not hr.kernel32.GetOverlappedResult(self.h, ctypes.byref(ov), ctypes.byref(n), False):
                    self.closed = True
                    return
            t = time.perf_counter() * 1e6
            self.on_report(buf.raw[:n.value], t)

    def on_report(self, r, t):
        if not r:
            return
        if r[0] == REPORT_LIVE:
            seq, sampled = struct.unpack_from("<HI", r, 1)
            self.inputs.append((t, seq, sampled, r[7], self.label))
        elif r[0] == REPORT_MSG:
            lane, n = r[1], r[2]
            p = r[3:3 + n]
            if lane == 0x7F:  # diagnostic build: the gamepad's counters
                outs, ticks, sent, errs, oid, olen, lerr, host, cans, st, peeks, mfails, mtype, merr, fmt, mreq, adopted, mlen =                     struct.unpack_from("<HHHHBBBBBBHHBBBBBB", p)
                if ticks % 250 == 0:
                    log("diag: outputs %d host %d | ticks %d sent %d errors %d (last %d) can_send %d hid %d | peeks %d msg fails %d "
                        "last msg type %d len %d | format %d mode request %d adopted %d" % (
                            outs, host, ticks, sent, errs, lerr - 256 if lerr > 127 else lerr, cans, st, peeks, mfails,
                            mtype, mlen, fmt, mreq, adopted))
            elif lane == LANE_BULK:
                self.bulk += 1
            elif lane == LANE_MSG and p:
                if p[0] == MSG_WAKE:
                    session, vid, pid, proto, flags, fw = struct.unpack_from("<HHHBBI", p, 1)
                    name = p[13:45].split(b"\0")[0].decode(errors="replace")
                    w = dict(mode=MODE_NAMES.get(session & 15, session & 15), vid=hex(vid), pid=hex(pid),
                             protocol=proto, forced=bool(flags & 1), fw=fw, name=name)
                    self.wakes.append(w)
                    log("WAKE %s" % w)
                    self.write_msg(LANE_STATUS, [1, 1, 0, 0, 0, 0])  # adopted, player 1
                elif p[0] == MSG_CONFIG:
                    self.configs.append(bytes(p[1:]))
                elif p[0] == MSG_REPORT:
                    self.reports.append(bytes(p[1:]))

    def close(self):
        self.stop = True
        self.thread.join(1.5)
        hr.kernel32.CloseHandle(self.h)


def config_read_static(link, block=0):
    link.configs.clear()
    t = time.perf_counter()
    # A full 64-byte WebUSB packet, as the config app sends: the largest message (65 bytes)
    link.write_msg(LANE_MSG, bytes([MSG_CONFIG, 0x03, block]) + bytes(62))
    end = t + 5
    while time.perf_counter() < end:
        time.sleep(0.01)
        if any(len(c) >= 4 and c[:2] == bytes([0x03, block]) and c[3] == 0xFF for c in list(link.configs)):
            break
    chunks = [c for c in list(link.configs) if len(c) >= 4 and c[:2] == bytes([0x03, block])]
    data = b"".join(c[4:4 + c[2]] for c in chunks if c[3] != 0xFF)
    return data, (time.perf_counter() - t) * 1000, any(c[3] == 0xFF for c in chunks)


class Session:
    """The app's side: keep the Link device open (reopen when Windows drops it), and repeat the host
    info every second until the gamepad says who it is (Windows keeps a paired HID device present
    while it is disconnected, so an open device doesn't mean a connected gamepad)."""

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
                f = find_link()
                if not f:
                    time.sleep(0.5)
                    continue
                path, in_len, out_len, info = f
                self.link = Link(path, in_len, out_len)
                self.link.inputs, self.link.wakes, self.link.configs = self.inputs, self.wakes, self.configs
                self.link.label = self.label
                log("Link device open (in %d, out %d)" % (in_len, out_len))
            self.link.label = self.label
            if not self.wakes or (self.inputs and time.perf_counter() * 1e6 - self.inputs[-1][0] > 1e6):
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

    def close(self):
        self.stop = True
        self.thread.join(2)
        if self.link:
            self.link.close()


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
            ses.set_label("SInput 4 ms")
            ses.host(1, 4000)
            time.sleep(ab)
        else:
            ses.set_label("SInput 4 ms")
            ses.host(1, 4000)
            time.sleep(15)
            ses.set_label("SInput 2 ms")
            ses.host(1, 2000)
            time.sleep(15)
            data, ms, closed = config_read_static(ses.link, 0)
            log("config: static block 0 %d bytes in %.0f ms, closed %s: %s" % (len(data), ms, closed, data[:40].split(b"\0")[0]))
            n = len(ses.wakes)
            ses.set_label("XInput 2 ms")
            ses.host(2, 2000)
            time.sleep(15)
            log("mode switch to XInput: %s" % ("WAKE in " + ses.wakes[-1]["mode"] if len(ses.wakes) > n else "no new WAKE"))
            n = len(ses.wakes)
            ses.set_label("SInput 2 ms again")
            ses.host(1, 2000)
            time.sleep(10)
            log("mode switch back to SInput: %s" % ("WAKE in " + ses.wakes[-1]["mode"] if len(ses.wakes) > n else "no new WAKE"))
    finally:
        ses.close()
    if save:
        json.dump(ses.inputs, open(save, "w"))
    print()
    print("live reports (first 10% of each phase dropped):")
    for label in ("SInput 4 ms", "SInput 2 ms", "XInput 2 ms", "SInput 2 ms again"):
        timing(ses.inputs, label)
    gaps = [(b[0] - a_[0]) / 1000 for a_, b in zip(ses.inputs, ses.inputs[1:])]
    print("gaps >= 20 ms: %d over the run; WAKEs: %d" % (sum(1 for g in gaps if g >= 20), len(ses.wakes)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
