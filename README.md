# DivineBeastsArena

DivineBeastsArena 是一个 UE 5 多人在线游戏的单仓库解决方案。根目录采用 `DBA_` 前缀扁平化应用结构，统一管理 UE 客户端、游戏后端、GM 后台、官网、启动器、部署脚本和生产文档。

## 开发纲领

项目级总控提示词已写入 `docs/Development/ZodiacArena_UE5_8_Codex_总控提示词.md`，并补充了
`docs/Development/ZodiacArena_UE5_8_Codex_纲领性总控文档.md`（阶段性交付与验收版），并以
`docs/Development/ZodiacArena_阶段交付看板.md` 持续记录当前 P0~P4 的进展、证据与阻塞。后续系统设计、代码生成、文档同步、验证与分阶段交付，应先阅读并遵守这些文档；若纲领内容与当前仓库实际结构不一致，先审查现状，再按最小可验证增量推进。

## 解决方案边界

当前仓库包含多个应用，但 Visual Studio 解决方案文件只覆盖后端：

- `DBA_GameBackend/GameBackend.sln`：.NET 后端解决方案，包含 API、Worker、共享契约、基础设施和测试。
- `DBA_GameClient/DivineBeastsArena.uproject`：UE 客户端与 Dedicated Server 工程，不在 `.sln` 中。
- `DBA_GameAdmin`：Angular 18+ GM 后台，独立构建。
- `DBA_GameWebsite/package.json`：Next.js 官网，独立构建。
- `DBA_GameLauncher/package.json`：Tauri 启动器，独立构建。

## 应用矩阵

| 目录 | 技术栈 | 职责 | 当前验证入口 |
| --- | --- | --- | --- |
| `DBA_GameClient` | Unreal Engine 5 / C++ / GAS | 客户端、Dedicated Server、玩法、UI、后端 UE 插件 | UE Editor / UnrealBuildTool |
| `DBA_GameBackend` | .NET 10 / ASP.NET Core / EF Core / PostgreSQL / Redis | 登录、玩家、配置、房间、匹配、会话、结算、背包、Runtime API、Worker | `dotnet build/test GameBackend.sln` |
| `DBA_GameAdmin` | Angular 18+ / TypeScript / Nginx | GM 管理后台、玩家/对局/服务器/配置/审计管理 | `npm run build` |
| `DBA_GameWebsite` | Next.js / React / TypeScript / Tailwind | 官网、下载、公告、FAQ、反馈 | `npm run build` |
| `DBA_GameLauncher` | Tauri / React / TypeScript / Rust | 游戏启动器、版本检查、下载、校验、修复、启动 | `npm run build` + `cargo test` + `cargo check` |
| `.github/workflows` | GitHub Actions | CI、镜像构建、部署、回滚、安全检查 | `solution-ci` |
| `scripts` | PowerShell / Bash | 仓库级预检、生产烟雾测试、安全审计、证据归档、分支保护 | `production-preflight.ps1` |

## 本机 UE 工具路径

当前 Windows 开发机可使用以下 UE 安装目录：

```powershell
D:\UnrealEngine-5.8.0-release
```

仓库级预检脚本会优先读取 `$env:UNREAL_ENGINE_ROOT`，若未设置则尝试上述路径下的 `Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe`。

## 常用验证命令

```powershell
cd DBA_GameBackend
dotnet build GameBackend.sln
dotnet test GameBackend.sln --no-build

cd ..\DBA_GameAdmin
npm ci
npm run build

cd ..\DBA_GameWebsite
npm install
npm run build

cd ..\DBA_GameLauncher
npm install
npm run build
cargo test --manifest-path src-tauri/Cargo.toml
cargo check --manifest-path src-tauri/Cargo.toml

cd ..\DBA_GameBackend
docker compose --env-file .env.example config
```

仓库级预检脚本：

