# 解决方案审查与生产部署计划

## 审查结论

当前仓库已经具备最小可上线外围应用的主体形态：后端 API、Worker、GM 后台、官网、启动器、UE 客户端、CI/CD、Docker Compose、观测和备份脚本都已存在。后端自动化测试和生产配置基础较完整，最大剩余风险集中在真实 UE Dedicated Server 包、真实客户端包、预生产部署资源和仓库权限配置。

## 应用审查

### DBA_GameBackend

定位：游戏核心后端、会话、匹配、配置、结算、背包、Runtime API、Dedicated Server 编排。

已具备：

- JWT 登录、刷新、登出和开发账号。
- 玩家资料、角色创建/选择、配置发布、房间、匹配、会话、结算、背包。
- Dedicated Server Orchestration LocalProcess 模式、端口分配、进程启动、维护超时。
- Runtime API 支持 Dedicated Server register、ready、heartbeat、player-joined、player-left、match started/ended/results。
- Prometheus metrics、health checks、Serilog/OpenTelemetry、Docker Compose。

主要缺口：

- 需要真实 UE Server 包完成端到端验证。
- 需要预生产压测基线。
- 需要镜像扫描和依赖扫描纳入流水线。

### DBA_GameClient

定位：UE 客户端和 Dedicated Server。

已具备：

- 登录、角色创建/选择、主大厅 UI 流程。
- 后端插件封装 Auth、Player、Config、Room、Match、Session。
- Session connection 正确携带 `SessionId`、`PlayerSessionToken`、`PlayerId`。
- Dedicated Server Runtime API 上报骨架已接入。

主要缺口：

- 已按本机 UE 路径完成 UE C++ Editor / Server 目标编译验证。
- 需要真实客户端包验证登录、角色、匹配、进服。
- 需要真实 Dedicated Server 包验证心跳、玩家进出和回收。

### DBA_GameAdmin

定位：GM 管理后台。

已具备：

- Angular 18+ 后台页面、登录状态管理、Admin API Client。
- 玩家、对局、服务器、配置、背包、审计日志页面。
- 高危操作 reason 和二次确认。
- Token 过期跳转和后端 RBAC。

主要缺口：

- 需要预生产账号执行真实运营验收。
- 建议补充 Playwright 或 bUnit UI 自动化。

### DBA_GameWebsite

定位：官网、下载、公告、反馈。

已具备：

- Next.js + TypeScript + Tailwind 页面。
- 下载页读取后端 manifest。
- 反馈表单基础校验。

主要缺口：

- 反馈接口当前仍是轻量实现，需要接入后端持久化/工单。
- 生产域名、SEO、隐私政策和客服流程需要最终确认。

### DBA_GameLauncher

定位：游戏启动器、更新器、修复工具。

已具备：

- manifest 获取、本地版本比较、下载、SHA256 校验、修复和启动命令。
- Tauri 图标和标题已替换。

主要缺口：

- 需要真实 CDN 文件、真实 SHA256 和断点续传场景验证。
- 需要签名、自动更新策略和安装包发布流程。

## 生产部署级开发计划

### P0：上线阻塞项

1. 构建真实 UE Dedicated Server 包。
2. 配置 `UE_SERVER_EXECUTABLE_PATH` 并在预生产环境启动 Worker。
3. 验证分配服务器、Runtime register/ready/heartbeat、player-joined/player-left。
4. 构建真实客户端包并验证启动器下载、校验、修复和启动。
5. 验证登录、创建角色、选择角色、匹配、获取连接信息、进服。

### P1：生产运维闭环

1. 执行 PostgreSQL 备份恢复演练。
2. 执行 k6 登录、匹配、服务器分配压测。
3. 启用 Grafana 看板、Prometheus 告警、Loki 日志查询。
4. 配置 HTTPS 域名、CORS、CDN 下载域名。
5. 设置 GitHub `main` 分支保护并要求 `solution-ci`。

### P2：安全和合规

