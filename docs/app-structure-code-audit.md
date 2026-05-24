# 各应用项目结构与代码审计

审计日期：2026-05-23

## 总体结论

当前仓库是一个单仓库多应用解决方案，目录边界总体清晰：`DBA_GameBackend` 承担平台后端，`DBA_GameAdmin` 承担 GM 后台，`DBA_GameWebsite` 承担官网，`DBA_GameLauncher` 承担本地启动器，`DBA_GameClient` 承担 UE 客户端与 Dedicated Server。工程命名与应用职责基本一致，最佳方案是继续保持“按应用独立构建、按领域拆分源码、共享契约从后端 Game.Shared 输出”的结构。

## DBA_GameBackend

结构判断：
- `Game.Api` 负责 Minimal API 路由、鉴权、请求/响应组装。
- `Game.Infrastructure` 负责 EF Core、实体、配置、种子数据和持久化基础设施。
- `Game.Shared` 负责跨应用 DTO、错误码和配置契约。
- `Game.ServerManagement` 负责 Dedicated Server 编排，避免把进程管理塞进 API 层。
- `Game.Worker` 适合承载定时任务、超时扫描和后台处理。

本轮修复：
- `DevelopmentDataSeeder` 增加完整 API mock 数据，覆盖服务器、房间、匹配、会话、背包、钱包、订单、排行、好友、邮件、活动、成就、战绩、客服、留存和配置发布日志。
- `CommerceEndpoints` 从“开发中”改为可执行 mock 购买流程，写入订单、钱包流水、背包和背包日志。
- `SocialEndpoints` 修复好友列表返回关系拥有者信息的问题，改为返回好友玩家资料。
- `PlayerHistoryEndpoints` 修复工单详情漏掉管理员回复的问题。
- `GameFeatureEndpointHelpers` 增加统一分页规范化，排行榜、背包日志、战绩、我的工单和反馈接口在未传 `page/pageSize` 时稳定返回默认第一页。
- `AdminEndpoints` 已拆出服务器和客户端版本管理 partial 文件，降低单文件膨胀。

后续建议：
- 将 GameFeatures 中仍直接访问 DbContext 的复杂写接口逐步下沉到服务层。
- 给完整种子数据增加关系完整性测试，避免以后新增接口时 mock 数据再次遗漏。

## DBA_GameAdmin

结构判断：
- Angular 页面位于 `src/app/pages`，路由位于 `src/app/app.routes.ts`，页面职责清晰。
- `src/app/core/admin-api.service.ts` 统一封装 Game.Api 调用和 API envelope 解包。
- `src/app/core/auth.service.ts` 管理 Admin JWT 本地会话，HTTP 拦截器统一附加授权头。

本轮修复：
- 修复 `ApiClient.cs` 文件头坏编码注释。
- 为 `ApiClient.Contracts.cs` 增加职责说明，明确该文件只承载后台展示 DTO。

后续建议：
- 后台 DTO 能直接复用 `Game.Shared` 时优先复用，只有展示裁剪或组合字段才保留本地 DTO。
- 高风险操作页面建议补充 bUnit 或 Playwright 级别的最小回归测试。

## DBA_GameWebsite

结构判断：
- Next.js 页面结构集中在 `src/app`，静态内容集中在 `src/data/siteContent.ts`，展示组件位于 `src/components`。
- 官网反馈通过 `src/app/api/feedback/route.ts` 代理到 Game.Api，减少浏览器端跨域和后端地址暴露。

本轮判断：
- 结构与官网职责一致，暂未发现需要拆分的高风险代码。

后续建议：
- 将下载页与启动器共用同一份后端 manifest 数据，减少官网与启动器版本信息漂移。
- 为反馈表单增加端到端冒烟测试，覆盖 API 代理失败和成功两条路径。

## DBA_GameLauncher

结构判断：
- Tauri 前端位于 `src`，本地高权限能力位于 `src-tauri/src/lib.rs`。
- Rust 侧已经包含路径穿越防护、SHA256 校验、修复下载和启动参数解析配合。

本轮判断：
- 启动器职责聚焦，当前单页结构可接受。

本轮修复：
- `App.tsx` 的后端状态 DTO 与 `LauncherStatusResponse` 对齐，避免 `/api/launcher/status` 返回后在界面显示 `undefined` 字段。

后续建议：
- 当下载、修复、日志、设置继续增长时，将 `App.tsx` 拆为 `components`、`hooks` 和 `services`。
- Rust 命令建议补充单元测试，尤其是 `safe_join`、`build_file_download_url` 和 manifest 校验。

## DBA_GameClient

结构判断：
- UE 工程通过 `GameCore`、`GameMoba`、`DivineBeastsArena` 三个模块分层，后端通信放在 `Plugins/GameBackendClient`，模块职责合理。
- `GameCore` 偏基础系统和前端流程，`GameMoba` 偏 MOBA/GAS 基类，`DivineBeastsArena` 承载具体游戏实现。

本轮判断：
- 模块命名与应用功能一致，UE Dedicated Server 与客户端共用工程是合理选择。

后续建议：
- 保持 `GameBackendClient` 插件独立，避免具体 UI 或玩法逻辑反向进入插件。
- 避免提交 `Binaries`、`Intermediate`、`Saved` 等生成目录的新变更；需要时通过 `.gitignore` 和 CI 检查约束。
- 继续用用户指定的 UE 路径 `E:\UnrealEngine-5.7.1-release` 做 Editor 与 Shipping 构建验证。

## 验证入口

- Backend：`dotnet test DBA_GameBackend/GameBackend.sln`
- Admin：`cd DBA_GameAdmin && npm ci && npm run build`
- Website：`npm run build` in `DBA_GameWebsite`
- Launcher：`npm run build` and `cargo check --manifest-path src-tauri/Cargo.toml` in `DBA_GameLauncher`
- UE Client：使用 `E:\UnrealEngine-5.7.1-release\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe`
