# SudoVDA driver package

This directory contains the complete Windows driver package for the
[SudoVDA](https://github.com/SudoMaker/SudoVDA) virtual display driver used by
Apollo's virtual display feature.

The `install.bat` script installs the self-signed certificate, creates the
device node, and installs the driver with `nefconc.exe`. Without both
`SudoVDA.dll` and `nefconc.exe`, the installer cannot create the virtual
display device.

## Contents and provenance

| File | Version | SHA-256 | Source |
|---|---|---|---|
| `SudoVDA.dll` | 1.10.9.289 (INF `DriverVer` 2025-07-14) | `47EE263CB5DE9382C6630A2D7F3DAFEC4A49419F953BEEC869CA5DD0C460FF63` | Extracted from the official Apollo 0.4.6 installer; signature and catalog match `sudovda.cat` / `sudovda.cer`. |
| `nefconc.exe` | 1.21.1 | `6AAE68269FC7BF3C2EC7B932B7D36248200E1F421513D6770E5F85DEBC89588A` | Official [nefarius/nefcon](https://github.com/nefarius/nefcon/releases/tag/v1.21.1) release, `x64/nefconc.exe`. |
| `SudoVDA.inf` | 1.10.9.289 | `AD69AC682756F0CF339B081FAC7E6E8159FDF2CA01CA69DF8945C7246C286925` | Apollo driver package. |
| `sudovda.cat` | - | `2F9189DE5604BEC9D86F51640CC540639E394D9AD0F8E689129375E95F2D22F8` | Apollo driver package. |
| `sudovda.cer` | - | `6ACCDCD519F6179D967DB4EAA20ECF25A732BA30E87F4CFFEBC768B2C13C9007` | Apollo driver package. |

The certificate is self-signed (`CN=sudovda@su.mk`) and valid until 2030-07-13.
`install.bat` adds it to the local machine's `Root` and `TrustedPublisher`
stores before installing the driver.

## Licenses

- SudoVDA: MIT / CC0 (see the upstream repository).
- nefcon: MIT (Copyright Nefarius Software Solutions e.U.).
