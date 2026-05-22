# Game.Api - 游戏API服务

## 项目概述

Game.Api 是基于 ASP.NET Core 10 Minimal API 构建的游戏后台主服务，提供所有RESTful API端点。

## 目录结构

```
Game.Api/
├── Endpoints/                    # API端点定义
│   ├── Auth/                     # 认证端点
│   ├── Player/                   # 玩家端点
│   ├── Config/                   # 配置端点
│   ├── Room/                     # 房间端点
│   ├── Match/                    # 匹配端点
│   ├── Session/                  # 会话端点
│   ├── Runtime/                  # 运行时端点
│   ├── Settlement/               # 结算端点
│   ├── GameServer/               # 游戏服务器端点
│   └── Operation/                # 运营端点
├── Extensions/                   # 扩展方法
│   ├── EndpointRouteBuilderExtensions.cs  # 端点注册扩展
│   └── EndpointResultsExtensions.cs       # 响应格式化扩展
├── Middleware/                   # 中间件
│   ├── TraceIdMiddleware.cs              # 请求跟踪ID
│   └── ExceptionHandlingMiddleware.cs    # 全局异常处理
├── Services/                     # 业务服务
│   └── Auth/                      # 认证服务
│       ├── AuthService.cs                # 认证服务实现
│       ├── AuthServiceResult.cs          # 认证结果
│       └── IAuthService.cs               # 认证服务接口
├── Properties/
│   └── launchSettings.json        # 启动配置
├── appsettings.json              # 应用配置
└── Program.cs                     # 入口点
```

## 核心功能

### 1. 认证接口 (AuthEndpoints)
- 访客登录 `/api/auth/guest-login`
- 开发者登录 `/api/auth/dev-login`
- 账号登录 `/api/auth/account/login`
- 账号注册 `/api/auth/account/register`
- 微信登录 `/api/auth/external/wechat`
- Steam登录 `/api/auth/external/steam`
- EOS登录 `/api/auth/external/eos`
- 刷新令牌 `/api/auth/refresh`
- 退出登录 `/api/auth/logout`
- 获取当前用户 `/api/auth/me`

### 2. 玩家接口 (PlayerEndpoints)
- 获取玩家资料 `/api/players/me/profile`
- 获取/更新玩家设置 `/api/players/me/settings`
- 获取玩家统计 `/api/players/me/statistics`
- 获取已解锁内容 `/api/players/me/unlocks`
- 获取玩家公开资料 `/api/players/{playerId}/public`
- 更新昵称 `/api/players/me/nickname`

### 3. 配置接口 (ConfigEndpoints)
- 获取配置清单 `/api/config/manifest`
- 获取当前配置 `/api/config/current`
- 获取指定配置 `/api/config/{key}`
- 发布配置 `/api/admin/config/publish`
- 回滚配置 `/api/admin/config/{key}/rollback`

### 4. 房间接口 (RoomEndpoints)
- 创建房间 `POST /api/rooms`
- 获取房间信息 `GET /api/rooms/{roomId}`
- 获取房间列表 `GET /api/rooms`
- 加入/离开房间 `/api/rooms/{roomId}/join|leave`
- 准备/开始游戏 `/api/rooms/{roomId}/ready|start`
- 踢出玩家 `/api/rooms/{roomId}/kick/{playerId}`

### 5. 匹配接口 (MatchEndpoints)
- 创建匹配票据 `POST /api/matchmaking/tickets`
- 获取匹配状态 `GET /api/matchmaking/tickets/{ticketId}`
- 取消匹配 `DELETE /api/matchmaking/tickets/{ticketId}`

### 6. 会话接口 (SessionEndpoints)
- 创建/获取会话 `/api/sessions`
- 加入/离开会话 `/api/sessions/{sessionId}/join|leave`
- 心跳检测 `/api/sessions/{sessionId}/heartbeat`
- 断线重连 `/api/sessions/{sessionId}/reconnect`

### 7. 运营接口 (OperationEndpoints)
- 背包 `/api/players/me/inventory`
- 排行榜 `/api/rankings/{mode}`
- 好友系统 `/api/friends`
- 邮件系统 `/api/mails`
- 商城 `/api/shop/items`
- 公告 `/api/announcements`
- 活动 `/api/events`
- 成就 `/api/players/me/achievements`
- 战绩 `/api/players/me/matches`
- 举报 `/api/reports`
- 客服工单 `/api/support/tickets`
- 版本检测 `/api/version/check`
- 运营统计 `/api/admin/analytics/overview|retention`

## 配置

### 启动配置 (Properties/launchSettings.json)
```json
{
  "profiles": {
    "Game.Api": {
      "commandName": "Project",
      "launchBrowser": true,
      "launchUrl": "swagger",
      "environmentVariables": {
        "ASPNETCORE_ENVIRONMENT": "Development"
      }
    }
  }
}
```

### 应用配置 (appsettings.json)
```json
{
  "Urls": "http://localhost:8080",
  "Jwt": {
    "Secret": "your-secret-key-min-32-chars-long!!",
    "Issuer": "GameApi",
    "Audience": "GameClients",
    "AccessTokenExpiryMinutes": 15,
    "RefreshTokenExpiryDays": 30
  },
  "Database": {
    "ConnectionString": "Host=localhost;Database=gamedb;Username=postgres;Password=postgres"
  },
  "Redis": {
    "ConnectionString": "localhost:6379"
  },
  "SeedData": {
    "Enabled": true
  }
}
```

## 中间件

### TraceIdMiddleware
为每个请求生成唯一的跟踪ID，在响应头中返回：`X-Trace-Id`

### ExceptionHandlingMiddleware
全局异常处理，将异常转换为统一的错误响应格式

## 统一响应格式

### 成功响应
```json
{
  "data": { ... },
  "success": true,
  "message": null,
  "timestamp": "2026-05-16T00:00:00Z"
}
```

### 错误响应（与成功格式统一）
```json
{
  "data": null,
  "success": false,
  "message": "404|Not Found|未找到资源",
  "timestamp": "2026-05-16T00:00:00Z"
}
```

## SignalR Hubs

### LobbyHub
- 路径：`/hubs/lobby`
- 用于大厅实时通信

## 健康检查

- `GET /health/live` - 存活检查
- `GET /health/ready` - 就绪检查（依赖数据库和Redis）

## 运行

```bash
cd Game.Api
dotnet run
# 访问 http://localhost:8080/swagger/index.html
```