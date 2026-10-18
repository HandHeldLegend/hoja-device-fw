"""hoja_ble_update.py <board image.bin> [--deflate] [--radio]: send a firmware update over the BLE config link,
the way the config app does (HOJA-LIB-RP2040 ble_config.c, boot_update.h). Press + and - on the gamepad, and
enter the code the first time. Needs bleak. A run that stops part way resumes when run again.

    python scripts/hoja_ble_update.py builds/gcu_2/gcu_2.bin
"""
import asyncio
import os
import struct
import sys
import time
import zlib

from bleak import BleakClient, BleakScanner

SVC = "b4c1f0a0-3e6d-4c5a-9f2e-6d1a7c8b9e10"
RX = "b4c1f0a0-3e6d-4c5a-9f2e-6d1a7c8b9e11"
TX = "b4c1f0a0-3e6d-4c5a-9f2e-6d1a7c8b9e12"
TOKEN_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ble_cfg_token.bin")  # This browser's token
ARROWS = {"U": "Up", "D": "Down", "L": "Left", "R": "Right"}
STATUS = ["ok", "unsupported", "bad request", "no room", "battery low", "out of order", "failed", "restarting"]
SECTOR = 4096
RADIO_HEADER_LEN = 72  # boot_radio_header_s

args = [a for a in sys.argv[1:] if not a.startswith("--")]
DEFLATE = "--deflate" in sys.argv
FORCE_RADIO = "--radio" in sys.argv

frames = asyncio.Queue()
acked = {"written": 0, "status": 0}
acked_event = asyncio.Event()


def on_notify(_, data: bytearray):
    t = data[0]
    if t in (0x86, 0x85, 0x87):
        return
    if t == 0x89:
        acked["status"] = data[1]
        acked["written"] = int.from_bytes(data[2:6], "little")
        acked_event.set()
    frames.put_nowait(bytes(data))


async def next_frame(kind, timeout=10):
    end = time.time() + timeout
    while True:
        f = await asyncio.wait_for(frames.get(), max(0.1, end - time.time()))
        if f[0] == kind:
            return f


def deflate(data):
    c = zlib.compressobj(9, zlib.DEFLATED, -15)
    return c.compress(data) + c.flush()


async def connect_allowed():
    print("searching (press + and - on the gamepad)...", flush=True)
    dev = await BleakScanner.find_device_by_filter(lambda d, ad: SVC in [s.lower() for s in ad.service_uuids],
                                                   timeout=600)
    if not dev:
        sys.exit("no gamepad advertising config")
    print("found %s" % dev.name, flush=True)
    client = BleakClient(dev, winrt={"use_cached_services": False})
    await client.connect()
    await client.start_notify(TX, on_notify)
    if os.path.exists(TOKEN_FILE):
        await client.write_gatt_char(RX, b"\x01" + open(TOKEN_FILE, "rb").read(), response=True)
        f = await next_frame(0x81)
        if f[1] == 0:
            print("allowed (token)", flush=True)
            return client
    await client.write_gatt_char(RX, b"\x02", response=True)
    while True:
        f = await asyncio.wait_for(frames.get(), 60)
        if f[0] == 0x82:
            print("CODE: " + "  ".join(ARROWS.get(ch, ch) for ch in f[1:7].decode()), flush=True)
        elif f[0] == 0x84:
            open(TOKEN_FILE, "wb").write(f[1:17])
            print("paired, token saved", flush=True)
            return client
        elif f[0] == 0x83 and f[2] == 2:
            sys.exit("code timed out")


