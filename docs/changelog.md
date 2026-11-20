# Changelog

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
