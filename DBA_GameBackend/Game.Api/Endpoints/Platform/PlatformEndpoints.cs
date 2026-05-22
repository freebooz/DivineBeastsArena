/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：定义 HTTP 接口路由、鉴权要求、请求解析和统一响应，是后端功能对外暴露的入口。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using Game.Shared.Common;
using Game.Shared.Contracts.Platform;

namespace Game.Api.Endpoints.Platform;

public static class PlatformEndpoints
{
    public static void MapPlatformEndpoints(this IEndpointRouteBuilder app)
    {
        var group = app.MapGroup("/api/platform").WithTags("Platform");

        group.MapGet("/applications", GetApplications)
            .WithSummary("Get platform application catalog")
            .WithDescription("Returns the application structure, goals, responsibilities, integration points, and next steps for the DivineBeastsArenaPlatform workspace.");
    }

    private static IResult GetApplications()
    {
        var applications = new List<PlatformApplicationDto>
        {
            new(
                "game-api",
                "Game.Api",
                "Backend",
                "DBA_GameBackend/Game.Api",
                ".NET 10 ASP.NET Core Minimal API",
                "Provide account, character, room, match, runtime, settlement, operations, version, and game service APIs for clients and internal services.",
                "Implemented",
                "dotnet run --project DBA_GameBackend/Game.Api/Game.Api.csproj",
                "/health/live, /health/ready, /api/version",
                new[]
                {
                    "Authenticate real player accounts and guest sessions.",
                    "Persist characters, selections, sessions, matches, inventory, announcements, events, and operational data.",
                    "Expose internal runtime and dedicated game server registration APIs."
                },
                new[]
                {
                    "DBA_GameClient uses /api/auth and /api/account.",
                    "DBA_GameAdmin uses /api/admin, /internal/servers, and /api/platform.",
                    "Ops probes /health and /metrics."
                },
                new[]
                {
                    "Move remaining mock operation data to database-backed services.",
                    "Add authorization policies for admin-only and internal-only endpoints.",
                    "Add contract tests for account, character, and platform catalog APIs."
                }),
            new(
                "game-worker",
                "Game.Worker",
                "Backend Worker",
                "DBA_GameBackend/Game.Worker",
                ".NET 10 Worker Service",
                "Run platform background jobs such as stale server cleanup, session timeout handling, metrics rollups, mail delivery, and maintenance jobs.",
                "Skeleton",
                "dotnet run --project DBA_GameBackend/Game.Worker/Game.Worker.csproj",
                "Process heartbeat logs",
                new[]
                {
                    "Host scheduled jobs outside the API request path.",
                    "Keep runtime/session state consistent during server crashes or disconnects.",
                    "Produce daily operational aggregates."
                },
                new[]
                {
                    "Reads the same database and Redis state as Game.Api.",
                    "Feeds dashboards and admin analytics through persisted aggregates."
                },
                new[]
                {
                    "Wire Game.Infrastructure into the worker.",
                    "Implement stale dedicated-server cleanup.",
                    "Implement daily stats and retention rollup jobs."
                }),
            new(
                "game-admin",
                "DBA_GameAdmin",
                "Admin",
                "DBA_GameAdmin",
                "Blazor Server + MudBlazor",
                "Provide an internal operations console for dashboard data, players, matches, game servers, configs, inventory, audit logs, and platform health.",
                "Partially implemented",
                "dotnet run --project DBA_GameAdmin/GameAdmin.csproj",
                "Calls Game.Api /health/live",
                new[]
                {
                    "Inspect live game and operations data.",
                    "Expose platform structure and service readiness.",
                    "Provide GM workflows for support and inventory operations."
                },
                new[]
                {
                    "Consumes Game.Api through ApiBaseUrl.",
                    "Should authenticate admins through backend admin auth before write operations."
                },
                new[]
                {
                    "Replace remaining placeholder tables with API-backed data.",
                    "Add admin token storage and authorization headers.",
                    "Add safe confirmation flows for GM write actions."
                }),
            new(
                "game-website",
                "DBA_GameWebsite",
                "Public Web",
                "DBA_GameWebsite",
                "Next.js + React",
                "Provide the public website for game introduction, news, changelog, FAQ, feedback, and downloads.",
                "Partially implemented",
                "npm run build --prefix DBA_GameWebsite",
                "Static build health",
                new[]
                {
                    "Present the game brand and world setting.",
                    "Guide players to downloads and current news.",
                    "Collect player feedback and support signals."
                },
                new[]
                {
                    "Can read public announcements and launcher version information from Game.Api.",
                    "Shares download metadata with DBA_GameLauncher."
                },
                new[]
                {
                    "Move static content to CMS/API-backed news and FAQ data.",
                    "Add real download URLs from version management.",
                    "Add analytics and feedback submission integration."
                }),
            new(
                "game-launcher",
                "DBA_GameLauncher",
                "Launcher",
                "DBA_GameLauncher",
                "Tauri + React + Rust",
                "Provide desktop game launch, version checking, file verification, repair entry points, log access, and backend connectivity checks.",
                "Partially implemented",
                "npm run build --prefix DBA_GameLauncher",
                "Local command execution and manifest fetch",
                new[]
                {
                    "Check local client version and remote manifest.",
                    "Verify client files before launch.",
                    "Start the game executable with backend connection arguments."
                },
                new[]
                {
                    "Uses Game.Api version/download endpoints.",
                    "Starts DBA_GameClient executable installed on the local machine."
                },
                new[]
                {
                    "Add resumable download and patch application.",
                    "Persist launcher settings.",
                    "Display server announcements from Game.Api."
                }),
            new(
                "game-client",
                "DBA_GameClient",
                "Game Client",
                "DBA_GameClient",
                "Unreal Engine",
                "Provide the playable game client, dedicated server target, combat, character selection, UI, VFX, audio, and backend login integration.",
                "Implemented",
                "UnrealBuildTool DivineBeastsArenaEditor Win64 Development",
                "Editor target build and packaged client smoke test",
                new[]
                {
                    "Run the actual player-facing game experience.",
                    "Authenticate through Game.Api and persist selected characters through backend APIs.",
                    "Package client and dedicated server binaries for release operations."
                },
                new[]
                {
                    "Uses Game.Api for account, character, launcher, and service configuration.",
                    "Can be started by DBA_GameLauncher with backend connection arguments.",
                    "Dedicated server target reports runtime state to the backend."
                },
                new[]
                {
                    "Automate packaged client smoke testing.",
                    "Publish version manifests for launcher validation.",
                    "Document Unreal packaging profiles for client and server builds."
                }),
            new(
                "game-platform-ops",
                "GamePlatformOps",
                "Operations",
                "GamePlatformOps",
                "Docker Compose + Nginx + Prometheus + Grafana + Loki",
                "Provide local and deployment operations assets for database, cache, observability, reverse proxy, backup, restore, migration, and monitoring.",
                "Implemented",
                "docker compose -f GamePlatformOps/docker/docker-compose.yml up",
                "Prometheus, Grafana, Loki, service health probes",
                new[]
                {
                    "Run platform dependencies and observability stack.",
                    "Store deployment and maintenance scripts.",
                    "Provide dashboards for API, game server, PostgreSQL, and Redis."
                },
                new[]
                {
                    "Reads Game.Api metrics and logs.",
                    "Supports backend database migration and backup flows."
                },
                new[]
                {
                    "Parameterize environment-specific secrets.",
                    "Add smoke-test scripts after deployment.",
                    "Document one-command local startup."
                }),
            new(
                "game-platform-configs",
                "GamePlatformConfigs",
                "Game Configuration",
                "GamePlatformConfigs",
                "YAML",
                "Store source-controlled gameplay configuration examples for characters, maps, matchmaking, rewards, and skills.",
                "Implemented",
                "n/a",
                "Config validation pipeline needed",
                new[]
                {
                    "Provide readable gameplay tuning data.",
                    "Support future import into backend config publishing APIs.",
                    "Keep gameplay constants reviewable outside binary assets."
                },
                new[]
                {
                    "Game.Api config endpoints can publish and serve these definitions.",
                    "DBA_GameClient can consume versioned manifests generated from these files."
                },
                new[]
                {
                    "Add schema validation for every YAML file.",
                    "Add import/export tooling between YAML and database config tables.",
                    "Add version and environment metadata."
                })
        };

        return Results.Ok(ApiResponse<PlatformApplicationsResponse>.Ok(new PlatformApplicationsResponse(
            DateTimeOffset.UtcNow,
            applications)));
    }
}

