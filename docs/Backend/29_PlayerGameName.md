# 玩家自动游戏名接口

## 目标与边界

首次成功认证时，服务端为新玩家自动生成一个 **3 至 5 个汉字**的游戏玩家名。该名称保存于 `player_profile.nickname`，与账号登录名、`PlayerIdentity.DisplayName`、角色名完全分离：

- 账号登录名用于认证，不被自动改写。
- 游戏玩家名用于玩家 Profile 展示。
- 角色名仍由 CharacterService 以 `server_id + normalized_name` 约束，不受本功能影响。

## 自动填充流程

1. 开户存储创建 `PlayerProfile` 时写入临时唯一昵称，并将 `game_name_initialized` 设为 `false`。
2. 注册、账号登录、游客登录、开发登录或刷新令牌获得认证主体后，`AuthService` 在生成响应前调用 `IPlayerService.EnsureGeneratedGameNameAsync`。
3. 服务端从 `PlayerGameName` 配置的单字姓氏和名字字库使用加密安全随机数生成 3–5 个汉字候选。
4. PostgreSQL 的 `nickname` 唯一索引与 `game_name_initialized` 并发标记共同保证并发首次登录不会覆盖先提交的昵称。
5. 成功认证响应中的 `nickname` 即为最终游戏玩家名。玩家主动改名后，初始化标记保持为 true，后续认证绝不覆盖。

历史账号的迁移默认 `game_name_initialized = true`，因此不会在上线后被批量自动改名。

## 接口

`POST /api/v1/auth/player-name/ensure`

- 需要 Bearer AccessToken。
- 不接受名称或 PlayerId 参数；服务端只读取 JWT 的 `player_id`，因此不能为其他账号生成或覆盖名称。
- 幂等：已完成初始化时返回既有昵称，`wasGenerated = false`。
- 初次补全时返回 `wasGenerated = true`。

成功响应数据：

```json
{
  "playerId": "uuid",
  "nickname": "云昭羽",
  "wasGenerated": true
}
```

若字库配置无效、数据库不可用或候选耗尽，返回结构化错误码 `PLAYER_GAME_NAME_GENERATION_FAILED`。该路径不记录 AccessToken、RefreshToken、密码或 GameTicket。

## 配置与迁移

- `Game.Api/appsettings.json` 的 `PlayerGameName` 是唯一字库、长度与重试次数配置入口。
- `20260809140000_AddPlayerGameNameInitialization` 增加 `player_profile.game_name_initialized`，默认 true 以保护历史玩家。
- 配置启动校验要求长度恰为 3–5，且姓氏和名字字库的每一项必须是单个 CJK 基本汉字。

## 人工审核建议

在可见开发环境中注册一个全新账号并登录，确认响应和 `/me` 均展示 3–5 个汉字昵称；再次登录应返回同一昵称。随后手工修改 Profile 昵称，再次认证应保留人工昵称。不得用自动登录或自动接口脚本代替人工业务验收。
