# 生产可上线任务计划

本文档记录 DivineBeastsArena 从当前基线推进到最小生产可上线版本的执行计划。每个阶段完成后都需要更新验收结果，避免生产化改造只停留在口头清单。

## 当前基线

- 仓库结构已经收敛为五个 `DBA_` 应用目录。
- `DBA_GameBackend` 已包含 API、Worker、Shared、Infrastructure 与测试项目。
- `DBA_GameAdmin` 已提供 GM 后台页面骨架和后端 API Client。
- `DBA_GameWebsite` 已提供官网、下载、公告、FAQ、反馈页面。
- `DBA_GameLauncher` 已提供 Tauri 启动器骨架和基础命令。
- `DBA_GameClient` 保留 Unreal Engine 客户端与 Dedicated Server 工程。

## 阶段 1：安全与配置生产化

- [x] 根目录移除旧 UE 工程残留文件和目录。
- [x] 生产默认配置不再写入真实密码或默认 JWT 密钥。
- [x] 开发 seed 默认只在非 Production 环境运行。
- [x] 为生产部署接入集中 secret 管理，支持 Docker/Kubernetes/Vault Agent 等 secret 文件投影。
- [x] 为登录和高危管理接口增加限流策略。

## 阶段 2：后端 API 可运营闭环

- [x] `/metrics` 使用真实 Prometheus scraping endpoint。
- [x] `/health/live` 与 `/health/ready` 分离。
- [x] 补充 Auth refresh token 轮换测试。
- [x] 补充 Room / Match / Session 状态测试，覆盖房主转移与房间会话幂等创建。
- [x] 补充 Settlement / Inventory 幂等和事务测试，覆盖战报重复提交与奖励重复发放保护。

## 阶段 3：Dedicated Server 接入生产化

- [x] 明确首发使用 LocalProcess 模式，详见 `docs/dedicated-server-production.md`。
- [ ] 用真实 UE Dedicated Server 包验证分配、启动、心跳和回收。
- [x] 补充 Game Server Manager 启动超时、心跳超时、端口释放和分配幂等自动化测试。

## 阶段 4：客户端联调上线闭环

- [ ] 客户端登录、创建角色、选择角色全部通过后端真实 API。
- [ ] 客户端匹配、获取连接信息、进入 Dedicated Server 全流程联调。
- [ ] 关闭客户端重新登录后能从数据库读取角色和账号状态。

## 阶段 5：GM 后台生产可用

- [x] 后台认证模型明确为 Admin Token 流程，前端校验 JWT 过期并由后端按 Admin 角色授权。
- [x] 后端 GM 背包发放/扣除强制填写 reason 并写审计日志。
- [x] GM 后台背包发放/扣除增加 reason 输入和二次确认。
- [x] 前端高危操作补充 reason 和二次确认，已覆盖背包发放/扣除与游戏服务器 Kill。
- [x] 按 SUPER_ADMIN / OPS / SUPPORT / VIEWER 拆分后台 API 权限，SUPER_ADMIN 保持全权限。

## 阶段 6：启动器与官网上线准备

- [x] 启动器项目名称和窗口标题改为 Divine Beasts Arena Launcher。
- [x] 替换默认 Tauri 图标，生成 DBA 专用 PNG / ICO / ICNS 图标资产。
- [x] 接入真实 CDN manifest 元数据发布接口；补丁包上传由发布流水线写入 CDN 后通过 Admin API 发布。
- [x] 官网下载页接入 Game.Api 版本清单，显示版本、下载地址、文件大小和 SHA256。

## 阶段 7：Ops 与部署生产化

- [x] `DBA_GameBackend/docker-compose.yml` 使用环境变量注入敏感配置。
- [x] 接入 HTTPS 证书自动续期，使用 Caddy edge profile 作为生产反向代理。
- [x] 提供 PostgreSQL 备份和恢复脚本入口。
- [x] 提供 PostgreSQL 备份恢复演练脚本，支持临时库恢复和表数量 sanity check。
- [x] Grafana 看板接入真实 API `/metrics` 指标，并提供 Prometheus/Loki/Promtail/Node Exporter 部署配置。

## 阶段 8：CI/CD 强化

- [x] 后端、后台、官网、启动器 CI 文件已存在。
- [x] 部署工作流改为当前 `DBA_GameBackend` 路径。
- [x] 增加 `solution-ci` 统一验收工作流。
- [ ] 在 GitHub 仓库设置中将 `solution-ci` 纳入主分支保护；已提供 `scripts/configure-branch-protection.sh`，需要仓库管理员令牌执行。
- [x] 部署工作流增加数据库迁移入口。
- [x] 部署工作流增加失败自动回滚，健康检查失败时回滚到上一版镜像 tag。

## 阶段 9：上线前压测与安全检查

- [x] 登录压测脚本已提供：`load-tests/k6-login.js`。
- [x] 匹配压测脚本已提供：`load-tests/k6-matchmaking.js`。
- [x] Dedicated Server 分配压测脚本已提供：`load-tests/k6-server-manager.js`。
- [x] 后台权限越权测试脚本已提供：`scripts/check-admin-rbac.sh`。
- [x] 备份恢复演练脚本已提供：`scripts/rehearse-backup-restore.sh`。
