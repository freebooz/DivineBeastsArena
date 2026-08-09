# 玩家自动游戏名接口

## 目标与边界

首次开户认证完成后，前台调用独立接口获取一个 **3 至 5 个汉字**的游戏玩家名。该名称保存于 `player_profile.nickname`，与账号登录名、`PlayerIdentity.DisplayName`、角色名完全分离：

- 账号登录名用于认证，不被自动改写。
- 游戏玩家名用于玩家 Profile 展示。
- 角色名仍由 CharacterService 以 `server_id + normalized_name` 约束，不受本功能影响。

## 自动填充流程

1. 开户存储创建 `PlayerProfile` 时写入临时唯一昵称，并将 `game_name_initialized` 设为 `false`。
2. `AuthService` 只签发 Token，不调用玩家名服务，也不在账号登录路径产生 Profile 写副作用。
3. UE 注册流程拿到 AccessToken 后自动调用 `POST /api/v1/auth/player-name/generate`；普通账号登录与刷新令牌不会调用该接口。
4. 服务端从 `PlayerGameName` 配置的单字姓氏和名字字库使用加密安全随机数生成 3–5 个汉字候选。
5. PostgreSQL 的 `nickname` 唯一索引与 `game_name_initialized` 并发标记共同保证并发首次调用不会覆盖先提交的昵称。玩家主动改名后，初始化标记保持为 true，后续调用只返回既有名称。

历史账号的迁移默认 `game_name_initialized = true`，因此不会在上线后被批量自动改名。

## 接口

`POST /api/v1/auth/player-name/generate`

- 需要 Bearer AccessToken。
- 不接受名称或 PlayerId 参数；服务端只读取 JWT 的 `player_id`，因此不能为其他账号生成或覆盖名称。
- 幂等：已完成初始化时返回既有昵称，`wasGenerated = false`。
- 初次补全时返回 `wasGenerated = true`。

旧 `/api/v1/auth/player-name/ensure` 仅为兼容适配器，返回 `Deprecation: true` 和 successor `Link`；新客户端不得调用。

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

在可见开发环境中注册一个全新账号，确认注册后出现一次独立 `/player-name/generate` 请求并展示 3–5 个汉字昵称；随后普通 `/login` 不得附带该请求。再次调用生成接口应返回同一昵称。不得用自动登录或自动接口脚本代替人工业务验收。
