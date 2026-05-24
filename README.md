# DivineBeastsArena

DivineBeastsArena 是一个 UE 5 多人在线游戏的单仓库解决方案。根目录采用 `DBA_` 前缀扁平化应用结构，统一管理 UE 客户端、游戏后端、GM 后台、官网、启动器、部署脚本和生产文档。

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
| `DBA_GameLauncher` | Tauri / React / TypeScript / Rust | 游戏启动器、版本检查、下载、校验、修复、启动 | `npm run build` + `cargo check` |
| `.github/workflows` | GitHub Actions | CI、镜像构建、部署、回滚、安全检查 | `solution-ci` |
| `scripts` | PowerShell / Bash | 仓库级预检、生产烟雾测试、分支保护 | `production-preflight.ps1` |

## 本机 UE 工具路径

当前 Windows 开发机可使用以下 UE 安装目录：

```powershell
E:\UnrealEngine-5.7.1-release
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
cargo check --manifest-path src-tauri/Cargo.toml

cd ..\DBA_GameBackend
docker compose --env-file .env.example config
```

仓库级预检脚本：

```powershell
.\scripts\production-preflight.ps1
```

已部署环境烟雾测试：

```powershell
.\scripts\production-smoke-backend.ps1 -BaseUrl "https://api.example.com"
```

## 生产计划

生产可上线计划记录在：

- `docs/solution-audit-and-production-plan.md`
- `docs/app-structure-code-audit.md`
- `DBA_GameBackend/docs/production-readiness-plan.md`

当前自动化层面已经具备后端测试、Compose 配置校验、观测配置、备份恢复脚本、部署回滚工作流、GM RBAC 和启动器版本清单能力。剩余上线阻塞主要是需要真实 UE Dedicated Server 包和真实客户端包进行端到端联调。

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
