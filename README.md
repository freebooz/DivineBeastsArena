# DivineBeastsArena

DivineBeastsArena 是一个 UE 多人在线游戏单仓库解决方案，当前根目录只保留五个正式应用工程：

- `DBA_GameClient`：Unreal Engine 客户端与 Dedicated Server 工程。
- `DBA_GameBackend`：.NET 游戏后端 API、Worker、共享契约、基础设施和测试。
- `DBA_GameAdmin`：Blazor GM 管理后台。
- `DBA_GameWebsite`：Next.js 官网。
- `DBA_GameLauncher`：Tauri 游戏启动器。

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

## 生产化计划

生产可上线任务计划记录在 `DBA_GameBackend/docs/production-readiness-plan.md`。当前重点是完成安全配置、真实指标、Dedicated Server 联调、客户端真实登录闭环、GM 审计、启动器补丁分发和部署回滚演练。

GitHub 主分支保护可由仓库管理员执行：

```bash
GH_TOKEN=<repo-admin-token> bash scripts/configure-branch-protection.sh freebooz DivineBeastsArena
```
