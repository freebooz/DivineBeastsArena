# DivineBeastsArena Platform

This repository contains a UE multiplayer game client plus the minimum surrounding online-game platform services.

## Layout

```text
DBA_GameBackend    .NET API, worker, shared contracts, infrastructure and tests
DBA_GameAdmin      Blazor/MudBlazor GM admin console
DBA_GameWebsite    Next.js game website
DBA_GameLauncher   Tauri game launcher/updater
DBA_GameClient     Unreal Engine game client and dedicated server project
ops                Docker Compose, Nginx, Prometheus, Grafana, Loki and scripts
docs               Architecture and operations documentation
configs            Example game configuration files
scripts            Local helper scripts
ai                 AI task workspace
tools              Repository maintenance tools
```

All application directories are flattened at repository root and use the `DBA_` prefix.

## Verification

```powershell
cd DBA_GameBackend
dotnet build GameBackend.sln
dotnet test GameBackend.sln

cd ..\DBA_GameAdmin
dotnet build

cd ..\DBA_GameWebsite
npm install
npm run lint
npm run build

cd ..\DBA_GameLauncher
npm install
npm run build
cargo check --manifest-path src-tauri/Cargo.toml

cd ..\ops\docker
docker compose config
```

## Unreal Client

`DBA_GameClient/DivineBeastsArena.uproject` is the UE project. Install Unreal Engine, enable Git LFS, and run the normal Unreal project generation/build workflow for editor or dedicated-server targets.

## Mocked Extension Points

- Steam/EOS login providers are currently mock providers.
- UE Dedicated Server startup supports Docker/LocalProcess wiring, but a real server executable/image must be configured.
- Launcher CDN URLs use example placeholders.
- Website feedback currently validates and logs submissions locally through `/api/feedback`.
