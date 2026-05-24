# DBA GameAdmin

Angular 18+ GM management console. The current project uses Angular 21, which satisfies the 18+ technology requirement, and calls `Game.Api` through `/api/admin/*`, `/api/live-ops/status`, and `/api/platform/applications`.

## Build

```bash
npm ci
npm run build
```

## Local Development

```bash
npm start
```

The production build uses same-origin API calls. During development, route `/api` to `Game.Api` through a reverse proxy or local gateway.

## Pages

- Dashboard: production operations metrics and platform health.
- Players and player detail.
- Matches and match detail.
- Dedicated Server management, including high-risk kill operations.
- Game configuration and client version management.
- Player feedback, support tickets, and audit logs.
- Platform application structure and run commands.