1. 将依赖扫描和容器镜像扫描加入 CI。
2. 禁止生产环境 dev-login 和 mock fallback。
3. 梳理后台权限矩阵和审计留存策略。
4. 完成隐私政策、用户协议、客服反馈闭环。

### P3：运营增强

1. 后台补充配置差异预览和发布审批。
2. 官网公告和 changelog 接入后台发布。
3. 启动器增加灰度渠道、强更策略、下载镜像选择。
4. Dedicated Server 扩展 Docker/Agones/Kubernetes 调度。

## 执行策略

- 当前机器可执行的任务：后端测试、Node 构建、Docker Compose config、文档、脚本、接口补强。
- 当前机器不可完全执行的任务：UE 编译、真实客户端包和 Dedicated Server 包验证、GitHub 分支保护启用、真实预生产部署。
- 不可执行项已经沉淀为脚本和文档，等拿到对应环境后按顺序运行。

## 当前分阶段修改计划

### P0：仓库基线修复与验证

目标：让仓库说明、解决方案边界和本机可执行验证都清晰可信。

待办：

1. 修正根 README 的仓库级应用矩阵和解决方案边界说明。
2. 说明中文文档 UTF-8 读取方式，避免 Windows PowerShell 默认编码导致误判为乱码。
3. 执行后端、GM 后台、官网、启动器和 Docker Compose 基础验证。

验收标准：

- README 能清楚说明 `.sln` 只覆盖后端，其他应用独立构建。
- `dotnet build/test`、`npm run build`、`cargo check`、`docker compose config` 的结果被记录。

执行记录：

- 已更新根 README，补充仓库级应用矩阵、解决方案边界、阶段计划和 UTF-8 读取说明。
- 已执行 `.\scripts\production-preflight.ps1 -SkipUnreal`，后端测试、Admin 构建、官网构建、启动器前端构建、启动器 `cargo check`、后端 Compose config、观测 Compose config 均通过。
- 已修复启动器 Rust 层编译问题：版本解析失败时回退到 `0.0.0`，下载流改用 `std::io::copy`，并修正 Tauri 入口 crate 名称。
- 已确认 UE 工具路径：`D:\UnrealEngine-5.8.0-release\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe`。
- 已更新预检脚本，使其支持 `$env:UNREAL_ENGINE_ROOT` 或本机默认 UE 路径。

### P1：后端边界与生产配置

目标：降低后端 API、Worker 和共享业务之间的耦合风险，补齐生产配置失败前置校验。

待办：

1. 梳理 `Game.Api` 引用 `Game.Worker` 的真实原因。
2. 如果只是复用 Dedicated Server 编排契约或配置，优先抽到共享层。
3. 为 `Jwt:Secret`、`InternalApi:Key`、数据库、Redis、`GameServerManager` 兼容配置节增加启动期配置校验。
4. 补充登录、角色、匹配、房间、Runtime 上报、结算、背包奖励主链路测试。

验收标准：

- API 与 Worker 的依赖关系有明确结论或已完成拆分。
- 生产关键配置缺失时启动失败且错误信息可读。
- 后端测试覆盖关键主流程。

执行记录：

