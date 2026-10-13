"""Windows Bluetooth pairing for bench work, no clicks: remove a device's pairing, or accept the
Just Works pairing a gamepad starts (it calls this PC without a key).

    python -I scripts/bt_pair_helper.py remove 6AF4B0E017D7
    python -I scripts/bt_pair_helper.py accept 6AF4B0E017D7 [seconds]   # waits, accepts, reports
    python -I scripts/bt_pair_helper.py enable-hid 6AF4B0E017D7          # install its HID service
    python -I scripts/bt_pair_helper.py pair 6AF4B0E017F1 [seconds]     # find it, pair as "Add device" does

Uses bthprops (BluetoothRemoveDevice, BluetoothRegisterForAuthenticationEx,
BluetoothSendAuthenticationResponseEx). No admin needed.
"""
import ctypes
import sys
import time
from ctypes import wintypes

bt = ctypes.WinDLL("bthprops.cpl")


class SYSTEMTIME(ctypes.Structure):
    _fields_ = [(n, wintypes.WORD) for n in ("y", "mo", "dow", "d", "h", "mi", "s", "ms")]


class BLUETOOTH_DEVICE_INFO(ctypes.Structure):
    _fields_ = [("dwSize", wintypes.DWORD),
                ("Address", ctypes.c_ulonglong),
                ("ulClassofDevice", wintypes.ULONG),
                ("fConnected", wintypes.BOOL),
                ("fRemembered", wintypes.BOOL),
                ("fAuthenticated", wintypes.BOOL),
                ("stLastSeen", SYSTEMTIME),
                ("stLastUsed", SYSTEMTIME),
                ("szName", wintypes.WCHAR * 248)]


class AUTH_PARAMS(ctypes.Structure):
    _fields_ = [("deviceInfo", BLUETOOTH_DEVICE_INFO),
                ("authenticationMethod", ctypes.c_int),
                ("ioCapability", ctypes.c_int),
                ("authenticationRequirements", ctypes.c_int),
                ("Numeric_Value", wintypes.ULONG)]


class AUTH_RESPONSE(ctypes.Structure):
    # bthAddressRemote, authMethod, union (largest: OOB data 32 bytes), negativeResponse
    _fields_ = [("bthAddressRemote", ctypes.c_ulonglong),
                ("authMethod", ctypes.c_int),
                ("value", ctypes.c_ubyte * 32),
                ("negativeResponse", ctypes.c_ubyte)]


CALLBACK = ctypes.WINFUNCTYPE(wintypes.BOOL, ctypes.c_void_p, ctypes.POINTER(AUTH_PARAMS))
METHODS = {1: "legacy PIN", 2: "OOB", 3: "numeric comparison / Just Works", 4: "passkey notification",
           5: "passkey"}
SERVICE_DISABLE, SERVICE_ENABLE = 0, 1
MITM_NOT_REQUIRED_GENERAL_BONDING = 4


def addr(text):
    return int(text.replace(":", ""), 16)


def remove(a):
    r = bt.BluetoothRemoveDevice(ctypes.byref(ctypes.c_ulonglong(a)))
    print("remove %012X: %s" % (a, "removed" if r == 0 else "result %d (1168 = not paired)" % r))
    return r in (0, 1168)


def info(a):
    d = BLUETOOTH_DEVICE_INFO()
    d.dwSize = ctypes.sizeof(d)
    d.Address = a
    r = bt.BluetoothGetDeviceInfo(None, ctypes.byref(d))
    return d if r == 0 else None


def accept(a, seconds):
    events = []

    def on_auth(param, p):
        prm = p.contents
        events.append((prm.authenticationMethod, prm.ioCapability, prm.Numeric_Value))
        resp = AUTH_RESPONSE()
        resp.bthAddressRemote = prm.deviceInfo.Address
        resp.authMethod = prm.authenticationMethod
        ctypes.memmove(resp.value, ctypes.byref(wintypes.ULONG(prm.Numeric_Value)), 4)
        resp.negativeResponse = 0
        r = bt.BluetoothSendAuthenticationResponseEx(None, ctypes.byref(resp))
        method = METHODS.get(prm.authenticationMethod, prm.authenticationMethod)
        print("  auth request: %s, io %d, value %d -> accepted (result %d)"
              % (method, prm.ioCapability, prm.Numeric_Value, r), flush=True)
        return True

    cb = CALLBACK(on_auth)
    d = BLUETOOTH_DEVICE_INFO()
    d.dwSize = ctypes.sizeof(d)
    d.Address = a
    handle = ctypes.c_void_p()
    r = bt.BluetoothRegisterForAuthenticationEx(ctypes.byref(d), ctypes.byref(handle), cb, None)
    print("waiting %.0f s for %012X to pair (registration result %d)" % (seconds, a, r), flush=True)
    end = time.time() + seconds
    paired = False
    while time.time() < end:
        time.sleep(0.5)
        i = info(a)
        if i and i.fAuthenticated:
            paired = True
            print("paired: %s, connected %s" % (i.szName, bool(i.fConnected)), flush=True)
            break
    bt.BluetoothUnregisterAuthentication(handle)
    if not paired:
        print("not paired after %.0f s; auth requests seen: %s" % (seconds, events))
    return paired


