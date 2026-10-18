"""Bench control for a HOJA gamepad and a HOJA dongle on one Windows PC: no buttons, no installs.

Talks to both over WebUSB (WinUSB, plain ctypes): reboots them into the bootloader or into a
transport, flashes UF2 images through the bootloader drive, and reports what is attached.

    python -I scripts/hoja_bench.py status
    python -I scripts/hoja_bench.py dongle-info
    python -I scripts/hoja_bench.py dongle-boot                 # dongle -> bootloader
    python -I scripts/hoja_bench.py dongle-mode <mode>          # the dongle's mode on a computer:
                                                                # switch sinput xinput slippi
    python -I scripts/hoja_bench.py pad-boot                    # gamepad -> bootloader (cable in)
    python -I scripts/hoja_bench.py pad-dongle [mode]           # gamepad: reboot onto the dongle link
    python -I scripts/hoja_bench.py pad-bt [mode]               # gamepad: reboot into Bluetooth
                                       [--pair]   # either: and start pairing
        mode: switch xinput slippi gamecube n64 snes sinput wii (kept, as a boot combo's would be);
        none keeps the current mode
    python -I scripts/hoja_bench.py flash-dongle <image.uf2>    # several dongles: --use hoja|pico|pico2
    python -I scripts/hoja_bench.py flash-pad <image.uf2> [--dongle]   # then back onto the dongle link
                                       [--in-bootloader]   # already there (put by hand)

Boards with the HOJA bootloader: pad-boot opens its USB update mode (WinUSB, no driver install).
    python -I scripts/hoja_bench.py update-flash <image.bin>    # the whole image from the start of flash,
                                                                # read back, then the firmware starts
    python -I scripts/hoja_bench.py update-stage <image.bin>    # staged as an update over the air would be,
                                       [--deflate]              # then the bootloader writes it into place.
                                       [--radio]                # The whole board image (.bin) is cut to the
                                       [--break]                # firmware, or with --radio to the radio
                                                                # region; --break erases the destination's
                                                                # first sector first, to prove the write
    python -I scripts/hoja_bench.py update-layout               # the layout as the bootloader reports it
    python -I scripts/hoja_bench.py update-read <addr> <len>    # hex dump
    python -I scripts/hoja_bench.py update-reboot               # back to the firmware
    python -I scripts/hoja_bench.py update-rom                  # into the boot ROM's bootloader

The gamepad is reached directly when it is on USB in a wired mode, else through the dongle's
tunnel. Its bootloader only appears with a cable in. Boards are told apart by USB serial: the
dongle reports 000000, the gamepad its chip id. Only one board may be in the bootloader at a
time (both share the bootloader's serial); a flash checks that the other one is still running.
"""
import ctypes
import os
import string
import struct
import sys
import time
import zlib
from ctypes import wintypes as wt

if sys.platform != "win32":
    sys.exit("hoja_bench.py is Windows-only")

WEBUSB_GUID = "{8B3E9D2E-7EEC-4994-AAE7-0C40DE84D36D}"
HOJA_VID = "VID_2E8A"
# XInput and Slippi keep the IDs games look for; on a computer the config interface sits beside them
OTHER_IDS = ("VID_045E&PID_028E", "VID_057E&PID_0337")
USB_DEVICE_GUID = "{A5DCBF10-6530-11D2-901F-00C04FB951ED}"  # GUID_DEVINTERFACE_USB_DEVICE
# The HOJA bootloader's update mode (HOJA-LIB-RP2040 bootloader/boot_usb.c)
UPDATE_GUID = "{2C8F6F2C-8CEF-4D98-8035-CE6AC66AB70A}"
# Bootloaders: RP2040 (the gamepads, Pico W dongles) and RP2350 (the HOJA dongle board, Pico 2 W)
BOOTSEL_PIDS = ("PID_0003", "PID_000F")
DONGLE_BOARDS = {"hoja": 1, "pico": 2, "pico2": 3}       # dongle_info board ids
DONGLE_CHIP_PID = {1: "PID_000F", 2: "PID_0003", 3: "PID_000F"}
DONGLE_SEL = None  # --use <board>: which dongle, when several are attached
DONGLE_SERIAL = "000000"

