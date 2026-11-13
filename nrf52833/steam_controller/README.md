# Steam Controller (2026)

HOJA on Valve's second Steam Controller: an nRF52833 behind Valve's own USB bootloader, which stays
in place. USB on the cable; off it, a Pro Controller 2 for a Switch 2 over Bluetooth LE.

**Running on a controller since 2026-11-12.** It shows up as an SInput gamepad with the config
interface and sends about 850 reports a second. Checked by hand on that controller: every button,
both sticks (direction and full range), both triggers, stick clicks, the motion axes, charger status,
battery level, the status LED's colours, rumble on the correct sides and the app's glyphs.

## What is in

- USB modes: SInput, XInput, Switch, and the others the library offers over USB, with the config app
- Off the cable: Switch mode as a Switch 2's Pro Controller 2, over Bluetooth LE (below)
- Buttons (A B X Y, d-pad, L1 R1, L4 L5 R4 R5, View, Menu, Steam, Quick Access, stick clicks)
- Trackpads: where a finger is, how hard it presses, and a click with its haptic pulse (below)
- Sticks and analog triggers
- Motion (LSM6DSV16X)
- Rumble through the two grip motors (I2S amplifier)
- Status LED
- Battery level and charger status; Steam held, or the cable pulled, powers off (Steam pressed
  starts it again)
- Settings and profiles saved in flash

## What is not

- Grip sensing and stick touch: the trackpad controller reads them, nothing uses them yet
- Trackpad haptics beyond a pad's own click
- The trackpads in any mode but SInput: only their clicks can be mapped elsewhere
- Any wireless mode but the Switch 2's. The chip has no Bluetooth Classic, so no first-Switch or Wii
  mode whatever is added later; Valve's 2.4 GHz link is not used
- The config app off the cable
- A power-off of its own when no console answers a start: it waits until Steam is held or the
  battery is low
- Stock calibration: the sticks and triggers start uncalibrated (calibrate in the config app)
- Saved settings across an update: the bootloader erases them with the firmware

## To confirm on hardware

- Report rate: about 850 a second rather than 1000. One core does everything here, and the motion
  read alone is about 0.35 ms of I2C.

Checked since: rumble at a full-strength request is fine.

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

## Trackpads

Both pads, the grips and the sticks' touch sensing are read by one Cirque touch controller on the
buttons' I2C bus (`trackpad.c`). It keeps no set-up over a reset, so the firmware gives it the stock
one at every start (86 commands, spread over the first quarter second), and it then sends a frame
250 times a second while something is touched or held. A frame is raw electrode readings: the
position, the touch and the click are worked out in the firmware.

- SInput reports them as two touchpads, each with a position, a pressure and a click.
- A pad has no switch. Its click is the pressure passing a threshold, and the pulse felt under the
  thumb is played by the firmware through the pad's actuator (`trackpad_haptics.c`), on the push
  and, a little lighter, on letting go.
- The clicks are inputs like any button ("L Pad", "R Pad"), mapped in the SInput profile by
  default and free to map in the others.
- Frames are read in the background: SInput stays at about 850 reports a second with the pads in
  use.

Checked on a controller (2026-11-13), from the reports on a PC and by feel: both pads track from
edge to edge the right way round, the click force and the two pulses. SDL opens the gamepad as
the Steam type with two touchpads, their clicks as `touchpad` and `misc2`, and both pads and both
clicks work in its gamepad test (`testcontroller`, SDL main of 2026-11-12). The Steam client of
2026-11-11 showed no trackpads for it. Not checked: the pads while in the Switch 2 mode. The click
threshold is one number for both pads, not each pad's factory range as on the stock firmware.

## Switch 2 (Bluetooth)

The chip is its own Bluetooth LE radio. The library runs NimBLE's link layer on it with no operating
system, and FLOSS over that as the host stack (`src/hal/nrf52/ble` in the library). It has the 2M
PHY a Switch 2 asks of its controllers, and no Bluetooth Classic for a first Switch, so on this build
Switch mode over Bluetooth is a Switch 2's Pro Controller 2 (`HOJA_NS2_BLE`, on by default).

- Off the cable the gamepad starts in it, whatever mode is picked or saved: it is the one mode the
  radio carries. On a charger (power, no computer) it goes there after three seconds.
- Buttons and sticks are the Switch profile's, as in Switch mode over USB. The Pro Controller 2's GL
  and GR have no place in that profile yet: they are L4 and R4 as wired.
