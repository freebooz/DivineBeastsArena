# 生产可上线任务计划

本文档记录 DivineBeastsArena 从当前基线推进到最小生产可上线版本的执行计划。每个阶段完成后都需要保留验证结果，避免生产化改造只停留在口头清单。

## 当前基线

- 根目录已经整理为五个 `DBA_` 应用目录。
- `DBA_GameBackend` 包含 API、Worker、Shared、Infrastructure 与测试项目。
- `DBA_GameAdmin` 提供 GM 后台、Admin API Client、角色权限和高危操作确认。
- `DBA_GameWebsite` 提供官网、下载页、公告、FAQ 和反馈页。
- `DBA_GameLauncher` 提供 Tauri 启动器、manifest 检查、下载、修复和启动逻辑。
- `DBA_GameClient` 保留 Unreal Engine 客户端与 Dedicated Server 工程，并已接入后端插件和 Runtime API 上报。

## 阶段 1：安全与配置生产化

- [x] 根目录移除旧工程残留文件和多余目录。
- [x] 生产默认配置不写入真实密码或默认 JWT 密钥。
- [x] 开发 seed 默认只在非 Production 环境运行。
- [x] 支持 Docker/Kubernetes/Vault Agent 等 secret 文件投影。
- [x] 登录和高危管理接口增加限流策略。

## 阶段 2：后端 API 可运营闭环

- [x] `/metrics` 使用 Prometheus scraping endpoint。
- [x] `/health/live` 与 `/health/ready` 分离。
- [x] 补充 Auth refresh token 轮换测试。
- [x] 补充 Room / Match / Session 状态测试。
- [x] 补充 Settlement / Inventory 幂等和事务测试。
- [x] Session connection 会重新签发短期 `playerSessionToken`，数据库只保存 hash。

## 阶段 3：Dedicated Server 接入生产化

- [x] 首发使用 LocalProcess 模式，详见 `docs/dedicated-server-production.md`。
- [x] Game Server Manager 覆盖分配幂等、启动超时、心跳超时和端口释放测试。
- [x] UE Dedicated Server 启动参数接入 `sessionId/serverId/backendUrl/runtimeToken`。
- [x] UE Dedicated Server 支持 Runtime register、ready、heartbeat、player-joined、player-left。
- [ ] 使用真实 UE Dedicated Server 包验证分配、启动、心跳、玩家加入、战报和回收。

## 阶段 4：客户端联调上线闭环

- [x] 登录、创建角色、选择角色接口已接入后端真实 API。
- [x] 客户端获取连接信息后使用正确 Travel URL 参数进服。
- [x] Travel URL 携带 `SessionId`、`PlayerSessionToken`、`PlayerId`。
- [ ] 使用真实客户端包验证登录、角色列表、创建角色、选择角色。
- [ ] 使用真实客户端包验证匹配、获取连接信息、进入 Dedicated Server。
- [ ] 关闭客户端重新登录后，从数据库读取角色和账号状态。

## 阶段 5：GM 后台生产可用

- [x] 后台认证使用 Admin Token 流程，前端校验 JWT 过期。
- [x] 后端按 SUPER_ADMIN / OPS / SUPPORT / VIEWER 拆分权限。
- [x] GM 背包发放/扣除强制填写 reason 并写审计日志。
- [x] 前端高危操作提供 reason 输入和二次确认。
- [x] 游戏服务器 Kill 接入 Admin API 与审计日志。
- [ ] 使用预生产环境账号执行一次完整 GM 验收。

## 阶段 6：启动器与官网上线准备

- [x] 启动器名称和窗口标题改为 Divine Beasts Arena Launcher。
- [x] 替换默认 Tauri 图标。
- [x] 后端提供客户端版本和 CDN manifest 元数据 API。
- [x] 官网下载页接入 Game.Api 版本清单。
- [x] 启动器默认 manifest URL 指向后端原始清单接口。
- [ ] 配置真实 CDN 下载地址、SHA256、大小和版本号。
- [ ] 使用真实客户端压缩包验证启动器下载、校验、修复和启动。

## 阶段 7：Ops 与部署生产化

- [x] `DBA_GameBackend/docker-compose.yml` 使用环境变量注入敏感配置。
- [x] Caddy edge profile 提供 HTTPS 反向代理。
- [x] PostgreSQL 备份、恢复、迁移脚本已提供。
- [x] 备份恢复演练脚本支持临时库恢复和表数量 sanity check。
- [x] Prometheus、Grafana、Loki、Promtail、Node Exporter 配置已提供。
- [x] 新增仓库级 `production-preflight.ps1` 和后端烟雾测试脚本。
- [ ] 在预生产服务器执行一次真实部署演练。
- [ ] 在预生产服务器执行一次备份恢复演练。

## 阶段 8：CI/CD 强化

- [x] 后端、后台、官网、启动器 CI 工作流已存在。
- [x] 增加 `solution-ci` 统一验收工作流。
- [x] 部署工作流包含数据库迁移入口。
- [x] 部署失败时支持自动回滚到上一个镜像 tag。
- [ ] 在 GitHub 仓库设置中将 `solution-ci` 纳入主分支保护。

## 阶段 9：上线前压测与安全检查

- [x] 登录压测脚本：`load-tests/k6-login.js`。
- [x] 匹配压测脚本：`load-tests/k6-matchmaking.js`。
- [x] Dedicated Server 分配压测脚本：`load-tests/k6-server-manager.js`。
- [x] 后台权限越权测试脚本：`scripts/check-admin-rbac.sh`。
- [x] 备份恢复演练脚本：`scripts/rehearse-backup-restore.sh`。
- [ ] 使用预生产环境执行 k6 压测并记录基线。
- [ ] 执行漏洞扫描、依赖扫描和容器镜像扫描。

## 当前上线阻塞

1. 真实 UE Dedicated Server 包尚未在当前机器完成端到端验证。
2. 真实客户端包尚未验证登录、角色、匹配和进服闭环。
3. GitHub 分支保护需要仓库管理员 token 执行。
4. 真实 CDN 下载地址、补丁包 SHA256 和发布流程需要预生产资源。

## 建议执行顺序

1. 构建 UE Dedicated Server 包并配置 `UE_SERVER_EXECUTABLE_PATH`。
2. 在预生产服务器启动 `game-api/game-worker/postgres/redis`。
3. 运行 `scripts/production-smoke-backend.ps1`。
4. 用启动器和真实客户端包执行登录、角色、匹配、进服。
5. 执行 k6 压测、备份恢复演练、GM 后台验收。
6. 配置 GitHub 分支保护和生产域名 HTTPS。
