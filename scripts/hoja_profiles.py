"""hoja_profiles.py: settings profiles over WebUSB (HOJA-LIB-RP2040 utilities/profiles.h), for the bench.

    python -I scripts/hoja_profiles.py list
    python -I scripts/hoja_profiles.py switch <n>          # 0..3
    python -I scripts/hoja_profiles.py rename <n> <name>
    python -I scripts/hoja_profiles.py copy <from> <to>
    python -I scripts/hoja_profiles.py block <block>       # a config block's bytes (hex), e.g. 3 for RGB
    python -I scripts/hoja_profiles.py selftest            # rename, switch, edit, switch back, check apart

The gamepad on its own cable (hoja_bench.find_webusb('pad')); the config app must not hold it.
"""
import sys
import time

sys.path.insert(0, __file__.rsplit("\\", 1)[0].rsplit("/", 1)[0])
import hoja_bench as hb  # noqa: E402

WEBUSB_ID_READ_CONFIG_BLOCK = 0x01
WEBUSB_ID_WRITE_CONFIG_BLOCK = 0x02
WEBUSB_ID_CONFIG_COMMAND = 0x04
WEBUSB_ID_PROFILE = 0x07
GAMEPAD_CMD_SAVE_ALL = 0xFF
CFG_BLOCK_RGB = 3
OP_INFO, OP_SWITCH, OP_RENAME, OP_COPY = 0, 1, 2, 3
CHUNK = 32


class Pad:
    def __init__(self):
        path = hb.find_webusb("pad")
        if not path:
            raise RuntimeError("no gamepad on a cable")
        self.d = hb.WebUsb(path)

    def close(self):
        self.d.close()

    def ask(self, packet, reply_id, tries=20):
        self.d.write(packet)
        for _ in range(tries):
            r = self.d.read()
            if r and r[0] == reply_id:
                return r
        return None

    def profile(self, op, n=0, extra=b""):
        r = self.ask(bytes([WEBUSB_ID_PROFILE, op, n]) + extra, WEBUSB_ID_PROFILE)
        if not r:
            raise RuntimeError("no profile answer (firmware without profiles?)")
        return {"done": bool(r[2]), "count": r[3], "active": r[4], "profile": r[5],
                "name": bytes(r[6:30]).split(b"\0")[0].decode(errors="replace")}

    def read_block(self, block):
        self.d.write([WEBUSB_ID_READ_CONFIG_BLOCK, block])
        data = b""
        for _ in range(200):
            r = self.d.read()
            if not r or r[0] != WEBUSB_ID_READ_CONFIG_BLOCK or r[1] != block:
                continue
            if r[3] == 0xFF:
                return data
            data += bytes(r[4:4 + r[2]])
        raise RuntimeError("block %d didn't arrive whole" % block)

    def write_block(self, block, data):
        for part in range(0, len(data), CHUNK):
            chunk = data[part:part + CHUNK]
            self.d.write(bytes([WEBUSB_ID_WRITE_CONFIG_BLOCK, block, len(chunk), part // CHUNK]) + chunk)
        self.d.write([WEBUSB_ID_WRITE_CONFIG_BLOCK, block, 0, 0xFF])

    def save(self):
        self.ask(bytes([WEBUSB_ID_CONFIG_COMMAND, 0, GAMEPAD_CMD_SAVE_ALL]), WEBUSB_ID_CONFIG_COMMAND)


def switch(pad, n, tries=20):
    """Switch profiles, waiting while a record is still being written. The last answer."""
    for _ in range(tries):
        r = pad.profile(OP_SWITCH, n)
        if r["done"]:
            break
        time.sleep(0.1)
    return r


def show(pad):
    for n in range(4):
        p = pad.profile(OP_INFO, n)
        if not p["count"]:
            print("no profiles on this firmware")
            return
        print("%s %d: %s" % ("*" if p["active"] == n else " ", n, p["name"] or "(unnamed)"))


def selftest(pad):
    start = pad.profile(OP_INFO, 0)
    assert start["count"] == 4, start
    print("profiles: 4, active %d" % start["active"])
    if start["active"] != 0:
        assert pad.profile(OP_SWITCH, 0)["done"]

    rgb0 = pad.read_block(CFG_BLOCK_RGB)
    assert pad.profile(OP_RENAME, 1, b"\0" + b"Bench B")["done"]
    r = switch(pad, 1)
    assert r["done"] and r["active"] == 1, r
    print("switched to 1:", pad.profile(OP_INFO, 1)["name"])

    # In profile 1: another RGB mode byte and brightness, saved
    rgb1 = bytearray(pad.read_block(CFG_BLOCK_RGB))
    assert rgb1 == rgb0, "a new profile starts as a copy"
    rgb1[1] = (rgb1[1] + 1) % 4
    rgb1[132] = 0x00
    rgb1[133] = 0x02  # brightness 512 of 4096 (rgb_brightness at offset 132)
    pad.write_block(CFG_BLOCK_RGB, bytes(rgb1))
    pad.save()
    time.sleep(0.5)
    assert pad.read_block(CFG_BLOCK_RGB) == bytes(rgb1)

    r = switch(pad, 0)
    assert r["done"], r
    back = pad.read_block(CFG_BLOCK_RGB)
    assert back == rgb0, "profile 0 kept its own RGB"
    print("profile 0 has its own RGB again")

    switch(pad, 1)
    assert pad.read_block(CFG_BLOCK_RGB) == bytes(rgb1), "profile 1 kept its edit"
    print("profile 1 kept its edit")
    return bytes(rgb1)


def main():
    a = sys.argv[1:]
    if not a:
        sys.exit(__doc__)
    pad = Pad()
    try:
        if a[0] == "list":
            show(pad)
        elif a[0] == "switch":
            print(switch(pad, int(a[1])))
        elif a[0] == "rename":
            print(pad.profile(OP_RENAME, int(a[1]), b"\0" + a[2].encode()[:24]))
        elif a[0] == "copy":
            print(pad.profile(OP_COPY, int(a[2]), bytes([int(a[1])])))
        elif a[0] == "block":
            print(pad.read_block(int(a[1])).hex(" "))
        elif a[0] == "selftest":
            selftest(pad)
            show(pad)
        else:
            sys.exit(__doc__)
    finally:
        pad.close()


if __name__ == "__main__":
    main()
