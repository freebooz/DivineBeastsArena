# DivineBeastsArena 客户端与独立服务器代码审查

## 开发纲领

项目级总控提示词位于 `docs/Development/ZodiacArena_UE5_8_Codex_总控提示词.md`。继续任何 Codex 开发、审查、代码生成或验证任务前，应先以该文档作为长期指导，并结合根目录 `AGENTS.md` 的 MCP 与安全约束执行。

## 审查范围

本次审查以持续开发 `DBA_GameClient` 为主，覆盖 UE 客户端、Dedicated Server Target、账号/角色联机链路，以及后端中直接支撑 Dedicated Server 分配、注册、心跳和结算的接口。未对 Admin、Website、Launcher 做深入审查，仅记录与客户端分发或联调相关的风险。

## 主要结论

当前仓库已经具备客户端、服务端 Target、GameCore 账号链路、后端 runtime API 与服务器编排服务的基本骨架。账号注册后 `LoginType` 被强制改为 Guest 的问题已在 `DBAOnlineAccountService.cpp` 中移除，`EndpointMissing` 也不再允许进入 Mock fallback，并已有测试覆盖。但 Dedicated Server 的真实联调仍然存在几个阻塞点：内部 runtime/settlement 接口鉴权边界不完整、服务器启动参数未做安全转义、Shipping/Server 模块依赖边界需要收紧，以及 Mock 分配仍可能掩盖真实 Dedicated Server 未启动的问题。

## 高优先级问题

### 1. 内部 runtime 与 settlement 接口缺少统一鉴权边界

证据：
- `DBA_GameBackend/Game.Api/Endpoints/Runtime/RuntimeEndpoints.cs:35` 暴露 `/runtime/servers`，通过 runtime token 校验服务器回调。
- `DBA_GameBackend/Game.Api/Endpoints/Runtime/RuntimeEndpoints.cs:45` 暴露 `/internal/runtime`，未看到 `.RequireAuthorization()` 或内部服务认证。
- `DBA_GameBackend/Game.Api/Endpoints/Settlement/SettlementEndpoints.cs:26` 暴露 `/internal/settlement`。
- `DBA_GameBackend/Game.Api/Endpoints/Settlement/SettlementEndpoints.cs:28` 允许直接提交比赛结果，但没有 runtime token、JWT 或内部签名校验。

影响：
真实联机阶段，任何能访问 API 的调用方都可能伪造服务器分配、心跳、结算或查询内部状态。对 Dedicated Server 联调来说，这会让“服务端是否真的启动并可信上报”变得不可验证。

建议：
先统一内部接口策略。`/runtime/servers/*` 保留 runtime token 模式；`/internal/runtime/*` 和 `/internal/settlement/*` 至少加服务间 JWT、固定内部 API key、mTLS 或仅内网网关访问约束。结算入口优先收敛到带 `RuntimeToken` 校验的 `/runtime/matches/results`。

### 2. Dedicated Server 启动参数未做参数转义

证据：
- `DBA_GameBackend/Game.ServerManagement/DedicatedServers/DedicatedServerOrchestrator.cs` 的 `BuildServerArgs` 直接拼接 `-sessionId`、`-serverId`、`-mapId`、`-mode`、`-backendUrl`、`-runtimeToken`。

影响：
当前 `mode`、`mapId`、`backendUrl` 如果来自配置或请求，包含空格、引号或 shell 特殊字符时会破坏 UE 参数解析；Docker 模式下风险更高，因为 `docker run ... {args}` 也是字符串拼接。

建议：
本地进程启动改用 `ProcessStartInfo.ArgumentList`。Docker 模式也应逐项构造参数，避免把整条命令作为单个字符串传入。UE 侧同步确认参数名：`serverId`、`sessionId`、`runtimeToken`、`backendUrl`、`port`。

### 3. Mock Server Allocation 仍可能遮蔽真实服务器缺失

证据：
- `DedicatedServerOrchestrator.LaunchAsync` 在 `UeServerExecutablePath` 缺失且 `AllowMockServerAllocation=true` 时，记录 `LAUNCH_SKIPPED_MOCK` 并保持分配流程继续。
- `RequiredOptionsValidator` 已要求 Production 禁用 Mock，但开发/联调环境仍会出现“分配成功但没有 UE Server 进程”的假阳性。

影响：
客户端可能拿到房间或 session 连接信息，但 Dedicated Server 实际未启动，后续表现为超时、无法 Travel、心跳缺失或角色进入失败。

建议：
联调阶段默认设置 `GameServerManager__AllowMockServerAllocation=false`，并要求 `UE_SERVER_EXECUTABLE_PATH` 指向真实 `DivineBeastsArenaServer` 包。保留 Mock 仅用于后端单测和 API 合约演示。

