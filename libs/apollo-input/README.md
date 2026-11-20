# apollo-input

`apollo-input` is a small, standalone C++ library that abstracts virtual gamepad
backends. It does not depend on Apollo, so it can be reused by other projects.

## Features

- One input model: buttons, sticks, triggers, touchpad, IMU, rumble/LED feedback.
- Capability discovery per backend and profile.
- Backends:
  - **Windows / ViGEmBus**: Xbox 360 and DualShock 4 (including DS4 gyro/accel,
    touchpad, rumble and lightbar through ViGEmBus).
  - **Linux / uinput**: simple Xbox 360-style gamepad (`/dev/uinput`).
  - **Null**: no-op backend for tests, previews and unsupported platforms.
- Dependency-free core and tests.
- CMake package export for `find_package(apollo-input)` or
  `add_subdirectory(libs/apollo-input)`.

## Layout

```text
libs/apollo-input/
├── include/apollo/input/   Public headers
├── src/core/               Portable state and backend selection
├── src/backends/           Platform backends
├── tests/                  Unit and integration tests
├── examples/               CLI demo
└── cmake/                  Package config template
```

## Feature matrix

| Backend | Platform | Profiles | IMU | Touchpad | Rumble/LED | Signing/license |
|---|---|---|---|---|---|---|
| ViGEmBus | Windows | Xbox 360, DualShock 4 | DS4 only | DS4 only | Yes | Driver already signed, BSD-3 |
| uinput | Linux | Xbox 360-style, generic | No | No | Not implemented | None |
| Null | All | All (no-op) | No | No | No | None |

ViGEmBus does **not** expose Xbox One/Series, DualSense or Switch Pro profiles.
Those require a HID-capable backend (UHID on Linux, VHF/UMDF2 on Windows) and a
Microsoft-signed driver on Windows.

## Build on Windows (MSYS2 UCRT64)

```bash
cmake -S libs/apollo-input -B cmake-build-apollo-input -G Ninja \
  -DAPOLLO_INPUT_VIGEMCLIENT_DIR=E:/path/to/Apollo-git/third-party/ViGEmClient
cmake --build cmake-build-apollo-input --parallel
```

If ViGEmClient is already a CMake target (for example when the library is added
inside Apollo), `APOLLO_INPUT_VIGEMCLIENT_DIR` can be omitted.

### Install ViGEmBus

The ViGEmBus driver is a separate BSD-3-Clause project and is **not** bundled by
this library. Install the official signed package from
<https://github.com/nefarius/ViGEmBus/releases>. The current release is 1.22.0
and is end-of-life, but it is signed and works on Windows 10/11.

Without ViGEmBus, the Windows backend reports `ViGEmBus error 0xE0000001`
(`VIGEM_ERROR_BUS_NOT_FOUND`).

## Build on Linux

```bash
cmake -S libs/apollo-input -B cmake-build-apollo-input -G Ninja
cmake --build cmake-build-apollo-input --parallel
```

uinput needs write access to `/dev/uinput` (usually a `udev` rule or the
`input` group).

## Run the demo

```bash
# Windows, Xbox 360 profile
./cmake-build-apollo-input/examples/apollo-input-demo --backend vigem --profile xbox_360 --seconds 8

# Windows, DualShock 4 profile with IMU/touchpad
./cmake-build-apollo-input/examples/apollo-input-demo --backend vigem --profile dualshock4 --seconds 8

# Linux
./cmake-build-apollo-input/examples/apollo-input-demo --backend uinput --profile xbox_360 --seconds 8
```

Open `joy.cpl` on Windows or `evtest`/`jstest` on Linux to inspect the device.

## Run the tests

```bash
ctest --test-dir cmake-build-apollo-input --output-on-failure
```

- `apollo-input-core` runs without hardware.
- `apollo-input-vigem-xinput` creates a ViGEmBus Xbox 360 controller and reads
  it back through XInput. It is skipped (exit code 77) when ViGEmBus is missing.

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
pad->update(state);
```

## Roadmap

1. Windows ViGEmBus: Xbox 360 and DualShock 4 (current).
2. Linux UHID backend for DualSense, Switch Pro and IMU.
3. Windows VHF/UMDF2 backend for DualSense, Switch Pro, Xbox One/Series,
   IMU, touchpad and Raw Input keyboard/mouse. Requires Microsoft signing.
4. macOS IOHID/DriverKit backend. Requires entitlements and notarization.
5. Adaptive trigger and LED feedback parity with Virtual HID Broker.

The Windows HID backend is a driver project with signing and maintenance costs;
it is intentionally separate from the portable core.

## License

MIT. See [LICENSE](LICENSE).

ViGEmClient is MIT and ViGEmBus is BSD-3-Clause; both retain their own notices.
No code from LizardByte's Source-Available License 1.0 components is used.