# WebUSB commands (HOJA-LIB-RP2040 settings_shared_types.h, the dongle's dongle_webusb.h)
WEBUSB_ID_CONFIG_COMMAND = 0x04
CFG_BLOCK_GAMEPAD = 0
GAMEPAD_CMD_RESET_TO_BOOTLOADER = 1
GAMEPAD_CMD_REBOOT_DONGLE = 3
GAMEPAD_CMD_REBOOT_BLUETOOTH = 4
REBOOT_ARG_MODE_SET, REBOOT_ARG_PAIR = 0x80, 0x40  # The reboots' argument: [mode given][pair][mode]
DONGLE_WEBUSB_INFO = 0xD0
DONGLE_WEBUSB_BOOTLOADER = 0xD1
DONGLE_WEBUSB_PC_MODE = 0xD2
DONGLE_MODES = {"switch": 0, "sinput": 1, "xinput": 2, "slippi": 3}  # dongle_mode_t

setupapi = ctypes.windll.setupapi
cfgmgr = ctypes.windll.cfgmgr32
k32 = ctypes.windll.kernel32
winusb = ctypes.windll.winusb
setupapi.SetupDiGetClassDevsW.restype = ctypes.c_void_p
k32.CreateFileW.restype = ctypes.c_void_p


class GUID(ctypes.Structure):
    _fields_ = [("Data1", wt.DWORD),
                ("Data2", wt.WORD),
                ("Data3", wt.WORD),
                ("Data4", ctypes.c_ubyte * 8)]


class SP_DEVICE_INTERFACE_DATA(ctypes.Structure):
    _fields_ = [("cbSize", wt.DWORD),
                ("InterfaceClassGuid", GUID),
                ("Flags", wt.DWORD),
                ("Reserved", ctypes.c_void_p)]


class SP_DEVINFO_DATA(ctypes.Structure):
    _fields_ = [("cbSize", wt.DWORD),
                ("ClassGuid", GUID),
                ("DevInst", wt.DWORD),
                ("Reserved", ctypes.c_void_p)]


class PIPE_INFO(ctypes.Structure):
    _fields_ = [("PipeType", ctypes.c_int),
                ("PipeId", ctypes.c_ubyte),
                ("MaximumPacketSize", ctypes.c_ushort),
                ("Interval", ctypes.c_ubyte)]


class IFACE_DESC(ctypes.Structure):
    _fields_ = [("bLength", ctypes.c_ubyte),
                ("bDescriptorType", ctypes.c_ubyte),
                ("bInterfaceNumber", ctypes.c_ubyte),
                ("bAlternateSetting", ctypes.c_ubyte),
                ("bNumEndpoints", ctypes.c_ubyte),
                ("bInterfaceClass", ctypes.c_ubyte),
                ("bInterfaceSubClass", ctypes.c_ubyte),
                ("bInterfaceProtocol", ctypes.c_ubyte),
                ("iInterface", ctypes.c_ubyte)]


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
    """[(path, parent device id)] of every present HOJA WebUSB interface. An adapter that Windows gave
    WinUSB as a whole (Zadig's driver for the GameCube adapter) has no interface of its own: its
    device path is listed, with its own id, and WebUsb reaches interface 1 through it."""
    out = _interfaces(WEBUSB_GUID, parent=True)
    out += [(path, did) for path, did in _interfaces(USB_DEVICE_GUID, parent=False)
            if any(i in did.upper() for i in OTHER_IDS) and _service(did) == "WINUSB"]
    return out


def _service(device_id):
    """The device's function driver service (upper case), or "" """
    devinst = wt.DWORD()
    if cfgmgr.CM_Locate_DevNodeW(ctypes.byref(devinst), ctypes.c_wchar_p(device_id), 0) != 0:
        return ""
    buf = ctypes.create_unicode_buffer(256)
    size = wt.ULONG(ctypes.sizeof(buf))
    # CM_DRP_SERVICE
    if cfgmgr.CM_Get_DevNode_Registry_PropertyW(devinst, 0x05, None, buf, ctypes.byref(size), 0) != 0:
        return ""
    return buf.value.upper()


