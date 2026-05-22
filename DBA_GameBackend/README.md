# DBA_GameBackend

DBA_GameBackend is the .NET backend for the UE multiplayer game platform. It contains:

- `Game.Api`: ASP.NET Core Minimal API, Auth, Player, Config, Room, Match, Session, Runtime, Settlement, Inventory and Admin APIs.
- `Game.Worker`: background jobs and Game Server Manager.
- `Game.Shared`: DTOs, common responses and options.
- `Game.Infrastructure`: EF Core DbContext, entities, Redis and auth infrastructure.
- `Game.Api.Tests`: unit tests for validators and API-adjacent rules.
- `Game.IntegrationTests`: persistence and integration-oriented tests.

## Implemented Modules

- Auth: `/api/auth/guest-login`, `/api/auth/dev-login`, `/api/auth/refresh`, `/api/auth/logout`, `/api/auth/me`, Steam/EOS mock providers. Access tokens default to 30 minutes; refresh tokens default to 30 days and are stored as hashes.
- Player: profile, settings, statistics, public profile and unlocks. Nicknames are 2-16 chars and allow Chinese, English, digits and underscore.
- Config: client manifest/config/bundle plus admin create, update, validate, publish and rollback. Published configs generate checksums.
- Room / Match / Session: create/join/ready/start rooms, matchmaking tickets, session creation, connection info and reconnect token.
- Game Server Manager: `Game.Worker/ServerManager` allocates UDP ports, creates server/runtime tokens, supports LocalProcess and Docker launch modes, handles timeout cleanup, release and kill.
- Runtime API: `/runtime/servers/*` and `/runtime/matches/results` validate runtime tokens, update server/session state and call settlement.
- Settlement / Inventory: idempotent match results, raw JSON persistence, player statistics, exp/level update, match rewards, inventory logs and GM inventory operations.
- UE client login bridge: `/api/account/characters` remains for existing DBA_GameClient builds, and `/api/players/me/characters` is available as the standard authenticated character API. Created characters and selected character state are persisted in PostgreSQL.

Internal endpoints require:

```http
X-Internal-Api-Key: <InternalApi:Key>
```

## Local Run

```bash
dotnet restore GameBackend.sln
dotnet build GameBackend.sln
dotnet test GameBackend.sln
dotnet run --project Game.Api/Game.Api.csproj
```

Important configuration keys:

- `Database:ConnectionString`
- `Redis:ConnectionString`
- `Jwt:Secret`
- `InternalApi:Key`
- `GameServerManager:*`

## Docker

The backend Dockerfiles and production compose file are kept in this directory.

```bash
cp .env.example .env
docker compose --env-file .env config
docker compose --env-file .env up -d
```

To enable the production HTTPS edge gateway, set `PUBLIC_DOMAIN` and `ACME_EMAIL` in `.env`, make sure DNS points to the server public IP, then start the `edge` profile:

```bash
docker compose --env-file .env --profile edge up -d
```

Observability stack:

```bash
docker compose --env-file .env -f docker-compose.observability.yml config
docker compose --env-file .env -f docker-compose.observability.yml up -d
```

Grafana is bound to `127.0.0.1:3001` by default. Prometheus scrapes the real `game-api:8080/metrics` endpoint and loads the `DBA Game API` dashboard automatically.

Operational scripts:

- `scripts/migrate-db.sh`: run EF Core migrations and exit.
- `scripts/backup-postgres.sh`: create a compressed PostgreSQL backup.
- `scripts/restore-postgres.sh <backup.sql.gz>`: restore a compressed PostgreSQL backup.
- `scripts/rehearse-backup-restore.sh`: create a backup, restore it into a temporary database, run a sanity query, then drop the temporary database.
- `scripts/run-load-tests.sh`: run k6 login, matchmaking and server allocation load tests.
- `scripts/check-admin-rbac.sh`: verify admin RBAC behavior through real HTTP requests.
