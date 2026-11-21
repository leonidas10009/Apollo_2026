# Changelog

## Unreleased — session and app cleanup

- The default Windows **Steam Big Picture** entry now uses `steam://exit` and
  `terminate-on-pause`, so leaving Big Picture and closing Steam happen when the
  last client disconnects instead of leaving the Steam window open on the desktop.
- Creating an application in the Web UI now assigns a UUID, and editing an app
  without sending its UUID preserves the existing identity.
- Repeated `proc::terminate()` calls are now safe no-ops, and stale prep-command
  iterators are cleared after termination.
- Added regression coverage for the default Windows Steam entry and for app UUID
  generation/preservation.

## Unreleased — ApolloInput Nova 0.3.0

Apollo HID backend update.

- Vendored `libs/apollo-input` **0.3.0** with the native **Apollo HID**
  (UMDF2 + VHF) backend for Windows.
- Apollo HID supports `generic`, `dualshock4`, DS5/Edge, Xbox
  One/Series/Elite, Switch Pro and Steam Controller/Deck. Advanced profiles use
  compatibility HID descriptors; Xbox profiles are HID, not XInput.
- Apollo HID supports one client handle per virtual device, up to 8 simultaneous
  controllers and DS4/DS5 feedback/touchpad/IMU.
- Added safe PnP driver-version probing; the backend refuses driver 0.1.0.5
  before opening a control handle.
- The host does **not** select Apollo HID yet: `gamepad_driver` is unchanged and
  virtual pads continue to use ViGEmBus/libvirtualhid. Integration is tracked in
  `apollo HID/docs/RELEASE-READINESS.md`.
- Tests: 2/2 `apollo-input` tests in the standalone and nested checkouts.

## Unreleased — ApolloInput Nova 0.2.0

Gamepad precision and compatibility update.

- Vendored `libs/apollo-input` **0.2.0** (ApolloInput Nova) with profiles for
  Xbox Elite, DualSense Edge, Steam Controller and Steam Deck, plus trigger
  rumble, player LEDs, battery, paddles, misc buttons and `Button::misc2`.
- Added `recommended_profile()`, `profile_capabilities()`, `profile_identity()`,
  `profile_compatibility_chain()` and `Backend::create_compatible()`.
- Added radial stick deadzone, center snap, trigger deadzone and
  `Device::set_deadzone()`.
- Made the Linux uinput backend profile-aware with profile-specific USB identity
  and extra buttons.
- Added the Windows controller tester; the standalone library also ships a
  browser-based visual tester.
- Apollo host: added `gamepad_stick_deadzone` (0–30 %, default 3) and
  `gamepad_trigger_deadzone` (0–30 %, default 2), radial filtering, resume
  neutralization and `LI_CTYPE_STEAM` detection.
- Web UI: deadzone controls and English locale strings.
- Web UI: application posters keep their aspect ratio with a contained image and
  a blurred backdrop instead of cropping square or landscape art.
- Web UI: Featured Apps is now a local directory with **Artemis 2026** only;
  official Moonlight clients are no longer advertised by Apollo.
- Web UI: the Input tab shows the ApolloInput Nova integration status, the
  active host backend and the precision values, and clarifies that advanced
  profiles require Virtual HID Broker.
- Added controller compatibility and integration documentation.
- Tests: 946 C++ tests (933 pass, 13 skipped, 0 failures), 258 Web UI tests and
  2/2 `apollo-input` tests.

## Apollo 2026 1.0.1 — 2026-11-20

Maintenance release focused on remote power control.

- Added authenticated `POST /servercmd?index=N` on the GameStream HTTPS server to run
  an entry from `server_cmd` without an active stream. Requires a paired client
  certificate and the `server_cmd` permission.
- Refactored `server_cmd` execution into `stream::execute_server_command()` and shared
  it with the control stream.
- Documented the endpoint, security requirements and a power-command example in
  [configuration.md](configuration.md).
- Artemis client: shut down / restart / cancel actions in the PC action sheet, with
  confirmation and no stream required.
- Version metadata: release builds now record the Git commit. Use Git for Windows when
  configuring on Windows to avoid a false dirty tree from the MSYS2 Git submodule check.

## Apollo 2026 1.0.0 — 2026-11-20

First stable Apollo 2026 release, based on Sunshine `2026.1116.25010`.

- Rebased Apollo on the Sunshine 2026 architecture while retaining Apollo features.
- Added the modern Vue 3 Web UI with sidebar shell, dashboard, command palette,
  themes, client management, app ordering, `.art` export and OTP/QR pairing.
- Restored virtual display (SudoVDA), per-client permissions, commands, input-only
  mode, rumble forwarding and VideoToolbox software policy.
- Windows target renamed to `apollo.exe`; installer includes the complete SudoVDA
  package and setup scripts.
- Added the standalone MIT [`libs/apollo-input`](../libs/apollo-input/README.md)
  library with ViGEmBus and uinput backends.
- Removed unregistered legacy Web UI handlers, unused Vue components and dead
  Sunshine icon assets.
- Hardened cover uploads with CSRF validation, fixed missing-UUID handling for
  virtual displays, and made optional update checks non-blocking.
- Dependency cleanup: `npm audit` reports 0 vulnerabilities.
- Tests: 942 C++ tests (931 pass, 11 skipped, 0 failures) and 238 Web UI tests.

### Known limitations

- Virtual HID Broker requires a paid machine license for driver-backed gamepads.
- ViGEmBus is EOL and only provides Xbox 360/DualShock 4 emulation.
- Code signing, clean-machine installer validation and macOS hardware validation
  remain pending.
- See [status.md](status.md) for the full open-work list.

See the [Apollo releases](https://github.com/leonidas10009/Apollo_2026/releases) for binaries and
release notes.