def _interfaces(guid, parent):
    """[(path, device id)] of present device interfaces of guid: the id of the device's parent, or
    its own"""
    g = _guid(guid)
    info_set = ctypes.c_void_p(setupapi.SetupDiGetClassDevsW(ctypes.byref(g), None, None,
                                                              0x12))  # PRESENT | DEVICEINTERFACE
    out = []
    i = 0
    while True:
        did = SP_DEVICE_INTERFACE_DATA()
        did.cbSize = ctypes.sizeof(did)
        if not setupapi.SetupDiEnumDeviceInterfaces(info_set, None, ctypes.byref(g), i, ctypes.byref(did)):
            break
        i += 1
        need = wt.DWORD()
        setupapi.SetupDiGetDeviceInterfaceDetailW(info_set, ctypes.byref(did), None, 0, ctypes.byref(need),
                                                  None)
        buf = ctypes.create_string_buffer(need.value)
        ctypes.cast(buf, ctypes.POINTER(wt.DWORD))[0] = 8 if ctypes.sizeof(ctypes.c_void_p) == 8 else 6
        dinfo = SP_DEVINFO_DATA()
        dinfo.cbSize = ctypes.sizeof(dinfo)
        if not setupapi.SetupDiGetDeviceInterfaceDetailW(info_set, ctypes.byref(did), buf, need, None,
                                                         ctypes.byref(dinfo)):
            continue
        path = ctypes.wstring_at(ctypes.addressof(buf) + 4)
        if parent:
            up = wt.DWORD()
            has_parent = cfgmgr.CM_Get_Parent(ctypes.byref(up), dinfo.DevInst, 0) == 0
            out.append((path, _device_id(up.value) if has_parent else ""))
        else:
            out.append((path, _device_id(dinfo.DevInst)))
    setupapi.SetupDiDestroyDeviceInfoList(info_set)
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
        if (HOJA_VID in did.upper() or any(i in did.upper() for i in OTHER_IDS)) and "&MI_" not in did.upper():
            out.append(did.upper())
    setupapi.SetupDiDestroyDeviceInfoList(ctypes.c_void_p(h))
    return out


def serial_of(device_id):
    return device_id.rsplit("\\", 1)[-1]


class WebUsb:
    def __init__(self, path):
        self.f = k32.CreateFileW(path, 0xC0000000, 3, None, 3, 0x40000080, None)
        if self.f in (None, ctypes.c_void_p(-1).value):
            raise RuntimeError("cannot open the WebUSB interface (is the config app open in a browser, "
                               "or Dolphin holding the adapter?)")
        self.h0 = ctypes.c_void_p()
        if not winusb.WinUsb_Initialize(ctypes.c_void_p(self.f), ctypes.byref(self.h0)):
            k32.CloseHandle(ctypes.c_void_p(self.f))
            raise RuntimeError("WinUsb_Initialize failed (%d)" % k32.GetLastError())
        self.h = self.h0
        d = IFACE_DESC()
        winusb.WinUsb_QueryInterfaceSettings(self.h, 0, ctypes.byref(d))
        if d.bInterfaceClass != 0xFF or d.bInterfaceSubClass != 0:
            # The whole adapter: the config interface is the next one
            self.h = ctypes.c_void_p()
            if not winusb.WinUsb_GetAssociatedInterface(self.h0, 0, ctypes.byref(self.h)):
                self.h = None
                self.close()
                raise RuntimeError("no config interface beside the adapter")
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
        if self.h and self.h is not self.h0:
            winusb.WinUsb_Free(self.h)
        winusb.WinUsb_Free(self.h0)
        k32.CloseHandle(ctypes.c_void_p(self.f))


def _info_from(r):
    if not r or len(r) < 22 or r[0] != DONGLE_WEBUSB_INFO:
        return None
    info = {
        "protocol": r[1], "board": r[2], "fw": int.from_bytes(r[4:8], "little"),
        "host_kind": r[10], "host_mode": r[11], "gamepad": bool(r[3]),
    }
    if r[3]:
        info.update({"gamepad_mode": r[12], "gamepad_fw": int.from_bytes(r[14:18], "little"),
                     "gamepad_vid": int.from_bytes(r[18:20], "little"),
                     "gamepad_pid": int.from_bytes(r[20:22], "little"),
                     "gamepad_name": r[22:54].split(b"\0")[0].decode(errors="replace")})
    return info