- Pairing: open Controllers > Change Grip/Order on the console. With no console saved the gamepad is
  found by any; with one saved it calls that one. Menu held while it starts makes it findable again.
- The console and its key are saved as the gamepad's Switch (the config app's saved Switch, and its
  forget). An update erases them with the other settings.
- Rumble and motion work. Buttons and motion are read 800 times a second, the rate of the motion
  data's own clock.
- The console shows the gamepad in its own colours (the config app's body, buttons and grips).
- Waking: started off the cable with Steam, the gamepad wakes its console, as a retail
  controller's Home does. Started on a cable (a charger) it waits to be taken and leaves a sleeping
  console asleep, until Home is pressed.
- Sleeping: a Switch 2 has no command that turns a controller off. Going to sleep it ends the link
  itself, and the gamepad powers off when it does. The same is expected when the console clears its
  controllers (Change Grip/Order). A link that is only lost is offered back, without waking the
  console.

Checked on a controller and a Switch 2 (2026-11-12): pairing, buttons, triggers, sticks and motion;
a start off the cable that went back to the console by itself, with the pairing kept over the
power-off; Menu held at start to pair again; the console put to sleep, which stayed asleep with the
gamepad off at once, and woke when the gamepad was started. Not checked: rumble since the mode
stopped being a bench build, the colours on the console, Home waking the console from a gamepad on
a cable, and Change Grip/Order with the gamepad connected.

Until the triggers are calibrated in the config app they learn their range at every start: pull
each one fully once.

`-DHOJA_NS2_BLE=OFF` builds without the mode: USB only, and off the cable the gamepad powers off.

### Bench

A build flag also starts the link beside the USB modes, where a log can be read: FLOSS's, and every
tenth second the main loop's pace and the link layer's counters. Start in Switch mode over USB for the
buttons to be right (the link reads that mode's profile).

```bash
cmake -S nrf52833 -B nrf52833/build -DSTEAM_CONTROLLER_NS2_BLE_TEST=ON
```

Read the log after a test, not during one: reading slows the gamepad's main loop.

```bash
python -I scripts/hoja_bench.py pad-log
```

`-DSTEAM_CONTROLLER_BLE_SNOOP=ON` adds every HCI packet to that log.

On the cable, the bench script restarts a gamepad into its Bluetooth mode with no buttons (it comes
back to USB when plugged in again):

```bash
python -I scripts/hoja_bench.py pad-bt
```

The Bluetooth mode itself has no USB to read a log over. The log is kept in RAM a restart leaves
alone (4 KB), and `-DSTEAM_CONTROLLER_BLE_RETURN_S=120` restarts the gamepad out of the Bluetooth
mode two seconds after its console lets go of it, or after that many seconds with no console: on
the cable it comes back on USB, and `pad-log` reads what the mode logged. A start that finds a log
waiting leaves the link beside USB off, so the log is not buried.

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

## Restarts and Valve's bootloader

Every restart passes through the bootloader, which uses the bottom of RAM: a note left there for
the firmware that follows (which mode to come back in) did not survive. The firmware's RAM ends
8 KB early (`steam_controller.ld`), and the note is kept at that end, where it does.

## Settings over an update

The bootloader erases everything it can write before an update, the firmware's saved settings
with it. So the firmware copies them into spare RAM as it hands over to the bootloader (18 KB
under its stack, `__record_carry` in `steam_controller.ld`), and the firmware that starts after
the update, finding nothing saved, writes them back. The gamepad's identity rides along, so the
PC sees the same device afterwards.

Checked on a controller (2026-11-13): a name, a stick calibration, a profile's name and the
identity all came back the same after an update.

It only works for an update started from the firmware (`hoja_bench.py pad-boot`, or the app).
The bootloader's own button combination, an update from firmware older than this, and a
controller that loses power part way all start with settings cleared.

## If it goes wrong

- The update erases the firmware before it writes. If it stops part way, or the new firmware does
  not start, the way back is the bootloader's own button combination above. It has brought this
  controller back from a firmware that never started.
- Steam also sees a controller in its bootloader: started with `-dev`, Settings, Developer, "Update
  all devices now" writes Valve's firmware.
- Without either, the only way back is a debug probe on the board's SWD pads.
- The bootloader, Valve's settings area (calibration, pairing) and the chip's configuration words
  are never written by this firmware or by the flash script.