```powershell
.\scripts\production-preflight.ps1
.\scripts\production-preflight.ps1 -SkipUnrealOnlineValidation
.\scripts\production-preflight.ps1 -CollectEvidence -EvidenceRoot .\Artifacts\ProductionEvidence -RunId local-preflight
.\scripts\production-preflight.ps1 -CollectEvidence -RequireReleaseReady -EvidenceRoot .\Artifacts\ProductionEvidence -RunId release-gate
```

生产安全审计：

```powershell
.\scripts\production-security-audit.ps1 -SkipContainerScan -EvidenceDir .\Artifacts\ProductionEvidence\security
.\scripts\production-security-audit.ps1 -UseDockerizedTrivy -EvidenceDir .\Artifacts\ProductionEvidence\security
```

生产证据 manifest：

```powershell
.\scripts\collect-production-evidence.ps1
.\scripts\collect-production-evidence.ps1 -EvidenceRoot .\Artifacts\ProductionEvidence -RequireAll
.\scripts\write-release-readiness-report.ps1 -EvidenceRoot .\Artifacts\ProductionEvidence
.\scripts\write-release-readiness-report.ps1 -EvidenceRoot .\Artifacts\ProductionEvidence -RequireReady
.\scripts\diagnose-release-blockers.ps1 -ReportPath .\Artifacts\ProductionEvidence\release-readiness-report.json
.\scripts\validate-release-blocker-actions.ps1 -ActionReportPath .\Artifacts\ProductionEvidence\release-blocker-actions.json -RequireValid
.\scripts\write-release-input-template.ps1 -ActionReportPath .\Artifacts\ProductionEvidence\release-blocker-actions.json
.\scripts\validate-release-input-template.ps1 -TemplatePath .\Artifacts\ProductionEvidence\release-input-template.json -RequireValid
```

`write-release-readiness-report.ps1` 会读取 `production-evidence-manifest.json`，输出 `release-readiness-report.md/json`。`-RequireReady` 适合作为最终发布门禁：任何证据类别为 `missing` 或 `incomplete` 都会失败，并在报告中列出阻塞项。
`diagnose-release-blockers.ps1` 会读取 `release-readiness-report.json`，输出 `release-blocker-actions.md/json`，把当前阻塞项映射到下一步脚本、责任域和解除条件，并从已归档 evidence JSON 提取 `observedReasons`（如 example CDN、`releaseReady=false`、`cdnReady=false`、`unsignedFileCount`），避免 release readiness 报告只停留在“知道缺什么”。原因按证据文件修改时间倒序排列，优先展示最新有效阻塞证据；报告同时输出 `latestEvidencePath` 和 `missingExternalInputs`，用于快速定位最新失败样本和还缺的真实 CDN、包根目录、签名身份等外部输入。若 blocker 仍依赖这些外部输入，报告会标记 `automationBlocked=true` 并列出 `blockingExternalInputs`，明确当前不能靠本地脚本继续伪造通过；`inputResolutionHints` 会把每个外部输入映射到对应脚本参数或环境变量，例如 `-ManifestUrl`、`-PackageRoot`、`-CertificateThumbprint`、`-TimestampUrl`。为避免历史证据刷屏，报告默认每个 blocker 展示最多 5 条原因，同时保留 `observedReasonCount`。
`validate-release-blocker-actions.ps1` 会读取 `release-blocker-actions.json`，输出 `release-blocker-action-validation.json`，验证每条 `nextCommand` 引用的脚本存在、命令参数已在目标 PowerShell 脚本 `param(...)` 中声明，并且 `<...>` 占位符可由 `inputResolutionHints` 推导得到；`-RequireValid` 用于防止 action report 里的下一步命令和真实脚本接口漂移。
`write-release-input-template.ps1` 会读取 `release-blocker-actions.json`，输出 `release-input-template.md/json`，把所有 `automationBlocked=true` 的外部输入去重成可填写模板行，并保留参数、环境变量、占位值和阻塞来源。模板还会生成 `suggestedCommands` / `Suggested commands` 命令草案，方便把真实 CDN、包根目录和签名输入填回发布脚本；这些命令不会自动执行，占位值必须替换为真实值。该模板是发布输入准备清单，不是 production evidence；`collect-production-evidence.ps1` 会排除它，防止派生模板污染 release readiness。
`validate-release-input-template.ps1` 会读取 `release-input-template.json`，输出 `release-input-template-validation.json`，验证 `suggestedCommands` 中的每个 `<...>` 占位符都能在 `inputs.placeholder` 中找到，同时检查命令草案引用的脚本存在、命令参数已在目标 PowerShell 脚本 `param(...)` 中声明；`-RequireValid` 适合 CI/本地门禁，防止命令草案和模板行漂移。该 validation 结果同样是派生检查结果，不作为 production evidence。
`production-preflight.ps1 -RequireReleaseReady` 会在预检末尾调用同一报告脚本并启用 `-RequireReady`；搭配 `-CollectEvidence` 可先刷新 manifest。无论是 `-CollectEvidence` 还是 `-RequireReleaseReady`，preflight 都会继续生成 `release-blocker-actions`、验证 blocker `nextCommand`、生成 release input template 并验证模板，确保失败时同时留下可执行的下一步输入清单。

