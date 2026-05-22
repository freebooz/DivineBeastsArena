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

## 认证模型

GameAdmin 使用明确的 Admin Token 流程：

1. 登录页调用 Game.Api 的 `POST /api/admin/auth/login`。
2. 后端返回只包含后台身份和角色的 Admin JWT。
3. `ApiClient` 对所有后台 API 请求附加 `Authorization: Bearer <token>`。
4. 后端按 `SUPER_ADMIN / OPS / SUPPORT / VIEWER` 做角色授权和审计。
5. 前端读取 JWT 过期时间，过期或未登录时自动回到 `/login`。

当前没有把普通玩家 Token 用作后台身份，普通玩家 Token 不能访问 `/api/admin/*`。

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
