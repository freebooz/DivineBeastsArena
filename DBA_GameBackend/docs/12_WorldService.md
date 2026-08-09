# 步骤 12：WorldService 区服目录

## 审计与迁移

现有 `GameServerInstance`、`game_server_instance` 及 `/internal/game-servers` 只表示短生命周期的 Dedicated Server 单局实例，包含端口、进程和运行时令牌相关信息。它们继续由编排系统拥有，禁止作为玩家选服目录或公开给客户端。

本步骤新增独立的 `game_servers` 逻辑区服目录：

| 现有对象 | 标记 | 迁移方式 |
| --- | --- | --- |
| `GameServerInstance` / 内部端点 | KEEP | 仅 DS 编排、会话分配与运维使用。 |
| `GameServerDirectoryEntry` / `game_servers` | KEEP | 玩家前台唯一权威区服目录。 |
| `ServerDirectoryService` | KEEP | 唯一目录查询、筛选、排序和缓存入口。 |
| `UDBAServerDirectorySubsystem` | ADAPT | UE 只读取稳定 `ServerId` 与展示 DTO；Flow 认证成功后请求目录。 |

## 公开契约

`GET /api/v1/servers?region={region}&clientVersion={version}&platform={platform}`

返回统一 `ApiResponse`，每项包含 `ServerId`、`Name`、`Region`、`Status`、`Population`、`Recommended`、`MaintenanceMessage`、`MinClientVersion` 和 `CanSelect`。状态 token 为 `Online`、`Busy`、`Full`、`Maintenance`、`Offline`。

- 仅 `Online` 与 `Busy` 的 `CanSelect=true`。
- 排序：可选择优先、推荐优先、人口较低优先、名称稳定排序。
- `region`、`platform`、`clientVersion` 均可选；平台支持 `ALL` 或精确平台，版本低于 `MinClientVersion` 的区服不返回。

## 权威性与缓存

PostgreSQL `game_servers` 是权威源，开发环境通过 `DevelopmentDataSeeder` 写入逻辑区服样本。Redis 缓存键包含全部筛选维度，TTL 由 `ServerDirectory:CacheTtlSeconds` 控制（生产默认 30 秒，开发默认 5 秒）。

目录写入管理能力在后续步骤接入时必须调用 `IServerDirectoryService.InvalidateCacheAsync`；主动失效失败时 TTL 自动过期兜底。Redis 不可用时服务记录中文告警并直接查询 PostgreSQL，目录可继续读取。

## UE 边界

`UDBAServerDirectorySubsystem` 是 UE 前台唯一目录读取入口，使用现有 `UDBAApiClientSubsystem`，不让 Widget 处理 HTTP 或原始 JSON。登录、注册和自动登录成功转入 `ServerSelect` 后，Flow 依据外部服务配置请求目录；选服时只接受已缓存且 `CanSelect=true` 的稳定 `ServerId`。

## 数据库迁移与验证

- 迁移：`20260809090000_AddGameServersDirectory`。
- 服务层测试覆盖推荐排序、维护/离线不可选与客户端版本过滤。
- 现有 `WebApplicationFactory` 在本仓库会于 Host 构建前退出，因此未将该既有测试基础设施作为本步骤 API 验收依据；端点映射随 `Game.Api` 编译通过，业务测试采用 InMemory `GameDbContext` 执行。
