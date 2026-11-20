# Apollo 2026 — current status

> Last updated: **2026-11-20**.
> This document describes the current application state, what was verified, and what
> remains open. It is not a changelog; see [changelog.md](changelog.md) for releases.

## Release

| Item | Value |
|---|---|
| Product | Apollo 2026 |
| Version | **1.0.0** |
| Upstream base | Sunshine `2026.1116.25010` (`5716a2ac`) |
| Git tag | **`v1.0.0`** at `a4c0d637` |
| Windows executable | `apollo.exe` (internal CMake target `sunshine`) |
| Publisher metadata | Apollo 2026 — https://github.com/leonidas10009/Apollo_2026 |
| License | GPL-3.0-only |

### Artifacts

| Artifact | Size | SHA-256 |
|---|---:|---|
| `apollo.exe` | 238,366,466 B | `464BF906F4BA681CF602EEC8BCDAB8382164D52D030E06692C7322F21A087D2A` |
| `Apollo-1.0.0-win64-setup.exe` (NSIS) | 45,692,784 B | `3CBB951098FFCBCD13A4A8090557DCBD6D0FA0FED86288345B57C9078AD2011C` |
| `Apollo-1.0.0-win64-portable.zip` | 62,806,439 B | `0B6606D97EC4A83555F65CE54A64D4C844B229BBF66EBFBA5E2EA0EA0A06E311` |

The portable package and installer include `LICENSE`, `NOTICE`, `CREDITS.md`,
`assets`, the SudoVDA driver package, setup scripts, `tools` and `zlib1.dll`.

## Verification

| Area | Result |
|---|---|
| Native build | Windows 11 / MSYS2 UCRT64 / GCC 16.2 / Ninja / Release |
| C++ tests | **942 tests / 931 pass / 11 skipped / 0 failed** |
| Web tests | **17 files / 238 tests pass** |
| npm audit | **0 vulnerabilities** |
| Conflict markers | None |
| Locale JSON | All parse |
| `git diff --check` | Clean |
| `apollo.exe --version` | `1.0.0`, commit `a4c0d637`, publisher Apollo 2026 |
| Host runtime smoke test | Ports `47984/47989/47990/48010` listening; `/api/config` 200 with auth |
| Virtual display driver | SudoVDA installed; `SudoMaker Virtual Display Adapter` present |
| ViGEmBus | 1.22.0 installed and running; XInput loopback test passes |
| Virtual HID Broker | Installed, service running, version compatible, license `unlicensed` |

The 11 skipped C++ tests are platform/hardware/session gated (tray screenshots,
AMD/Intel encoders absent on this host, absolute mouse behavior, Flatpak/Linux,
Virtual HID Broker without a license).

## Implemented features

- Sunshine 2026 streaming core and CMake/build system.
- Apollo branding, `ApolloService`, `apollo.exe`, Apollo icons.
- Web UI (Vue 3 SPA) with sidebar shell, dashboard, command palette and themes.
- Client management: permissions, display-mode override, legacy ordering,
  virtual-display policy, connect/disconnect commands, unpair, disconnect.
- App management: create/edit/delete, covers, launch/close, manual ordering,
  drag-and-drop, Alphabetize, Save Order, `.art` launcher export.
- Pairing: legacy PIN and OTP with QR, manual host/port, 3-minute expiry.
- Global prep/state/server commands.
- Virtual display via SudoVDA, including HDR and per-client identity.
- Gamepad backends:
  - Virtual HID Broker (libvirtualhid) with license management.
  - ViGEmBus fallback for Xbox 360/DS4.
- Virtual-input status/license UI and release/download links.
- Quit/restart, logs, permissions, troubleshooting.
- Windows installer with SudoVDA auto-install and firewall setup.

## Drivers and backends

### SudoVDA (virtual display)

- Bundled in the installer and portable package.
- Installed on this machine; device `SudoMaker Virtual Display Adapter`
  (`ROOT\DISPLAY\0000`).
- Still needs a real stream test for HDR, refresh and isolation.

### ViGEmBus (gamepads)

- BSD-3-Clause, EOL (1.22.0, 2023) but signed and working.
- Installed as the free fallback for Xbox 360 and DualShock 4.
- The new `libs/apollo-input` library tests it end-to-end: create virtual pad,
  read through XInput, press A, move stick, release.
- Does not support Xbox One/Series, DualSense, Switch Pro or adaptive triggers.

### Virtual HID Broker (libvirtualhid)

- Installed and compatible (`2026.914.1218.10`).
- License state: **unlicensed**; the broker is running and the API returns
  purchase/manage URLs.
- A paid machine license is required for driver-backed devices. Apollo cannot
  bundle or bypass it: the broker/driver use the LizardByte Source-Available
  License 1.0, which prohibits redistribution and circumvention.

### `libs/apollo-input`

A standalone MIT library with a portable gamepad model and ViGEmBus/uinput/null
backends. It is not yet wired into Apollo's main build. See
[apollo-input.md](apollo-input.md).

> The library was added to `main` after the `v1.0.0` tag; it is not part of the
> 1.0.0 installer. A future release can integrate it into the host build.

## Known limitations and open work

| Priority | Item | Notes |
|---|---|---|
| High | Real pairing/streaming with Artemis | Firewall/Private network, credentials, PIN/OTP, video/audio/input, close session |
| High | SudoVDA stream validation | Resolution, HDR, refresh, isolation |
| High | NSIS installer on a clean machine | Install, upgrade, uninstall, SudoVDA |
| Medium | Virtual HID Broker license | Needed for DualSense/Switch Pro/adaptive triggers/IMU on Windows |
| Medium | Code signing | `.exe` and installer are unsigned |
| Medium | Publish `main` + `v1.0.0` | Not pushed to `apollo2026` yet |
| Medium | Integrate `apollo-input` into Apollo | Optional; current host input still uses `src/platform/windows/input.cpp` |
| Medium | Native CI + release automation | Current CI covers Web UI, Pester and structure only |
| Low | macOS `vt_software` | Not tested on macOS hardware |
| Low | UHID backend / SDL3 test backend | Planned in `apollo-input` |
| Low | Linux/macOS native validation | Windows is the tested platform |

## How to run a real test

1. Install Apollo with `Apollo-1.0.0-win64-setup.exe` or extract the portable ZIP.
2. Open `https://<host>:47990/welcome` and create credentials.
3. Allow `apollo.exe` in Windows Firewall and use a Private network profile.
4. Add the host in Artemis (automatic discovery or manual IP).
5. Pair with PIN or OTP; confirm the client appears in **Clients**.
6. Launch **Desktop** or **Steam Big Picture**; verify video, audio and input.
7. Close the session and check that the virtual display/processes are removed.
8. For gamepads:
   - ViGEmBus fallback: select the ViGEmBus backend in **Configuration → Input**;
   - or activate a Virtual HID Broker license in **Troubleshooting**.

## Documentation

- [Getting started](getting_started.md)
- [Configuration](configuration.md)
- [Troubleshooting](troubleshooting.md)
- [API](api.md)
- [Building](building.md)
- [apollo-input](apollo-input.md)
- [Changelog](changelog.md)
