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
- 已确认 UE 工具路径：`E:\UnrealEngine-5.7.1-release\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe`。
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
- 本机 Docker daemon 当前未运行，`docker build` 无法连接 `dockerDesktopLinuxEngine`，镜像构建仍需在 Docker 可用环境或 CI 中验证。

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

- 已使用 `E:\UnrealEngine-5.7.1-release\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe` 完成 `DivineBeastsArenaEditor Win64 Development` 编译验证。
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
