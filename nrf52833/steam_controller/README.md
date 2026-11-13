# Steam Controller (2026)

HOJA on Valve's second Steam Controller: an nRF52833 behind Valve's own USB bootloader, which stays
in place. USB only for now.

**Running on a controller since 2026-11-12.** It shows up as an SInput gamepad with the config
interface and sends about 850 reports a second. Checked by hand on that controller: every button,
both sticks (direction and full range), both triggers, stick clicks, the motion axes, charger status,
battery level, the status LED's colours, rumble on the correct sides and the app's glyphs.

## What is in

- USB modes: SInput, XInput, Switch, and the others the library offers over USB, with the config app
- Buttons (A B X Y, d-pad, L1 R1, L4 L5 R4 R5, View, Menu, Steam, Quick Access, stick clicks)
- Sticks and analog triggers
- Motion (LSM6DSV16X)
- Rumble through the two grip motors (I2S amplifier)
- Status LED
- Battery level and charger status; Steam held, or the cable pulled, powers off
- Settings and profiles saved in flash

## What is not

- Trackpads, grip sensing and stick touch (all behind the trackpad controller)
- Trackpad haptics
- Any radio: the chip has Bluetooth LE and Valve's 2.4 GHz link but no Bluetooth Classic, so no
  wireless Switch mode whatever is added later
- Stock calibration: the sticks and triggers start uncalibrated (calibrate in the config app)
- Saved settings across an update: the bootloader erases them with the firmware

## To confirm on hardware

Not checked yet; each is marked "To confirm" in `main.c` where it has a setting.

- Rumble at a full-strength request (the amplifier is capped at a quarter of its range, which felt
  right at a low request)
- Powering off: Steam held, and pulling the cable
- Report rate: about 850 a second rather than 1000. One core does everything here, and the motion
  read alone is about 0.35 ms of I2C.

## Build

Needs the Arm GNU toolchain and Pico SDK that the other boards build with (the SDK only for its copy
of TinyUSB). nrfx and CMSIS are cloned into `nrf52833/deps` on the first configure.

```bash
cmake -S nrf52833 -B nrf52833/build -G Ninja
```

```bash
cmake --build nrf52833/build
```

The result is `nrf52833/steam_controller/build/steam_controller.fw`.

## Flash

The bootloader is a USB serial port (28DE:1005). Put the controller in it by holding View, Menu and
A and pressing Steam, then:

```bash
python scripts/steam_controller_fw.py flash nrf52833/steam_controller/build/steam_controller.fw
```

Once HOJA is on it, the bench script restarts it into the bootloader with no buttons:

```bash
python -I scripts/hoja_bench.py pad-boot
```

L1 + Menu held while plugging in should do the same.

## Back to stock

Steam keeps Valve's firmware in the same container, in its `bin/hardwareupdater` folder
(`IBEX_FW_*.fw`). Copy one somewhere safe first, then flash it the same way:

```bash
python scripts/steam_controller_fw.py flash IBEX_FW_xxxxxxxx.fw
```

## If it goes wrong

- The update erases the firmware before it writes. If it stops part way, or the new firmware does
  not start, the way back is the bootloader's own button combination above. It has brought this
  controller back from a firmware that never started.
- Steam also sees a controller in its bootloader: started with `-dev`, Settings, Developer, "Update
  all devices now" writes Valve's firmware.
- Without either, the only way back is a debug probe on the board's SWD pads.
- The bootloader, Valve's settings area (calibration, pairing) and the chip's configuration words
  are never written by this firmware or by the flash script.
