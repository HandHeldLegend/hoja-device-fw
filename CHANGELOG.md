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
