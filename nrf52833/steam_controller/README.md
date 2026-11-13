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
- A wireless mode to start in: Bluetooth LE runs, but only from the bench build below. The chip has
  no Bluetooth Classic, so no first-Switch or Wii mode whatever is added later; Valve's 2.4 GHz
  link is not used
- Stock calibration: the sticks and triggers start uncalibrated (calibrate in the config app)
- Saved settings across an update: the bootloader erases them with the firmware

## To confirm on hardware

- Report rate: about 850 a second rather than 1000. One core does everything here, and the motion
  read alone is about 0.35 ms of I2C.

Checked since: rumble at a full-strength request is fine, and off the cable it flashes white twice
and powers off (it has no wireless mode to stay on for).

## Build

Needs the Arm GNU toolchain and Pico SDK that the other boards build with (the SDK only for its copy
of TinyUSB), and Python with PyYAML. nrfx, CMSIS and NimBLE are cloned into `nrf52833/deps` on the
first configure.

```bash
cmake -S nrf52833 -B nrf52833/build -G Ninja
```

```bash
cmake --build nrf52833/build
```

The result is `nrf52833/steam_controller/build/steam_controller.fw`.

## Bluetooth (bench)

The chip is its own Bluetooth LE radio. The library runs NimBLE's link layer on it with no operating
system, and FLOSS over that as the host stack (`src/hal/nrf52/ble` in the library). The default
build carries none of it.

One mode exists so far, started beside USB by a build flag: a Pro Controller 2 for a Switch 2.

```bash
cmake -S nrf52833 -B nrf52833/build -DSTEAM_CONTROLLER_NS2_BLE_TEST=ON
```

Keep the gamepad on its cable, open Controllers > Change Grip/Order on the console, and it pairs.
Buttons go by their printed labels, whatever profile the USB mode has. Rumble and motion work. The
pairing is kept in RAM: it is lost at power off and with every flash.

The same flag keeps a log in the gamepad (FLOSS's, and every tenth second the main loop's pace and
the link layer's counters), read over the config interface. Read it after a test, not during one:
reading slows the gamepad's main loop.

```bash
python -I scripts/hoja_bench.py pad-log
```

`-DSTEAM_CONTROLLER_BLE_SNOOP=ON` adds every HCI packet to that log.

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
