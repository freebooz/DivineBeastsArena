# Game.Infrastructure - 基础设施层

## 概述

Game.Infrastructure 包含数据库访问、Redis缓存、认证实现等基础设施代码。

## 项目结构

```
Game.Infrastructure/
├── Auth/                 # 认证实现
│   └── JwtTokenService.cs # JWT令牌服务
├── Database/            # 数据库相关
│   ├── GameDbContext.cs # EF Core数据库上下文
│   ├── Entities/       # 数据库实体
│   │   ├── AuthEntities.cs    # 认证相关实体
│   │   ├── PlayerEntities.cs  # 玩家相关实体
│   │   ├── GameEntities.cs    # 游戏相关实体
│   │   └── GameFeatureEntities.cs # 游戏功能与运营支撑实体
│   └── Configurations/ # EF Core配置
│       ├── AuthConfigurations.cs
│       ├── PlayerConfigurations.cs
│       └── ...
├── Redis/              # Redis缓存
│   └── RedisConnectionFactory.cs
├── Database/Migrations # 数据库迁移
└── Seed/               # 数据种子
    └── DevelopmentDataSeeder.cs
```

## 数据库实体

### 认证相关 (AuthEntities)
- `Account` - 账号表
- `PlayerIdentity` - 玩家身份表
- `RefreshToken` - 刷新令牌表
- `DeviceLogin` - 设备登录表
- `BanRecord` - 封禁记录表

### 玩家相关 (PlayerEntities)
- `PlayerProfile` - 玩家资料表
- `PlayerSettings` - 玩家设置表
- `PlayerStatistics` - 玩家统计数据表
- `PlayerUnlock` - 玩家解锁记录表
- `PlayerEventLog` - 玩家事件日志表
- `InventoryItem` - 背包物品表
- `InventoryLog` - 物品日志表

### 游戏相关 (GameEntities)
- `GameConfig` - 游戏配置表
- `GameConfigPublishLog` - 配置发布日志表
- `GameRoom` - 游戏房间表
- `GameRoomPlayer` - 房间玩家表
- `MatchmakingTicket` - 匹配票据表
- `GameSession` - 游戏会话表
- `PlayerSession` - 玩家会话表
- `SessionEvent` - 会话事件表
- `GameServerInstance` - 游戏服务器实例表
- `GameServerEvent` - 服务器事件表
- `PortAllocation` - 端口分配表
- `MatchResult` - 比赛结果表
- `MatchPlayerResult` - 玩家比赛结果表

### 运营相关 (GameFeatureEntities)
- `PlayerRanking` - 玩家排行榜表
- `FriendRequest` - 好友请求表
- `FriendRelation` - 好友关系表
- `Mail` - 邮件表
- `MailAttachment` - 邮件附件表
- `Announcement` - 公告表
- `GameEvent` - 活动表
- `PlayerEventProgress` - 玩家活动进度表
- `Achievement` - 成就表
- `PlayerAchievement` - 玩家成就表
- `PlayerMatchHistory` - 玩家比赛历史表
- `Report` - 举报表
- `SupportTicket` - 客服工单表
- `TicketReply` - 工单回复表
- `ClientVersion` - 客户端版本表
- `DailyStats` - 每日统计表
- `RetentionCohort` - 留存队列表

## Entity Framework Core

### 数据库上下文 (GameDbContext)

```csharp
public class GameDbContext : DbContext
{
    public DbSet<Account> Accounts => Set<Account>();
    public DbSet<PlayerProfile> PlayerProfiles => Set<PlayerProfile>();
    // ... 其他实体集

    protected override void OnModelCreating(ModelBuilder modelBuilder)
    {
        base.OnModelCreating(modelBuilder);
        modelBuilder.ApplyConfigurationsFromAssembly(typeof(GameDbContext).Assembly);
    }
}
```

### 数据库迁移

```bash
# 添加迁移
dotnet ef migrations add <MigrationName> --project Game.Infrastructure --startup-project Game.Api --output-dir Database/Migrations

# 应用迁移
dotnet ef database update --project Game.Infrastructure --startup-project Game.Api

# 列出所有迁移
dotnet ef migrations list --project Game.Infrastructure --startup-project Game.Api
```

## Redis

### 连接工厂

```csharp
public interface IRedisConnectionFactory
{
    IConnectionMultiplexer GetConnection();
    IDatabase GetDatabase(int db = -1);
}
```

## JWT认证

### JwtTokenService

```csharp
public interface IJwtTokenService
{
    (string AccessToken, string RefreshToken, DateTimeOffset ExpiresAt) GenerateTokens(Account account, PlayerIdentity identity);
    string HashToken(string token);
    bool ValidateToken(string token);
}
```

## 数据种子

### DevelopmentDataSeeder

在开发环境下自动创建测试数据：

**管理员账号：**
| Username | Password | Role |
|----------|----------|------|
| admin | Admin@123456 | Admin |
| ops | Ops@123456 | Operator |
| frontend_debug | Frontend@123456 | Debug |
| dba_dev_01 | Dev@123456 | Client Dev |
| dba_dev_02 | Dev@123456 | Client Dev |
| dba_dev_03 | Dev@123456 | Client Dev |

**玩家账号：**
- player_001 ~ player_020
- 密码：Player@123456