async def main():
    image = open(args[0], "rb").read()
    client = await connect_allowed()
    print("mtu", client.mtu_size, flush=True)

    await client.write_gatt_char(RX, b"\x06", response=True)
    f = await next_frame(0x88)
    ver, flags, battery = f[1], f[2], f[3]
    app_off, radio_off, radio_size, header_off, running_end, radio_crc = struct.unpack_from("<6I", f, 5)
    window, data_max = struct.unpack_from("<HH", f, 29)
    print("info: supported %d battery ok %d (%d%%) app 0x%x radio 0x%x+0x%x header 0x%x running end 0x%x "
          "window %d data %d" % (flags & 1, (flags >> 1) & 1, battery, app_off, radio_off, radio_size, header_off,
                                 running_end, window, data_max), flush=True)

    # The parts: the radio region when the image's differs from the gamepad's, then the firmware
    parts = []
    radio_image = image[radio_off:app_off].rstrip(b"\xff")
    if radio_size and (FORCE_RADIO or zlib.crc32(image[radio_off:radio_off + RADIO_HEADER_LEN]) != radio_crc):
        parts.append((1, radio_image))
    parts.append((0, image[app_off:]))
    stream = b""
    begin = bytes([len(parts)])
    for dest, img in parts:
        enc = 1 if (DEFLATE or len(parts) > 1) else 0
        staged = deflate(img) if enc else img
        begin += struct.pack("<BBIIII", dest, enc, len(staged), zlib.crc32(staged), len(img), zlib.crc32(img))
        stream += staged + b"\xff" * ((-len(staged)) % SECTOR)
        print("part: %s %d bytes as %d (%s)" % ("radio" if dest else "firmware", len(img), len(staged),
                                                "deflate" if enc else "stored"), flush=True)

    await client.write_gatt_char(RX, b"\x07" + begin, response=True)
    f = await next_frame(0x89)
    if f[1] != 0:
        sys.exit("begin refused: %s" % STATUS[f[1]])
    # The same update begun again carries on from what the gamepad has written
    acked["written"] = int.from_bytes(f[2:6], "little")
    if acked["written"]:
        print("resuming at %d KB" % (acked["written"] // 1024), flush=True)

    t0 = time.time()
    start = acked["written"]
    sent = acked["written"]
    last_print = 0
    last_ack = time.time()
    seen = acked["written"]
    while acked["written"] < len(stream):
        if acked["written"] != seen:
            seen, last_ack = acked["written"], time.time()
        if time.time() - last_ack > 10:
            sys.exit("no progress for 10 s at %d KB: run again to resume" % (acked["written"] // 1024))
        if acked["status"] == 5:  # out of order: resend from what is written
            print("resend from %d" % acked["written"], flush=True)
            sent = acked["written"]
            acked["status"] = 0
        if sent < len(stream) and sent - acked["written"] + data_max <= window:
            n = min(data_max, len(stream) - sent)
            await client.write_gatt_char(RX, b"\x08" + struct.pack("<I", sent) + stream[sent:sent + n],
                                         response=False)
            sent += n
            continue
        acked_event.clear()
        try:
            await asyncio.wait_for(acked_event.wait(), 2)
        except asyncio.TimeoutError:
            print("no acknowledgement for 2 s at %d (sent %d): resending" % (acked["written"], sent), flush=True)
            sent = acked["written"]
        if time.time() - last_print > 2:
            last_print = time.time()
            rate = (acked["written"] - start) / max(0.001, time.time() - t0) / 1024
            print("  %d / %d KB  (%.1f KB/s)" % (acked["written"] // 1024, len(stream) // 1024, rate), flush=True)
    dt = time.time() - t0
    print("sent %d KB in %.1f s (%.1f KB/s)" % ((len(stream) - start) // 1024, dt, (len(stream) - start) / dt / 1024), flush=True)

    await client.write_gatt_char(RX, b"\x09", response=True)
    # Repeated acknowledgements (status ok) may come first: wait for the answer to FINISH
    while True:
        f = await next_frame(0x89, timeout=20)
        if f[1] != 0:
            break
    print("finish: %s" % STATUS[f[1]], flush=True)
    t_restart = time.time()
    try:
        await client.disconnect()
    except Exception:
        pass

    if f[1] == 7:
        print("waiting for the gamepad to come back (its beacon opens by itself)...", flush=True)
        dev = await BleakScanner.find_device_by_filter(lambda d, ad: SVC in [s.lower() for s in ad.service_uuids],
                                                       timeout=60)
        print("beacon back after %.1f s: %s" % (time.time() - t_restart, dev.name if dev else None), flush=True)


asyncio.run(main())
