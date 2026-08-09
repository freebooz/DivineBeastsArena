# DBA_GameBackend API 文档

基础地址：`http://localhost:8080`

统一响应格式：

```json
{
  "data": {},
  "success": true,
  "message": null,
  "timestamp": "2026-05-22T00:00:00Z"
}
```

## 生产认证 `/api/v1/auth`

| 方法 | 地址 | 说明 | 鉴权 |
| --- | --- | --- | --- |
| `POST` | `/api/v1/auth/register` | 注册账号并返回 Token；前台随后调用独立玩家名接口完成首次开户编排。 | 无 |
| `POST` | `/api/v1/auth/login` | 仅执行账号认证与 Token 签发，不生成或修改玩家名。 | 无 |
| `POST` | `/api/v1/auth/refresh` | 刷新令牌 Rotation 入口。 | 无 |
| `POST` | `/api/v1/auth/logout` | 注销并撤销刷新令牌。 | Bearer |
| `GET` | `/api/v1/auth/me` | 当前账号与玩家 Profile 摘要。 | Bearer |
| `POST` | `/api/v1/auth/player-name/generate` | 获取当前 JWT 玩家 3–5 个汉字游戏名；首次生成并持久化，后续幂等返回。 | Bearer |
| `POST` | `/api/v1/auth/player-name/ensure` | Deprecated 兼容路径，迁移到 `/generate`。 | Bearer |

## 兼容认证 `/api/auth`（Deprecated）

已有 v1 successor 的登录、注册、刷新、登出和当前账号路径只供旧客户端迁移，响应包含 `Deprecation: true` 与 successor `Link`。游客、开发、外部平台和密码管理接口在获得正式 v1 successor 前保持现状。

| 方法 | 地址 | 说明 | 鉴权 |
| --- | --- | --- | --- |
| `POST` | `/api/auth/guest-login` | 游客登录，按设备 ID 创建或关联游客账号。 | 无 |
| `POST` | `/api/auth/account/login` | 玩家账号真实登录。 | 无 |
| `POST` | `/api/auth/account/register` | 注册玩家账号。 | 无 |
| `POST` | `/api/auth/dev-login` | 开发登录，仅开发环境使用。 | 无 |
| `POST` | `/api/auth/refresh` | 刷新访问令牌。 | 无 |
| `POST` | `/api/auth/logout` | 注销账号并撤销刷新令牌。 | Bearer |
| `GET` | `/api/auth/me` | 当前账号信息。 | Bearer |

开发账号见 `docs/dev-login-accounts.md`。

## 旧角色接口（Deprecated）

`/api/account/characters` 与 `/api/players/me/characters` 只供已发布客户端兼容，响应包含 `Deprecation: true` 和 `Link: </api/v1/characters>; rel="successor-version"`。新客户端只能使用下列 v1 CharacterService：

| 方法 | 地址 | 说明 | 鉴权 |
| --- | --- | --- | --- |
| `GET` | `/api/v1/characters?serverId=` | 获取账号在指定区服的角色列表。 | Bearer |
| `GET` | `/api/v1/characters/{id}` | 获取属主角色详情。 | Bearer |
| `POST` | `/api/v1/characters` | 幂等创建角色并进行权威外观/构筑校验。 | Bearer |
| `DELETE` | `/api/v1/characters/{id}` | 二次确认语义的软删除。 | Bearer |
| `POST` | `/api/v1/characters/{id}/select` | 选择属主角色。 | Bearer |
| `GET` | `/api/v1/servers` | 获取可缓存的权威区服目录。 | Bearer |
| `POST` | `/api/v1/game/enter` | 获取一次性 EnterWorld Ticket。 | Bearer |

### 历史兼容路径

| 方法 | 地址 | 说明 | 鉴权 |
| --- | --- | --- | --- |
| `GET` | `/api/account/characters` | 读取当前账号角色列表和已选角色。 | Bearer |
| `POST` | `/api/account/characters` | 创建角色并写入数据库。 | Bearer |
| `POST` | `/api/account/characters/{characterId}/select` | 选择角色并持久化。 | Bearer |

## 玩家 `/api/players`