客户端包与启动器 manifest 证据：

```powershell
# 先通过 UAT 产出 Windows client staged package，例如：
.\scripts\package-unreal-dedicated-server.ps1 -IncludeClientCook

# 再生成 launcher manifest、全文件 SHA256 和客户端包证据：
.\scripts\collect-client-package-evidence.ps1 -PackageRoot .\.tmp\packaged-server\Windows -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-client-package -Version 0.1.0.0
```

发布前应先把 staged Shipping 包拆成 public package 与 symbols package，再对 public package 生成证据：

```powershell
.\scripts\prepare-client-release-package.ps1 -StagedPackageRoot .\.tmp\packaged-client-shipping-fixed\Windows -RunId local-client-release
.\scripts\collect-client-package-evidence.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-client-release -Version 0.1.0.0 -BuildConfiguration Shipping -DisallowDebugSymbols -CopyInstallSmoke -DownloadUrl "https://cdn.example.com/releases/0.1.0.0/"
```

该证据会纳入 `collect-production-evidence.ps1 -RequireAll` 的 `client.package_launcher` 类别。Development 包可用于验证 manifest/hash 合同；真正 release-ready 证据必须是 Shipping、public 包不含调试符号、HTTPS 且非示例 CDN URL，并完成本地安装复制/校验烟雾。

客户端发布输入预检：

```powershell
.\scripts\diagnose-client-release-prerequisites.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -DownloadUrl "https://cdn.example.com/releases/0.1.0.0/" -ManifestUrl "https://cdn.example.com/releases/0.1.0.0/launcher-manifest.json" -RequireManifestUrl -FailOnBlockingIssues
.\scripts\diagnose-client-release-prerequisites.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -DownloadUrl "https://cdn.your-domain.example/releases/0.1.0.0/" -ManifestUrl "https://cdn.your-domain.example/releases/0.1.0.0/launcher-manifest.json" -RequireManifestUrl -RequireSigningIdentity -CertificateThumbprint "<thumbprint>" -RequireSignTool -FailOnBlockingIssues
```

该预检不会签名、上传或下载文件，只检查 public package、真实 HTTPS CDN URL、非示例 URL、签名身份和 `signtool.exe` 等正式发布输入。它会输出 `client-release-prerequisites-*.json`，并可用 `-FailOnBlockingIssues` 在 CI 或本地 release run 早停。`collect-production-evidence.ps1` 会将 `readyForReleaseInputs=true` 的预检报告纳入 `client.release_prerequisites` 类别；示例 URL 或缺签名输入会让该类别保持 `incomplete`。

