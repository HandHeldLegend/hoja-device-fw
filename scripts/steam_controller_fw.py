"""Firmware images for the Steam Controller (2026), and sending one through Valve's bootloader.

  pack  firmware.bin firmware.fw   wrap a raw image (linked at 0x8000) in the bootloader's container
  info  firmware.fw                show a container's header, checked
  flash firmware.fw [--port COMx]  send a container to a controller in its bootloader
  probe [--port COMx]              ask a controller's bootloader who it is; writes nothing

The container is Valve's own (the IBEX_FW_*.fw files in Steam's bin/hardwareupdater folder are the
stock firmware in it): a 32-byte header, then the image as it sits in flash from 0x8000. `flash`
takes either, so the stock firmware goes back on the same way.

The bootloader is a USB serial port (28DE:1005). The controller is in it when:
  - View, Menu and A are held and Steam is pressed (works whatever firmware is installed), or
  - the firmware restarted into it (HOJA: the config app's update button, or L1 + Menu held while
    plugging in).

`flash` needs pyserial (pip install pyserial).
"""

import argparse
import struct
import sys
import time
import zlib

MAGIC_CONTROLLER = 0xD2D86467
HEADER_LEN = 32

APP_ADDRESS = 0x8000
# Up to the bootloader's copy of the header in the page at 0x7C000
APP_MAX_LEN = 0x7CFE0 - APP_ADDRESS
RAM_START = 0x20000000
RAM_END = 0x20020000

BOOTLOADER_VID = 0x28DE
BOOTLOADER_PID = 0x1005

FRAME_START = 0xAD
FRAME_END = 0xAE
FRAME_ESCAPE = 0xAC

MSG_INFO = 0x1233
MSG_FW_BEGIN = 0x1234
MSG_FW_DATA = 0x1235
MSG_FW_END = 0x1236
MSG_RESET = 0x1237

INFO_MAGIC = 0xAC32A429
# A data frame decodes to 0x8000 bytes: the message id, the chunk's length, the chunk
CHUNK_LEN = 0x7FFC
# The erase at FW_BEGIN takes about ten seconds
REPLY_TIMEOUT_S = 60


class FirmwareError(Exception):
    pass


def check_image(image):
    """Raises unless image looks like firmware for the application slot."""
    if len(image) < 8 or len(image) % 4:
        raise FirmwareError("the image is not a whole number of words")
    if len(image) > APP_MAX_LEN:
        raise FirmwareError(f"the image is {len(image)} bytes, over the slot's {APP_MAX_LEN}")
    stack, reset = struct.unpack_from("<II", image, 0)
    if not RAM_START < stack <= RAM_END:
        raise FirmwareError(f"the stack pointer {stack:#010x} is not in RAM")
    if not reset & 1 or not APP_ADDRESS <= (reset & ~1) < APP_ADDRESS + len(image):
        raise FirmwareError(f"the reset vector {reset:#010x} is not inside the image at {APP_ADDRESS:#x}")


def pack(image):
    image = bytes(image) + b"\xff" * (-len(image) % 4)
    check_image(image)
    header = struct.pack("<III", MAGIC_CONTROLLER, len(image), zlib.crc32(image)) + bytes(HEADER_LEN - 12)
    return header + image


def unpack(container):
    """The header and image of a container, checked."""
    if len(container) < HEADER_LEN:
        raise FirmwareError("too short for a header")
    magic, length, crc = struct.unpack_from("<III", container, 0)
    if magic != MAGIC_CONTROLLER:
        raise FirmwareError(f"magic {magic:#010x} is not the controller's ({MAGIC_CONTROLLER:#010x})")
    if any(container[12:HEADER_LEN]):
        raise FirmwareError("the header's reserved words are not zero")
    image = container[HEADER_LEN:]
    if length != len(image):
        raise FirmwareError(f"the header says {length} bytes, the file holds {len(image)}")
    if crc != zlib.crc32(image):
        raise FirmwareError("the checksum does not match")
    check_image(image)
    return container[:HEADER_LEN], image


def encode_frame(message, body=b""):
    out = bytearray([FRAME_START])
    for byte in struct.pack("<H", message) + body:
        if byte in (FRAME_ESCAPE, FRAME_START, FRAME_END):
            out += bytes([FRAME_ESCAPE, byte - FRAME_ESCAPE])
        else:
            out.append(byte)
    out.append(FRAME_END)
    return bytes(out)


def read_frame(port):
    """One reply frame, unescaped. Raises on a timeout or a malformed frame."""
    deadline = time.monotonic() + REPLY_TIMEOUT_S
    frame = None
    escaped = False
    while time.monotonic() < deadline:
        data = port.read(1)
        if not data:
            continue
        byte = data[0]
        if frame is None:
            if byte == FRAME_START:
                frame = bytearray()
            continue
        if escaped:
            if byte > 2:
                raise FirmwareError("a malformed reply (bad escape)")
            frame.append(FRAME_ESCAPE + byte)
            escaped = False
        elif byte == FRAME_ESCAPE:
            escaped = True
        elif byte == FRAME_END:
            return bytes(frame)
        elif byte == FRAME_START:
            raise FirmwareError("a malformed reply (a frame inside a frame)")
        else:
            frame.append(byte)
    raise FirmwareError("no reply from the bootloader")