- 已新增 `Game.ServerManagement` 共享项目，将 Dedicated Server 编排服务、分配命令和管理 DTO 从 `Game.Worker` 中拆出。
- `Game.Api` 已改为依赖 `Game.ServerManagement`，不再直接引用 `Game.Worker`。
- `Game.Worker` 保留 `DedicatedServerMaintenanceWorker` 后台循环，并依赖共享的服务器管理服务。
- 已新增 `RequiredOptionsValidator`，集中校验 `Database`、`Redis`、`Jwt`、`InternalApi:Key` 和 Dedicated Server 编排配置。
- 已补充配置校验单元测试，后端测试从 15 个增加到 19 个：`Game.Api.Tests` 18 个、`Game.IntegrationTests` 1 个，全部通过。
- 已更新 API/Worker Dockerfile 的项目复制清单，包含新增的 `Game.ServerManagement` 项目。
- 已新增 `scripts/validate-unreal-module-boundaries.ps1`，自动检查 `GameCore -> GameMoba -> DivineBeastsArena` 的依赖方向与源码 include 边界，并已接入 `scripts/production-preflight.ps1`。
- 已新增 `scripts/validate-unreal-baseline-entrypoints.ps1`，自动检查共享日志通道、基础 DataAsset 基类与原生 GameplayTag 注册入口是否仍位于正确层级，并已接入 `scripts/production-preflight.ps1`。
- 已执行 `.\scripts\production-preflight.ps1 -SkipNode -SkipDocker -SkipCargo -SkipUnreal`，确认新的 Unreal 模块边界校验、baseline entrypoint 校验、source guardrail、backend `dotnet test` 与 Admin 构建均通过。
- 已新增 `scripts/diagnose-local-ue-online-readiness.ps1`，只读检查 Docker、Compose、backend health、UE Editor 路径与项目路径。
- 已执行联机就绪诊断，当前结果显示：`.env` 存在但关键 secret 仍为占位值，`localhost:8080/8083` 的 `/health/live` 不可达，本地 `postgres/redis` 容器仅处于 `Created`，说明 P2 的直接阻塞点仍是 backend runtime 未启动成功。
- 已继续排查本地 Compose 运行态：`postgres` 与 `redis` 已可进入 `healthy`，`game-api` 可构建但在 `Production + LocalProcess` 下因缺少挂载的 `/opt/game-server/GameServer.sh` 被启动期配置校验拒绝。
- 已新增 `scripts/start-local-backend-compose.ps1`，为本地 UE 联机验证提供 Development Compose 启动入口，默认先执行 EF migrations，并启用 mock server allocation，让 `start-local-ue-validation.ps1` 负责拉起本机 Dedicated Server；生产环境仍保持禁止 mock allocation 和要求真实 server executable。
- 已执行 `scripts/start-local-backend-compose.ps1 -SkipBuild`，确认 `game-api` 进入 `healthy`，`game-worker` 已启动，`postgres` 与 `redis` 均为 `healthy`。
- 已执行 `scripts/production-smoke-backend.ps1 -BaseUrl http://localhost:8080`，live、ready、version、launcher manifest、metrics 均通过。
- 已执行 `scripts/local-backend-flow-smoke.ps1 -BaseUrl http://localhost:8080`，guest 登录、角色、房间、会话、server allocation、connection token 与 matchmaking ticket 主流程均通过；当前 allocation 使用本地 Development mock server allocation，尚未证明真实 UE Dedicated Server 启动与心跳闭环。
- 已修复 `scripts/start-local-ue-validation.ps1` 的 PowerShell 解析错误和 release cleanup 415 问题，并执行 `scripts/start-local-ue-validation.ps1 -BaseUrl http://localhost:8080 -SkipClientLaunch`，确认后端分配会话、启动 UE Dedicated Server、等待 server log、释放 allocation、停止 server 进程的 server-only 联调路径通过。
- 已继续修复 `scripts/start-local-ue-validation.ps1` 的双客户端验证：新增 `ClientValidationWaitSec`，自动读取本地 `UnrealEditor.exe -server` 实际 `IpNetDriver` 监听端口并设置 `ClientConnectPort`，解决后端分配端口 `7777/7778` 与 Editor server 实际监听 `7878` 的偏移问题。
- 已将双客户端日志门槛固化进 `scripts/start-local-ue-validation.ps1`：服务器必须出现两次 `Join succeeded`、两次 `runtime/servers/player-joined` 业务 OK，且不得出现业务 ERROR；客户端 A/B 必须完成 `TravelCompleted Pending net game travel completed`，且不得出现 `ConnectionTimeout`、`NetworkFailure`、`Fatal error` 或 `Assertion failed`。
- 已执行 `scripts/start-local-ue-validation.ps1 -BaseUrl http://localhost:8080 -ClientValidationWaitSec 45`，确认本地 Development 后端、UE Editor Dedicated Server、双客户端连接、Runtime register/ready/heartbeat/player-joined、allocation release 与进程清理均通过。
- 已新增 `scripts/diagnose-unreal-packaged-server-readiness.ps1`，用于只读区分“已编译 raw server target”和“已 cook/stage 的 packaged server 包”。
- 已新增 `scripts/package-unreal-dedicated-server.ps1`，封装 `RunUAT BuildCookRun` 的 WindowsServer build/cook/stage/archive 入口，并支持 `-WhatIf` 预览命令。
- 已修正 `scripts/smoke-unreal-dedicated-server.ps1 -UsePackagedServer`：不再默认运行 `DBA_GameClient/Binaries/Win64/DivineBeastsArenaServer.exe` 裸目标，而是要求 staged/cooked server executable 和内容。
- 已执行 packaged server 轻量诊断：`DBA_GameClient/Binaries/Win64/DivineBeastsArenaServer.exe` 存在，但 `.tmp/packaged-server`、`DBA_GameClient/Saved/StagedBuilds` 与 `Artifacts/UnrealServer` 中均未发现 staged server 包，因此当前生产化 server 包验证仍未就绪。
- 已执行 raw packaged smoke 旧路径并确认失败根因：裸 `DivineBeastsArenaServer.exe` 会因 premade asset registry/package serialization 断言崩溃，说明不能把编译产物直接当作生产包使用。
- 已执行 `scripts/package-unreal-dedicated-server.ps1 -NoArchive -SkipBuild -SkipCook -NoPak -WhatIf`，确认 UAT 入口可生成预览命令；实际 cook/stage/package 尚未执行。
- 已执行 `scripts/package-unreal-dedicated-server.ps1` 的实际 UAT 打包。首次默认同时 cook `Windows+WindowsServer` 时失败，Cook 日志显示 6 个 `LogNiagara: Error`，集中在 `/Game/ProjectileHitVFX/NS/NS_MeteoriteStraight` 的 Grid3D Gas 参数；这说明客户端 cook 仍有 Niagara 资源阻塞。
- 已将 `scripts/package-unreal-dedicated-server.ps1` 默认改为 server-only `-noclient`，保留 `-IncludeClientCook` 用于后续客户端 cook 验证。
- 已执行 `scripts/package-unreal-dedicated-server.ps1 -SkipBuild`，确认 `WindowsServer` cook/stage/pak/archive 成功，输出位于 `.tmp/packaged-server/WindowsServer`，并生成 `DivineBeastsArena-WindowsServer.pak` 与 `.utoc`。
- 已执行 `scripts/diagnose-unreal-packaged-server-readiness.ps1`，确认 packaged server readiness 通过，发现 staged server executable 与 cooked content。
- 已执行 `scripts/smoke-unreal-dedicated-server.ps1 -UsePackagedServer -RunSeconds 20 -Port 17779`，确认 packaged Dedicated Server 可启动并完成 `/Game/Maps/Lobby/LobbyMap` 加载。
- 已修复 `scripts/start-local-ue-validation.ps1 -UsePackagedServer` 的 staged package 解析、工作目录和 packaged server 参数顺序问题。此前 packaged server 会收到 `/Game/Maps/Lobby/LobbyMap-log` 或 `/Game/Maps/Lobby/LobbyMap-abslog=...`，需要用户确认默认地图弹窗后才继续；脚本现在将 packaged server 开关置于地图参数前，并扫描 `LobbyMap-log`、`LobbyMap-abslog` 与默认地图弹窗文本，防止误判通过。
- 已执行 `scripts/start-local-ue-validation.ps1 -BaseUrl http://localhost:8080 -UsePackagedServer -SkipClientLaunch`，确认本地后端可分配 session、启动 packaged Dedicated Server、加载 LobbyMap、释放 allocation 并清理进程。
- 已执行 `scripts/start-local-ue-validation.ps1 -BaseUrl http://localhost:8080 -UsePackagedServer -ClientValidationWaitSec 45`，RunId `7bb2d9c1`：server log 显示正确命令行、`Load map complete /Game/Maps/Lobby/LobbyMap`、两次 `Join succeeded` 与两次 `/runtime/servers/player-joined` HTTP 200；client A/B log 均显示 `TravelCompleted Pending net game travel completed`，且脚本未发现连接超时、网络失败、Fatal error、Assertion failed 或默认地图弹窗回退。
- 已定位并修复显式客户端 cook 阻塞：`DBA_GameClient/Config/DefaultGame.ini` 与 `DBA_GameClient/Config/DefaultEngine.ini` 中的 `+DirectoriesToAlwaysCook=(Path="/Game/ProjectileHitVFX")` 会把未被源码引用的第三方 Niagara 资产 `/Game/ProjectileHitVFX/NS/NS_MeteoriteStraight` 强制纳入 cook，导致 UE 5.8 下 Grid3D Gas 参数报错。当前修复仅移除宽泛 always-cook 配置，未直接修改 `.uasset` / `.umap`。
- 已重新执行 `scripts/package-unreal-dedicated-server.ps1 -SkipBuild -IncludeClientCook`，确认 `Windows+WindowsServer` cook/stage/pak/archive 成功，`AutomationTool exiting with ExitCode=0 (Success)`。
- 已检查 packaged server UFS/Pak/IoStore 命令清单，确认不再包含 `NS_MeteoriteStraight` 或宽泛 `ProjectileHitVFX` cook 条目。
- 已在配置修复后重新执行 `scripts/diagnose-unreal-packaged-server-readiness.ps1` 与 `scripts/smoke-unreal-dedicated-server.ps1 -UsePackagedServer -RunSeconds 20 -Port 17779`，确认 packaged server readiness 与 LobbyMap 启动冒烟仍通过。
- 注意：如果后续玩法确实需要 `NS_MeteoriteStraight`，应通过 Unreal Editor/MCP 正规修复并编译保存该 Niagara 资产，而不是直接编辑二进制资源。

