"""Measure the actual HID input report rate of a HOJA gamepad.

Windows-only, no third-party packages: talks to hid.dll / setupapi.dll via
ctypes. Put the gamepad in Switch mode (or pass --vid/--pid) and this reports
the observed rate in Hz plus the inter-report interval distribution.

    python scripts/hid_rate.py --list
    python scripts/hid_rate.py --seconds 20

IMPORTANT: this measures the device against *Windows'* USB polling, not the
Switch's. Both honour the descriptor's bInterval, so the rate is comparable,
but console-side behaviour cannot be observed this way.
"""

import argparse
import ctypes
import statistics
import sys
import time
from collections import Counter
from ctypes import wintypes

if sys.platform != "win32":
    sys.exit("hid_rate.py is Windows-only (uses hid.dll / setupapi.dll).")

setupapi = ctypes.WinDLL("setupapi")
hid = ctypes.WinDLL("hid")
kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)

DIGCF_PRESENT = 0x02
DIGCF_DEVICEINTERFACE = 0x10
GENERIC_READ = 0x80000000
FILE_SHARE_READ = 0x01
FILE_SHARE_WRITE = 0x02
OPEN_EXISTING = 3
FILE_FLAG_OVERLAPPED = 0x40000000
INVALID_HANDLE_VALUE = wintypes.HANDLE(-1).value
WAIT_TIMEOUT = 0x102
ERROR_IO_PENDING = 997
HIDP_STATUS_SUCCESS = 0x00110000

# Cadences we care about, as (label, milliseconds per report).
CANDIDATES = [
    ("7/7/8  (22ms per 3)", 22.0 / 3.0),
    ("flat 8ms", 8.0),
    ("8/8/9  (25ms per 3)", 25.0 / 3.0),
]


class GUID(ctypes.Structure):
    _fields_ = [("Data1", wintypes.DWORD), ("Data2", wintypes.WORD),
                ("Data3", wintypes.WORD), ("Data4", ctypes.c_ubyte * 8)]


def _prototype(fn, restype, *argtypes):
    fn.restype = restype
    fn.argtypes = list(argtypes) or None


# Handle-returning calls MUST be prototyped: ctypes defaults restype to int,
# which truncates 64-bit handles and silently breaks everything downstream.
_prototype(setupapi.SetupDiGetClassDevsW, wintypes.HANDLE,
           ctypes.c_void_p, wintypes.LPCWSTR, wintypes.HWND, wintypes.DWORD)
_prototype(setupapi.SetupDiDestroyDeviceInfoList, wintypes.BOOL, wintypes.HANDLE)
_prototype(kernel32.CreateFileW, wintypes.HANDLE,
           wintypes.LPCWSTR, wintypes.DWORD, wintypes.DWORD, ctypes.c_void_p,
           wintypes.DWORD, wintypes.DWORD, wintypes.HANDLE)
_prototype(kernel32.CreateEventW, wintypes.HANDLE,
           ctypes.c_void_p, wintypes.BOOL, wintypes.BOOL, wintypes.LPCWSTR)
_prototype(kernel32.CloseHandle, wintypes.BOOL, wintypes.HANDLE)
_prototype(kernel32.GetCurrentProcess, wintypes.HANDLE)
_prototype(kernel32.GetCurrentThread, wintypes.HANDLE)
_prototype(hid.HidP_GetCaps, ctypes.c_long, ctypes.c_void_p, ctypes.c_void_p)
_prototype(hid.HidD_GetPreparsedData, wintypes.BOOL,
           wintypes.HANDLE, ctypes.c_void_p)
_prototype(hid.HidD_FreePreparsedData, wintypes.BOOL, ctypes.c_void_p)
# Handle-taking calls need argtypes too, or ctypes truncates the argument.
_prototype(setupapi.SetupDiEnumDeviceInterfaces, wintypes.BOOL,
           wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, wintypes.DWORD,
           ctypes.c_void_p)
_prototype(setupapi.SetupDiGetDeviceInterfaceDetailW, wintypes.BOOL,
           wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, wintypes.DWORD,
           ctypes.c_void_p, ctypes.c_void_p)
_prototype(hid.HidD_GetHidGuid, None, ctypes.c_void_p)
_prototype(hid.HidD_GetAttributes, wintypes.BOOL, wintypes.HANDLE, ctypes.c_void_p)
_prototype(hid.HidD_GetProductString, wintypes.BOOL,
           wintypes.HANDLE, ctypes.c_void_p, wintypes.ULONG)
