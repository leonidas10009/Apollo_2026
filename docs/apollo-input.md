# apollo-input

`libs/apollo-input` is a standalone, MIT-licensed C++ library for creating virtual
gamepads. It is deliberately independent from Apollo so it can be reused by other
projects.

> This document is the reference for the library. The application status is tracked in
> [status.md](status.md).

## Why a separate library

Apollo needs the same input concepts in several places:

- the streaming host (virtual gamepad for games),
- the Web UI/demo/preview tools,
- tests that must run without a driver,
- future clients or companion tools.

Keeping the state model, profiles and backends in one library avoids duplicating
button mappings and makes the Windows/Linux implementations interchangeable.

The library does **not** contain Apollo headers, configuration or protocol code.

## Current status

- Version: **0.3.0** (ApolloInput Nova).
- License: **MIT**.
- Location: `libs/apollo-input`.
- Windows backend: **Apollo HID** (native UMDF2 + VHF, generic + DualShock 4,
  one client handle per device) and **ViGEmBus** (Xbox 360 and DualShock 4, with
  compatibility fallback for the other profiles).
- Linux backend: **uinput** (all built-in profiles, profile-aware USB identity).
- Test backend: **null** (no-op).
- Build: CMake 3.20+, C++20, installable/exportable package.
- Includes a Windows diagnostic tester; the standalone repository also ships a
  browser-based visual tester.
- Host build integration: the library is **vendored but not linked into Apollo's
  main build yet**. The streaming host keeps using
  `src/platform/windows/input.cpp` and `libvirtualhid`; the 0.2.0 precision
  changes are applied directly in that path. The library remains available for
  `add_subdirectory(libs/apollo-input)` or `find_package(apollo-input)`. The
  Apollo HID backend compiles and is ready for the host selector.

## Architecture

```text
apollo-input
├── core
│   ├── GamepadState: buttons, sticks, triggers, touchpad, IMU, paddles, misc
│   ├── Profile: Xbox 360/One/Series/Elite, DS4, DualSense/Edge,
│   │           Switch Pro, Steam Controller/Deck, generic
│   ├── Profile helpers: aliases, capabilities, USB identity,
│   │                   recommended_profile(), compatibility chain
│   ├── Precision: radial stick deadzone, center snap, trigger deadzone
│   ├── Capabilities: rumble, trigger rumble, LEDs, battery, IMU, touchpad,
│   │                 adaptive triggers, raw keyboard/mouse
│   ├── Feedback: rumble, LEDs, adaptive triggers
│   └── Backend/Device interfaces and factories
├── Windows backends
│   ├── Apollo HID: generic + DualShock 4 over UMDF2 + VHF
│   └── ViGEmBus: Xbox 360 + DualShock 4, with create_compatible() fallback
├── Linux backend
│   └── uinput: all built-in profiles with profile-specific USB identity
└── null backend
    └── deterministic no-op device for tests and previews
```

The public API is exposed through:

```cpp
#include <apollo/input/input.hpp>

auto backend = apollo::input::create_backend(apollo::input::BackendKind::automatic);
std::string error;
if (!backend->available(&error)) {
  // Show error.
}

auto pad = backend->create(apollo::input::Profile::xbox_360, "My Virtual Pad", &error);

apollo::input::GamepadState state;
state.set(apollo::input::Button::a, true);
state.left_x = 12345;
pad->update(state);

apollo::input::ImuSample imu;
imu.gyro_x = 0.1F;
imu.accel_z = 9.81F;
pad->update_imu(imu);

pad->disconnect();
```

`Device::update()` returns `false` when the backend does not support the requested
operation or the device is disconnected. Capabilities tell the caller what is
available before submitting optional data.

## Profiles and compatibility

The library models these virtual profiles. Names and aliases are
case-insensitive.

| Canonical profile | Aliases | Intended controller |
|---|---|---|
| `xbox_360` | `x360`, `xbox360` | Xbox 360, universal XInput fallback |
| `xbox_one` | `xone`, `xboxone` | Xbox One |
| `xbox_series` | `xseries`, `xboxseries` | Xbox Series with Share |
| `xbox_elite` | `elite`, `elite_2`, `xboxelite` | Xbox Elite with paddles |
| `dualshock4` | `ds4`, `ps4` | DualShock 4 |
| `dualsense` | `ds5`, `ps5` | DualSense |
| `dualsense_edge` | `ds5_edge`, `ps5_edge` | DualSense Edge with paddles |
| `switch_pro` | `switch`, `pro_controller` | Switch Pro Controller |
| `steam_controller` | `steam`, `valve_steam_controller` | Steam Controller |
| `steam_deck` | `deck`, `steamdeck` | Steam Deck controls |
| `generic` | `hid` | Generic HID gamepad |

