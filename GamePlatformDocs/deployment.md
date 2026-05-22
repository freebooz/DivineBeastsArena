# 部署说明

## 本地检查

```powershell
cd D:\DivineBeastsArenaPlatform
.\GamePlatformScripts\check-platform.ps1
```

如需验证 Unreal 编辑器目标：

```powershell
.\GamePlatformScripts\check-platform.ps1 -IncludeGameClient
```

`-IncludeGameClient` 需要本机安装 Unreal Engine，并且 `-UnrealBuildBat` 指向正确的 `Build.bat`。

## 容器启动

```powershell
cd D:\DivineBeastsArenaPlatform\GamePlatformOps\docker
docker compose up --build
```

默认服务：

- `Game.Api`: `http://localhost:8080`
- `DBA_GameAdmin`: `http://localhost:8081`
- PostgreSQL: `localhost:5432`
- Redis: `localhost:6379`
- Prometheus: `http://localhost:9090`
- Grafana: `http://localhost:3000`

## 数据库迁移

```powershell
cd D:\DivineBeastsArenaPlatform\DBA_GameBackend
dotnet ef database update --project Game.Infrastructure\Game.Infrastructure.csproj --startup-project Game.Api\Game.Api.csproj --context GameDbContext
```

Linux 容器中可以使用：

```bash
./GamePlatformOps/scripts/migrate-db.sh
```

## 单应用启动

```powershell
dotnet run --project DBA_GameBackend\Game.Api\Game.Api.csproj
dotnet run --project DBA_GameAdmin\GameAdmin.csproj
cd DBA_GameWebsite; npm run dev
cd DBA_GameLauncher; npm run dev
```