_prototype(kernel32.ReadFile, wintypes.BOOL,
           wintypes.HANDLE, ctypes.c_void_p, wintypes.DWORD, ctypes.c_void_p,
           ctypes.c_void_p)
_prototype(kernel32.GetOverlappedResult, wintypes.BOOL,
           wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, wintypes.BOOL)
_prototype(kernel32.WaitForSingleObject, wintypes.DWORD,
           wintypes.HANDLE, wintypes.DWORD)
_prototype(kernel32.CancelIo, wintypes.BOOL, wintypes.HANDLE)
_prototype(kernel32.ResetEvent, wintypes.BOOL, wintypes.HANDLE)
_prototype(kernel32.SetPriorityClass, wintypes.BOOL, wintypes.HANDLE, wintypes.DWORD)
_prototype(kernel32.SetThreadPriority, wintypes.BOOL, wintypes.HANDLE, ctypes.c_int)


class SP_DEVICE_INTERFACE_DATA(ctypes.Structure):
    _fields_ = [("cbSize", wintypes.DWORD), ("InterfaceClassGuid", GUID),
                ("Flags", wintypes.DWORD),
                ("Reserved", ctypes.POINTER(ctypes.c_ulong))]


class HIDD_ATTRIBUTES(ctypes.Structure):
    _fields_ = [("Size", wintypes.ULONG), ("VendorID", wintypes.USHORT),
                ("ProductID", wintypes.USHORT), ("VersionNumber", wintypes.USHORT)]


class HIDP_CAPS(ctypes.Structure):
    _fields_ = [
        ("Usage", wintypes.USHORT), ("UsagePage", wintypes.USHORT),
        ("InputReportByteLength", wintypes.USHORT),
        ("OutputReportByteLength", wintypes.USHORT),
        ("FeatureReportByteLength", wintypes.USHORT),
        ("Reserved", wintypes.USHORT * 17),
        ("NumberLinkCollectionNodes", wintypes.USHORT),
        ("NumberInputButtonCaps", wintypes.USHORT),
        ("NumberInputValueCaps", wintypes.USHORT),
        ("NumberInputDataIndices", wintypes.USHORT),
        ("NumberOutputButtonCaps", wintypes.USHORT),
        ("NumberOutputValueCaps", wintypes.USHORT),
        ("NumberOutputDataIndices", wintypes.USHORT),
        ("NumberFeatureButtonCaps", wintypes.USHORT),
        ("NumberFeatureValueCaps", wintypes.USHORT),
        ("NumberFeatureDataIndices", wintypes.USHORT),
    ]


class OVERLAPPED(ctypes.Structure):
    _fields_ = [("Internal", ctypes.POINTER(ctypes.c_ulong)),
                ("InternalHigh", ctypes.POINTER(ctypes.c_ulong)),
                ("Offset", wintypes.DWORD), ("OffsetHigh", wintypes.DWORD),
                ("hEvent", wintypes.HANDLE)]


def open_path(path, overlapped=False):
    flags = FILE_FLAG_OVERLAPPED if overlapped else 0
    handle = kernel32.CreateFileW(
        path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        None, OPEN_EXISTING, flags, None)
    return None if handle == INVALID_HANDLE_VALUE else handle


def describe(path):
    """Probe a HID path for VID/PID/product/report length. None if unopenable."""
    handle = open_path(path)
    if handle is None:
        return None
    try:
        attrs = HIDD_ATTRIBUTES()
        attrs.Size = ctypes.sizeof(HIDD_ATTRIBUTES)
        if not hid.HidD_GetAttributes(handle, ctypes.byref(attrs)):
            return None

        name = ctypes.create_unicode_buffer(256)
        product = (name.value if hid.HidD_GetProductString(
            handle, name, ctypes.sizeof(name)) else "")

        preparsed = ctypes.c_void_p()
        report_len = 0
        if hid.HidD_GetPreparsedData(handle, ctypes.byref(preparsed)):
            try:
                caps = HIDP_CAPS()
                if hid.HidP_GetCaps(preparsed, ctypes.byref(caps)) == HIDP_STATUS_SUCCESS:
                    report_len = caps.InputReportByteLength
            finally:
                hid.HidD_FreePreparsedData(preparsed)

        return (path, attrs.VendorID, attrs.ProductID, product, report_len)
    finally:
        kernel32.CloseHandle(handle)