`Backend::create_compatible()` tries the requested profile first and then walks
`profile_compatibility_chain()`. On ViGEmBus, for example, a DualSense request
falls back to DualShock 4 and then to Xbox 360. The returned device reports the
profile it actually emulates through `DeviceInfo::profile`.

`recommended_profile()` chooses a profile from the controller family, touchpad,
motion, paddles and misc buttons reported by the client.

## Backend matrix

| Backend | Platform | Profiles | IMU | Touchpad | Feedback | Notes |
|---|---|---|---|---|---|---|
| Apollo HID | Windows | All built-in profiles; advanced ones use compatibility HID descriptors | DS4/DS5 | DS4/DS5 | DS4/DS5 rumble + lightbar; Xbox rumble; player LEDs | Requires Apollo HID driver 0.1.0.6+ (protocol 2) |
| ViGEmBus | Windows | Xbox 360, DualShock 4; others via `create_compatible()` | DS4 | DS4 | Rumble, DS4 lightbar | Requires the ViGEmBus driver |
| uinput | Linux | All built-in profiles | No | No | Not implemented | Requires `/dev/uinput` access |
| null | All | All | No | No | No | Does nothing; useful for tests |

## Windows: Apollo HID

The Apollo HID backend uses the project's own UMDF2 + VHF driver. It supports:

- **generic**: 8 buttons, D-pad, four axes and two triggers.
- **DualShock 4**: full USB report 0x01 with touchpad, gyroscope, accelerometer,
  battery, rumble and lightbar.
- **DualSense/Edge, Xbox One/Series/Elite, Switch Pro and Steam Controller/Deck**:
  compatibility HID descriptors with common buttons/sticks/triggers, rumble,
  touchpad/IMU where applicable and paddles. These are **not bit-exact vendor
  descriptors** and need hardware/Steam validation. Xbox profiles are HID, not
  XUSB/XInput.
- Up to **8 independent controllers**. Each device owns its own client handle,
  so the driver isolates controllers per file object and closing one device does
  not remove another.
- Slots are scoped to the handle; `AUTO_SLOT` asks the driver for the lowest
  free slot.

Select it explicitly:

```cpp
auto backend = apollo::input::create_backend(apollo::input::BackendKind::apollo_hid);
```

`automatic` still prefers ViGEmBus on Windows while it is built. Apollo HID
requires the driver package and the sibling `apollo HID` checkout; CMake
auto-detects it or accepts `APOLLO_INPUT_APOLLO_HID_DIR`. The backend compiles
the small Apollo HID client/core sources into `libapollo-input`, so the
installed archive is self-contained.

## Windows: ViGEmBus

The Windows backend uses the MIT-licensed `ViGEmClient` and the BSD-3-Clause
ViGEmBus driver. It supports:

- **Xbox 360**: buttons, D-pad, sticks, triggers, rumble.
- **DualShock 4**: buttons, D-pad, sticks, triggers, touchpad, gyroscope,
  accelerometer, rumble and lightbar. ViGEmClient exposes the DS4 motion and
  touchpad fields through `DS4_REPORT_EX`.

ViGEmBus is **end-of-life** (last release 1.22.0, 2023) but remains signed and
works on Windows 10/11. It does not expose Xbox One/Series, DualSense or Switch Pro
profiles, and it does not implement adaptive triggers. ApolloInput Nova 0.3.0
reports accurate capabilities for the profile it actually emulates and falls back
through `create_compatible()` instead of failing the virtual pad creation.

The library does **not** bundle ViGEmBus. Install the official package from:

<https://github.com/nefarius/ViGEmBus/releases>

On this machine ViGEmBus 1.22.0 is installed:

- Service: `ViGEmBus` (running).
- Device: `Nefarius Virtual Gamepad Emulation Bus` (`ROOT\SYSTEM\0001`).
- Installer SHA-256: `89220A7865076B342892F98865F3499FB7C4CFD673159E89D352C360FD014C6A`.
- Authenticode: valid, signed by Nefarius Software Solutions e.U.

The official installer uses Advanced Installer. `/VERYSILENT` is not valid; use:

```powershell
ViGEmBus_1.22.0_x64_x86_arm64.exe /exenoui /qn /norestart /L*v install.log
```

The integration test creates a virtual Xbox 360 pad, reads it back through XInput,
presses A, moves the left stick, releases A and verifies the release:

```text
100% tests passed, 0 tests failed out of 2
```

## Linux: uinput

The uinput backend opens `/dev/uinput`, registers a virtual gamepad with
`UI_SET_EVBIT`/`UI_SET_ABS...`, and emits `input_event` records for buttons, sticks,
triggers, D-pad, paddles, Share and touchpad-click buttons. It is profile-aware:
the device exposes the USB vendor/product/version and capabilities of the
requested profile.

Limitations:

- uinput exposes Linux input events, not arbitrary HID report descriptors.
- It cannot expose IMU, a real touchpad surface, battery, player LEDs or force
  feedback.
- Adaptive triggers require a HID-capable backend.
- Access to `/dev/uinput` is required (udev rule or `input` group).
- The backend is implemented but has **not been validated on a Linux host** in this
  session; the validated backend is ViGEmBus on Windows.

For full HID profiles and IMU on Linux, a future **UHID** backend is the correct
path. UHID accepts a HID report descriptor and can expose DualSense/Switch Pro
profiles, motion and touchpads without a proprietary license.

## Null backend

The null backend accepts any profile, records the last state and returns success.
It is used by unit tests, UI previews and CI on machines without a driver.

## Precision and anti-drift

`sanitize_gamepad_state()` applies:

- a **radial deadzone** to both sticks,
- exact `0,0` center snap inside the deadzone,
- rescaling outside the deadzone to preserve full travel,
- an independent trigger deadzone.

Defaults are **3 %** for sticks and **2 %** for triggers.
`Device::set_deadzone()` changes them per device. The separate Apollo host patch
applies the same idea in the real streaming path, makes both percentages
configurable, and submits a neutral gamepad state when a paused session resumes.
See [controller-compatibility.md](controller-compatibility.md).

## IMU and touchpad

IMU and touchpad are capabilities, not separate backends:

- The core carries `ImuSample` (gyro/accel with timestamp) and `TouchpadState`.
- A backend returns `true` from `update_imu()`/`update_touchpad()` only when it can
  expose them.
- ViGEmBus exposes DS4 IMU/touchpad; uinput does not.
- Apollo's protocol already carries `SS_CONTROLLER_MOTION_MAGIC`; the missing piece
  on Windows without Virtual HID Broker is a backend that exposes motion.

## SDL3: where it fits

SDL3 has an excellent gamepad API and a virtual joystick API with touchpad and
sensor support (`SDL_AttachVirtualJoystick`,
`SDL_SetJoystickVirtualTouchpad`, `SDL_SendJoystickVirtualSensorData`).

However, SDL3 virtual joysticks are **process-local**: the SDL documentation says
they "look like a normal joystick to SDL, just not backed by a hardware driver".
They do not create a system-wide HID/XInput device that another game process can see.

Therefore:

| Use | SDL3 | ViGEmBus/uinput/UHID |
|---|---|---|
| Client (Artemis) reading physical controllers, IMU, touchpad | Excellent | Not applicable |
| Tests/preview inside the same process | Excellent | Overkill |
| Host-side virtual pad visible to games | No | Yes |
| Host IMU/touchpad | In-process only | DS4/VHF/UHID |

An optional SDL3 backend is planned for tests, demos and previews. It would not be
selected as the host backend automatically.

## ViGEmBus fork / custom driver

Forking ViGEmBus is legally possible (BSD-3-Clause) but is a separate driver project:

- ViGEmBus is KMDF and only covers Xbox 360/DS4.
- Adding Xbox One/Series, DualSense, Switch Pro, IMU, adaptive triggers and Raw
  Input keyboard/mouse requires new HID descriptors/protocols and substantial work.
- Windows distribution requires Microsoft signing (EV certificate, Hardware Dev
  Center, attestation/WHQL). Without it, users would need test signing or Secure
  Boot disabled.
- Kernel drivers can cause BSODs and require ongoing Windows/anti-cheat testing.
- Virtual HID Broker's broker/driver code is **LB-SAL 1.0** and cannot be copied or
  adapted; only independent implementations based on public specifications are
  acceptable.

The recommended path is:

1. Use ViGEmBus 1.22.0 as a free Windows fallback for Xbox 360/DS4.
2. Add UHID on Linux for DualSense/Switch Pro/IMU.
3. Ask LizardByte for permission to redistribute the official broker, or a free
   license for Apollo users.
4. Only then consider a signed UMDF2+VHF driver as a long-term project.

## Roadmap

| Phase | Content | Status |
|---|---|---|
| 0 | Core + ViGEmBus (Xbox 360/DS4) + uinput + null + precision + testers | **Done (0.2.0)** |
| 1 | Apollo HID (UMDF2 + VHF): generic + DS4, multi-controller, per-device handle | **Done (0.3.0)** |
| 2 | Apollo host integration with `gamepad_driver = apollohid` and fallback | Next |
| 3 | Apollo HID advanced profiles (DualSense, Xbox, Elite, Edge, Switch, Steam) | Planned |
| 4 | Apollo HID EV signing and Microsoft attestation | Pending external certificate |
| 5 | SDL3 test/preview backend | Planned |
| 6 | Linux UHID (DualSense/Switch Pro/IMU) | Planned |
| 7 | macOS IOHID/DriverKit | Future project |
| 8 | Adaptive triggers and full feedback parity | Future project |