def command(port, message, body=b"", what="", bare=False):
    """Sends a message; its reply's data. Anything but an acknowledgement stops the update.

    bare: the reply must be the acknowledgement and nothing else, and nothing may be waiting from
    before. The write steps use it: a stray frame taken for the erase's acknowledgement would let
    data go out before the bootloader has set where it writes.
    """
    if bare:
        port.reset_input_buffer()
    port.write(encode_frame(message, body))
    port.flush()
    reply = read_frame(port)
    if not reply or reply[0] != 0 or (bare and len(reply) != 1):
        status = reply[0] if reply else None
        raise FirmwareError(f"the bootloader refused {what or hex(message)} (status {status})")
    return reply[1:]


def find_port():
    from serial.tools import list_ports

    ports = [p.device for p in list_ports.comports() if (p.vid, p.pid) == (BOOTLOADER_VID, BOOTLOADER_PID)]
    if not ports:
        raise FirmwareError("no controller in its bootloader (28DE:1005). Hold View, Menu and A, press Steam.")
    if len(ports) > 1:
        raise FirmwareError(f"more than one bootloader found ({', '.join(ports)}): pick one with --port")
    return ports[0]


def probe(port_name=None):
    """Asks the bootloader for its info block: a read, nothing on the controller changes."""
    try:
        import serial
    except ImportError:
        raise FirmwareError("pyserial is not installed (pip install pyserial)")

    port_name = port_name or find_port()
    with serial.Serial(port_name, 115200, timeout=0.5) as port:
        port.reset_input_buffer()
        info = command(port, MSG_INFO, what="the info request")

    at = info.find(struct.pack("<I", INFO_MAGIC))
    print(f"Bootloader on {port_name}: {len(info)} bytes of info")
    if at < 0:
        raise FirmwareError("the reply does not carry the controller bootloader's mark")
    print(f"  its mark is at byte {at}")
    if len(info) >= at + 8:
        print(f"  hardware id {struct.unpack_from('<I', info, at + 4)[0]:#010x}")


def flash(container, port_name=None):
    try:
        import serial
    except ImportError:
        raise FirmwareError("pyserial is not installed (pip install pyserial)")

    header, image = unpack(container)
    port_name = port_name or find_port()

    with serial.Serial(port_name, 115200, timeout=0.5) as port:
        port.reset_input_buffer()

        info = command(port, MSG_INFO, what="the info request")
        if struct.pack("<I", INFO_MAGIC) not in info:
            raise FirmwareError("the device on this port did not answer as the controller's bootloader")
        print(f"Bootloader on {port_name}")

        # From here the old firmware is gone. Data is only ever sent after the erase was acknowledged:
        # the bootloader takes its write position from it.
        print("Erasing (about 10 s)...")
        command(port, MSG_FW_BEGIN, what="the erase", bare=True)

        for at in range(0, len(image), CHUNK_LEN):
            chunk = image[at:at + CHUNK_LEN]
            command(port, MSG_FW_DATA, struct.pack("<H", len(chunk)) + chunk, what=f"the data at {at:#x}", bare=True)
            print(f"  {min(at + len(chunk), len(image))} / {len(image)} bytes")

        command(port, MSG_FW_END, header, what="the header (the image's checksum)", bare=True)
        print("Written and checked. Restarting.")

        # The controller leaves the bus as it restarts: no reply is no failure
        try:
            port.write(encode_frame(MSG_RESET))
            port.flush()
        except Exception:
            pass


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="action", required=True)

    p = sub.add_parser("pack")
    p.add_argument("image")
    p.add_argument("container")

    p = sub.add_parser("info")
    p.add_argument("container")

    p = sub.add_parser("flash")
    p.add_argument("container")
    p.add_argument("--port")

    p = sub.add_parser("probe")
    p.add_argument("--port")

    args = parser.parse_args()
    try:
        if args.action == "pack":
            with open(args.image, "rb") as f:
                container = pack(f.read())
            with open(args.container, "wb") as f:
                f.write(container)
            print(f"{args.container}: {len(container) - HEADER_LEN} bytes of firmware")
        elif args.action == "info":
            with open(args.container, "rb") as f:
                header, image = unpack(f.read())
            stack, reset = struct.unpack_from("<II", image, 0)
            print(f"length {len(image)}  crc32 {zlib.crc32(image):#010x}  stack {stack:#010x}  reset {reset:#010x}")
        elif args.action == "probe":
            probe(args.port)
        else:
            with open(args.container, "rb") as f:
                flash(f.read(), args.port)
    except FirmwareError as error:
        sys.exit(f"error: {error}")


if __name__ == "__main__":
    main()