def enumerate_hid():
    """Yield (path, vid, pid, product, input_report_len) for present HID devices."""
    guid = GUID()
    hid.HidD_GetHidGuid(ctypes.byref(guid))

    dev_info = setupapi.SetupDiGetClassDevsW(
        ctypes.byref(guid), None, None, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)
    if dev_info == INVALID_HANDLE_VALUE:
        raise ctypes.WinError(ctypes.get_last_error())

    try:
        index = 0
        while True:
            iface = SP_DEVICE_INTERFACE_DATA()
            iface.cbSize = ctypes.sizeof(SP_DEVICE_INTERFACE_DATA)
            if not setupapi.SetupDiEnumDeviceInterfaces(
                    dev_info, None, ctypes.byref(guid), index, ctypes.byref(iface)):
                break
            index += 1

            needed = wintypes.DWORD()
            setupapi.SetupDiGetDeviceInterfaceDetailW(
                dev_info, ctypes.byref(iface), None, 0, ctypes.byref(needed), None)
            if not needed.value:
                continue

            buf = ctypes.create_string_buffer(needed.value)
            # cbSize is the size of the fixed header, not the whole buffer:
            # 8 on 64-bit, 6 on 32-bit.
            ctypes.cast(buf, ctypes.POINTER(wintypes.DWORD))[0] = (
                8 if ctypes.sizeof(ctypes.c_void_p) == 8 else 6)
            if not setupapi.SetupDiGetDeviceInterfaceDetailW(
                    dev_info, ctypes.byref(iface), buf, needed.value,
                    ctypes.byref(needed), None):
                continue

            path = ctypes.wstring_at(
                ctypes.addressof(buf) + ctypes.sizeof(wintypes.DWORD))
            info = describe(path)
            if info:
                yield info
    finally:
        setupapi.SetupDiDestroyDeviceInfoList(dev_info)


def boost_priority():
    """Reduce scheduler jitter so inter-arrival timing is meaningful."""
    try:
        ctypes.WinDLL("winmm").timeBeginPeriod(1)
        kernel32.SetPriorityClass(kernel32.GetCurrentProcess(), 0x00000080)  # HIGH
        kernel32.SetThreadPriority(kernel32.GetCurrentThread(), 15)  # TIME_CRITICAL
        return True
    except OSError:
        return False


def measure(path, report_len, seconds, quiet):
    handle = open_path(path, overlapped=True)
    if handle is None:
        raise SystemExit("Could not open device (already in use?):\n  " + path)

    event = kernel32.CreateEventW(None, True, False, None)
    buf = ctypes.create_string_buffer(max(report_len, 64))
    read_bytes = wintypes.DWORD()

    stamps = []
    ids = Counter()
    start = time.perf_counter()
    next_tick = start + 1.0
    last_count = 0

    try:
        while time.perf_counter() - start < seconds:
            ov = OVERLAPPED()
            ov.hEvent = event
            kernel32.ResetEvent(event)

            ok = kernel32.ReadFile(handle, buf, len(buf),
                                   ctypes.byref(read_bytes), ctypes.byref(ov))
            if not ok:
                if ctypes.get_last_error() != ERROR_IO_PENDING:
                    raise ctypes.WinError(ctypes.get_last_error())
                if kernel32.WaitForSingleObject(event, 2000) == WAIT_TIMEOUT:
                    kernel32.CancelIo(handle)
                    print("  ! 2s with no report - is the pad awake and in Switch mode?")
                    continue
                if not kernel32.GetOverlappedResult(
                        handle, ctypes.byref(ov), ctypes.byref(read_bytes), False):
                    continue

            now = time.perf_counter()
            if read_bytes.value:
                stamps.append(now)
                ids[buf.raw[0]] += 1

            if not quiet and now >= next_tick:
                got = len(stamps) - last_count
                print("  %5.1fs   %4d reports in last second (%d Hz)"
                      % (now - start, got, got))
                last_count = len(stamps)
                next_tick += 1.0
    except KeyboardInterrupt:
        print("\n  (interrupted)")
    finally:
        kernel32.CancelIo(handle)
        kernel32.CloseHandle(event)
        kernel32.CloseHandle(handle)

    return stamps, ids


def find_cycle(seq, max_len=8):
    """Shortest repeating cycle covering >=90% of seq, or None."""
    for size in range(1, max_len + 1):
        cycle = seq[:size]
        hits = sum(1 for i, v in enumerate(seq) if v == cycle[i % size])
        if hits >= 0.9 * len(seq):
            return cycle
    return None