## Build

### Windows (MSYS2 UCRT64)

```bash
cmake -S libs/apollo-input -B cmake-build-apollo-input -G Ninja \
  -DAPOLLO_INPUT_VIGEMCLIENT_DIR="$PWD/third-party/ViGEmClient" \
  -DAPOLLO_INPUT_APOLLO_HID_DIR="$PWD/../../apollo HID"
cmake --build cmake-build-apollo-input --parallel
ctest --test-dir cmake-build-apollo-input --output-on-failure
```

The CMake script also finds `third-party/ViGEmClient` automatically in a standalone
checkout and `../../third-party/ViGEmClient` in a nested Apollo checkout.

Build outputs:

```text
cmake-build-apollo-input/libapollo-input.a
cmake-build-apollo-input/examples/apollo-input-demo.exe
cmake-build-apollo-input/tests/apollo-input-core-tests.exe
cmake-build-apollo-input/tests/apollo-input-xinput-loopback.exe
cmake-build-apollo-input/tools/apollo-input-controller-tester.exe
```

### Linux

```bash
cmake -S libs/apollo-input -B cmake-build-apollo-input -G Ninja
cmake --build cmake-build-apollo-input --parallel
ctest --test-dir cmake-build-apollo-input --output-on-failure
```

## Use from another project

```cmake
add_subdirectory(libs/apollo-input)
target_link_libraries(my_app PRIVATE apollo::input)
```

Or after installing:

```cmake
find_package(apollo-input CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE apollo::input)
```

The install exports both `apollo-input` and the MIT `ViGEmClient` dependency on
Windows. A consumer project using `find_package(apollo-input CONFIG REQUIRED)` was
built successfully as a verification.

## Tests

- `apollo-input-core`: dependency-free state, profile, fallback, precision and
  null-backend tests.
- `apollo-input-vigem-xinput`: creates a virtual Xbox 360 pad, reads it through
  XInput, verifies A/stick press and release, and checks the
  DualSense → DualShock 4 fallback. Skipped (exit 77) when ViGEmBus is not
  installed.

Validated result on this host (MSYS2 UCRT64, GCC 16.2):

```text
100% tests passed, 0 tests failed out of 2
```

## Controller tester

```bash
./cmake-build-apollo-input/tools/apollo-input-controller-tester.exe list
./cmake-build-apollo-input/tools/apollo-input-controller-tester.exe monitor xinput 0
./cmake-build-apollo-input/tools/apollo-input-controller-tester.exe drift xinput 0 --seconds 10
./cmake-build-apollo-input/tools/apollo-input-controller-tester.exe bridge xinput 0 --profile ds5 --seconds 120
./cmake-build-apollo-input/tools/apollo-input-controller-tester.exe selftest --profile xseries --seconds 20
./cmake-build-apollo-input/tools/apollo-input-controller-tester.exe verify
```

The tester enumerates XInput, DirectInput and HID interfaces, measures raw and
filtered drift, creates virtual pads with profile fallback, bridges a physical
controller to a virtual one, forwards rumble and runs a visible self-test.

## Apollo host integration

The library and the host patch are separate deliverables:

- `libs/apollo-input` contains the 0.3.0 library and its tester.
- `apollo-streaming-precision-0.2.0.patch` adds the precision path to the Apollo
  host (`src/config.cpp`, `src/input.cpp`,
  `src/platform/virtualhid_input.cpp`, `src/platform/windows/input.cpp`).

The host options are documented in [configuration.md](configuration.md):

- `gamepad_stick_deadzone` (0–30 %, default 3),
- `gamepad_trigger_deadzone` (0–30 %, default 2).

The library is **not linked into the host build yet**. Apollo still creates its
virtual pads through `src/platform/windows/input.cpp` and `libvirtualhid`; the
0.2.0 precision behavior is integrated there by the host patch. Linking
`apollo::input` and adding `gamepad_driver = apollohid` remains the next host
integration step.

## Demo

```bash
# Xbox 360
./cmake-build-apollo-input/examples/apollo-input-demo --backend vigem --profile xbox_360 --seconds 8

# DualShock 4 with IMU/touchpad
./cmake-build-apollo-input/examples/apollo-input-demo --backend vigem --profile dualshock4 --seconds 8

# No driver required
./cmake-build-apollo-input/examples/apollo-input-demo --backend null --profile xbox_360 --seconds 2
```

## Licensing and provenance

- Library: MIT (see `libs/apollo-input/LICENSE`).
- `ViGEmClient`: MIT.
- `ViGEmBus`: BSD-3-Clause.
- uinput: Linux kernel user-space API; no additional library license.
- No LizardByte LB-SAL code is used.

Third-party trademarks and driver signatures remain the property of their owners.
