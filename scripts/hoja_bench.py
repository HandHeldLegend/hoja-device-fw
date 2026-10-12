"""Bench control for a HOJA gamepad and a HOJA dongle on one Windows PC: no buttons, no installs.

Talks to both over WebUSB (WinUSB, plain ctypes): reboots them into the bootloader or into a
transport, flashes UF2 images through the bootloader drive, and reports what is attached.

    python -I scripts/hoja_bench.py status
    python -I scripts/hoja_bench.py dongle-info
    python -I scripts/hoja_bench.py dongle-boot                 # dongle -> bootloader
    python -I scripts/hoja_bench.py pad-boot                    # gamepad -> bootloader (cable in)
    python -I scripts/hoja_bench.py pad-dongle                  # gamepad: reboot onto the dongle link
    python -I scripts/hoja_bench.py pad-bt                      # gamepad: reboot into Bluetooth
    python -I scripts/hoja_bench.py flash-dongle <image.uf2>
    python -I scripts/hoja_bench.py flash-pad <image.uf2> [--dongle]   # then back onto the dongle link

The gamepad is reached directly when it is on USB in a wired mode, else through the dongle's
tunnel. Its bootloader only appears with a cable in. Boards are told apart by USB serial: the
dongle reports 000000, the gamepad its chip id. Only one board may be in the bootloader at a
time (both share the bootloader's serial); a flash checks that the other one is still running.
"""
import ctypes
import os
import shutil
import string
import sys
import time
from ctypes import wintypes as wt

if sys.platform != "win32":
    sys.exit("hoja_bench.py is Windows-only")

WEBUSB_GUID = "{8B3E9D2E-7EEC-4994-AAE7-0C40DE84D36D}"
HOJA_VID = "VID_2E8A"
BOOTSEL_PID = "PID_0003"
DONGLE_SERIAL = "000000"

# WebUSB commands (HOJA-LIB-RP2040 settings_shared_types.h, the dongle's dongle_webusb.h)
WEBUSB_ID_CONFIG_COMMAND = 0x04
CFG_BLOCK_GAMEPAD = 0
GAMEPAD_CMD_RESET_TO_BOOTLOADER = 1
GAMEPAD_CMD_REBOOT_DONGLE = 3
GAMEPAD_CMD_REBOOT_BLUETOOTH = 4
DONGLE_WEBUSB_INFO = 0xD0
DONGLE_WEBUSB_BOOTLOADER = 0xD1

setupapi = ctypes.windll.setupapi
cfgmgr = ctypes.windll.cfgmgr32
k32 = ctypes.windll.kernel32
winusb = ctypes.windll.winusb
setupapi.SetupDiGetClassDevsW.restype = ctypes.c_void_p
k32.CreateFileW.restype = ctypes.c_void_p


class GUID(ctypes.Structure):
    _fields_ = [("Data1", wt.DWORD), ("Data2", wt.WORD), ("Data3", wt.WORD), ("Data4", ctypes.c_ubyte * 8)]


class SP_DEVICE_INTERFACE_DATA(ctypes.Structure):
    _fields_ = [("cbSize", wt.DWORD), ("InterfaceClassGuid", GUID), ("Flags", wt.DWORD), ("Reserved", ctypes.c_void_p)]


class SP_DEVINFO_DATA(ctypes.Structure):
    _fields_ = [("cbSize", wt.DWORD), ("ClassGuid", GUID), ("DevInst", wt.DWORD), ("Reserved", ctypes.c_void_p)]


class PIPE_INFO(ctypes.Structure):
    _fields_ = [("PipeType", ctypes.c_int), ("PipeId", ctypes.c_ubyte), ("MaximumPacketSize", ctypes.c_ushort),
                ("Interval", ctypes.c_ubyte)]


