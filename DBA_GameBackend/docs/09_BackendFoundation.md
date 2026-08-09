# 步骤 09：后端基础工程、YARP、PostgreSQL、Redis

## 当前收敛结论

当前后端不是空工程：`Game.Api` 已实现账号、角色、服务器目录、会话与对局相关端点；`Game.Application`、`Game.Infrastructure`、`Game.Shared` 和 `Game.ServerManagement` 已形成单向依赖，且只有 `GameDbContext` 与其 EF migrations。因而本步骤不拆出平行的 Auth、Character、World 或 GameSession 微服务。

阶段性部署边界如下：

| 目标边界 | 当前唯一 Owner | 状态 | 迁移方式 |
| --- | --- | --- | --- |
| `DBA.ApiGateway` | 新增 YARP 网关 | KEEP | 仅反向代理 `/api/*` 与 `/hubs/*`，负责限流、CorrelationId、访问日志、健康检查和指标。 |
| `DBA.AuthService` | `Game.Api` Auth + Account endpoints / Application Auth | ADAPT | 保持同一业务实现；未来提取时迁移端点与 Use Case，不复制认证逻辑。 |
| `DBA.CharacterService` | `Game.Api` Player endpoints / Application Characters | ADAPT | 保持 `CharacterBuildPolicy` 和 `GameDbContext.PlayerCharacters` 唯一。 |
| `DBA.WorldService` | `Game.Api` GameServer、Runtime、Session services | ADAPT | 服务目录与世界状态仍为单一模块化单体 Owner。 |
| `DBA.GameSessionService` | `Game.Api` Session、Match、Room services | ADAPT | Join Ticket、会话状态和分配逻辑不另建库或 DbContext。 |
| Shared Contracts | `Game.Shared` | KEEP | 所有 DTO、Options、错误码与 `ApiResponse` 继续唯一。 |
| Infrastructure | `Game.Infrastructure` | KEEP | `GameDbContext`、EF migrations、Redis 连接工厂唯一。 |

## API 合约

全工程选择 `ApiResponse<T>` 作为统一外部响应信封。成功和失败均返回 `success`、机器可读 `code`、安全 `message`、`data` 与时间戳；HTTP 状态码仍表达协议语义。`ErrorResponse` 仅作为内部构造对象，最终由 `ToApiResponse()` 输出信封。客户端依据 `code` 和 HTTP 状态码映射本地化文案，不解析英文消息。

## 数据与运行时

- PostgreSQL 是 `GameDbContext` 的权威持久层；EF migration 由 `db-migrate` 一次性容器执行。
- Redis 由 `RedisConnectionFactory` 唯一管理，用于短期会话、缓存及 Ticket。
- API 与 Worker 都使用 Npgsql 连接池配置（`Database:MinPoolSize`、`Database:MaxPoolSize`）及连接失败重试。
- `Game.Api` 继续拥有 `/health/live`、`/health/ready`、Serilog JSON 日志、OpenTelemetry/Prometheus；Gateway 提供同名健康端点与指标。
- Secret 仅来自环境变量或 `<配置键>_FILE`；不在 appsettings、Compose 或文档中提交真实密码。

## 开发 Compose

`docker-compose.dev.yml` 的默认链路为：

```text
客户端 -> dba-api-gateway:8080 -> game-backend:8080
                                  -> PostgreSQL / Redis
db-migrate（一次性） -> PostgreSQL
```

`game-backend` 不再直接暴露 HTTP 端口；开发流量统一经 Gateway。Compose 复用本地忽略的 `.env`，示例值只保留在 `.env.example`。默认服务包含 PostgreSQL、Redis、migration、既有 API+Worker 容器与 Gateway。

## 不可删除列表

- `GameDbContext`、`GameDbContextFactory`、`Game.Infrastructure/Database/Migrations/*`
- `Game.Shared/Common/ApiResponse.cs`、`ErrorResponse.cs`
- `Game.Infrastructure/Redis/RedisConnectionFactory.cs`
- `Game.Api` 账号、角色、会话、服务器目录端点与对应 Application Use Cases
- `Game.Worker`：维护和 Dedicated Server 生命周期仍依赖同一个基础设施层

## 下一阶段规则

当某个目标服务真正独立部署时，必须先迁移其 Application Use Case、Contracts 和数据库所有权，再更新 Gateway Cluster；不得从现有 API 复制 Controller、DbContext 或 DTO。