### P2：UE 端到端联调

目标：从后端模拟能力进入真实客户端和真实 Dedicated Server 验证。

待办：

1. 构建真实 UE Dedicated Server 包。
2. 配置 `UE_SERVER_EXECUTABLE_PATH` 并通过 Worker 启动。
3. 验证 Runtime register、ready、heartbeat、player-joined、player-left、match results。
4. 构建真实客户端包并验证登录、角色、匹配、连接信息、进服。
5. 补充 `DBA_GameClient` 构建环境、命令和联调说明。

验收标准：

- Dedicated Server 可被 Worker 分配、启动、回收。
- 客户端可完成登录到进服主链路。

执行记录：

- 已使用 `D:\UnrealEngine-5.8.0-release\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe` 完成 `DivineBeastsArenaEditor Win64 Development` 编译验证。
- 已完成 `DivineBeastsArenaServer Win64 Development` 编译验证，首次构建约 8 分钟。
- 已修复 UE 插件命名不一致问题：插件名、模块名、Build.cs、模块依赖统一为 `GameBackendClient`。
- 已修复 GameBackendClient 插件头文件生成 include 命名，`.generated.h` 与实际头文件名保持一致。
- 已修复项目中旧的 `DBA_GameBackend*.h` include 文件名。
- 已修复 Unity Build 下 `DBASkillProjectileBase.cpp` 的匿名命名空间 helper 函数重名冲突。
- 已补充 `DBAZodiacSkillVFXComponent_Generic.cpp` 对 `Engine/OverlapResult.h` 的 include。
- 当前剩余 P2 工作已从“C++ 无法编译”推进为“需要真实 Server 包路径、Worker 启动和客户端进服联调”。

