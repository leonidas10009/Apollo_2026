# Apollo 2026 — current status

> Last updated: **2026-11-20**.
> This document describes the current application state, what was verified, and what
> remains open. It is not a changelog; see [changelog.md](changelog.md) for releases.

## Release

| Item | Value |
|---|---|
| Product | Apollo 2026 |
| Version | **1.0.1** |
| Upstream base | Sunshine `2026.1116.25010` (`5716a2ac`) |
| Git tag | **`v1.0.1`** at `60143517` |
| GitHub release | https://github.com/leonidas10009/Apollo_2026/releases/tag/v1.0.1 |
| Windows executable | `apollo.exe` (internal CMake target `sunshine`) |
| Publisher metadata | Apollo 2026 — https://github.com/leonidas10009/Apollo_2026 |
| License | GPL-3.0-only |

### Artifacts

| Artifact | Size | SHA-256 |
|---|---:|---|
| `apollo.exe` | 238,372,486 B | `05147916E64541F6F8CD117F03B097E4099ACF46A2A7FD5951E43AA8C3A37D41` |
| `Apollo-1.0.1-win64-setup.exe` (NSIS) | 45,701,310 B | `C1544FA36B6191190C250718823517F3096D9AB9D5D73C01687D09CDB9D0CF67` |
| `Apollo-1.0.1-win64-portable.zip` | 62,809,252 B | `7A1BA3B54DCFF1C71CC9CDAB40876EF58021ED43B2CD2189F17E094DFCB22E15` |

The portable package and installer include `LICENSE`, `NOTICE`, `CREDITS.md`,
`assets`, the SudoVDA driver package, setup scripts, `tools` and `zlib1.dll`.

## Verification

| Area | Result |
|---|---|
| Native build | Windows 11 / MSYS2 UCRT64 / GCC 16.2 / Ninja / Release |
| C++ tests | **943 tests / 930 pass / 13 skipped / 0 failed** (last recorded run) |
| Web tests | **17 files / 248 tests pass** (last recorded run) |
| npm audit | **0 vulnerabilities** |
| Conflict markers | None |
| Locale JSON | All parse |
| `git diff --check` | Clean |
| `apollo.exe --version` | `1.0.1`, commit `60143517`, publisher Apollo 2026 |
| HTTPS `/servercmd` | Paired client cert + `server_cmd` required; unauthenticated connection aborted |
| Host runtime smoke test | Ports `47984/47989/47990/48010` listening; `/api/config` 200 with auth |
| Virtual display driver | SudoVDA installed; `SudoMaker Virtual Display Adapter` present |
| ViGEmBus | 1.22.0 installed and running; XInput loopback test passes |
| Virtual HID Broker | Installed, service running, version compatible, license `unlicensed` |

The 13 skipped C++ tests are platform/hardware/session gated (tray screenshots,
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
- Remote server commands without a stream: authenticated `POST /servercmd?index=N`,
  used by Artemis for shutdown/restart/cancel actions.
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
> 1.0.0 or 1.0.1 installers. A future release can integrate it into the host build.

## Known limitations and open work

| Priority | Item | Notes |
|---|---|---|
| High | Real pairing/streaming with Artemis | Firewall/Private network, credentials, PIN/OTP, video/audio/input, close session |
| High | SudoVDA stream validation | Resolution, HDR, refresh, isolation |
| High | NSIS installer on a clean machine | Install, upgrade, uninstall, SudoVDA |
| Medium | Virtual HID Broker license | Needed for DualSense/Switch Pro/adaptive triggers/IMU on Windows |
| Medium | Code signing | `.exe` and installer are unsigned |
| Medium | End-to-end power actions from Artemis | Test `/servercmd` with a paired device and `server_cmd`; verify shutdown, restart and cancel |
| Medium | Integrate `apollo-input` into Apollo | Optional; current host input still uses `src/platform/windows/input.cpp` |
| Medium | Native CI + release automation | Current CI covers Web UI, Pester and structure only |
| Low | macOS `vt_software` | Not tested on macOS hardware |
| Low | UHID backend / SDL3 test backend | Planned in `apollo-input` |
| Low | Linux/macOS native validation | Windows is the tested platform |

## How to run a real test

1. Install Apollo with `Apollo-1.0.1-win64-setup.exe` or extract the portable ZIP.
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