class IFACE_DESC(ctypes.Structure):
    _fields_ = [("bLength", ctypes.c_ubyte), ("bDescriptorType", ctypes.c_ubyte), ("bInterfaceNumber", ctypes.c_ubyte),
                ("bAlternateSetting", ctypes.c_ubyte), ("bNumEndpoints", ctypes.c_ubyte),
                ("bInterfaceClass", ctypes.c_ubyte), ("bInterfaceSubClass", ctypes.c_ubyte),
                ("bInterfaceProtocol", ctypes.c_ubyte), ("iInterface", ctypes.c_ubyte)]


def _guid(s):
    g = GUID()
    ctypes.oledll.ole32.CLSIDFromString(ctypes.c_wchar_p(s), ctypes.byref(g))
    return g


def _device_id(devinst):
    buf = ctypes.create_unicode_buffer(512)
    if cfgmgr.CM_Get_Device_IDW(devinst, buf, 512, 0) != 0:
        return ""
    return buf.value


def webusb_devices():
    """[(path, parent device id)] of every present HOJA WebUSB interface."""
    g = _guid(WEBUSB_GUID)
    h = setupapi.SetupDiGetClassDevsW(ctypes.byref(g), None, None, 0x12)  # PRESENT | DEVICEINTERFACE
    out = []
    i = 0
    while True:
        did = SP_DEVICE_INTERFACE_DATA()
        did.cbSize = ctypes.sizeof(did)
        if not setupapi.SetupDiEnumDeviceInterfaces(ctypes.c_void_p(h), None, ctypes.byref(g), i, ctypes.byref(did)):
            break
        i += 1
        need = wt.DWORD()
        setupapi.SetupDiGetDeviceInterfaceDetailW(ctypes.c_void_p(h), ctypes.byref(did), None, 0, ctypes.byref(need), None)
        buf = ctypes.create_string_buffer(need.value)
        ctypes.cast(buf, ctypes.POINTER(wt.DWORD))[0] = 8 if ctypes.sizeof(ctypes.c_void_p) == 8 else 6
        dinfo = SP_DEVINFO_DATA()
        dinfo.cbSize = ctypes.sizeof(dinfo)
        if not setupapi.SetupDiGetDeviceInterfaceDetailW(ctypes.c_void_p(h), ctypes.byref(did), buf, need, None,
                                                         ctypes.byref(dinfo)):
            continue
        path = ctypes.wstring_at(ctypes.addressof(buf) + 4)
        parent = wt.DWORD()
        pid = _device_id(parent.value) if cfgmgr.CM_Get_Parent(ctypes.byref(parent), dinfo.DevInst, 0) == 0 else ""
        out.append((path, pid))
    setupapi.SetupDiDestroyDeviceInfoList(ctypes.c_void_p(h))
    return out


def usb_devices():
    """Instance ids of present HOJA USB devices (composite parents and bootloaders)."""
    h = setupapi.SetupDiGetClassDevsW(None, ctypes.c_wchar_p("USB"), None, 0x06)  # PRESENT | ALLCLASSES
    out = []
    i = 0
    while True:
        dinfo = SP_DEVINFO_DATA()
        dinfo.cbSize = ctypes.sizeof(dinfo)
        if not setupapi.SetupDiEnumDeviceInfo(ctypes.c_void_p(h), i, ctypes.byref(dinfo)):
            break
        i += 1
        did = _device_id(dinfo.DevInst)
        if HOJA_VID in did.upper() and "&MI_" not in did.upper():
            out.append(did.upper())
    setupapi.SetupDiDestroyDeviceInfoList(ctypes.c_void_p(h))
    return out


def serial_of(device_id):
    return device_id.rsplit("\\", 1)[-1]


