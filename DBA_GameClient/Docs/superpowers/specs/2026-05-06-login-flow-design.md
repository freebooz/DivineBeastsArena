# 用户登录闭环设计

日期：2026-05-06

## 目标

第一版登录功能的目标是打通可玩前台闭环：

```text
启动 -> 登录 -> 角色列表 -> 创建角色 -> 大厅
```

本设计采用“真实后端优先，Mock 兜底”的方案。客户端优先连接真实登录服务器；当服务器不可用、接口未实现或请求超时时，降级到本地 Mock 账号服务，保证开发阶段仍可完整进入角色创建和主大厅。

## 已确认约束

- 使用现有 `UDBAAccountServiceBase` 作为 UI 和前台流程依赖的统一账号服务接口。
- 新增 `UDBAOnlineAccountService` 作为真实后端适配层。
- 保留并复用现有 `UDBAMockAccountService` 作为本地兜底。
- 第一版要承接玩法 v4/v4.1：角色创建包含生肖、元素、阵营，生成固定技能组引用，并为后续 8 核心属性初始化提供数据。
- 第一版不实现完整账号平台，不处理支付、好友、实名、防沉迷、复杂权限、跨平台账号绑定等系统。

## 账号服务架构

服务结构：

```text
UDBAAccountServiceBase
  ├─ UDBAOnlineAccountService
  └─ UDBAMockAccountService
```

`UDBAOnlineAccountService` 继承 `UDBAAccountServiceBase`，实现在线登录、注册、Token 刷新、角色列表、角色创建和角色选择。它读取 `DefaultGame.ini` 中的登录服务器配置：

```text
DefaultLoginServer=127.0.0.1
DefaultLoginPort=8080
bAutoLogin=False
bRememberAccount=True
```

UI 只依赖 `UDBAAccountServiceBase`，不直接判断当前使用 Online 还是 Mock。账号服务内部负责兜底策略，并通过日志和可选状态提示暴露当前模式。

登录成功后统一写入：

```text
CurrentAccountInfo
SessionToken
角色列表缓存
FrontendSessionState
```

角色创建成功后保存：

```text
CharacterId
CharacterName
Zodiac
PrimaryElement
FiveCamp
FixedSkillGroupId
基础属性模板引用
```

## 前台流程

状态流：

```text
Startup
  ↓
TryAutoLogin
  ↓
LoginScreen
  ↓ 登录成功
LoadCharacterList
  ↓
有角色 -> CharacterSelect -> MainLobby
无角色 -> CharacterCreate -> MainLobby
```

### Startup

启动后读取配置和本地存档。如果启用了自动登录，或存在有效 `SessionToken`，进入自动登录尝试；否则显示登录页。

### TryAutoLogin

`UDBAOnlineAccountService` 先请求后端校验或刷新 Token。成功后进入角色列表加载。Token 无效时回到登录页。后端不可用、超时或接口未实现时，允许降级到 Mock 自动登录。

### LoginScreen

提供账号登录和注册入口。第一版可以保留游客登录按钮作为开发入口，但主路径是账号登录。

### LoadCharacterList

登录成功后立即拉取账号下角色列表。这里决定进入角色选择还是角色创建。

### CharacterSelect

已有角色时展示角色列表。玩家选择角色后调用角色选择接口，并进入 `MainLobby`。

### CharacterCreate

没有角色时进入角色创建流程：

```text
生肖选择 -> 元素选择 -> 阵营选择 -> 确认创建
```

创建成功后生成固定技能组引用和角色基础数据，然后进入 `MainLobby`。

### MainLobby

设置 `UDBAFrontendSessionSubsystem` 状态为 `MainLobby`。后续队伍、匹配、角色展示等系统从这里继续。

## 角色创建数据落点

创建请求最小字段：

```text
CharacterName
Zodiac
PrimaryElement
FiveCamp
```

创建成功返回角色摘要：

```text
CharacterId
CharacterName
Zodiac
PrimaryElement
FiveCamp
FixedSkillGroupId
Level
CreateTime
LastUsedTime
```

字段含义：

- `Zodiac` 决定基础形象、生肖定位、被动来源和基础属性模板。
- `PrimaryElement` 决定五行元素之力、属性加成、普通技能和终极技能的元素属性。
- `FiveCamp` 仅影响外观皮肤系和特效风格，不绑定元素，不影响属性和技能数值。
- `FixedSkillGroupId` 由 `Zodiac + PrimaryElement` 查表生成，例如 `FSG_Rat_Water`。

第一版不允许玩家自由装配技能。固定技能组来自玩法 v4 的生肖乘元素配置。

属性系统遵循 v4.1 的 8 个核心属性：

```text
MaxHealth
AttackPower
Defense
MoveSpeed
MaxEnergy
EnergyRegen
CriticalRate
CriticalMultiplier
```

以下数据不进入登录和角色存档第一版：

