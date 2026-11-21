# Apollo 2026

[![Release](https://img.shields.io/github/v/release/leonidas10009/Apollo_2026?label=release)](https://github.com/leonidas10009/Apollo_2026/releases)
[![License: GPL v3](https://img.shields.io/badge/license-GPLv3-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)](https://github.com/leonidas10009/Apollo_2026/releases)

**Apollo 2026** is a self-hosted, low-latency game streaming host for **Moonlight**, **Artemis 2026**, and **Sunshine** clients. It supports AMD, Intel, and NVIDIA hardware encoding, software encoding, native client resolution, HDR virtual displays, and a modern Web UI for pairing and configuration from any browser.

**Companion Android client:** [Artemis 2026](https://github.com/leonidas10009/Artemis-2026), based on [Artemis / Moonlight Noir](https://github.com/ClassicOldSong/moonlight-android).

## Downloads

- [Apollo 2026 releases](https://github.com/leonidas10009/Apollo_2026/releases)
- [Artemis 2026 Android client](https://github.com/leonidas10009/Artemis-2026/releases)
- [Configuration reference](docs/configuration.md)

## Major features:

- [x] Built-in Virtual Display with HDR support that matches the resolution/framerate config of your client automatically
- [x] Modern responsive Web UI with a host dashboard, sidebar navigation, command palette (`Ctrl+K`), and theme switcher
- [x] Permission management for clients
- [x] Clipboard sync
- [x] Commands for client connection/disconnection (checkout [Auto pause/resume games](https://github.com/ClassicOldSong/Apollo/wiki/Auto-pause-resume-games))
- [x] Input only mode

## FAQ

### What is Apollo 2026?

Apollo 2026 is a self-hosted game streaming host for Windows and Linux. It streams a desktop or game to Moonlight-compatible clients, including the Artemis 2026 Android client, over the local network or the internet.

### Does Apollo work with Moonlight and Artemis?

Yes. Apollo works with Moonlight clients and with Artemis 2026. Artemis adds Apollo-specific features such as virtual display control, clipboard sync, server commands, and remote power actions.

### Does Apollo support Android TV?

Yes. Use the Artemis 2026 `arm64-v8a` or `armeabi-v7a` APK on Android TV/Google TV. The client includes a leanback launcher entry and does not require a touchscreen.

### Which GPUs are supported?

Apollo supports NVIDIA NVENC, AMD AMF, Intel Quick Sync, and software encoding. The Web UI shows the encoders available on the host.

### Is Apollo free?

Yes. Apollo 2026 is free and open source under GPL-3.0. Source code and releases are available on GitHub.

## Credits

Apollo 2026 is a continuation of **Apollo**, originally created by
**ClassicOldSong (Yukino Song)** — https://github.com/ClassicOldSong/Apollo — and is based on
**Sunshine** by LizardByte and contributors — https://github.com/SunshineStream/Sunshine.

The full Git history, authorship, GPL-3.0 license, and upstream notices are retained. See
[CREDITS.md](CREDITS.md) for details.

## Reusable components

- [`libs/apollo-input`](libs/apollo-input/README.md) — ApolloInput Nova 0.3.0, a
  standalone MIT-licensed C++ input abstraction library with Apollo HID (Windows,
  UMDF2 + VHF), ViGEmBus (Windows) and uinput (Linux) backends, controller profiles,
  compatibility fallback, anti-drift filtering and a Windows tester. It is independent
  of Apollo and can be embedded in other projects with `add_subdirectory` or
  `find_package`. See [docs/apollo-input.md](docs/apollo-input.md) for architecture,
  profiles, SDL3 analysis and roadmap.

## Current status

**Apollo 2026 1.0.1** is the current release, based on Sunshine `2026.1116.25010`.

- Windows binaries: `apollo.exe`, NSIS installer and portable ZIP.
- Native tests: **946 tests / 933 pass / 13 skipped / 0 failed** (last recorded run).
- Web UI: **258 tests pass**; `npm audit` reports 0 vulnerabilities.
- Remote server commands: authenticated `POST /servercmd?index=N` for power actions
  from Artemis without starting a stream.
- SudoVDA is bundled and installed by the Windows installer.
- Virtual HID Broker is optional and requires a paid machine license.
- ViGEmBus 1.22.0 can be installed separately as a free Xbox 360/DS4 fallback.
- Configurable gamepad stick/trigger deadzones, radial anti-drift filtering and
  resume neutralization (ApolloInput Nova precision path).
- Apollo HID (UMDF2 + VHF) backend is implemented in ApolloInput Nova but **not
  yet selected by the host**; `gamepad_driver` remains ViGEmBus/libvirtualhid.

See [docs/status.md](docs/status.md) for artifacts, verification, known limitations and the
remaining runtime tests (pairing, streaming, clean-machine installer and code signing).

## Usage

Apollo is based on the Sunshine 2026 codebase. The documentation in this repository is the
primary reference:

- [Getting started](docs/getting_started.md)
- [Status](docs/status.md)
- [Configuration](docs/configuration.md)
- [Troubleshooting](docs/troubleshooting.md)
- [API](docs/api.md)
- [Building](docs/building.md)
- [apollo-input](docs/apollo-input.md)
- [Apollo Wiki](https://github.com/ClassicOldSong/Apollo/wiki)

> [!NOTE]
> Some inherited upstream commands, package names, environment variables, and paths still
> use `sunshine`/`Sunshine` where those are the actual artifact or compatibility identifiers.
> The Windows executable is `apollo.exe`; Linux and macOS keep the upstream `sunshine`
> binary name for packaging compatibility. Apollo-specific behavior is called out in the
> documentation.

Currently Virtual Display support is Windows only, Linux support is planned and will be implemented in the future.

## About Permission System

Check out the [Wiki](https://github.com/ClassicOldSong/Apollo/wiki/Permission-System)

> [!NOTE]
> The **FIRST** client paired with Apollo will be granted with FULL permissions, then other newly paired clients will only be granted with `View Streams` and `List Apps` permission. If you encounter `Permission Denied` error when trying to launch any app, go check the permission for that device and grant `Launch Apps` permission. The same applies to the situation when you find that you can't move mouse or type with keyboard on newly paired clients, grant the corresponding client `Mouse Input` and `Keyboard Input` permissions.

## About Virtual Display

> [!WARNING]
> ***It is highly recommend to remove any other virtual display solutions from your system and Apollo config, to reduce confusions and compatibility issues.***

> [!NOTE]
> **TL;DR** Just treat your Artemis/Moonlight client like a dedicated PnP monitor with Apollo.

Apollo uses SudoVDA for virtual display. It features auto resolution and framerate matching for your Artemis/Moonlight clients. The virtual display is created upon the stream starts and removed once the app quits. **If you do not see a new virtual display added or removed when the stream starts or stops, there may be a driver misconfiguration, or another persistent virtual display might still be active.**

The Windows installer bundles the complete SudoVDA driver package, including the signed driver and the `nefconc` device-installation tool. ViGEmBus is **not** bundled because it is end-of-life; see the gamepad section in the documentation for the supported backends.

> [!NOTE]
> The Web UI **Applications** page lists your user-defined applications (by default **Desktop** and **Steam Big Picture**). Apollo also exposes special client-side entries — **Virtual Display**, **Remote Input** and **Terminate** — which are generated by the host, are not stored in `apps.json`, and are not editable from the Web UI.

The virtual display works just like any physically attached monitors with SudoVDA, there's completely no need for a super complicated solution to "fix" resolution configurations for your devices. Unlike all other solutions that reuses one identity or generate a random one each time for any virtual display sessions, **Apollo assigns a fixed identity for each Artemis/Moonlight client, so your display configuration will be automatically remembered and managed by Windows natively.**

## Configuration for dual GPU laptops

Apollo supports dual GPUs seamlessly.

If you want to use your dGPU, just set the `Adapter Name` to your dGPU and enable `Headless mode` in `Audio/Video` tab, save and restart your computer. No dummy plug is needed any more, the image will be rendered and encoded directly from your dGPU.

## About HDR

HDR starts supporting from Windows 11 23H2 and generally supported on 24H2. Some systems might not have HDR toggle on 23H2 and you just need to upgrade to 24H2. Any system lower than 23H2/Windows 10 will not have HDR option available.

> [!NOTE]
> The below section is written for professional media workers. It doesn't stop you from enabling HDR if you know what you're doing and have deep understanding about how HDR works.
>
> Apollo and SudoVDA can handle HDR just fine like any other streaming solutions.
>
> If you have had good experience with HDR previously, you can safely ignore this section.
>
> If you're curious, read on, but don't blame Apollo for poor HDR support.

Whether HDR streaming looks good, it depends completely on your client.

In short, ICC color correction should be totally useless while streaming HDR. It's your client's job to get HDR content displayed right, not the host. But in fact, it does affect the captured video stream and reflect changes on devices that can handle HDR correctly. On other devices that can't, the info is not respected at all.

It's very complicated to explain why HDR is a total mess, and why enabling HDR makes the image appear dark/yellow. If it's your first time got HDR streaming working, and thinks HDR looks awful, you're right, but that's not Apollo's fault, it's your device that tone mapped SDR content to the maximum of the capability of its screen, there's no headroom for anything beyond that actual peak brightness for HDR. For details, please take a look [here](https://github.com/ClassicOldSong/Apollo/issues/164).

For client devices, usually Apple products that have HDR capability can be trusted to have good results, other than that, your luck depends.

<details>
<summary>DEPRECATION ALERT</summary>

Enabling HDR is **generally not recommended** with **ANY streaming solutions** at this moment, probably in the long term. The issue with **HDR itself** is huge, with loads of semi-incompatible standards, and massive variance between device configurations and capabilities. Game support for HDR is still choppy.

SDR actually provides much more stable color accuracy, and are widely supported throughout most devices you can imagine. For games, art style can easily overcome the shortcoming with no HDR, and SDR has pretty standard workflows to ensure their visual performance. So HDR isn't *that* important in most of the cases.

</details>

## How to run multiple instances of Apollo for multiple virtual displays

Follow the instructions in the [Wiki](https://github.com/ClassicOldSong/Apollo/wiki/How-to-start-multiple-instances-of-Apollo).

## FAQ
Moved to [WiKi](https://github.com/ClassicOldSong/Apollo/wiki/FAQ)

## Stuttering Clinic
Here're some common causes and solutions for stutters: [WiKi](https://github.com/ClassicOldSong/Apollo/wiki/Stuttering-Clinic).

## Device specific setups
- Pixel devices might not be able to use native resolution:
  - Change the device resolution to Max: https://github.com/ClassicOldSong/Apollo/issues/700

## System Requirements

> **Warning**: This table is a work in progress. Do not purchase hardware based on this.

**Minimum Requirements**

| **Component** | **Description** |
|---------------|-----------------|
| GPU           | AMD: VCE 1.0 or higher, see: [obs-amd hardware support](https://github.com/obsproject/obs-amd-encoder/wiki/Hardware-Support) |
|               | Intel: VAAPI-compatible, see: [VAAPI hardware support](https://www.intel.com/content/www/us/en/developer/articles/technical/linuxmedia-vaapi.html) |
|               | Nvidia: NVENC enabled cards, see: [nvenc support matrix](https://developer.nvidia.com/video-encode-and-decode-gpu-support-matrix-new) |
| CPU           | AMD: Ryzen 3 or higher |
|               | Intel: Core i3 or higher |
| RAM           | 4GB or more |
| OS            | Windows: 10 22H2+ or 11 (SudoVDA virtual display requires 22H2+; gamepads need Virtual HID Broker or a manually installed ViGEmBus fallback) |
|               | macOS: 12+ |
|               | Linux/Debian: 11 (bullseye) |
|               | Linux/Fedora: 39+ |
|               | Linux/Ubuntu: 22.04+ (jammy) |
| Network       | Host: 5GHz, 802.11ac |
|               | Client: 5GHz, 802.11ac |

**4k Suggestions**

| **Component** | **Description** |
|---------------|-----------------|
| GPU           | AMD: Video Coding Engine 3.1 or higher |
|               | Intel: HD Graphics 510 or higher |
|               | Nvidia: GeForce GTX 1080 or higher |
| CPU           | AMD: Ryzen 5 or higher |
|               | Intel: Core i5 or higher |
| Network       | Host: CAT5e ethernet or better |
|               | Client: CAT5e ethernet or better |

**HDR Suggestions**

| **Component** | **Description** |
|---------------|-----------------|
| GPU           | AMD: Video Coding Engine 3.4 or higher |
|               | Intel: UHD Graphics 730 or higher |
|               | Nvidia: Pascal-based GPU (GTX 10-series) or higher |
| CPU           | AMD: todo |
|               | Intel: todo |
| Network       | Host: CAT5e ethernet or better |
|               | Client: CAT5e ethernet or better |

## Integrations

SudoVDA: Virtual Display Adapter Driver used in Apollo

[Artemis](https://github.com/ClassicOldSong/moonlight-android): Integrated Virtual Display options control from client side

**NOTE**: Artemis currently supports Android only. Other platforms will come later.

## Support

Currently support is only provided via GitHub Issues/Discussions.

No real time chat support will ever be provided for Apollo and Artemis. Including but not limited to:

- Discord
- Telegram
- Whatsapp
- QQ
- WeChat 

> When there's a chat, there're dramas. -- Confucius

## Downloads

### Direct Download

**Recommended**

[Releases](https://github.com/leonidas10009/Apollo_2026/releases)

### WinGet

**Note:** Community maintained and currently tracks the original Apollo project, not Apollo 2026.

In an elevated PowerShell window, run

```pwsh
winget install ClassicOldSong.Apollo

```

You'll need WinGet installed first.

### Chocolatey

**Note:** Community maintained and currently tracks the original Apollo project, not Apollo 2026.

You can also install the apollo streaming server with chocolatey.

Install Chocolatey if you don't have it, then run the following command in an elevated PowerShell/CMD window:

```pwsh
choco upgrade apollo -y 
```

Same command can be used to upgrade, add to a scheduled task to automate updates.

See more details on the chocolatey package [here](https://community.chocolatey.org/packages/apollo)

## Disclaimer

I got kicked from Moonlight and Sunshine's Discord server and banned from Sunshine's GitHub repo literally for helping people out.

This is what I got for finding a bug, opened an issue, getting no response, troubleshoot myself, fixed the issue myself, shared it by PR to the main repo hoping my efforts can help someone else during the maintenance gap.

Yes, I'm going away. [Apollo](https://github.com/ClassicOldSong/Apollo) and [Artemis(Moonlight Noir)](https://github.com/ClassicOldSong/moonlight-android) will no longer be compatible with OG Sunshine and OG Moonlight eventually, but they'll work even better with much more carefully designed features.

The Moonlight repo had stayed silent for 5 months, with nobody actually responding to issues, and people are getting totally no help besides the limited FAQ in their Discord server. I tried to answer issues and questions, solve problems within my ability but I got kicked out just for helping others.

**PRs for feature improvements are welcomed here unlike the main repo, your ideas are more likely to be appreciated and your efforts are actually being respected. We welcome people who can and willing to share their efforts, helping yourselves and other people in need.**

**Update**: They have contacted me and apologized for this incident, but the fact it **happened** still motivated me to start my own fork.

## License

GPLv3
