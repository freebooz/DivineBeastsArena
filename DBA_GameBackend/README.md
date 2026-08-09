# DBA_GameBackend

神兽竞技场的 .NET 10 后端，使用 ASP.NET Core、PostgreSQL、Redis、YARP/网关基础设施与 OpenTelemetry。`GameDbContext` 是唯一 EF Core DbContext，PostgreSQL 是权威持久层，Redis 只保存缓存、短期会话与一次性 Ticket。

## 工程边界

- `Game.Api`：外部 REST/JSON、鉴权、限流、统一错误响应和健康检查。
- `Game.Application`：Auth、Character、Session 等用例与领域契约。
- `Game.Infrastructure`：EF Core、PostgreSQL、Redis、认证与 Dedicated Server 基础设施。
- `Game.Shared`：跨项目 DTO、错误码和配置类型。
- `Game.Worker`：后台维护任务。
- `Game.ServerManagement`：Dedicated Server 实例编排。
- `Game.Api.Tests`、`Game.IntegrationTests`：工程契约与集成测试项目；是否执行遵循人工审核策略。

## 生产 API 基线

- Auth：`/api/v1/auth/*`，支持注册、登录、刷新、登出、当前账号和独立玩家名生成。
- Server Directory：`GET /api/v1/servers`。
- Character：`/api/v1/characters`，以 `ServerId` 为角色目录边界。
- Enter World：`POST /api/v1/game/enter`，签发短 TTL、一次性 GameTicket。

`/api/auth` 中已有 v1 successor 的路径、`/api/account/characters` 与 `/api/players/me/characters` 仅为旧客户端兼容。它们返回 `Deprecation: true` 和 successor `Link` 响应头；新客户端不得继续调用。

首次开户后，UE 使用 AccessToken 自动调用 `/api/v1/auth/player-name/generate`，由服务端从 `PlayerGameName` 配置字库生成 3–5 个汉字并保存到 `player_profile.nickname`。`/api/v1/auth/login` 只认证，不调用玩家名接口。账号登录名、玩家名和角色名是三个不同概念。

## 工程检查命令

```powershell
dotnet restore GameBackend.sln
dotnet build GameBackend.sln
dotnet test GameBackend.sln --no-build
```

## 必需配置

- `Database:ConnectionString`
- `Redis:ConnectionString`
- `Jwt:Secret`
- `InternalApi:Key`
- `PlayerGameName:*`
- `DedicatedServerOrchestration:*`

真实密码、JWT 密钥、内部 API Key 和证书只能通过环境变量、User Secrets 或部署 Secret 提供，不得提交到仓库。

详细接口见 [docs/api.md](./docs/api.md)，数据库最终基线见 [docs/29_DatabaseBaseline.md](./docs/29_DatabaseBaseline.md)。