客户端发布证据一键编排：

```powershell
.\scripts\run-client-release-evidence.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -EvidenceRoot .\Artifacts\ProductionEvidence -RunId local-client-release -Version 0.1.0.0 -BuildConfiguration Shipping
.\scripts\run-client-release-evidence.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -EvidenceRoot .\Artifacts\ProductionEvidence -RunId prod-client-release -Version 0.1.0.0 -BuildConfiguration Shipping -DownloadUrl "https://cdn.example.com/releases/0.1.0.0/" -ManifestUrl "https://cdn.example.com/releases/0.1.0.0/launcher-manifest.json" -RequireSigned
.\scripts\run-client-release-evidence.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -EvidenceRoot .\Artifacts\ProductionEvidence -RunId prod-client-release-signed -Version 0.1.0.0 -BuildConfiguration Shipping -DownloadUrl "https://cdn.example.com/releases/0.1.0.0/" -ManifestUrl "https://cdn.example.com/releases/0.1.0.0/launcher-manifest.json" -PrepareCdnPayload -PayloadRoot .\.tmp\cdn-upload\0.1.0.0 -RunLocalCdnPayloadSmoke -CaptureLauncherUiEvidence -SkipCdnSmoke -SignPackage -CertificateThumbprint "<thumbprint>" -RequireSigned
```

该编排脚本会顺序运行客户端包证据、代码签名证据、启动器安装/更新核心烟雾，并在提供 `-ManifestUrl`、`-RequireSigned` 或 `-SignPackage` 时先调用 `diagnose-client-release-prerequisites.ps1` 做发布输入早停检查，最终输出 `client-release-evidence-*.json` 汇总。传入 `-PrepareCdnPayload` 时会生成可上传 CDN payload 与 `cdn-upload-manifest`；传入 `-RunLocalCdnPayloadSmoke` 时会在上传前临时启动本地静态服务验证该 payload 可被启动器式下载；传入 `-CaptureLauncherUiEvidence` 时会构建启动器并采集玩家可见 UI 截图/DOM 证据；上传尚未完成时可加 `-SkipCdnSmoke`，待真实 CDN 可访问后再执行 CDN smoke；传入 `-SignPackage` 时会先调用 `sign-client-release-package.ps1` 完成签名再采集签名证据。若仍使用示例 CDN、未签名包或未提供真实 CDN manifest，汇总会明确保持 `releaseReady=false`。只有在故意复现旧流程或调试局部脚本时才使用 `-SkipReleasePrerequisiteCheck`。

GitHub Actions 中的 `.github/workflows/client-release-evidence.yml` 是手动触发的客户端发布证据工作流，要求自托管 Windows runner 带有 `self-hosted`、`Windows`、`ClientRelease` 标签，并配置：

- `DBA_CODE_SIGNING_PFX_PASSWORD` secret：使用 `-PfxPath` 时的 PFX 密码。
- runner 上的 public client package 路径、真实 HTTPS CDN `download_url` / `manifest_url`、签名证书 thumbprint/subject 或 PFX 路径。

该 workflow 会先运行 `diagnose-client-release-prerequisites.ps1`，再调用 `run-client-release-evidence.ps1`，随后刷新 `production-evidence-manifest.json` 与 `release-readiness-report.md/json`。在最终 `require_ready` 判定前，它还会生成并验证 `release-blocker-actions`、`release-blocker-action-validation`、`release-input-template` 与 `release-input-template-validation`，确保失败运行也会上传可执行的下一步输入清单。默认 `require_ready=true`，因此正式运行会在任一发布门禁仍为 `missing` 或 `incomplete` 时失败。

接入 self-hosted runner 前可先本地运行只读诊断：

```powershell
.\scripts\diagnose-client-release-runner.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -DownloadUrl "https://cdn.your-domain.example/releases/0.1.0.0/" -ManifestUrl "https://cdn.your-domain.example/releases/0.1.0.0/launcher-manifest.json" -JsonOutputPath .\Artifacts\ProductionEvidence\client\client-release-runner-diagnostic-local.json -SkipSigningProbe
```

