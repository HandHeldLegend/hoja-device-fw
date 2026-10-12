"""Stand in for HHL Gamepad Link on this PC and test a gamepad's app link (dongle_link.h, "The PC app").

Publishes the app's RFCOMM service (SDP record with its UUID, channel 5 when free), waits for the
gamepad, then runs a script: SInput at 4 ms, 2 ms, a switch to XInput and back, and a config app
read through the link. Prints what arrived and the timing of the live reports (arrival minus the
gamepad's sample time, drift removed).

    python -I scripts/app_link_peer.py [--wait 120]
    python -I scripts/app_link_peer.py --ab 240 [--save ab.json]   # A/B build (HOJA_BT_APP_ASSESS_AB)

Windows only (Winsock Bluetooth). No admin needed.
"""
import ctypes
import socket
import struct
import sys
import time
import uuid
from ctypes import wintypes

UUID = uuid.UUID("bbebd868-289d-4930-81dd-45d882b64bbf")
CHANNEL = 5
LANE_INPUT, LANE_BULK, LANE_MSG, LANE_STATUS = 1, 2, 3, 4
MSG_HOST, MSG_WAKE, MSG_REPORT, MSG_OUTPUT, MSG_CONFIG = 1, 2, 3, 4, 5
MODE_NAMES = {0: "Switch", 1: "SInput", 2: "XInput", 3: "Slippi", 15: "unknown"}
T0 = time.perf_counter()


def log(msg):
    print("%7.2f s  %s" % (time.perf_counter() - T0, msg), flush=True)


# --- SDP record, as the app publishes it (WSASetService) ---
class SOCKET_ADDRESS(ctypes.Structure):
    _fields_ = [("lpSockaddr", ctypes.c_void_p), ("iSockaddrLength", ctypes.c_int)]


class CSADDR_INFO(ctypes.Structure):
    _fields_ = [("LocalAddr", SOCKET_ADDRESS), ("RemoteAddr", SOCKET_ADDRESS),
                ("iSocketType", ctypes.c_int), ("iProtocol", ctypes.c_int)]


class GUID(ctypes.Structure):
    _fields_ = [("d1", ctypes.c_uint32), ("d2", ctypes.c_uint16), ("d3", ctypes.c_uint16), ("d4", ctypes.c_ubyte * 8)]


class WSAQUERYSETW(ctypes.Structure):
    _fields_ = [("dwSize", wintypes.DWORD), ("lpszServiceInstanceName", ctypes.c_wchar_p),
                ("lpServiceClassId", ctypes.POINTER(GUID)), ("lpVersion", ctypes.c_void_p),
                ("lpszComment", ctypes.c_wchar_p), ("dwNameSpace", wintypes.DWORD),
                ("lpNSProviderId", ctypes.c_void_p), ("lpszContext", ctypes.c_wchar_p),
                ("dwNumberOfProtocols", wintypes.DWORD), ("lpafpProtocols", ctypes.c_void_p),
                ("lpszQueryString", ctypes.c_wchar_p), ("dwNumberOfCsAddrs", wintypes.DWORD),
                ("lpcsaBuffer", ctypes.POINTER(CSADDR_INFO)), ("dwOutputFlags", wintypes.DWORD),
                ("lpBlob", ctypes.c_void_p)]


def publish(channel, register=True):
    ws2 = ctypes.WinDLL("ws2_32")
    addr = (ctypes.c_ubyte * 30)()
    struct.pack_into("<HQ16sI", addr, 0, 32, 0, b"\0" * 16, channel)
    csa = CSADDR_INFO()
    csa.LocalAddr = SOCKET_ADDRESS(ctypes.cast(addr, ctypes.c_void_p), 30)
    csa.iSocketType, csa.iProtocol = 1, 3
    g = GUID(UUID.fields[0], UUID.fields[1], UUID.fields[2], (ctypes.c_ubyte * 8)(*UUID.bytes[8:]))
    q = WSAQUERYSETW()
    q.dwSize = ctypes.sizeof(q)
    q.lpszServiceInstanceName = "HHL Gamepad Link"
    q.lpServiceClassId = ctypes.pointer(g)
    q.dwNameSpace = 16  # NS_BTH
    q.dwNumberOfCsAddrs = 1
    q.lpcsaBuffer = ctypes.pointer(csa)
    r = ws2.WSASetServiceW(ctypes.byref(q), 0 if register else 2, 0)
    return r == 0, ws2.WSAGetLastError() if r else 0


def frame(lane, payload):
    return bytes([lane, len(payload)]) + bytes(payload)


def host(mode, interval_us, version=0x000200):
    return frame(LANE_MSG, bytes([MSG_HOST]) + struct.pack("<BBBBIH", 3, 4, 1, mode, version, interval_us))