def dongles():
    """[(path, info)] of every attached dongle answering WebUSB (they all report serial 000000)."""
    out = []
    for path, parent in webusb_devices():
        if serial_of(parent) != DONGLE_SERIAL:
            continue
        try:
            out.append((path, _info_from(send_path(path, [DONGLE_WEBUSB_INFO], read=True))))
        except RuntimeError:
            pass
    return out


def find_dongle(prefer_gamepad=False):
    """The selected dongle (--use), else the one with a gamepad when asked, else the only one."""
    ds = [(p, i) for p, i in dongles() if i]
    if DONGLE_SEL:
        ds = [(p, i) for p, i in ds if i["board"] == DONGLE_BOARDS[DONGLE_SEL]]
    if prefer_gamepad and any(i["gamepad"] for _, i in ds):
        ds = [(p, i) for p, i in ds if i["gamepad"]]
    if len(ds) > 1 and not prefer_gamepad:
        raise RuntimeError("several dongles attached: pick one with --use %s" % "|".join(DONGLE_BOARDS))
    return ds[0] if ds else (None, None)


def find_webusb(which):
    """The WebUSB path of 'dongle' (see find_dongle) or 'pad' (the gamepad on its own cable), or None."""
    if which == "dongle":
        return find_dongle(prefer_gamepad=True)[0]
    for path, parent in webusb_devices():
        if serial_of(parent) != DONGLE_SERIAL:
            return path
    return None


def send(which, data, read=False):
    path = find_webusb(which)
    if not path:
        raise RuntimeError("no %s WebUSB interface present" % which)
    return send_path(path, data, read)


def send_path(path, data, read=False, reply=None):
    """reply: the reply's first byte, when it isn't the command's own"""
    d = WebUsb(path)
    try:
        if not d.write(data):
            raise RuntimeError("write to %s failed" % path)
        if read:
            for _ in range(10):
                r = d.read()
                if r and r[0] == (data[0] if reply is None else reply):
                    return r
            return None
    finally:
        d.close()


def wait_dongle_gamepad(timeout):
    """True once a gamepad is connected through the dongle (the dongle drops tunnel traffic before)."""
    end = time.time() + timeout
    while time.time() < end:
        try:
            i = dongle_info()
            if i and i["gamepad"]:
                return True
        except RuntimeError:
            pass
        time.sleep(0.5)
    return False


MODES = {"switch": 0, "xinput": 1, "slippi": 2, "gamecube": 3, "n64": 4, "snes": 5, "sinput": 6, "wii": 7}


def pad_command(cmd, arg=None):
    """A gamepad config command: on its own cable if it is there, else through the dongle. arg is the
    command's argument byte as sent (for the reboots: REBOOT_ARG_*)."""
    data = [WEBUSB_ID_CONFIG_COMMAND, CFG_BLOCK_GAMEPAD, cmd] + ([arg] if arg is not None else [])
    if find_webusb("pad"):
        send("pad", data)
        return "cable"
    if not wait_dongle_gamepad(20):
        raise RuntimeError("no gamepad on its cable or through the dongle")
    send("dongle", data)
    return "dongle"


# --- HOJA bootloader update mode (PICOBOOT over WinUSB) ---

PB_MAGIC = 0x431FD10B
PB_REBOOT, PB_ERASE, PB_WRITE, PB_READ = 0x02, 0x03, 0x05, 0x84
PB_ROM_BOOTLOADER = 0x21
FLASH_BASE = 0x10000000
SECTOR = 4096
BOOT_SIZE = 0x8000  # boot_layout.h
STAGE_MAGIC = 0x45474154
STAGE_VERSION = 1
STAGE_STORED, STAGE_DEFLATE = 0, 1
STAGE_TO_APP, STAGE_TO_RADIO = 0, 1
REQ_FAULT, REQ_LAYOUT = 0x48, 0x49


