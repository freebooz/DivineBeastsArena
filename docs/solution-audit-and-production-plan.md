# 解决方案审查与生产部署计划

## 审查结论

当前仓库已经具备最小可上线外围应用的主体形态：后端 API、Worker、GM 后台、官网、启动器、UE 客户端、CI/CD、Docker Compose、观测和备份脚本都已存在。后端自动化测试和生产配置基础较完整，最大剩余风险集中在真实 UE Dedicated Server 包、真实客户端包、预生产部署资源和仓库权限配置。

## 应用审查

### DBA_GameBackend

定位：游戏核心后端、会话、匹配、配置、结算、背包、Runtime API、Game Server Manager。

已具备：

- JWT 登录、刷新、登出和开发账号。
- 玩家资料、角色创建/选择、配置发布、房间、匹配、会话、结算、背包。
- Game Server Manager LocalProcess 模式、端口分配、进程启动、维护超时。
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

- 当前机器未发现 UnrealBuildTool，无法做 UE C++ 编译验证。
- 需要真实客户端包验证登录、角色、匹配、进服。
- 需要真实 Dedicated Server 包验证心跳、玩家进出和回收。

### DBA_GameAdmin

定位：GM 管理后台。

已具备：

- Blazor 后台页面、登录状态管理、Admin API Client。
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