class WebUsb:
    def __init__(self, path):
        self.f = k32.CreateFileW(path, 0xC0000000, 3, None, 3, 0x40000080, None)
        if self.f in (None, ctypes.c_void_p(-1).value):
            raise RuntimeError("cannot open the WebUSB interface (is the config app open in a browser?)")
        self.h = ctypes.c_void_p()
        if not winusb.WinUsb_Initialize(ctypes.c_void_p(self.f), ctypes.byref(self.h)):
            k32.CloseHandle(ctypes.c_void_p(self.f))
            raise RuntimeError("WinUsb_Initialize failed (%d)" % k32.GetLastError())
        d = IFACE_DESC()
        winusb.WinUsb_QueryInterfaceSettings(self.h, 0, ctypes.byref(d))
        self.ep_in = self.ep_out = None
        for n in range(d.bNumEndpoints):
            pi = PIPE_INFO()
            winusb.WinUsb_QueryPipe(self.h, 0, n, ctypes.byref(pi))
            if pi.PipeId & 0x80:
                self.ep_in = pi.PipeId
            else:
                self.ep_out = pi.PipeId
        t = wt.ULONG(300)
        winusb.WinUsb_SetPipePolicy(self.h, self.ep_in, 0x03, 4, ctypes.byref(t))  # PIPE_TRANSFER_TIMEOUT

    def write(self, data):
        b = (ctypes.c_ubyte * 64)(*data)
        n = wt.ULONG()
        return bool(winusb.WinUsb_WritePipe(self.h, self.ep_out, b, len(data), ctypes.byref(n), None))

    def read(self):
        b = (ctypes.c_ubyte * 64)()
        n = wt.ULONG()
        if winusb.WinUsb_ReadPipe(self.h, self.ep_in, b, 64, ctypes.byref(n), None):
            return bytes(b[:n.value])
        return None

    def close(self):
        winusb.WinUsb_Free(self.h)
        k32.CloseHandle(ctypes.c_void_p(self.f))


def find_webusb(which):
    """The WebUSB path of 'dongle' or 'pad' (the gamepad on its own cable), or None."""
    for path, parent in webusb_devices():
        s = serial_of(parent)
        if (which == "dongle") == (s == DONGLE_SERIAL):
            return path
    return None


def send(which, data, read=False):
    path = find_webusb(which)
    if not path:
        raise RuntimeError("no %s WebUSB interface present" % which)
    d = WebUsb(path)
    try:
        if not d.write(data):
            raise RuntimeError("write to the %s failed" % which)
        if read:
            for _ in range(10):
                r = d.read()
                if r and r[0] == data[0]:
                    return r
            return None
    finally:
        d.close()


def pad_command(cmd):
    """A gamepad config command: on its own cable if it is there, else through the dongle."""
    data = [WEBUSB_ID_CONFIG_COMMAND, CFG_BLOCK_GAMEPAD, cmd]
    if find_webusb("pad"):
        send("pad", data)
        return "cable"
    send("dongle", data)
    return "dongle"


# --- state ---

def bootloader_drives():
    out = []
    for letter in string.ascii_uppercase:
        p = "%s:\\INFO_UF2.TXT" % letter
        if os.path.exists(p):
            try:
                if "RPI-RP2" in open(p, errors="ignore").read():
                    out.append("%s:\\" % letter)
            except OSError:
                pass
    return out


def state():
    devs = usb_devices()
    boot = [d for d in devs if BOOTSEL_PID in d]
    dongle = [d for d in devs if BOOTSEL_PID not in d and serial_of(d) == DONGLE_SERIAL]
    pad = [d for d in devs if BOOTSEL_PID not in d and serial_of(d) != DONGLE_SERIAL]
    return {"bootloader": boot, "dongle": dongle, "pad": pad, "drives": bootloader_drives()}


def wait_for(cond, timeout, what):
    end = time.time() + timeout
    while time.time() < end:
        st = state()
        if cond(st):
            return st
        time.sleep(0.25)
    raise RuntimeError("timed out waiting for %s" % what)


