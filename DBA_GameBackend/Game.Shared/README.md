# Game.Shared - 共享代码库

## 概述

Game.Shared 包含所有项目共享的DTOs、Contracts、Error Codes和通用类。

## 项目结构

```
Game.Shared/
├── Common/              # 通用类
│   ├── ApiResponse.cs  # 统一API响应格式
│   └── ErrorResponse.cs # 错误响应格式
├── Contracts/           # 数据契约
│   ├── Auth/           # 认证相关DTO
│   ├── Player/         # 玩家相关DTO
│   ├── Config/         # 配置相关DTO
│   ├── Room/           # 房间相关DTO
│   ├── Match/          # 匹配相关DTO
│   ├── Session/        # 会话相关DTO
│   └── GameFeatures/      # 运营相关DTO
├── Errors/             # 错误码定义
│   └── ErrorCodes.cs  # 错误码常量
└── Options/            # 配置选项
    └── JwtOptions.cs  # JWT配置
```

## ApiResponse<T> 统一响应格式

```csharp
public class ApiResponse<T>
{
    public T? Data { get; init; }
    public bool Success { get; init; } = true;
    public string? Message { get; init; }
    public DateTimeOffset Timestamp { get; init; } = DateTimeOffset.UtcNow;

    public static ApiResponse<T> Ok(T data, string? message = null) => new()
    {
        Data = data,
        Success = true,
        Message = message
    };

    public static ApiResponse<T> Fail(string message) => new()
    {
        Data = default,
        Success = false,
        Message = message
    };
}
```

## 错误码 (ErrorCodes)

### 认证错误
- `INVALID_CREDENTIALS` - 账号不存在、密码错误或凭据缺失
- `TOKEN_EXPIRED` - 访问令牌或刷新令牌已过期/无效
- `REFRESH_REUSED` - 已撤销的刷新令牌被重放，活动刷新凭据已撤销
- `ACCOUNT_DISABLED` - 账号已禁用或封禁
- `AUTH_DEV_LOGIN_DISABLED` - 开发者登录已禁用
- `AUTH_STEAM_MOCK_ONLY` - Steam登录仅为模拟模式
- `AUTH_EOS_MOCK_ONLY` - EOS登录仅为模拟模式
- `AUTH_WECHAT_MOCK_ONLY` - 微信登录仅为模拟模式

### 玩家错误 (PLAYER_*)
- `PLAYER_NICKNAME_TAKEN` - 昵称已被占用
- `PLAYER_NOT_FOUND` - 玩家不存在
- `PLAYER_INVALID_NICKNAME` - 无效的昵称

### 配置错误 (CONFIG_*)
- `CONFIG_NOT_FOUND` - 配置不存在
- `CONFIG_ALREADY_PUBLISHED` - 配置已发布

### 房间错误 (ROOM_*)`
- `ROOM_NOT_FOUND` - 房间不存在
- `ROOM_FULL` - 房间已满
- `ROOM_ALREADY_JOINED` - 已加入房间
- `ROOM_NOT_OWNER` - 非房主

### 匹配错误 (MATCH_*)`
- `MATCH_TICKET_NOT_FOUND` - 匹配票据不存在
- `MATCH_TICKET_TIMEOUT` - 匹配超时

### 会话错误 (SESSION_*`)
- `SESSION_NOT_FOUND` - 会话不存在
- `SESSION_INVALID_STATE` - 无效的会话状态

### 游戏服务器错误 (SERVER_*)`
- `SERVER_NOT_FOUND` - 服务器不存在
- `SERVER_ALLOCATION_FAILED` - 服务器分配失败

### 库存错误 (INVENTORY_*`)
- `INVENTORY_INSUFFICIENT_QUANTITY` - 物品数量不足

### 管理员错误 (ADMIN_*)`
- `ADMIN_NOT_FOUND` - 管理员不存在
- `ADMIN_INVALID_CREDENTIALS` - 管理员凭证无效

## 使用示例

```csharp
// 成功响应
return Results.Ok(ApiResponse<PlayerProfileResponse>.Ok(profile));

// 错误响应
return ErrorResponse.NotFound(ErrorCodes.PlayerNotFound).ToProblem();
```
