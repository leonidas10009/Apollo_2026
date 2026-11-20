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

- Version: **0.1.0**.
- License: **MIT**.
- Location: `libs/apollo-input`.
- Windows backend: **ViGEmBus** (Xbox 360 and DualShock 4).
- Linux backend: **uinput** (simple Xbox 360-style gamepad).
- Test backend: **null** (no-op).
- Build: CMake 3.20+, C++20, installable/exportable package.
- Integration into Apollo's main build: **not wired yet**; it is consumed with
  `add_subdirectory` or `find_package(apollo-input)`.

## Architecture

```text
apollo-input
├── core
│   ├── GamepadState: buttons, sticks, triggers
│   ├── Profile: Xbox 360, Xbox One/Series, DS4, DualSense, Switch Pro, generic
│   ├── Capabilities: rumble, LEDs, IMU, touchpad, adaptive triggers, raw keyboard/mouse
│   ├── Feedback: rumble, LEDs, adaptive triggers
│   └── Backend/Device interfaces and factories
├── Windows backend
│   └── ViGEmBus: Xbox 360 + DualShock 4
├── Linux backend
│   └── uinput: simple virtual gamepad
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

## Backend matrix

| Backend | Platform | Profiles | IMU | Touchpad | Feedback | Notes |
|---|---|---|---|---|---|---|
| ViGEmBus | Windows | Xbox 360, DualShock 4 | DS4 | DS4 | Rumble, DS4 lightbar | Requires the ViGEmBus driver |
| uinput | Linux | Xbox 360-style, generic | No | No | Not implemented | Requires `/dev/uinput` access |
| null | All | All | No | No | No | Does nothing; useful for tests |

## Windows: ViGEmBus

The Windows backend uses the MIT-licensed `ViGEmClient` and the BSD-3-Clause
ViGEmBus driver. It supports:

- **Xbox 360**: buttons, D-pad, sticks, triggers, rumble.
- **DualShock 4**: buttons, D-pad, sticks, triggers, touchpad, gyroscope,
  accelerometer, rumble and lightbar. ViGEmClient exposes the DS4 motion and
  touchpad fields through `DS4_REPORT_EX`.

ViGEmBus is **end-of-life** (last release 1.22.0, 2023) but remains signed and
works on Windows 10/11. It does not expose Xbox One/Series, DualSense or Switch Pro
profiles, and it does not implement adaptive triggers.

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
triggers and the D-pad.

Limitations:

- uinput exposes Linux input events, not arbitrary HID report descriptors.
- No IMU, touchpad, DualSense, Switch Pro or adaptive triggers.
- Force feedback is not implemented yet.
- Access to `/dev/uinput` is required (udev rule or `input` group).
- The backend is implemented but has **not been validated on a Linux host** in this
  session; the validated backend is ViGEmBus on Windows.

For full HID profiles and IMU on Linux, a future **UHID** backend is the correct
path. UHID accepts a HID report descriptor and can expose DualSense/Switch Pro
profiles, motion and touchpads without a proprietary license.

## Null backend

The null backend accepts any profile, records the last state and returns success.
It is used by unit tests, UI previews and CI on machines without a driver.

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
| 0 | Core + ViGEmBus (Xbox 360/DS4) + uinput + null | **Done** |
| 1 | SDL3 test/preview backend | Planned |
| 2 | Linux UHID (DualSense/Switch Pro/IMU) | Planned |
| 3 | Windows VHF/UMDF2 + Microsoft signing | Future project |
| 4 | macOS IOHID/DriverKit | Future project |
| 5 | Adaptive triggers and full feedback parity | Future project |

## Build

### Windows (MSYS2 UCRT64)

```bash
cmake -S libs/apollo-input -B cmake-build-apollo-input -G Ninja \
  -DAPOLLO_INPUT_VIGEMCLIENT_DIR="$PWD/third-party/ViGEmClient"
cmake --build cmake-build-apollo-input --parallel
ctest --test-dir cmake-build-apollo-input --output-on-failure
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

- `apollo-input-core`: dependency-free state/profile/null tests.
- `apollo-input-vigem-xinput`: creates a virtual Xbox 360 pad, reads it through
  XInput, verifies A/stick press and release. Skipped (exit 77) when ViGEmBus is
  not installed.

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
