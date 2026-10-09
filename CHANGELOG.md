<!--
How to add to this changelog (the HHL Gamepad Config app reads this file straight from GitHub):

1. When you publish builds, add a release at the top, under "Board groups":

     ## 2026-10-07
     Boards: gcu_2, gcu_r4k        <- the builds this release updates, or "all"

   An optional title can follow the date:  ## 2026-10-07 - Motion flicks

2. Under the release, add one "### Section" per area that changed. Sections (the app's filter):
     Input, Joysticks, Snapback, Motion, RGB, Haptics, Battery, Wireless, Modes, System

3. One bullet per change, starting with its kind:
     - New: ...      a new feature or setting
     - Fix: ...      something that was broken now works
     - Change: ...   behaves differently now
     - Action: ...   the owner has to do something after updating (shown before they update)

   To limit a bullet to some of the release's boards, put them in brackets after the kind. Use build
   ids or group names from "Board groups":
     - Action: [esp32] Pair again in Switch mode after updating.

   A long bullet can wrap onto lines indented by two spaces. `code` is shown as code.

4. Check it: node <hoja3>/tools/check-changelog.mjs   (also part of the app's tools/test.mjs)
-->

# HOJA firmware changelog

## Board groups
- esp32: gcu_1, gcu_proto, gcu_r4k, progcc_3.1, progcc_3.2, progcc_3p, super_gamepad

## 2026-10-09 - Stronger Flick Down, Auto connects to PCs faster
Boards: esp32, gcu_2, gcu_2s, latte_pro, padbox_gs_c, phob_2, pico_w, progcc_3, progcc_3s

### Motion
- Fix: Flick Down is as strong and quick as Flick Up, just the other way. Before, it felt weaker.
- Change: [esp32, gcu_2, latte_pro, pico_w] The default Wii layouts use Flick Up instead of Flick
  Down: Remote Flick Up on R (and ZR in Sideways), Nunchuk Flick Up on the left stick click. Saved
  layouts are not changed. Reset the Wii layout to defaults to get the new ones.

### Wireless
- Change: [esp32, gcu_2, latte_pro, pico_w] On battery, Auto only checks for a saved Switch or Wii.
  If neither is on, it goes straight to SInput to connect to your PC, so PCs connect sooner.
- Fix: [esp32, gcu_2, latte_pro, pico_w] Several controllers turned on at the same time all connect
  to a PC. Before, one could miss its turn and stay unconnected. A controller that can't connect
  now tries again a few times.

## 2026-10-09 - ESP32 recovery
Boards: esp32

### Wireless
- Fix: Bluetooth recovers by itself if the ESP32 stops responding, also while pairing or
  reconnecting. Before, Bluetooth stayed dead until the controller was turned off, and a console
  could keep showing it as connected.
- Fix: With the latest ESP32 firmware (update it from the app), a console sees the controller
  disconnect if the controller freezes, instead of keeping a controller that does nothing.

## 2026-10-09 - Bluetooth recovery and power button reset
Boards: esp32, gcu_2, gcu_2s, latte_pro, padbox_gs_c, phob_2, pico_w, progcc_3, progcc_3s

### Wireless
- Fix: [esp32] Bluetooth recovers by itself if the ESP32 restarts or its link backs up. Before,
  the controller could look connected but do nothing until it was restarted.
- Fix: [esp32, gcu_2, latte_pro, pico_w] A connected controller no longer shows up when a Switch
  or PC searches for new controllers, like Wii mode already does.
- Change: [esp32, gcu_2, latte_pro, pico_w] In Wii mode the controller only sends what changed
  unless the game asks for continuous updates, like a real Wii Remote.

### System
- New: [esp32, gcu_2, latte_pro] Hold the power button for 15 seconds to force a restart if the
  controller ever freezes, with no need to unplug the battery.

## 2026-10-09 - Wii pairing with several controllers
Boards: esp32, gcu_2, gcu_2s, latte_pro, padbox_gs_c, phob_2, pico_w, progcc_3, progcc_3s

### Wireless
- Fix: [esp32, gcu_2, latte_pro, pico_w] Pairing another controller with a Wii no longer stalls
  while one is already connected. Before, the Wii needed a restart and could freeze.

### System
- Fix: [esp32] Fixed a case where the controller stayed frozen after turning off and needed its
  battery unplugged.

## 2026-10-09 - Auto mode
Boards: esp32, gcu_2, gcu_2s, latte_pro, padbox_gs_c, phob_2, pico_w, progcc_3, progcc_3s

### Modes
- New: Auto mode picks the mode at startup from what the controller is plugged into: a PC, a
  Switch, a GameCube, an N64, an SNES or an NES. It switches without restarting.
- New: [esp32, gcu_2, latte_pro, pico_w] On battery, Auto connects to whichever saved console or
  PC answers first: Switch, then Wii, then PC. If none answers, it waits in Switch mode.
- New: Separate default modes for wired and for battery. Choose them on the Gamepad page of the
  app.
- Change: After updating, a default mode of Switch Pro (the factory setting) becomes Auto, wired
  and on battery. Any other default keeps working as before.
- Change: [gcu_2, latte_pro] On battery, a default of GameCube, XInput or Slippi no longer starts
  WLAN. The battery default is used instead. Use the WLAN button combo at power-on to start WLAN.

### RGB
- Change: In Authentic lighting, face buttons light by their printed letter: Nintendo colors in
  Switch and SNES modes, Xbox colors in SInput and XInput modes.
- Change: Brightness changes fade instead of jumping.
- New: While Auto is choosing a mode, every button shows the Power LED color and the Power LED
  blinks. The mode's lighting fades in once it is chosen.

### Motion
- Fix: [esp32, gcu_2, latte_pro, pico_w] The Wii pointer reaches the whole screen with the Wii's
  sensor bar set to above the TV.

## 2026-10-08 - SInput at 1000 Hz
Boards: esp32, gcu_2, gcu_2s, latte_pro, padbox_gs_c, phob_2, pico_w, progcc_3, progcc_3s

### Input
- Fix: SInput over USB reports at 1000 Hz again. On the GCU 2 it had dropped to about 100 Hz.

## 2026-10-08 - ESP32 version
Boards: esp32

### Wireless
- Fix: The controller reads the ESP32 firmware version correctly. Some read it as 0xFFFF, which
  stopped Bluetooth on the old ESP32 firmware and hid the ESP32 update in the app.

## 2026-10-08 - Switch mode over Bluetooth
Boards: all

### Motion
- Fix: Motion works again in Switch mode over Bluetooth on a Switch console.

### Wireless
- Fix: [esp32] Switch mode over Bluetooth no longer stutters on a PC. It reports at a steady
  125 Hz.

## 2026-10-07 - GCU 1
Boards: all

### Wireless
- New: [gcu_1, gcu_r4k] The GCU R4K is now the GCU 1.
- New: [esp32] The same Bluetooth as the GCU 2, Wii mode included. Update the ESP32 firmware too
  to get it. Until then Bluetooth keeps working as before.
- Fix: [esp32, gcu_2, pico_w, latte_pro, progcc_3s] SInput over Bluetooth on a PC reports at a
  steady 125 Hz.
- Action: [esp32] Pair again after updating the ESP32 firmware: turn the controller on while holding
  Plus (or Start), then pair it from the Switch (Change Grip/Order) or the device's Bluetooth settings.

### Battery
- New: [gcu_1, gcu_r4k] Battery level over Bluetooth.

### System
- Action: [esp32] Settings are reset by this update. Calibrate the sticks again afterwards.

## 2026-10-07 - Motion flicks
Boards: all

### Motion
- New: Flick buttons. Bind Flick Up, Down, Left or Right to any button to play a real wrist flick on
  the motion sensor, for games that read motion (Super Mario Odyssey's cap throws, shake actions).
- New: Turn motion on or off per mode (Switch, SInput, Wii) in the Motion settings.
- Change: Flicks follow the ground: Flick Up moves away from the floor however you hold the controller.
- Change: With motion turned off, the controller reports lying flat on a table instead of no
  reading, so games keep a sensible orientation and flicks still work.
- New: [gcu_2] Wii mode has Remote and Nunchuk flicks in all four directions.

### Input
- New: Flick Up, Down, Left and Right outputs in the Switch remap picker.
- New: [gcu_2] Remote and Nunchuk flick outputs in the Wii remap pickers. Default layouts flick the
  Remote on R and the Nunchuk on the left stick click.

### Wireless
- Fix: [esp32] Switch over Bluetooth no longer clashes with a Switch the controller was plugged into
  over USB.
- Action: [esp32] Pair again in Switch mode after updating: on the Switch, open Change Grip/Order,
  then turn the controller on while holding Plus (or Start). You only need to do this once.

### System
- Fix: The firmware ignores button bindings it doesn't recognise, so settings saved by a newer app
  can't cause trouble.
