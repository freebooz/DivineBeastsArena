# Launcher Manifest Validation

This document records the launcher manifest contract enforced by Game.Api and
DBA_GameLauncher.

## Admin metadata rules

- `checksum` must be a 64-character SHA256 hex string.
- `downloadUrl` must be an absolute HTTP/HTTPS URL.
- `sizeBytes` must be greater than zero.
- Publishing an active version deactivates the previous active version for the
  same `channel + platform`.

## Launcher runtime rules

- Manifests are validated after fetch and before repair.
- File lists must not be empty.
- File names must be relative and must not contain path traversal.
- File names must be unique after separator and case normalization.
- File checksums must be 64-character SHA256 hex strings.
- Multi-file manifests must use a directory-style `downloadUrl` ending in `/`.
- Directory-style `downloadUrl` values are accepted for single-file manifests
  and resolve by appending the manifest file name.
- A successful repair writes `version.txt` with the manifest version after all
  files have been downloaded and verified.
- Three-part and four-part numeric versions are accepted, for example `1.2.5`
  and `1.2.5.0`.

## Current verification

- `dotnet test DBA_GameBackend/Game.Api.Tests/Game.Api.Tests.csproj --no-restore`
- `cargo test --manifest-path DBA_GameLauncher/src-tauri/Cargo.toml`
- `cargo check --manifest-path DBA_GameLauncher/src-tauri/Cargo.toml`