class Picoboot:
    def __init__(self, path):
        self.f = k32.CreateFileW(path, 0xC0000000, 3, None, 3, 0x40000080, None)
        if self.f in (None, ctypes.c_void_p(-1).value):
            raise RuntimeError("cannot open the update mode (is the config app holding it?)")
        self.h = ctypes.c_void_p()
        if not winusb.WinUsb_Initialize(ctypes.c_void_p(self.f), ctypes.byref(self.h)):
            k32.CloseHandle(ctypes.c_void_p(self.f))
            raise RuntimeError("WinUsb_Initialize failed (%d)" % k32.GetLastError())
        d = IFACE_DESC()
        winusb.WinUsb_QueryInterfaceSettings(self.h, 0, ctypes.byref(d))
        for n in range(d.bNumEndpoints):
            pi = PIPE_INFO()
            winusb.WinUsb_QueryPipe(self.h, 0, n, ctypes.byref(pi))
            if pi.PipeId & 0x80:
                self.ep_in = pi.PipeId
            else:
                self.ep_out = pi.PipeId
        t = wt.ULONG(5000)
        for ep in (self.ep_in, self.ep_out):
            winusb.WinUsb_SetPipePolicy(self.h, ep, 0x03, 4, ctypes.byref(t))  # PIPE_TRANSFER_TIMEOUT
        self.token = 1

    def reset(self):
        """Clear a refused command: the pipes' halt on both ends, then PICOBOOT's interface reset"""
        for ep in (self.ep_in, self.ep_out):
            winusb.WinUsb_ResetPipe(self.h, ep)
        setup = struct.pack("<BBHHH", 0x41, 0x41, 0, 0, 0)
        n = wt.ULONG()
        winusb.WinUsb_ControlTransfer(self.h, ctypes.c_uint64(int.from_bytes(setup, "little")), None, 0,
                                      ctypes.byref(n), None)

    def _out(self, data):
        data = bytes(data)
        b = (ctypes.c_ubyte * max(1, len(data))).from_buffer_copy(data or b"\0")
        n = wt.ULONG()
        if not winusb.WinUsb_WritePipe(self.h, self.ep_out, b, len(data), ctypes.byref(n), None):
            raise RuntimeError("update mode: write failed (%d)" % k32.GetLastError())

    def _in(self, length):
        b = (ctypes.c_ubyte * max(64, length))()
        n = wt.ULONG()
        if not winusb.WinUsb_ReadPipe(self.h, self.ep_in, b, max(64, length), ctypes.byref(n), None):
            raise RuntimeError("update mode: read failed (%d)" % k32.GetLastError())
        return bytes(b[:n.value])

    def cmd(self, cmd_id, args=b"", data=None, read_len=0):
        """One PICOBOOT command: its packet, its data either way, then the acknowledgement"""
        transfer = len(data) if data is not None else read_len
        self._out(struct.pack("<IIBBHI", PB_MAGIC, self.token, cmd_id, len(args), 0, transfer) + args.ljust(16, b"\0"))
        self.token += 1
        if cmd_id & 0x80:
            got = self._in(read_len)
            self._out(b"")
            return got
        if data is not None:
            self._out(data)
        if self._in(0):
            raise RuntimeError("update mode: acknowledgement carried data")
        return None

    def erase(self, addr, length):
        self.cmd(PB_ERASE, struct.pack("<II", addr, length))

    def write(self, addr, data):
        self.cmd(PB_WRITE, struct.pack("<II", addr, len(data)), data=data)

    def read(self, addr, length):
        """In sectors: PICOBOOT moves at most one per command"""
        out = b""
        for at in range(addr, addr + length, SECTOR):
            n = min(SECTOR, addr + length - at)
            out += self.cmd(PB_READ, struct.pack("<II", at, n), read_len=n)
        return out

    def control_in(self, request, length):
        """A PICOBOOT-style interface request, device to host"""
        setup = struct.pack("<BBHHH", 0xC1, request, 0, 0, length)
        b = (ctypes.c_ubyte * length)()
        n = wt.ULONG()
        if not winusb.WinUsb_ControlTransfer(self.h, ctypes.c_uint64(int.from_bytes(setup, "little")), b, length,
                                             ctypes.byref(n), None):
            raise RuntimeError("update mode: request 0x%02x failed (%d)" % (request, k32.GetLastError()))
        return bytes(b[:n.value])

    def layout(self):
        v = struct.unpack("<B3xIIIIII", self.control_in(REQ_LAYOUT, 28))
        keys = ("version", "flash_size", "radio_offset", "radio_size", "app_offset", "stage_header_offset",
                "top_offset")
        return dict(zip(keys, v))

    def fault(self):
        return struct.unpack("<II", self.control_in(REQ_FAULT, 8))

    def reboot(self, rom=False):
        try:
            if rom:
                self.cmd(PB_ROM_BOOTLOADER)
            else:
                self.cmd(PB_REBOOT, struct.pack("<III", 0, 0x20042000, 100))
        except RuntimeError:
            pass  # It may be gone before the acknowledgement is read

    def close(self):
        winusb.WinUsb_Free(self.h)
        k32.CloseHandle(ctypes.c_void_p(self.f))