class Peer:
    def __init__(self, conn):
        self.c = conn
        self.buf = b""
        self.inputs = []  # (t_pc_us, seq, sampled_us, len, interval_label)
        self.wakes = []
        self.configs = []
        self.reports = []
        self.bulk = 0
        self.label = ""

    def send(self, data):
        self.c.sendall(data)

    def pump(self, seconds):
        end = time.perf_counter() + seconds
        while time.perf_counter() < end:
            self.c.settimeout(max(0.001, end - time.perf_counter()))
            try:
                d = self.c.recv(4096)
            except socket.timeout:
                break
            if not d:
                raise ConnectionError("gamepad closed the link")
            t = time.perf_counter() * 1e6
            self.buf += d
            while len(self.buf) >= 2 and len(self.buf) >= 2 + self.buf[1]:
                lane, n = self.buf[0], self.buf[1]
                p, self.buf = self.buf[2:2 + n], self.buf[2 + n:]
                self.on_frame(lane, p, t)

    def on_frame(self, lane, p, t):
        if lane == LANE_INPUT:
            seq, sampled = struct.unpack_from("<HI", p)
            self.inputs.append((t, seq, sampled, p[6], self.label))
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
                self.send(frame(LANE_STATUS, [1, 1, 0, 0, 0, 0]))  # adopted, player 1
            elif p[0] == MSG_CONFIG:
                self.configs.append(bytes(p[1:]))
            elif p[0] == MSG_REPORT:
                self.reports.append(bytes(p[1:]))


def pct(v, q):
    v = sorted(v)
    return v[min(len(v) - 1, int(q * len(v)))] if v else float("nan")