def dongle_info():
    r = send("dongle", [DONGLE_WEBUSB_INFO], read=True)
    if not r or len(r) < 22:
        return None
    info = {
        "protocol": r[1], "board": r[2], "fw": int.from_bytes(r[4:8], "little"),
        "host_kind": r[10], "host_mode": r[11], "gamepad": bool(r[3]),
    }
    if r[3]:
        info.update({"gamepad_mode": r[12], "gamepad_fw": int.from_bytes(r[14:18], "little"),
                     "gamepad_vid": int.from_bytes(r[18:20], "little"), "gamepad_pid": int.from_bytes(r[20:22], "little"),
                     "gamepad_name": r[22:54].split(b"\0")[0].decode(errors="replace")})
    return info


# --- flashing ---

def flash(image, target):
    other = "pad" if target == "dongle" else "dongle"
    st = wait_for(lambda s: len(s["drives"]) == 1 and len(s["bootloader"]) == 1, 15, "the bootloader drive")
    if st[target]:
        raise RuntimeError("the %s is still running: the board in the bootloader is not it" % target)
    if other == "dongle" and not st["dongle"]:
        raise RuntimeError("the dongle is not running: cannot tell which board is in the bootloader")
    drive = st["drives"][0]
    shutil.copyfile(image, os.path.join(drive, os.path.basename(image)))
    wait_for(lambda s: not s["drives"], 15, "the bootloader to finish")
    print("flashed %s -> %s" % (os.path.basename(image), target))


def flash_dongle(image):
    if state()["drives"]:
        raise RuntimeError("a board is already in the bootloader")
    send("dongle", [DONGLE_WEBUSB_BOOTLOADER])
    flash(image, "dongle")
    wait_for(lambda s: s["dongle"], 20, "the dongle to come back")


def flash_pad(image, then_dongle):
    if state()["drives"]:
        raise RuntimeError("a board is already in the bootloader")
    via = pad_command(GAMEPAD_CMD_RESET_TO_BOOTLOADER)
    print("gamepad -> bootloader (%s)" % via)
    flash(image, "pad")
    # With a cable in and nothing in boot memory, it comes back wired
    wait_for(lambda s: s["pad"], 20, "the gamepad to come back on its cable")
    if then_dongle:
        time.sleep(1.0)
        pad_command(GAMEPAD_CMD_REBOOT_DONGLE)
        print("gamepad -> dongle link")
        wait_for(lambda s: not s["pad"], 10, "the gamepad to leave USB")
        end = time.time() + 20
        while time.time() < end:
            try:
                i = dongle_info()
                if i and i["gamepad"]:
                    print("gamepad connected through the dongle: %s" % i.get("gamepad_name"))
                    return
            except RuntimeError:
                pass
            time.sleep(0.5)
        raise RuntimeError("the gamepad did not connect to the dongle")


def main():
    a = sys.argv[1:]
    if not a:
        sys.exit(__doc__)
    cmd = a[0]
    try:
        if cmd == "status":
            st = state()
            for k in ("dongle", "pad", "bootloader", "drives"):
                print("%-10s %s" % (k, ", ".join(st[k]) or "-"))
            for path, parent in webusb_devices():
                print("webusb     %s" % parent)
        elif cmd == "dongle-info":
            print(dongle_info())
        elif cmd == "dongle-boot":
            send("dongle", [DONGLE_WEBUSB_BOOTLOADER])
        elif cmd == "pad-boot":
            print("via", pad_command(GAMEPAD_CMD_RESET_TO_BOOTLOADER))
        elif cmd == "pad-dongle":
            print("via", pad_command(GAMEPAD_CMD_REBOOT_DONGLE))
        elif cmd == "pad-bt":
            print("via", pad_command(GAMEPAD_CMD_REBOOT_BLUETOOTH))
        elif cmd == "flash-dongle":
            flash_dongle(a[1])
        elif cmd == "flash-pad":
            flash_pad(a[1], "--dongle" in a[2:])
        else:
            sys.exit(__doc__)
    except RuntimeError as e:
        sys.exit("error: %s" % e)


if __name__ == "__main__":
    main()