def update_devices():
    """[(path, device id)] of boards in the HOJA update mode"""
    return _interfaces(UPDATE_GUID, parent=False)


def open_update(timeout=15):
    end = time.time() + timeout
    while time.time() < end:
        devs = update_devices()
        if len(devs) > 1:
            raise RuntimeError("several boards in the update mode")
        if devs:
            path, did = devs[0]
            print("update mode: %s (driver %s)" % (did, _service(did) or "?"))
            return Picoboot(path)
        time.sleep(0.25)
    raise RuntimeError("no board in the update mode")


def write_image(pb, addr, image, label):
    image = image + b"\xff" * ((-len(image)) % SECTOR)
    t0 = time.time()
    for off in range(0, len(image), SECTOR):
        pb.erase(addr + off, SECTOR)
        pb.write(addr + off, image[off:off + SECTOR])
    t1 = time.time()
    back = pb.read(addr, len(image))
    t2 = time.time()
    # The bootloader never rewrites its own sectors: compare the rest
    skip = max(0, FLASH_BASE + BOOT_SIZE - addr)
    if back[skip:] != image[skip:]:
        bad = next(i for i in range(skip, len(image)) if back[i] != image[i])
        raise RuntimeError("%s: read back differs at 0x%08x" % (label, addr + bad))
    print("%s: %d KB written in %.1f s, read back in %.1f s, matches" % (label, len(image) // 1024, t1 - t0, t2 - t1))


def update_flash(image_path):
    image = open(image_path, "rb").read()
    pb = open_update()
    try:
        if pb.read(FLASH_BASE, BOOT_SIZE) != image[:BOOT_SIZE]:
            print("note: this image's bootloader differs from the board's, which stays as it is")
        write_image(pb, FLASH_BASE, image, "image")
        pb.reboot()
    finally:
        pb.close()


def stage_part(dest, encoding, staged_offset, staged, image):
    """One part of a stage header (boot_layout.h boot_stage_part_s)"""
    return struct.pack("<BBxxIIIII", dest, encoding, staged_offset, len(staged), zlib.crc32(staged) & 0xFFFFFFFF,
                       len(image), zlib.crc32(image) & 0xFFFFFFFF)


def stage_header(parts):
    """The stage header (boot_stage_header_s): up to four parts, written together"""
    return struct.pack("<IIBBxx", STAGE_MAGIC, STAGE_MAGIC ^ 0xFFFFFFFF, STAGE_VERSION, len(parts)) + \
        b"".join(parts).ljust(4 * 24, b"\0")


def update_stage(image_path, deflate, radio, break_dest):
    image = open(image_path, "rb").read()
    pb = open_update()
    try:
        lay = pb.layout()
        if radio:
            # The radio region of a whole board image, without its unused tail
            part = image[lay["radio_offset"]:lay["app_offset"]].rstrip(b"\xff")
            dest, dest_offset, room = STAGE_TO_RADIO, lay["radio_offset"], lay["radio_size"]
        else:
            # A whole board image is cut to the firmware; a firmware image is taken as it is
            whole = image[lay["app_offset"]:lay["app_offset"] + 256] == image[:256]
            part = image[lay["app_offset"]:] if whole else image
            dest, dest_offset, room = STAGE_TO_APP, lay["app_offset"], lay["stage_header_offset"] - lay["app_offset"]
        if not part or len(part) > room:
            raise RuntimeError("nothing to stage, or too large for its destination")

        if deflate:
            c = zlib.compressobj(9, zlib.DEFLATED, -15)
            staged = c.compress(part) + c.flush()
        else:
            staged = part
        data_offset = lay["stage_header_offset"] - (len(staged) + SECTOR - 1) // SECTOR * SECTOR
        if data_offset < dest_offset + len(part):
            raise RuntimeError("no room to stage %d bytes above a %d byte image" % (len(staged), len(part)))

        write_image(pb, FLASH_BASE + data_offset, staged, "staged data")
        if break_dest:
            pb.erase(FLASH_BASE + dest_offset, SECTOR)
            print("the destination's first sector erased")
        header = stage_header([stage_part(dest, STAGE_DEFLATE if deflate else STAGE_STORED, data_offset, staged,
                                          part)])
        pb.erase(FLASH_BASE + lay["stage_header_offset"], SECTOR)
        pb.write(FLASH_BASE + lay["stage_header_offset"], header.ljust(256, b"\xff"))
        print("staged %s: %d bytes as %d (%s) at 0x%06x" % ("radio" if radio else "firmware", len(part), len(staged),
                                                           "deflate" if deflate else "stored", data_offset))
        pb.reboot()
    finally:
        pb.close()


# --- state ---

def bootloader_drives():
    out = []
    for letter in string.ascii_uppercase:
        p = "%s:\\INFO_UF2.TXT" % letter
        if os.path.exists(p):
            try:
                txt = open(p, errors="ignore").read()
                if "RPI-RP2" in txt or "RP2350" in txt:
                    out.append("%s:\\" % letter)
            except OSError:
                pass
    return out


def state():
    devs = usb_devices()
    isboot = lambda d: any(p in d for p in BOOTSEL_PIDS)
    boot = [d for d in devs if isboot(d)]
    dongle = [d for d in devs if not isboot(d) and serial_of(d) == DONGLE_SERIAL]
    pad = [d for d in devs if not isboot(d) and serial_of(d) != DONGLE_SERIAL]
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
    return find_dongle(prefer_gamepad=True)[1]


# --- flashing ---

def copy_to_drive(image, drive):
    """Copy a UF2 onto the bootloader drive. Python's writes to it fail now and then right after it
    appears (EINVAL); PowerShell's copy has not: retry once, then hand it to that."""
    import subprocess
    target = os.path.join(drive, os.path.basename(image))
    for attempt in range(2):
        try:
            with open(image, "rb") as src, open(target, "wb") as dst:
                for chunk in iter(lambda: src.read(64 * 1024), b""):
                    dst.write(chunk)
            return
        except OSError as e:
            print("copy to %s failed (%s), %s" % (drive, e, "retrying" if attempt == 0 else "using PowerShell"))
            time.sleep(1.0)
    subprocess.run(["powershell", "-NoProfile", "-Command", "Copy-Item -LiteralPath '%s' -Destination '%s' "
                    "-ErrorAction Stop" % (image, target)], check=True)


def flash(image, target, before, chip_pid=None):
    """Copy image to the one bootloader drive, once the board sent there (target) has left and every
    other board is still running: then the bootloader can only be it. chip_pid: its bootloader's PID."""
    st = wait_for(lambda s: len(s["drives"]) == 1 and len(s["bootloader"]) == 1, 15, "the bootloader drive")
    others = "pad" if target == "dongle" else "dongle"
    if len(st[target]) != len(before[target]) - 1 or len(st[others]) != len(before[others]):
        raise RuntimeError("cannot tell which board is in the bootloader: not flashing")
    if chip_pid and chip_pid not in st["bootloader"][0]:
        raise RuntimeError("the bootloader is the wrong chip (%s): not flashing" % st["bootloader"][0])
    drive = st["drives"][0]
    copy_to_drive(image, drive)
    wait_for(lambda s: not s["drives"], 15, "the bootloader to finish")
    print("flashed %s -> %s" % (os.path.basename(image), target))


def flash_dongle(image):
    before = state()
    if before["drives"]:
        raise RuntimeError("a board is already in the bootloader")
    path, info = find_dongle()
    if not path:
        raise RuntimeError("no dongle")
    send_path(path, [DONGLE_WEBUSB_BOOTLOADER])
    flash(image, "dongle", before, DONGLE_CHIP_PID.get(info["board"]))
    wait_for(lambda s: len(s["dongle"]) == len(before["dongle"]), 20, "the dongle to come back")


def flash_pad(image, then_dongle, in_bootloader=False):
    before = state()
    if not before["pad"]:
        before["pad"] = ["(on a dongle, or in the bootloader)"]  # it still counts as one gamepad
    if not in_bootloader:
        if before["drives"]:
            raise RuntimeError("a board is already in the bootloader")
        via = pad_command(GAMEPAD_CMD_RESET_TO_BOOTLOADER)
        print("gamepad -> bootloader (%s)" % via)
    flash(image, "pad", before, "PID_0003")
    # With a cable in and nothing in boot memory, it comes back wired
    wait_for(lambda s: s["pad"], 20, "the gamepad to come back on its cable")
    if then_dongle:
        time.sleep(1.0)
        pad_command(GAMEPAD_CMD_REBOOT_DONGLE)
        print("gamepad -> dongle link")
        wait_for(lambda s: not s["pad"], 10, "the gamepad to leave USB")
        if not wait_dongle_gamepad(20):
            raise RuntimeError("the gamepad did not connect to the dongle")
        print("gamepad connected through the dongle")


def main():
    global DONGLE_SEL
    a = sys.argv[1:]
    if "--use" in a:
        i = a.index("--use")
        DONGLE_SEL = a[i + 1]
        del a[i:i + 2]
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
            for path, did in update_devices():
                print("update     %s (driver %s)" % (did, _service(did) or "?"))
        elif cmd == "dongle-info":
            for path, info in dongles():
                print(info)
        elif cmd == "dongle-boot":
            path, _ = find_dongle()
            send_path(path, [DONGLE_WEBUSB_BOOTLOADER])
        elif cmd == "dongle-mode":
            path, _ = find_dongle(prefer_gamepad=True)
            if not path:
                raise RuntimeError("no dongle")
            print(_info_from(send_path(path, [DONGLE_WEBUSB_PC_MODE, DONGLE_MODES[a[1]]], read=True,
                                       reply=DONGLE_WEBUSB_INFO)))
        elif cmd == "pad-boot":
            print("via", pad_command(GAMEPAD_CMD_RESET_TO_BOOTLOADER))
        elif cmd in ("pad-dongle", "pad-bt"):
            pair = "--pair" in a
            names = [x for x in a[1:] if x != "--pair"]
            arg = (REBOOT_ARG_MODE_SET | MODES[names[0]]) if names else 0
            arg |= REBOOT_ARG_PAIR if pair else 0
            reboot = GAMEPAD_CMD_REBOOT_DONGLE if cmd == "pad-dongle" else GAMEPAD_CMD_REBOOT_BLUETOOTH
            print("via", pad_command(reboot, arg if arg else None))
        elif cmd == "flash-dongle":
            flash_dongle(a[1])
        elif cmd == "flash-pad":
            flash_pad(a[1], "--dongle" in a[2:], "--in-bootloader" in a[2:])
        elif cmd == "update-flash":
            update_flash(a[1])
        elif cmd == "update-stage":
            update_stage(a[1], "--deflate" in a[2:], "--radio" in a[2:], "--break" in a[2:])
        elif cmd == "update-layout":
            pb = open_update()
            try:
                for k, v in pb.layout().items():
                    print("%-20s 0x%06x" % (k, v))
                print("%-20s %08x %08x" % ("last fault", *pb.fault()))
            finally:
                pb.close()
        elif cmd == "update-read":
            addr = int(a[1], 0)
            pb = open_update()
            try:
                data = pb.read(addr, int(a[2], 0))
            finally:
                pb.close()
            for i in range(0, len(data), 16):
                print("%08x  %s" % (addr + i, data[i:i + 16].hex(" ")))
        elif cmd in ("update-reboot", "update-rom"):
            pb = open_update()
            try:
                pb.reboot(rom=(cmd == "update-rom"))
            finally:
                pb.close()
        else:
            sys.exit(__doc__)
    except RuntimeError as e:
        sys.exit("error: %s" % e)


if __name__ == "__main__":
    main()
