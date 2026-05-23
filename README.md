# DivineBeastsArena

DivineBeastsArena 是一个 UE 5 多人在线游戏的单仓库解决方案。根目录采用 `DBA_` 前缀扁平化应用结构，方便外层 Git 仓库统一管理源码、外围应用、部署脚本和生产文档。

## 应用目录

- `DBA_GameClient`：Unreal Engine 客户端与 Dedicated Server 工程。
- `DBA_GameBackend`：.NET 游戏后端 API、Worker、共享契约、基础设施和测试。
- `DBA_GameAdmin`：Blazor GM 管理后台。
- `DBA_GameWebsite`：Next.js 游戏官网。
- `DBA_GameLauncher`：Tauri 游戏启动器与更新器。
- `.github/workflows`：CI/CD 工作流。
- `scripts`：仓库级运维、预检和分支保护脚本。

## 常用验证命令

```powershell
cd DBA_GameBackend
dotnet build GameBackend.sln
dotnet test GameBackend.sln --no-build

cd ..\DBA_GameAdmin
dotnet build

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
- `DBA_GameBackend/docs/production-readiness-plan.md`

当前自动化层面已经具备后端测试、Compose 配置校验、观测配置、备份恢复脚本、部署回滚工作流、GM RBAC 和启动器版本清单能力。剩余上线阻塞主要是需要真实 UE Dedicated Server 包和真实客户端包进行端到端联调。

## GitHub 分支保护

仓库管理员可执行：

```bash
GH_TOKEN=<repo-admin-token> bash scripts/configure-branch-protection.sh freebooz DivineBeastsArena
```

该脚本会把 `solution-ci` 纳入 `main` 分支保护要求。
