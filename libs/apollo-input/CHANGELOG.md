# Changelog

## 0.3.0 — 2026-11-21

- Added the Apollo HID backend (`BackendKind::apollo_hid`) for the UMDF2 + VHF
  driver.
- Added a non-invasive `DEVPKEY_Device_DriverVersion` check so probing driver
  0.1.0.5 cannot trigger its global controller cleanup.
- Each Apollo HID device owns a dedicated client handle, so multiple controllers
  are isolated per file object.
- Added DS4 touchpad, IMU, rumble and lightbar passthrough through Apollo HID.
- Added compatibility HID profiles for DualSense/Edge, Xbox One/Series/Elite,
  Switch Pro and Steam Controller/Deck.
- Added player LED, trigger rumble and adaptive-trigger feedback fields.
- Added `apollo_hid`, `apollohid` and `apollo-hid` backend aliases.
- Embedded the Apollo HID client/core sources so the installed archive is
  self-contained.
- Added the `APOLLO_INPUT_WITH_APOLLO_HID` and `APOLLO_INPUT_APOLLO_HID_DIR`
  CMake options.
- Added `--backend <name>` to the controller tester `selftest` and `bridge`
  commands.
- The automatic Windows backend still prefers ViGEmBus when it is built; Apollo
  HID is selected explicitly or when ViGEmBus is absent.

## 0.2.0 — 2026-11-21

- Added `xbox_elite`, `dualsense_edge`, `steam_controller` and `steam_deck`
  profiles.
- Added trigger rumble, player LEDs, battery, paddles, misc buttons and
  `Button::misc2`.
- Added `profile_capabilities()`, `profile_identity()`, `recommended_profile()`
  and `profile_compatibility_chain()`.
- Added `Backend::create_compatible()`.
- Added case-insensitive profile, family and backend aliases.
- Added radial stick deadzone, center snap, trigger deadzone and
  `Device::set_deadzone()`.
- Made the Linux uinput backend profile-aware with correct USB identity and
  extra evdev buttons.
- Added the Windows controller tester.
- Added the visual browser-based tester.
- Added the Apollo streaming precision/anti-drift patch.
- Added controller compatibility studies and guides.
- Added Doxygen API documentation and a GitHub Pages deployment workflow.

## 0.1.0

- Initial standalone library with ViGEmBus, uinput and null backends.