### P3：启动器、官网和 GM 运营闭环

目标：让外围应用接入真实发布和运营数据。

待办：

1. 启动器接入真实 CDN manifest、真实 SHA256 和发布文件。
2. 官网下载页读取真实发布 manifest。
3. 官网反馈接入后端持久化或 GM 工单。
4. Admin 使用预生产账号完成玩家、对局、服务器、配置、背包、反馈、审计验收。

验收标准：

- 启动器可下载、校验、修复并启动真实客户端包。
- 官网反馈能进入可追踪处理流程。
- GM 后台可以完成核心运营操作。

当前状态：

- 官网反馈表单已提交到 `Game.Api` 的 `/api/feedback/`，后端会持久化为 `PlayerFeedback`，GM 后台已有反馈列表入口。
- 官网下载页已读取 `Game.Api` 的 `/api/launcher/manifest?channel=stable&platform=Windows`，并展示版本、下载地址、大小、SHA256、强更和 release notes。
- 启动器 Rust 层已通过 `cargo check`，具备 manifest 拉取、文件下载、SHA256 校验和启动命令基础能力。
- 已补强启动器发布清单合同：`DBA_GameLauncher` 在 `fetch_manifest` 与 `repair_game` 入口校验 manifest，拒绝空文件列表、路径穿越、重复文件名、非法 SHA256、多文件但非目录型 `downloadUrl`，并兼容后端当前三段/四段数字版本格式。
- 已补强启动器本地修复闭环：`repair_game` 成功下载并校验所有文件后会写入 `version.txt`，避免修复完成后本地版本仍停留在旧值；同时修复单文件 manifest 使用目录型 `downloadUrl` 时会误下载目录 URL 的问题。
- 已补强 Admin 版本发布入口：`/api/admin/client-versions` 现在要求 `checksum` 为 64 位 SHA256 十六进制字符串，避免写入启动器会拒绝的清单元数据；开发种子 checksum 已从文本占位值调整为 64 位十六进制占位值。
- 已新增 `DBA_GameBackend/docs/launcher-manifest-validation.md`，固化 Game.Api 与 DBA_GameLauncher 之间的发布清单合同。
- 已验证 `dotnet test DBA_GameBackend/Game.Api.Tests/Game.Api.Tests.csproj --no-restore` 通过 23 个测试，`cargo test --manifest-path DBA_GameLauncher/src-tauri/Cargo.toml` 通过 11 个测试，`cargo check --manifest-path DBA_GameLauncher/src-tauri/Cargo.toml` 通过。
- 已将 `launcher cargo test` 接入 `scripts/production-preflight.ps1` 的 Cargo 分支，生产预检现在会在 `cargo check` 前先执行启动器 Rust 单元/本地包修复冒烟测试；已验证 `scripts/production-preflight.ps1 -SkipNode -SkipDocker -SkipUnreal` 通过，其中 backend Release 测试 29 个通过、Admin 构建通过、Launcher Rust 11 个测试通过、`cargo check` 通过。
- 剩余工作依赖真实 CDN 文件、真实客户端安装包、真实 SHA256 和预生产 GM 账号验收。

