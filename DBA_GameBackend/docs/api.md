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

## 认证 `/api/auth`

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

## 客户端账号兼容 `/api/account`

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
| `GET` | `/api/players/{playerId}/public` | 玩家公开资料。 | 无 |

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