在 GitHub Actions 中，workflow 会先输出 `client-release-runner-diagnostic-<RunId>.json`，再进入 release prerequisite gate 和 evidence bundle。

发布证据自动化总验证入口：

```powershell
.\scripts\test-production-evidence-automation.ps1
```

该脚本用于本地和 CI 的轻量回归验证，会顺序运行客户端发布输入 fixture、客户端发布 runner 诊断 fixture、发布就绪报告 fixture、发布阻塞行动清单 fixture、Unreal 源码守护 fixture、生产证据契约检查、PowerShell 语法解析与 GitHub Actions YAML 解析。它不会启动 Unreal Editor、不会签名、不会上传 CDN、不会运行外部服务；适合在正式 runner/CDN/证书接入前持续防止发布证据脚本和 workflow 合同漂移。

`collect-production-evidence.ps1` 会按文件名排除派生输出 `production-evidence-manifest.json`、`release-readiness-report.json/md`、`release-blocker-actions.json/md`、`release-blocker-action-validation.json`、`release-input-template.json/md` 和 `release-input-template-validation.json`，即使这些文件被写到子目录中，也不会反过来污染下一次证据文件索引。

`.github/workflows/solution-ci.yml` 的 `evidence-structure` job 会先通过 `actions/setup-python` 准备 Python，再安装 PyYAML 并运行该总验证入口，因此普通 PR/Push CI 会覆盖发布证据脚本、fixture 和 workflow 结构合同；真正的 UE 打包、签名和 CDN smoke 仍由手动触发的 Unreal/ClientRelease self-hosted workflow 负责。

如果 `evidence-structure` 失败，workflow 会上传 `.tmp` 下的 `production-evidence-test-diagnostics` artifact，并保留 14 天，用于查看 fixture 生成的 JSON、报告和诊断输出。
该 job 设置了 15 分钟超时，防止轻量证据门禁在 CI 环境异常时长时间悬挂。

客户端代码签名证据：

```powershell
.\scripts\collect-code-signing-evidence.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-client-signing
.\scripts\collect-code-signing-evidence.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-client-signing-required -RequireSigned
.\scripts\sign-client-release-package.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -CertificateThumbprint "<thumbprint>" -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId prod-client-signing -RequireSigned
```

该证据会纳入 `collect-production-evidence.ps1 -RequireAll` 的 `client.code_signing` 类别。生产发布必须让 public package 内的 `.exe`、`.dll`、`.msi`、`.msix`、`.appx` 等可签名文件通过 `Get-AuthenticodeSignature` 且 `signingReady=true`。当前本地未签名包可生成证据，但会被 `-RequireSigned` 与生产 `-RequireAll` 拦截。
`sign-client-release-package.ps1` 会通过 Windows SDK `signtool.exe` 对 public package 内可签名文件执行 SHA256 + RFC3161 时间戳签名，支持 `-CertificateThumbprint`、`-CertificateSubject` 或 `-PfxPath` 三选一；签名后默认立即调用 `collect-code-signing-evidence.ps1` 刷新证据。

CDN/启动器下载烟雾：

```powershell
.\scripts\prepare-client-cdn-payload.ps1 -PackageRoot .\.tmp\client-release\public\local-client-release -PayloadRoot .\.tmp\cdn-upload\0.1.0.0 -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId prod-cdn-payload -Version 0.1.0.0 -DownloadUrl "https://cdn.example.com/releases/0.1.0.0/"
.\scripts\run-local-cdn-payload-smoke.ps1 -PayloadRoot .\.tmp\cdn-upload\0.1.0.0 -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-cdn-payload-smoke
.\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl "https://cdn.example.com/releases/0.1.0.0/launcher-manifest.json" -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId prod-cdn-smoke
```