| 方法 | 地址 | 说明 | 鉴权 |
| --- | --- | --- | --- |
| `GET` | `/api/players/me/profile` | 当前玩家资料。 | Bearer |
| `PUT` | `/api/players/me/profile` | 更新当前玩家资料。 | Bearer |
| `GET` | `/api/players/me/settings` | 当前玩家设置。 | Bearer |
| `PUT` | `/api/players/me/settings` | 更新当前玩家设置。 | Bearer |
| `GET` | `/api/players/me/stats` | 当前玩家统计。 | Bearer |
| `GET` | `/api/players/me/matches` | 当前玩家战绩列表。 | Bearer |
| `GET` | `/api/players/{playerId}/public` | 玩家公开资料。 | 无 |

### Player match history

`GET /api/players/me/matches` 从当前 Bearer JWT 派生玩家身份，不接受客户端传入 `playerId` 覆盖。每条战绩包含 `sessionId`、`mode`、`mapId`、冻结队伍 `team`、规范化 `result`、KDA、`score`、`durationSeconds`、`playedAt`，并从权威 `MatchResult` / `MatchPlayerResult` 派生原始 `resultJson`、结构化 `winnerTeam`（兼容 `winnerTeam` / `winner_team`）、`expDelta` 与 `rewards` 字典，供玩家侧赛后面板、历史战绩和奖励提示复用。

## 运营后台 `/api/admin`

| 方法 | 地址 | 说明 | 鉴权 |
| --- | --- | --- | --- |
| `POST` | `/api/admin/auth/login` | 管理员登录，返回后台 JWT。 | 无 |
| `GET` | `/api/admin/me` | 当前管理员资料。 | Bearer |
| `GET` | `/api/admin/players` | 玩家列表。 | Bearer |
| `GET` | `/api/admin/players/{playerId}` | 玩家详情、统计、背包、解锁内容。 | Bearer |
| `GET` | `/api/admin/matches` | 对局结算列表。 | Bearer |
| `GET` | `/api/admin/matches/{matchId}` | 对局结算详情。 | Bearer |
| `GET` | `/api/admin/servers` | 游戏服务器实例列表。 | Bearer |
| `GET` | `/api/admin/configs` | 配置列表。 | Bearer |
| `GET` | `/api/admin/inventory/logs` | 背包流水。 | Bearer |
| `GET` | `/api/admin/feedback` | 玩家反馈。 | Bearer |
| `GET` | `/api/admin/support/tickets` | 客服工单。 | Bearer |
| `GET` | `/api/admin/audit-logs` | 管理员审计日志。 | Bearer |

### Admin match result diagnostics

`GET /api/admin/matches` 的列表项包含结构化 `winnerTeam` 和原始 `resultJson`，用于运营快速查看 Dedicated Server 结算结果。当前 Admin 前端会优先读取列表项 `winnerTeam`，并兼容历史开发数据中的 `resultJson.winnerTeam` / `winner_team`，再与 `schema` 组合成列表摘要。

Team outcome summary：`GET /api/admin/matches/{matchId}` 的详情响应包含结构化 `winnerTeam` 与 `teamDistribution` 字段，分别从 `resultJson.winnerTeam` / `winner_team` 和玩家结果项的 `team` 字段生成。Admin 前端优先消费这些结构化字段，并保留 `resultJson` / 玩家列表 fallback，便于运营核对 Dedicated Server 权威结算是否与 `PlayerSession.Team` 一致。

`GET /api/admin/matches/{matchId}` 的玩家结果项包含 `rewards` 字典，展示本场结算写入的金币、荣誉、道具或其他整数奖励。该字段来自 Settlement 结果中的 `RewardJson`，为空时前端显示 `-`。

## 启动器和官网

| 方法 | 地址 | 说明 | 鉴权 |
| --- | --- | --- | --- |
| `GET` | `/launcher/manifest.json` | 启动器原始版本清单。 | 无 |
| `GET` | `/api/launcher/manifest` | 带统一响应包装的版本清单。 | 无 |
| `GET` | `/api/launcher/status` | 启动器后端状态。 | 无 |
| `POST` | `/api/feedback/` | 官网或客户端提交反馈。 | 无 |
| `GET` | `/api/platform/applications` | 平台应用结构清单。 | 无 |
| `GET` | `/api/live-ops/status` | LiveOps 运营概览。`/api/operations/status` 保留为兼容路由。 | 无 |

## 内部接口

| 前缀 | 说明 |
| --- | --- |
| `/internal/servers` | 游戏独立服务端注册、心跳、状态查询。 |
| `/internal/runtime` | 游戏服务器运行时管理。 |
| `/internal/settlement` | 对局结算提交和查询。 |

内部接口当前用于本地和内网联调，生产环境需要增加服务端身份认证、来源限制和审计。