def timing(inputs, label):
    rows = [r for r in inputs if r[4] == label]
    if len(rows) < 10:
        print("  %-16s %d reports" % (label, len(rows)))
        return
    rows = rows[len(rows) // 10:]  # settle
    dur = (rows[-1][0] - rows[0][0]) / 1e6
    seqs = [r[1] for r in rows]
    span = ((seqs[-1] - seqs[0]) & 0xFFFF) + 1
    # delay: arrival - sampled, drift removed by a line through per-second minimums
    pts = [(r[2], r[0] - r[2]) for r in rows]
    by_s = {}
    for s_, d in pts:
        k = s_ // 1_000_000
        by_s[k] = min(by_s.get(k, d), d)
    xs = sorted(by_s)
    if len(xs) > 2:
        mx = sum(xs) / len(xs)
        my = sum(by_s[x] for x in xs) / len(xs)
        slope = sum((x - mx) * (by_s[x] - my) for x in xs) / sum((x - mx) ** 2 for x in xs) / 1e6
    else:
        slope = 0
    d = [dd - slope * s_ for s_, dd in pts]
    lo = min(d)
    d = [(x - lo) / 1000 for x in d]
    gaps = [(b[0] - a[0]) / 1000 for a, b in zip(rows, rows[1:])]
    print("  %-16s %6.0f/s  sent-seq span %5d got %5d | PC gap p50 %.2f p99 %.2f max %.1f ms | delay p50 %.2f p90 %.2f p99 %.2f max %.1f ms" % (
        label, len(rows) / dur, span, len(set(seqs)), pct(gaps, .5), pct(gaps, .99), max(gaps),
        pct(d, .5), pct(d, .9), pct(d, .99), max(d)))


def config_read_static(peer, block=0):
    """WebUSB: [0x03, block] -> chunks [0x03][block][size][part][data], closed by part 0xFF."""
    peer.configs.clear()
    t = time.perf_counter()
    peer.send(frame(LANE_MSG, bytes([MSG_CONFIG, 0x03, block])))
    end = t + 5
    while time.perf_counter() < end:
        peer.pump(0.05)
        if any(c[:2] == bytes([0x03, block]) and c[3] == 0xFF for c in peer.configs if len(c) >= 4):
            break
    chunks = [c for c in peer.configs if len(c) >= 4 and c[:2] == bytes([0x03, block])]
    data = b"".join(c[4:4 + c[2]] for c in chunks if c[3] != 0xFF)
    return data, (time.perf_counter() - t) * 1000, any(c[3] == 0xFF for c in chunks)


def ab_report(inputs, interval_ms):
    """Per state (bit 15 of seq): reports/s, gaps over 8 ms with their lengths and spacing."""
    for state, name in ((0, "RFCOMM alone"), (1, "HID up too")):
        rows = [r for r in inputs if (r[1] >> 15) == state]
        # Contiguous stretches of one state, without the first 0.5 s after each flip
        runs, cur = [], []
        for a in rows:
            if cur and a[0] - cur[-1][0] > 1e6:
                runs.append(cur)
                cur = []
            cur.append(a)
        if cur:
            runs.append(cur)
        dur, n, gaps, stall_times = 0.0, 0, [], []
        for run in runs:
            run = [r for r in run if r[0] - run[0][0] > 5e5]
            if len(run) < 10:
                continue
            dur += (run[-1][0] - run[0][0]) / 1e6
            n += len(run)
            for a, b in zip(run, run[1:]):
                g = (b[0] - a[0]) / 1000
                gaps.append(g)
                if g >= 8:
                    stall_times.append((a[0], g))
        if not dur:
            continue
        lens = sorted(g for _, g in stall_times)
        spacing = sorted((b[0] - a[0]) / 1000 for a, b in zip(stall_times, stall_times[1:]))
        print("  %-15s %5.0f s  %4.0f/s | gaps >= 8 ms: %4d (%.2f/s), length p50 %.1f max %.1f ms, spacing p50 %s ms | gap p99 %.2f ms" % (
            name, dur, n / dur, len(stall_times), len(stall_times) / dur,
            pct(lens, .5) if lens else 0, lens[-1] if lens else 0,
            "%.0f" % pct(spacing, .5) if spacing else "-", pct(gaps, .99)))
        hist = {}
        for g in lens:
            k = int(g // 5) * 5
            hist[k] = hist.get(k, 0) + 1
        print("                  gap lengths: " + ", ".join("%d-%d ms: %d" % (k, k + 5, v) for k, v in sorted(hist.items())))


def main():
    if "--ab" in sys.argv:
        return main_ab(float(sys.argv[sys.argv.index("--ab") + 1]))
    wait = 120.0
    if "--wait" in sys.argv:
        wait = float(sys.argv[sys.argv.index("--wait") + 1])
    srv = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    srv.bind(("00:00:00:00:00:00", CHANNEL))
    srv.listen(1)
    ok, err = publish(CHANNEL)
    log("listening on channel %d; SDP record %s" % (CHANNEL, "published" if ok else "NOT published (error %d)" % err))
    srv.settimeout(wait)
    try:
        conn, addr = srv.accept()
    except socket.timeout:
        log("no gamepad connected in %.0f s" % wait)
        publish(CHANNEL, register=False)
        return 1
    log("gamepad connected from %s" % (addr,))
    peer = Peer(conn)
    try:
        peer.label = "SInput 4 ms"
        peer.send(host(1, 4000))
        peer.pump(15)
        peer.label = "SInput 2 ms"
        peer.send(host(1, 2000))
        peer.pump(15)

        data, ms, closed = config_read_static(peer, 0)
        log("config: static block 0 (device) %d bytes in %.0f ms, closed %s: %s" % (
            len(data), ms, closed, data[:48].split(b"\0")[0][:40]))

        n_wake = len(peer.wakes)
        peer.label = "XInput 2 ms"
        peer.send(host(2, 2000))
        peer.pump(15)
        log("mode switch to XInput: %s" % ("WAKE in " + peer.wakes[-1]["mode"] if len(peer.wakes) > n_wake else "no new WAKE"))
        n_wake = len(peer.wakes)
        peer.label = "SInput 2 ms again"
        peer.send(host(1, 2000))
        peer.pump(10)
        log("mode switch back to SInput: %s" % ("WAKE in " + peer.wakes[-1]["mode"] if len(peer.wakes) > n_wake else "no new WAKE"))
    except ConnectionError as e:
        log(str(e))
    finally:
        conn.close()
        srv.close()
        publish(CHANNEL, register=False)

    print("\nlive reports (first 10% of each phase dropped):")
    for label in ("SInput 4 ms", "SInput 2 ms", "XInput 2 ms", "SInput 2 ms again"):
        timing(peer.inputs, label)
    print("reliable reports (command replies): %d, bulk: %d, WAKEs: %d" % (len(peer.reports), peer.bulk, len(peer.wakes)))
    return 0


def main_ab(seconds):
    srv = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    srv.bind(("00:00:00:00:00:00", CHANNEL))
    srv.listen(1)
    ok, err = publish(CHANNEL)
    log("A/B: listening; SDP record %s" % ("published" if ok else "NOT published (%d)" % err))
    # The gamepad may drop and call again (an A/B that changes the connection): keep accepting
    peer = None
    end = time.perf_counter() + seconds
    srv.settimeout(120)
    while time.perf_counter() < end:
        try:
            conn, addr = srv.accept()
        except socket.timeout:
            log("no gamepad for 120 s")
            break
        log("gamepad connected from %s" % (addr,))
        if peer is None:
            peer = Peer(conn)
        else:
            peer.c, peer.buf = conn, b""
        try:
            peer.send(host(1, 4000))
            peer.pump(end - time.perf_counter())
        except ConnectionError as e:
            log(str(e))
        finally:
            conn.close()
    srv.close()
    publish(CHANNEL, register=False)
    if peer is None:
        return 1
    if "--save" in sys.argv:
        import json
        json.dump(peer.inputs, open(sys.argv[sys.argv.index("--save") + 1], "w"))
    print()
    print("SInput at 4 ms, HID next to the link every other 45 s:")
    ab_report(peer.inputs, 4)
    return 0


if __name__ == "__main__":
    sys.exit(main())