### P4：生产化与发布治理

目标：形成可重复执行的上线、回滚、观测和安全闭环。

待办：

1. 执行 PostgreSQL 备份恢复演练。
2. 执行 k6 登录、匹配、服务器分配压测。
3. 启用 Grafana、Prometheus 告警和 Loki 日志查询。
4. 将依赖扫描和容器镜像扫描纳入 CI。
5. 设置 `main` 分支保护并要求 `solution-ci`。
6. 固化上线 Runbook 和回滚 Runbook。

验收标准：

- 生产预检脚本、备份恢复、压测、观测和回滚流程均有记录。
- 生产环境禁用 dev-login 和 mock fallback。

执行记录：

- 已确认 `.github/workflows/security-ci.yml` 覆盖 NuGet 漏洞检查、Admin/Website/Launcher 生产 npm audit，以及 API/Worker Docker 镜像 Trivy 扫描。
- 已补强 `security-ci` 证据归档：NuGet 漏洞检查输出 `vulnerability-report.txt` artifact，Admin/Website/Launcher 输出生产 `npm audit` JSON artifact，API/Worker Trivy SARIF 既上传 Code Scanning 也作为 workflow artifact 留存。
- 已将 `cargo test --manifest-path src-tauri/Cargo.toml` 接入 `.github/workflows/solution-ci.yml` 与 `.github/workflows/launcher-ci.yml`，使启动器 manifest 校验和本地包修复冒烟测试进入统一 CI 与启动器专用 CI。
- 已本地验证 `cargo test --manifest-path DBA_GameLauncher/src-tauri/Cargo.toml` 通过 11 个测试，`cargo check --manifest-path DBA_GameLauncher/src-tauri/Cargo.toml` 通过；并用结构检查确认两个 workflow 均包含 `cargo test --manifest-path src-tauri/Cargo.toml`。
- 已新增 `scripts/production-security-audit.ps1` 作为本地生产安全审计入口，镜像 `security-ci` 的 NuGet 漏洞检查、Admin/Website/Launcher 生产 npm audit，以及 API/Worker Trivy 镜像扫描；在未安装本机 Trivy 时可使用 `-UseDockerizedTrivy`。
- 已将 `DBA_GameAdmin` Angular 运行时、CLI 和 compiler 依赖从 `21.2.12` 升级到 `21.2.17`，修复生产 npm audit 中 6 个 Angular high 项；已验证 `npm run build` 通过，`scripts/production-security-audit.ps1 -SkipContainerScan` 通过。
- 已验证 `scripts/production-security-audit.ps1 -UseDockerizedTrivy` 通过：NuGet 无漏洞包，Admin/Website/Launcher 生产 npm audit high=0 critical=0，API/Worker 容器 Trivy HIGH/CRITICAL 漏洞扫描均为 0。
- 已补强 `scripts/production-security-audit.ps1` 的证据输出，支持 `-EvidenceDir` 与 `-RunId`；已本地生成 `vulnerability-report.txt`、`npm-audit-admin/website/launcher-*.json`、`trivy-api/worker-*.sarif`，并确认 manifest 中 `security.nuget`、`security.npm`、`security.trivy` 均为 present。
- 已新增 `scripts/collect-production-evidence.ps1`，用于把 security-ci、生产安全审计、k6、备份恢复、部署/回滚结果汇总为 `production-evidence-manifest.json`，并为每个证据文件记录 SHA256、大小和更新时间；已验证空证据目录在 `-RequireAll` 下会失败，完整模拟证据集会通过。
- 已补强 `DBA_GameBackend/scripts/run-load-tests.sh`，k6 登录、匹配和 Dedicated Server 编排压测会输出 summary JSON、日志与 meta 记录到 `Artifacts/ProductionEvidence/load` 或自定义 `EVIDENCE_DIR`；当前机器未安装本机 k6，但 runner 已支持 `USE_DOCKER_K6=1` 自动通过 Docker `grafana/k6:latest` 执行，并已本地完成 guest 模式登录与匹配链路压测。
- 已补强 `DBA_GameBackend/scripts/rehearse-backup-restore.sh`，备份恢复演练会写入 `Artifacts/ProductionEvidence/ops/backup-restore-rehearsal-*.json/log`；已本地执行通过，恢复临时库 `game_platform_restore_rehearsal_20260628095502`，public 表数量为 51，并刷新 `production-evidence-manifest.json`。
- 已补强 `scripts/production-smoke-backend.ps1` 的证据输出，支持 `-EvidenceDir` 与 `-RunId`；已本地执行 `-BaseUrl http://localhost:8080 -GuestLogin` 通过 live、ready、version、launcher manifest、metrics 与 guest login，并生成 `production-smoke-backend-local-smoke-20260628T021000Z.json/log`。
- 当前普通全量 `npm audit` 仍会报告开发/工具链侧漏洞，生产门禁范围内的 `--omit=dev --audit-level=high` 已清零；后续可单独治理 dev dependency 审计。
- 剩余 P4 缺口：需要在 GitHub 仓库设置中要求 `solution-ci`，并在预生产环境记录正式 k6 基线与真实部署/回滚演练，再用 manifest 固化最终发布 artifact 清单；GitHub Actions 正式 `security-ci` artifact 链接仍需在发布记录中留存。