```text
CurrentHealth
CurrentEnergy
UltimateEnergy
ChainLevel
ElementResonanceLevel
```

它们属于对局运行时状态或本地推导状态。

角色创建完成后的流程：

```text
CreateCharacter 成功
  ↓
保存角色摘要
  ↓
SelectCharacter
  ↓
FrontendSessionState = MainLobby
```

## HTTP 协议

最小接口：

```text
POST /api/auth/login
POST /api/auth/register
POST /api/auth/refresh
GET  /api/account/characters
POST /api/account/characters
POST /api/account/characters/select
```

登录请求示例：

```json
{
  "email": "player@test.com",
  "password": "123456",
  "deviceId": "local-device-id"
}
```

登录响应示例：

```json
{
  "success": true,
  "token": "session-token",
  "account": {
    "accountId": "account_001",
    "displayName": "玩家001",
    "loginType": "Email",
    "status": "Normal",
    "level": 1,
    "experience": 0
  }
}
```

角色列表响应示例：

```json
{
  "success": true,
  "characters": [
    {
      "characterId": "char_001",
      "characterName": "水灵鼠",
      "zodiac": "Rat",
      "primaryElement": "Water",
      "fiveCamp": "WhiteTiger",
      "fixedSkillGroupId": "FSG_Rat_Water",
      "level": 1
    }
  ]
}
```

创建角色请求示例：

```json
{
  "characterName": "水灵鼠",
  "zodiac": "Rat",
  "primaryElement": "Water",
  "fiveCamp": "WhiteTiger"
}
```

登录后的请求统一携带：

```text
Authorization: Bearer <SessionToken>
```

## Mock 兜底策略

允许兜底的情况：

- 网络连接失败。
- 请求超时。
- HTTP 404 或 501，表示接口未实现。
- 后端返回明确的服务不可用。

不允许兜底的情况：

- 账号密码错误。
- 账号不存在。
- 账号封禁或冻结。
- 角色创建参数非法。
- Token 过期且刷新失败。

Token 过期时先尝试 `POST /api/auth/refresh`。刷新失败后回到登录页。

日志必须区分来源：

```text
[Account] Online login succeeded
[Account] Online unavailable, fallback to mock
[Account] Mock login succeeded
[Account] Login failed: invalid credentials
```

本地存档策略：

- 在线登录成功后保存 `AccountInfo + SessionToken`。
- Mock 登录成功后保存本地游客账号。
- Online 和 Mock 都通过统一的 `FDBACharacterSummary` 暴露角色数据。
- UI 不直接依赖后端模式，但可以展示当前模式提示。

## 错误处理

- 后端不可用：显示“服务器不可用，已切换本地模式”，或根据配置回到登录页。
- 登录失败：停留登录页，显示后端错误消息。
- 角色列表加载失败：允许重试；Mock 兜底时可继续本地角色流。
- 角色创建失败：停留确认页，显示失败原因，不丢失已选择的生肖、元素和阵营。
- Token 无效：尝试刷新；刷新失败后清理本地 Token 并回到登录页。

## 测试与验收

主流程必须可通过：

```text
启动游戏
  ↓
进入登录页
  ↓
账号登录成功或 Mock 兜底成功
  ↓
加载角色列表
  ↓
无角色时进入角色创建
  ↓
选择生肖、元素、阵营
  ↓
创建角色成功
  ↓
进入主大厅
```

测试场景：

- 在线登录成功：后端返回 Token 和账号信息，客户端保存 `SessionToken`，进入角色列表。
- 后端不可用：HTTP 连接失败或超时，自动切换 Mock，仍能进入角色创建和大厅闭环。
- 登录失败：密码错误、账号不存在、封禁账号，不进入 Mock，登录页显示错误。
- Token 自动登录：本地存在 Token 时，启动后尝试刷新或校验；成功后跳过登录页。
- 无角色账号：角色列表为空，自动进入角色创建流程。
- 已有角色账号：显示角色列表，选择角色后进入大厅。
- 角色创建成功：保存 `Zodiac / PrimaryElement / FiveCamp / FixedSkillGroupId`，进入大厅。
- 角色创建失败：停留确认页，不丢失前三步选择。
- v4/v4.1 数据约束：创建角色只保存选择和模板引用，不保存运行时状态。

验收标准：

- 登录功能对 UI 暴露统一的账号服务接口。
- Online 和 Mock 模式都能跑通前台闭环。
- 日志能明确区分在线成功、在线失败和 Mock 兜底。
- 角色创建结果能支持后续固定技能组和 8 核心属性初始化。
- 后端失败不会让玩家卡死在启动页。
- 账号密码错误、封禁、冻结不会走 Mock 伪装成功。

## 非目标

- 不实现完整真实后端。
- 不实现支付、好友、排行榜、防沉迷、实名、跨平台账号绑定。
- 不把对局运行时状态写入账号存档。
- 不在本阶段实现完整美术 UI，只提供 C++/Blueprint 可绑定的流程接口。
