# Disabled upstream workflows

These workflows were inherited from Sunshine/LizardByte and are intentionally
inactive for Apollo 2026:

- `ci-release.yml`, `ci-cloudsmith.yml` and `ci-web.yml` publish to LizardByte
  infrastructure (NPM, Copr, Cloudsmith) and require upstream secrets.
- `publish-artifacts.yml` and `publish-screenshots.yml` wait for a `CI` workflow
  that Apollo does not run.
- `_top-issues.yml` is a scheduled LizardByte organization dashboard.
- The platform workflows (`ci-alpine`, `ci-archlinux`, `ci-freebsd`, `ci-macos`,
  `ci-powershell`) are `workflow_call` helpers for an upstream caller that is not
  part of this repository.

The active Apollo checks live in `.github/workflows/ci.yml`. Re-enable a file
only after replacing upstream repositories, secrets, signing identities and
publication destinations with Apollo-owned equivalents.
