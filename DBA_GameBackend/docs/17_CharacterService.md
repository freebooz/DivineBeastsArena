# 步骤 17：CharacterService、角色数据库与外观权威校验

## 收敛关系

| 现有实现 | 状态 | 目标 | 迁移办法 |
| --- | --- | --- | --- |
| `PlayerCharacter` / `EfPlayerCharacterStore` | ADAPT | `characters` 表和 `EfCharacterRosterStore` | 保留旧账号端点兼容，新增服务与旧链共享同一实体和 DbContext。 |
| `player_character` | DEPRECATE | `characters` | 迁移重命名表，增加 `server_id`、`normalized_name`、软删除和幂等键。 |
| `CoreAttributesJson` | KEEP | 既有对局兼容属性 | 新增 `character_progress` 作为进度权威表，Level 先保持投影兼容。 |

## 权威边界

`/api/v1/characters` 仅以 JWT 的 `player_id` 作为账号身份，所有读取、选择、删除均附带该所有权过滤，禁止由客户端传入 AccountId。角色名以 `server_id + normalized_name` 建立数据库唯一约束；角色 ID 为 UUIDv7。创建请求可带 `Idempotency-Key`，重复提交返回同一既有角色。

服务端独立读取 `CharacterCreation` 配置中的生肖、元素、五大阵营和每生肖外观规则版本；不会读取或信任 UE DataAsset。外观请求只允许稳定 Option ID，拒绝不属于当前生肖的 ID，也不持久化资产路径。

删除使用 `X-Character-Delete-Confirm: true` 二次确认并写入 `is_deleted/deleted_at`，不物理删除。`TeamId` 不出现在 CharacterService，绝不从 Zodiac、Element 或 FiveCamp 推导。

## 不可删除列表

- `PlayerCharacter` CLR 类型及既有 `IPlayerCharacterStore`：Session、Match、Admission 和旧 UE 账号端点仍在使用。
- `CharacterBuildPolicy`：对局固定技能组仍依赖其 Zodiac + Element 规则；FiveCamp 仅为表现维度。
