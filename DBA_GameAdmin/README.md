# GameAdmin

Blazor Server 管理后台，使用 MudBlazor UI 组件库。

## 构建

```bash
dotnet build
```

## 运行

```bash
dotnet run
```

## 配置

`appsettings.json` 中配置 `Backend.BaseUrl` 指向 Game.Api 服务。

## 功能页面

- Dashboard - 系统概览
- Players - 玩家管理
- PlayerDetail - 玩家详情
- Matches - 比赛记录
- MatchDetail - 比赛详情
- GameServers - 游戏服务器
- Configs - 游戏配置
- Inventory - 背包管理
- AuditLogs - 审计日志

## 高危操作

以下操作需要二次确认并填写 reason：
- Ban / Unban 玩家
- Grant / Deduct 物品
- Kill 游戏服务器
- Publish / Rollback 配置