class GUID(ctypes.Structure):
    _fields_ = [("d1", ctypes.c_uint32),
                ("d2", ctypes.c_uint16),
                ("d3", ctypes.c_uint16),
                ("d4", ctypes.c_ubyte * 8)]


# HumanInterfaceDeviceServiceClass_UUID: 00001124-0000-1000-8000-00805F9B34FB
HID_SERVICE = GUID(0x00001124, 0x0000, 0x1000,
                   (ctypes.c_ubyte * 8)(0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB))


def disable_hid(a):
    """Turn the HID service off (Windows closes HID; the pairing stays)."""
    d = info(a)
    if not d:
        print("%012X is not paired" % a)
        return False
    r = bt.BluetoothSetServiceState(None, ctypes.byref(d), ctypes.byref(HID_SERVICE), SERVICE_DISABLE)
    print("disable HID service on %012X: result %d" % (a, r))
    return r == 0


def enable_hid(a):
    """Turn on the HID service for a paired device (what Settings does after pairing a HID device
    from the PC's side). A device that paired by calling us gets no services by itself."""
    d = info(a)
    if not d:
        print("%012X is not paired" % a)
        return False
    r = bt.BluetoothSetServiceState(None, ctypes.byref(d), ctypes.byref(HID_SERVICE), SERVICE_ENABLE)
    print("enable HID service on %012X: result %d" % (a, r))
    return r == 0


class SEARCH_PARAMS(ctypes.Structure):
    _fields_ = [("dwSize", wintypes.DWORD),
                ("fReturnAuthenticated", wintypes.BOOL),
                ("fReturnRemembered", wintypes.BOOL),
                ("fReturnUnknown", wintypes.BOOL),
                ("fReturnConnected", wintypes.BOOL),
                ("fIssueInquiry", wintypes.BOOL),
                ("cTimeoutMultiplier", ctypes.c_ubyte),
                ("hRadio", wintypes.HANDLE)]


def find(a, seconds):
    """Inquiry until the device answers (it has to be findable). Returns its BLUETOOTH_DEVICE_INFO."""
    end = time.time() + seconds
    while time.time() < end:
        sp = SEARCH_PARAMS(ctypes.sizeof(SEARCH_PARAMS), 1, 1, 1, 1, 1, 4, None)  # 4 x 1.28 s inquiry
        d = BLUETOOTH_DEVICE_INFO()
        d.dwSize = ctypes.sizeof(d)
        bt.BluetoothFindFirstDevice.restype = ctypes.c_void_p
        h = bt.BluetoothFindFirstDevice(ctypes.byref(sp), ctypes.byref(d))
        found = None
        while h:
            if d.Address == a:
                found = d
                break
            if not bt.BluetoothFindNextDevice(ctypes.c_void_p(h), ctypes.byref(d)):
                break
        if h:
            bt.BluetoothFindDeviceClose(ctypes.c_void_p(h))
        if found:
            print("found %012X: %s" % (a, found.szName), flush=True)
            return found
    return None


def pair(a, seconds):
    """Pair from this PC's side, as Settings' Add device does: inquiry, Just Works accepted here, then
    the HID service on (Windows connects HID for the first time)."""
    d = find(a, seconds)
    if not d:
        print("%012X not found (is it findable?)" % a)
        return False
    if d.fAuthenticated:
        print("already paired")
    else:
        def on_auth(param, p):
            prm = p.contents
            resp = AUTH_RESPONSE()
            resp.bthAddressRemote = prm.deviceInfo.Address
            resp.authMethod = prm.authenticationMethod
            ctypes.memmove(resp.value, ctypes.byref(wintypes.ULONG(prm.Numeric_Value)), 4)
            r = bt.BluetoothSendAuthenticationResponseEx(None, ctypes.byref(resp))
            method = METHODS.get(prm.authenticationMethod)
            print("  auth request: %s -> accepted (result %d)" % (method, r), flush=True)
            return True
        cb = CALLBACK(on_auth)
        handle = ctypes.c_void_p()
        bt.BluetoothRegisterForAuthenticationEx(ctypes.byref(d), ctypes.byref(handle), cb, None)
        bonding = MITM_NOT_REQUIRED_GENERAL_BONDING
        r = bt.BluetoothAuthenticateDeviceEx(None, None, ctypes.byref(d), None, bonding)
        bt.BluetoothUnregisterAuthentication(handle)
        print("pairing: result %d" % r)
        if r != 0:
            return False
    return enable_hid(a)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    a = addr(sys.argv[2])
    if sys.argv[1] == "remove":
        sys.exit(0 if remove(a) else 1)
    if sys.argv[1] == "pair":
        sys.exit(0 if pair(a, float(sys.argv[3]) if len(sys.argv) > 3 else 40) else 1)
    if sys.argv[1] == "disable-hid":
        sys.exit(0 if disable_hid(a) else 1)
    if sys.argv[1] == "enable-hid":
        sys.exit(0 if enable_hid(a) else 1)
    if sys.argv[1] == "accept":
        sys.exit(0 if accept(a, float(sys.argv[3]) if len(sys.argv) > 3 else 60) else 1)
    sys.exit(__doc__)
