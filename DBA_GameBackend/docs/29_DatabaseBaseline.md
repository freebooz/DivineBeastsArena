# 步骤29数据库最终基线

## 唯一权威上下文

`Game.Infrastructure.Database.GameDbContext` 是当前 Solution 唯一 EF Core DbContext。Design-time factory 只负责迁移工具创建同一个上下文，不是第二套数据库模型。

## 核心表与所有权

| 数据 | 权威表/存储 | Owner | Redis 用途 |
| --- | --- | --- | --- |
| 账号与密码凭据 | `account` 及认证关联表 | AuthService | 不保存密码 |
| 刷新令牌 | `refresh_token`，仅保存哈希、Rotation/Revoke 状态 | AuthService | 非权威 |
| 玩家 Profile | `player_profile` | PlayerService | 只做短缓存 |
| 区服目录 | `game_servers` 对应模型/配置源 | World/ServerDirectory | 短 TTL 目录缓存 |
| 角色 | `characters` | CharacterService | 列表缓存不得替代数据库 |
| 角色外观 | `character_appearances` | CharacterService | 非权威 |
| 角色进度 | `character_progress` | CharacterService | 非权威 |
| 游戏会话与连接票据 | Session/JoinTicket 表 | GameSessionService | 短 TTL Ticket 绑定与原子 consume |

`TeamId` 只来自会话队伍/槽位，不从 Zodiac、Element 或 FiveCamp 推导。数据库模型没有 Faction/FactionSelect 权威字段。

## 本轮迁移

`20260809140000_AddPlayerGameNameInitialization` 为 `player_profile` 增加 `game_name_initialized`。历史数据默认 true，避免上线迁移覆盖旧玩家昵称；新开户显式写 false，首次成功认证生成 3–5 个汉字玩家名后原子切换为 true。

## 迁移要求

在具备 .NET 10 SDK、目标 PostgreSQL 和正确 Secret 的环境中先执行 Backend Build，再由受控发布流程应用 EF migration。不得把开发连接串、真实密码或生产密钥写入迁移、源码或日志。