## 中优先级问题

### 4. 客户端 AutoLogin 的 Mock fallback 没有复用 Shipping 防线

证据：
- `DBA_GameClient/Source/GameCore/Private/GameCore/Account/DBAOnlineAccountService.cpp:272` 的 `ShouldFallback` 在 Shipping 返回 false。
- 同文件 `:446` 在 `RefreshToken` 为空时直接检查 `OnlineConfig.bAllowMockFallback` 并调用 `FallbackAutoLogin`，未经过 `ShouldFallback`。

影响：
如果 Shipping 包中配置误开 `bAllowMockFallback`，自动登录路径仍可能进入本地 Mock 账号，和真实后端账号链路混淆。

建议：
所有 fallback 入口统一经过一个带 `UE_BUILD_SHIPPING` 防线的方法。`AutoLogin` 的本地空 token 分支也应显式禁止 Shipping fallback。

### 5. Server Target 仍包含较多客户端表现模块依赖

证据：
- `DivineBeastsArena.Build.cs` 在非 Server 时排除了 `RenderCore`、`RHI`、`AudioMixer`，但 `MediaAssets` 始终加入。
- Server Target 仍加载 `DivineBeastsArena`、`GameCore`、`GameMoba` 三个模块。

影响：
Dedicated Server 包体、加载时间和无头运行稳定性可能受到 UI、媒体、表现资源依赖影响。后续若某些 UMG、Media、Niagara 资源在 Server 初始化路径被引用，会产生 cook 或运行时问题。

建议：
继续把战斗规则、会话、队列、账号契约沉到 `GameCore`/`GameMoba`，将 UI、视频、VFX、展示舞台严格隔离在客户端路径。`MediaAssets` 建议按 `Target.Type != TargetType.Server` 条件加入。

### 6. 角色链路返回空列表无法区分“无角色”和“请求失败”

证据：
- `GetCharacterList` 在 HTTP 失败或 JSON 解析失败时返回空数组。
- `SelectCharacter` 失败时返回无效 `FDBACharacterId`。

影响：
UI 可以避免误造成功，但仍可能把后端 401、404、500、解析失败都表现为“没有角色”或“选择失败”。联调时排查成本较高。

建议：
后续把角色列表回调升级为带错误码/错误消息的结果类型，例如 `FDBACharacterListResult`。短期至少在 UI 层展示最近一次账号服务错误。

## 已有正向基础

- `CanFallbackToMock` 已不再允许 `EndpointMissing` fallback，并有 `DBAOnlineAccountServiceTests.cpp` 覆盖。
- 登录 JSON 解析已有 `LoginType` 测试，Email 注册链路不会再被 JSON 形状误判为 Guest。
- `GetCharacterList` 和 `SelectCharacter` 已不再把 404 自动转成 profile fallback 或选择成功。
- Dedicated Server Target 已定义 `DBA_SERVER=1`、`DBA_CLIENT=0`，并启用 Shipping 日志，利于服务器侧排障。

## 建议开发顺序

1. 先打通真实 Dedicated Server 包路径：配置 `UE_SERVER_EXECUTABLE_PATH`，关闭 Mock 分配，启动后确认 `/runtime/servers/register`、`ready`、`heartbeat` 全链路。
2. 收紧 runtime/settlement 内部接口鉴权，保证只有可信 Dedicated Server 或内部 Worker 能提交状态与结算。
3. 修复服务器启动参数构造，避免 map/mode/backendUrl 破坏命令行。
4. 收敛客户端 Mock fallback，特别是 `AutoLogin` 的 Shipping 边界。
5. 清理 Server Target 依赖，逐步把纯客户端 UI/VFX/Media 依赖从服务端包中剥离。
6. 给账号-角色-匹配-进入地图链路补 UE 自动化测试与双客户端手工联调脚本。

## 推荐验证命令

```powershell
# 后端基础验证
cd DBA_GameBackend
dotnet build GameBackend.sln
dotnet test GameBackend.sln --no-build

# UE 服务器构建示例，按本机 UE 路径调整
& "D:\UnrealEngine-5.8.0-release\Engine\Build\BatchFiles\Build.bat" DivineBeastsArenaServer Win64 Development -Project="E:\work\Game\DivineBeastsArena\DBA_GameClient\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE

# 双客户端参数示例
-DBASaveSlotSuffix=ClientA -DBAGuestDeviceId=ClientA -BackendBaseUrl=http://localhost:8080
-DBASaveSlotSuffix=ClientB -DBAGuestDeviceId=ClientB -BackendBaseUrl=http://localhost:8080
```