def report(stamps, ids):
    if len(stamps) < 10:
        print("\nNot enough reports to analyse.")
        return

    span = stamps[-1] - stamps[0]
    n = len(stamps) - 1
    gaps = [(b - a) * 1000.0 for a, b in zip(stamps, stamps[1:])]
    mean_ms = span * 1000.0 / n
    hz = n / span

    print("\n" + "=" * 62)
    print("  reports          %d over %.2fs" % (len(stamps), span))
    print("  RATE             %.2f Hz" % hz)
    print("  mean interval    %.3f ms" % mean_ms)
    print("  median           %.3f ms" % statistics.median(gaps))
    print("  stdev            %.3f ms" % statistics.pstdev(gaps))
    srt = sorted(gaps)
    print("  min / p99 / max  %.2f / %.2f / %.2f ms"
          % (srt[0], srt[int(len(srt) * 0.99)], srt[-1]))

    bursts = sum(1 for g in gaps if g < 1.0)
    if bursts > len(gaps) * 0.02:
        print("\n  ! %d intervals under 1ms (%.1f%%) - the read loop is falling"
              % (bursts, 100.0 * bursts / len(gaps)))
        print("    behind and Windows is batching reports. Per-interval numbers")
        print("    are unreliable; the mean rate is still valid.")

    print("\n  interval histogram (ms)")
    hist = Counter(round(g * 2) / 2 for g in gaps)
    peak = max(hist.values())
    for bucket in sorted(hist):
        count = hist[bucket]
        if count < len(gaps) * 0.005:
            continue
        bar = "#" * int(40 * count / peak)
        print("    %6.1f  %-40s %5d  %5.1f%%"
              % (bucket, bar, count, 100.0 * count / len(gaps)))

    cycle = find_cycle([round(g) for g in gaps[len(gaps) // 4:]])
    if cycle:
        print("\n  repeating pattern   %s ms" % "/".join(str(c) for c in cycle))

    if ids:
        shown = ", ".join("0x%02X:%d" % (k, v) for k, v in ids.most_common(4))
        print("  report IDs          %s" % shown)

    print("\n  closest known cadence:")
    for label, ms in sorted(CANDIDATES, key=lambda c: abs(c[1] - mean_ms)):
        mark = "<--" if abs(ms - mean_ms) < 0.15 else "   "
        print("    %s %-22s %7.2f Hz   (off by %+.3f ms)"
              % (mark, label, 1000.0 / ms, mean_ms - ms))
    print("=" * 62)


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--vid", type=lambda s: int(s, 0), default=0x057E,
                    help="vendor ID (default 0x057E, Nintendo)")
    ap.add_argument("--pid", type=lambda s: int(s, 0), default=None,
                    help="product ID (default: any from --vid)")
    ap.add_argument("--seconds", type=float, default=15.0, help="capture duration")
    ap.add_argument("--index", type=int, default=0,
                    help="which match to use when several collections appear")
    ap.add_argument("--list", action="store_true", help="list HID devices and exit")
    ap.add_argument("--quiet", action="store_true", help="suppress per-second output")
    args = ap.parse_args()

    devices = list(enumerate_hid())

    if args.list:
        print("%d HID device(s):\n" % len(devices))
        for path, vid, pid, product, rlen in devices:
            print("  %04X:%04X  in_len=%-4d %s" % (vid, pid, rlen, product or "(no name)"))
            print("      %s" % path)
        return

    matches = [d for d in devices
               if d[1] == args.vid and (args.pid is None or d[2] == args.pid)]
    if not matches:
        want = ("%04X:%04X" % (args.vid, args.pid)) if args.pid else ("%04X:*" % args.vid)
        sys.exit("No HID device matching %s. Try --list." % want)
    if args.index >= len(matches):
        sys.exit("--index %d out of range (%d matches)." % (args.index, len(matches)))

    path, vid, pid, product, rlen = matches[args.index]
    if len(matches) > 1:
        print("note: %d matching collections; using --index %d" % (len(matches), args.index))

    print("device   %04X:%04X  %s" % (vid, pid, product or "(no name)"))
    print("in_len   %d bytes" % rlen)
    print("priority %s" % ("boosted" if boost_priority()
                           else "default (timing may be noisy)"))
    print("capturing %.0fs - Ctrl+C to stop early\n" % args.seconds)

    stamps, ids = measure(path, rlen or 64, args.seconds, args.quiet)
    report(stamps, ids)


if __name__ == "__main__":
    main()
