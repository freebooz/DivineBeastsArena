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
- [x] Dedicated Server Orchestration 覆盖分配幂等、启动超时、心跳超时和端口释放测试。
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
- [x] `solution-ci` 与 `launcher-ci` 均执行 `cargo test --manifest-path src-tauri/Cargo.toml` 和 `cargo check --manifest-path src-tauri/Cargo.toml`，覆盖启动器 manifest 校验与本地包修复冒烟测试。
- [x] 部署工作流包含数据库迁移入口。
- [x] 部署失败时支持自动回滚到上一个镜像 tag。
- [ ] 在 GitHub 仓库设置中将 `solution-ci` 纳入主分支保护。

## 阶段 9：上线前压测与安全检查

- [x] 登录压测脚本：`load-tests/k6-login.js`。
- [x] 匹配压测脚本：`load-tests/k6-matchmaking.js`。
- [x] Dedicated Server 分配压测脚本：`load-tests/k6-dedicated-server-orchestration.js`。
- [x] 后台权限越权测试脚本：`scripts/check-admin-rbac.sh`。
- [x] 备份恢复演练脚本：`scripts/rehearse-backup-restore.sh`。
- [x] 备份恢复演练脚本已写入 manifest-ready 证据：`Artifacts/ProductionEvidence/ops/backup-restore-rehearsal-*.json` 与对应日志。
- [x] `security-ci` 已覆盖 NuGet 漏洞检查、Admin/Website/Launcher 生产 npm audit 和 API/Worker 容器 Trivy 扫描，并归档 NuGet 报告、npm audit JSON 和 Trivy SARIF artifact。
- [x] 新增本地生产安全审计入口：`scripts/production-security-audit.ps1`，覆盖 NuGet 漏洞检查、Admin/Website/Launcher 生产 npm audit，并可通过本机 Trivy 或 `-UseDockerizedTrivy` 执行 API/Worker 镜像扫描。
- [x] Admin Angular 依赖从 `21.2.12` 升级到 `21.2.17`，清除生产 npm audit 中的 Angular high 漏洞；已验证 `scripts/production-security-audit.ps1 -SkipContainerScan` 通过。
- [x] 已验证 `scripts/production-security-audit.ps1 -UseDockerizedTrivy` 通过，API/Worker Docker 镜像 Trivy HIGH/CRITICAL 漏洞扫描均为 0。
- [x] `scripts/production-security-audit.ps1` 已支持 `-EvidenceDir` 与 `-RunId`，本地审计可直接输出 `vulnerability-report.txt`、`npm-audit-*.json` 与 `trivy-*.sarif` 到生产证据目录。
- [x] 新增 `scripts/collect-production-evidence.ps1`，可汇总 security-ci、安全审计、k6、备份恢复、部署/回滚证据并生成带 SHA256 的 manifest；`-RequireAll` 可作为上线前证据完整性门禁。
- [x] 新增 `scripts/collect-client-package-evidence.ps1`，可从 staged Windows 客户端包生成 launcher manifest、全文件 SHA256、客户端可执行文件 hash 和 manifest-ready summary；`collect-production-evidence.ps1 -RequireAll` 已加入 `client.package_launcher` 类别。
- [x] `DBA_GameBackend/scripts/run-load-tests.sh` 已支持将 k6 summary JSON、运行日志和 meta 记录写入 `Artifacts/ProductionEvidence/load` 或自定义 `EVIDENCE_DIR`，并可通过 `USE_DOCKER_K6=1` 使用 Docker `grafana/k6:latest`，便于纳入生产证据 manifest。
- [x] 已本地执行 `bash DBA_GameBackend/scripts/rehearse-backup-restore.sh`，生成 `backup-restore-rehearsal-20260628T015502Z.json/log`，恢复临时库 `game_platform_restore_rehearsal_20260628095502` 并确认 public 表数量为 51。
- [x] 已本地执行 `scripts/production-security-audit.ps1 -SkipContainerScan -RunId local-security-20260628T020000Z` 与 `scripts/production-security-audit.ps1 -SkipNuGet -SkipNpm -UseDockerizedTrivy -RunId local-security-20260628T020000Z`，生成 1 份 NuGet 报告、3 份 npm audit JSON、2 份 Trivy SARIF；刷新 manifest 后 `security.nuget`、`security.npm`、`security.trivy` 均为 present。
- [x] `scripts/production-smoke-backend.ps1` 已支持 `-EvidenceDir` 与 `-RunId`，会输出 `production-smoke-backend-*.json/log`；已本地执行 `-BaseUrl http://localhost:8080 -GuestLogin`，live、ready、version、launcher manifest、metrics、guest login 均通过，刷新 manifest 后 `ops.deploy_rollback` 为 present。
- [x] 已使用 Docker `grafana/k6:latest` 本地执行 guest 模式 k6 登录与匹配链路压测，生成 `k6-login-local-k6-login-20260628T022527Z.json/log` 与 `k6-matchmaking-local-k6-matchmaking-20260628T022812Z.json/log`；刷新 manifest 后 `load.k6` 为 present，全部生产证据类别已齐。
- [x] 已对当前 staged Windows client package 执行 `scripts/collect-client-package-evidence.ps1 -RunId client-package-launcher-local-20260628T071000Z`，生成 `client-package-launcher-*.json` 与 `launcher-manifest-*.json`；刷新 manifest 后 `client.package_launcher` 为 present。
- [x] 已执行 `scripts/collect-client-package-evidence.ps1 -RunId client-package-launcher-copy-smoke-20260628T072000Z -CopyInstallSmoke`，将 53 个 staged client package 文件复制到临时安装目录、逐个校验 SHA256，并写入 `version.txt`，作为本地 launcher repair/install 合同烟雾。
- [x] 已修复 `scripts/package-unreal-dedicated-server.ps1 -IncludeClientCook -Configuration Shipping` 的 UAT 参数，使客户端和服务器都使用 Shipping 配置。
- [x] 已新增 `scripts/prepare-client-release-package.ps1`，将 staged Shipping Windows client package 分离成 public runtime package 与 symbols package，并生成 SHA256 证据。
- [x] 已对 `.tmp\client-release\public\client-release-public-symbols-20260628T082500Z` 执行严格客户端包证据：Shipping、无调试符号、复制安装烟雾通过。
- [x] 新增 `scripts/run-client-release-evidence.ps1`，可一键编排客户端包证据、CDN payload、上传前本地 payload smoke、启动器 UI 视觉证据、代码签名/签名证据、启动器安装/更新核心烟雾，并在提供 `-ManifestUrl` 时追加 CDN smoke，最终输出 `client-release-evidence-*.json` 汇总。
- [x] 已执行 `client-release-bundle-local-20260628T103500Z`，成功生成 package/signing/launcher-install-update 证据和 bundle summary；当前 `releaseReady=false`，原因是仍使用示例 CDN、签名未就绪且未提供真实 CDN manifest。
- [x] 新增 `scripts/collect-code-signing-evidence.ps1`，可对 public client package 的 `.exe`、`.dll`、`.msi`、`.msix`、`.appx` 执行 `Get-AuthenticodeSignature`，输出 `code-signing-*.json`，并支持 `-RequireSigned` 作为签名门禁。
- [x] 新增 `scripts/sign-client-release-package.ps1`，可通过 Windows SDK `signtool.exe` 使用证书指纹、证书主题或 PFX 对 public client package 执行 SHA256 时间戳签名，并自动刷新 code-signing 证据。
- [x] 已对 `.tmp\client-release\public\client-release-public-symbols-20260628T082500Z` 执行签名证据采集；当前 15 个可签名文件中 8 个为 trusted valid，7 个未签名，因此 `signingReady=false`。
- [x] 新增 `scripts/run-launcher-install-update-smoke.ps1`，封装 Tauri Rust 定向测试，输出 `launcher-install-update-smoke-*.json/log`，并接入 `client.launcher_install_update` 生产证据类别。
- [x] 已执行 `launcher-install-update-local-20260628T101500Z`，验证 manifest 拉取、本地包下载、SHA256 校验、修复安装和 `version.txt` 持久化，证据中 `installUpdateReady=true`。
- [ ] `client.package_launcher` 的生产 `-RequireAll` 当前仍因真实 CDN 未接入而保持 incomplete；需要使用非示例 HTTPS CDN URL 重新生成证据。
- [ ] `client.code_signing` 的生产 `-RequireAll` 当前因 public package 存在未签名文件而保持 incomplete；需要使用正式代码签名证书签署项目 exe/dll 后重新生成 `signingReady=true` 证据。
- [x] 新增 `scripts/run-launcher-cdn-smoke.ps1`，可从 manifest URL 拉取清单、下载全部客户端包文件、校验 SHA256/大小并写入 `version.txt`。
- [x] 新增 `scripts/prepare-client-cdn-payload.ps1`，可从 public client package 生成 CDN 上传目录、固定名 `launcher-manifest.json` 与 `cdn-upload-manifest-*.json`，用于真实 CDN 发布前核对文件列表、SHA256 和大小。
- [x] 新增 `scripts/run-local-cdn-payload-smoke.ps1`，可对准备好的 CDN payload 临时启动 localhost 静态服务，并复用 `run-launcher-cdn-smoke.ps1` 验证上传前 payload 可被启动器式下载和校验。
- [x] 已使用 localhost 静态服务器执行 `launcher-cdn-local-http-smoke-20260628T090000Z`，验证 34 个 public client package 文件的下载与安装目录落地。
- [ ] `client.cdn_launcher_smoke` 的生产 `-RequireAll` 当前仍因真实 HTTPS CDN 未接入而保持 incomplete；需要上传 public package/manifest 到真实 CDN 后无 `-AllowLocalHttp` 重新执行。
- [x] 新增 `scripts/capture-launcher-ui-evidence.ps1`，可构建启动器、启动 Vite preview、使用 headless 浏览器截图玩家可见启动器首屏，并输出 `launcher-ui-visual-evidence-*.json/png/dom.html`。
- [x] `scripts/run-client-release-evidence.ps1` 已支持 `-CaptureLauncherUiEvidence`，可在客户端发布 bundle 内同步采集并汇总 `launcherUiVisualReady`。
- [ ] 启动器玩家可见 UI 证据入口已具备；正式发布候选包仍需在 `Artifacts/ProductionEvidence/client` 下重新采集并归档。
- [x] 新增 `scripts/write-release-readiness-report.ps1`，可从 `production-evidence-manifest.json` 输出 `release-readiness-report.md/json`，集中列出 `present/missing/incomplete` 状态和阻塞项。
- [x] 已执行 readiness report，本地报告显示 8/11 个生产证据类别 present，阻塞项为 `client.package_launcher`、`client.cdn_launcher_smoke`、`client.code_signing`。
- [x] `scripts/production-preflight.ps1` 已接入 `-RequireReleaseReady`，可在预检末尾生成 release readiness report，并把缺失或 incomplete 生产证据作为最终发布硬门禁。
- [x] 已验证 `USE_DOCKER_K6=1 AUTH_MODE=guest bash scripts/run-load-tests.sh` 可在无本机 k6 的环境下完成 login 与 matchmaking 短时压测，并生成 `local-runner-docker-20260628T024614Z` 证据。
- [ ] 使用预生产环境执行 k6 压测并记录基线。
- [ ] 将预生产 k6、真实部署/回滚演练证据，以及 GitHub Actions 正式 `security-ci` artifact 链接放入最终发布记录。

## 当前上线阻塞

1. 本地 packaged UE Dedicated Server 已完成端到端验证；上线仍缺预生产自托管 runner 官方证据归档。
2. 真实客户端包尚未完成登录、角色、匹配和进服闭环的发布候选验证。
3. GitHub 分支保护需要仓库管理员 token 执行。
4. 真实 CDN 下载地址、补丁包 SHA256 和发布流程需要预生产资源。

## 建议执行顺序

1. 在预生产自托管 UE runner 上复跑 packaged Dedicated Server 联机证据并归档 workflow artifact。
2. 在预生产服务器启动 `game-api/game-worker/postgres/redis`。
3. 运行 `scripts/production-smoke-backend.ps1`。
4. 用启动器和真实客户端发布候选包执行登录、角色、匹配、进服。
5. 执行 k6 压测、备份恢复演练、GM 后台验收。
6. 配置 GitHub 分支保护和生产域名 HTTPS。

