# Architecture

## Overview

`DivineBeastsArenaPlatform` is a flattened workspace for the Unreal game client, backend services, admin dashboard, public website, desktop launcher, operations assets, shared configs, scripts, and documentation.

## Applications

- `DBA_GameClient`: Unreal Engine game client and dedicated server targets.
- `DBA_GameBackend`: .NET backend solution containing `Game.Api`, `Game.Worker`, `Game.Shared`, `Game.Infrastructure`, and tests.
- `DBA_GameAdmin`: Blazor Server admin dashboard.
- `DBA_GameWebsite`: Next.js public website.
- `DBA_GameLauncher`: Tauri desktop launcher.
- `ops`: Docker Compose, Nginx, Prometheus, Grafana, Loki, migration, backup, and restore assets.
- `configs`: Source-controlled gameplay configuration examples.
- `docs`: Architecture, deployment, operations, review, and release documentation.
- `scripts`: Build, test, audit, and maintenance scripts.

## Platform Catalog

`Game.Api` exposes `GET /api/platform/applications` as the shared catalog of platform applications. The endpoint describes each app's directory, runtime, goal, responsibilities, integration points, health check, run command, and next steps. `DBA_GameAdmin` uses this endpoint on `/platform` to make the workspace structure visible to developers and operators.

## Tech Stack

- Game: Unreal Engine
- API: ASP.NET Core 10, EF Core, PostgreSQL, Redis
- Auth: JWT Bearer
- Observability: OpenTelemetry, Prometheus, Serilog
- Frontend: Blazor, Next.js, Tauri