`prepare-client-cdn-payload.ps1` 会把 public package 复制到可上传目录，生成固定名 `launcher-manifest.json`，并输出 `cdn-upload-manifest-*.json`，用于发布前核对 CDN 上传文件、SHA256 和大小。`run-local-cdn-payload-smoke.ps1` 会临时启动 localhost 静态服务并复用 CDN smoke 逻辑验证 payload 可被启动器式下载。本地调试可显式使用 `-AllowLocalHttp` 指向 localhost 静态服务器；生产 `client.cdn_launcher_smoke` 证据必须使用真实 HTTPS CDN，脚本会下载 manifest 中的全部文件、校验 SHA256 与大小，并写入安装目录 `version.txt`。

启动器安装/更新核心烟雾：

```powershell
.\scripts\run-launcher-install-update-smoke.ps1 -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-launcher-install-update
.\scripts\capture-launcher-ui-evidence.ps1 -EvidenceDir .\Artifacts\ProductionEvidence\client -RunId local-launcher-ui
```

安装/更新核心烟雾会纳入 `collect-production-evidence.ps1 -RequireAll` 的 `client.launcher_install_update` 类别。它运行 Tauri Rust 层定向测试，覆盖 manifest 拉取、本地包下载、SHA256 校验、修复安装和 `version.txt` 持久化。UI 视觉证据会纳入 `client.launcher_ui_visual` 类别；`capture-launcher-ui-evidence.ps1` 会构建启动器、启动 Vite preview、用 headless 浏览器截图并校验渲染 DOM 中的关键界面标记。正式发布候选包仍需重新采集并归档该证据。

k6 压测证据默认写入 `Artifacts/ProductionEvidence/load`，也可通过 `EVIDENCE_DIR` 指定：

```bash
cd DBA_GameBackend
BASE_URL=http://localhost:8080 EVIDENCE_DIR=../Artifacts/ProductionEvidence/load bash scripts/run-load-tests.sh
```

若本机未安装 `k6`，`run-load-tests.sh` 可通过 `USE_DOCKER_K6=1` 使用 Docker 镜像 `grafana/k6:latest`。本地 Production-like 环境建议使用 `AUTH_MODE=guest`，并降低 VU/时长，避免与认证限流冲突：

```bash
cd DBA_GameBackend
USE_DOCKER_K6=1 AUTH_MODE=guest BASE_URL=http://localhost:8080 \
LOGIN_RAMP_TARGET_VUS=1 LOGIN_RAMP_UP_DURATION=1s LOGIN_RAMP_HOLD_DURATION=20s LOGIN_RAMP_DOWN_DURATION=1s LOGIN_SLEEP_SECONDS=2 \
MATCHMAKING_VUS=1 MATCHMAKING_DURATION=20s MATCHMAKING_SLEEP_SECONDS=5 \
bash scripts/run-load-tests.sh
```

备份恢复演练默认写入 `Artifacts/ProductionEvidence/ops`，包含运行日志与 summary JSON：

```bash
cd DBA_GameBackend
bash scripts/rehearse-backup-restore.sh
```

联机验证分支（仅后端服务链路）：

```powershell
.\scripts\production-preflight.ps1 -SkipUnrealAutomation -SkipUnrealServerSmoke
```

联机客户端联调（启动双客户端窗口）：

```powershell
.\scripts\production-preflight.ps1 -SkipUnrealAutomation -SkipUnrealServerSmoke -RunUnrealOnlineClients
.\scripts\production-preflight.ps1 -SkipUnrealAutomation -SkipUnrealServerSmoke -RunUnrealOnlineClients -UsePackagedUnrealServer -CollectEvidence -EvidenceRoot .\Artifacts\ProductionEvidence -RunId packaged-ue-online-local
```

`production-preflight.ps1` 会优先使用 `-InternalApiKey` 或环境变量 `DBA_INTERNAL_API_KEY`；本地开发环境未显式传入时，会从 `DBA_GameBackend/.env` 读取 `INTERNAL_API_KEY`，但不会把密钥写入证据文件。

GitHub Actions 中的 `.github/workflows/unreal-evidence.yml` 是手动触发的 UE 证据工作流，要求自托管 Windows runner 同时带有 `self-hosted`、`Windows`、`Unreal` 标签，并配置：

- Repository Secret：`DBA_INTERNAL_API_KEY`
- Repository Variable：`UNREAL_ENGINE_ROOT`，默认可使用 `D:\UnrealEngine-5.8.0-release`
- runner 本机可访问的 backend `base_url`

本地与 GitHub 自托管 runner 使用同一个入口 `scripts/run-unreal-evidence.ps1`。接入前可先执行只读诊断；正式采集可复用 workflow 同款入口：

```powershell
.\scripts\diagnose-unreal-evidence-runner.ps1 -BaseUrl "http://localhost:8080" -JsonOutputPath .\Artifacts\ProductionEvidence\unreal\runner-diagnostic-local.json
.\scripts\run-unreal-evidence.ps1 -BaseUrl "http://localhost:8080" -UsePackagedServer -EvidenceRoot .\Artifacts\ProductionEvidence -RunId local-ue-evidence
```

本地由 `run-unreal-evidence.ps1` 外部启动 packaged Dedicated Server 时，backend 编排模式应使用 `GAME_SERVER_MODE=External`，由脚本负责进程生命周期和日志证据。UE online validation 现在不仅检查双客户端 travel 和 `player-joined` 请求发出，还要求 server log 中出现两次 `/runtime/servers/player-joined` 业务 `OK`，并拒绝 `ERROR`，避免 HTTP 200 但业务失败的假阳性。

已部署环境烟雾测试：

```powershell
.\scripts\production-smoke-backend.ps1 -BaseUrl "https://api.example.com" -EvidenceDir .\Artifacts\ProductionEvidence\ops
```

## 生产计划

生产可上线计划记录在：

- `docs/solution-audit-and-production-plan.md`
- `docs/app-structure-code-audit.md`
- `DBA_GameBackend/docs/production-readiness-plan.md`

当前自动化层面已经具备后端测试、Compose 配置校验、观测配置、备份恢复脚本、部署回滚工作流、GM RBAC、启动器版本清单、本地 packaged UE Dedicated Server 双客户端端到端证据、Shipping public client package 符号分离证据，以及本地 HTTP CDN/启动器下载烟雾能力。剩余上线阻塞主要是预生产自托管 UE runner 官方证据、真实 HTTPS CDN 发布证据、启动器 UI 安装更新链路、签名和上线回滚演练归档。

## 分阶段修改计划

当前修改计划按以下顺序推进：

1. `P0`：修正文档说明、明确解决方案边界、执行基础构建验证。
2. `P1`：梳理后端 API/Worker 依赖边界，强化生产配置校验和关键链路测试。
3. `P2`：接入真实 UE Dedicated Server 包与客户端包，完成登录到进服端到端联调。
4. `P3`：完善启动器 CDN manifest、官网反馈/下载闭环、Admin 运营验收。
5. `P4`：完成观测、备份恢复、安全扫描、分支保护和上线/回滚 Runbook。

详细审查和计划见 `docs/solution-audit-and-production-plan.md`。

## 文档编码

中文文档统一按 UTF-8 保存。若在 Windows PowerShell 5.1 中直接 `Get-Content` 出现乱码，请使用：

```powershell
Get-Content README.md -Encoding UTF8
```

## GitHub 分支保护

仓库管理员可执行：

```bash
GH_TOKEN=<repo-admin-token> bash scripts/configure-branch-protection.sh freebooz DivineBeastsArena
```

该脚本会把 `solution-ci` 纳入 `main` 分支保护要求。
