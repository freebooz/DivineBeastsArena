# 步骤18：UE CharacterRosterSubsystem、缓存与 DTO→Domain 映射

## 审计结论与迁移表

| 现有实现 | 当前引用情况 | 本步骤结论 | 迁移办法 |
| --- | --- | --- | --- |
| `UDBAFrontendFlowSubsystem::CachedCharacters` | 角色选择 Widget、Controller、旧 Flow 正在读取 | ADAPT | 仅保留为 UI 兼容投影；唯一权威缓存迁至 `UDBACharacterRosterSubsystem`。 |
| `UDBAOnlineAccountService::GetCharacterList/CreateCharacter/SelectCharacter` | 旧 Flow 直接调用 | DEPRECATE-LATER | 账号认证仍保留在该服务；角色列表、创建与选择已改由 Roster 调用 `/api/v1/characters`。后续删除旧角色 API 前须完成资产与蓝图引用审计。 |
| `FDBACharacterSummary` | GameCore、前台角色选择与服务接口正在使用 | KEEP | 继续作为角色列表唯一领域摘要，不新建同义 Summary。 |
| `FDBACharacterProfile` | 旧账户服务使用 | KEEP | 保持旧扩展档案契约；新增 `FDBACharacterDetails` 仅承载前台列表所需外观与选中状态。 |
| `FDBAOnlineAccountJson` | 旧账户 JSON 适配器 | ADAPT | Roster 只复用其已存在的 Zodiac/Element/FiveCamp 映射，新的 v1 Character DTO 解析在 Roster 内部完成。 |
| `UDBAApiClientSubsystem` | 已是唯一前台 HTTP 入口 | KEEP | Roster 经其发送请求；补充受限的业务附加 Header 通道，以支持幂等键与删除二次确认。 |

未发现已存在的 `UDBACharacterRosterSubsystem`、`FDBACharacterDetails` 或可复用的 v1 Character DTO Mapper。未移动或删除任何 `.uasset`、地图、蓝图或旧账户服务实现。

## 最终职责与调用链

```text
CharacterSelect Widget / ViewModel / Controller
  -> UDBAFrontendFlowSubsystem（状态转换与 UI 兼容投影）
  -> UDBACharacterRosterSubsystem（唯一角色领域入口、缓存、DTO 映射）
  -> UDBAApiClientSubsystem（认证、取消、超时、错误模型）
  -> GameBackendClient transport
  -> /api/v1/characters
```

- Widget 不接触 `DomainJson`、HTTP DTO、Token 或请求 Header。
- `FDBACharacterSummary` 是列表 UI 的领域模型；`FDBACharacterDetails` 仅在 Roster 中缓存外观 ID、稳定 ServerId 与选中状态，不持有角色 UObject 或资产路径。
- `FrontendFlow` 写入的会话信息仅为 `SelectedCharacterId` 与已有 Zodiac/Element/FiveCamp 摘要字段；Token、密码、RefreshToken 与进服 Ticket 均不进入 SessionContext。

## 缓存、异步与一致性

缓存键严格为 `(AccountId, ServerId)`。选服、登出、Token 失效及子系统销毁都会清空缓存；选服在写入新 ServerId 前取消并失效旧请求。刷新请求带内部代次，只有代次、账号和区服均匹配时才允许写入，故 `Server A -> Server B` 快速切换时 A 的迟到结果不会覆盖 B。

创建与删除采用“本地显式更新 → 广播领域快照 → 发起刷新”的策略。创建携带 `Idempotency-Key`；删除携带 `X-Character-Delete-Confirm: true`。选择成功后才写入 Flow 的 SelectedCharacterId，并由 Flow 决定进入世界状态，Roster 不创建、销毁或跳转任何 Widget。

角色 API 的网关地址、API 版本与超时不在 Roster 中硬编码：路径沿用唯一 ApiClient 的版本化网关配置，超时回落到 `UDBAExternalServiceSettings.RequestTimeoutSeconds`。

网络错误和无效 JSON 不覆盖已有缓存，回调返回 `FDBAOperationResult`。UI 应基于其中的结构化 `ApiError` / ErrorCode 展示错误，不能判断远端英文消息。

## 测试契约

新增 `DBACharacterRosterSubsystemTests.cpp`，覆盖：

- 空列表、单角色和配置最大槽位的 DTO→Domain 映射；
- 外观仅保留稳定 Option ID；
- 网络失败不被误判为成功；
- 账号/区服/请求代次不匹配时拒绝旧响应，覆盖换服竞态。

这些是工程自动化契约，不替代项目规定的人工 UI 审核。角色选择、创建、删角与进服仍需在 Editor/客户端可见运行环境中由人工执行确认。

## 仍保留的后续迁移项

`UDBAOnlineAccountService` 仍含旧角色 API 入口和本地账户状态保存，因现有认证与旧资产引用而不可在本步骤删除。后续在所有 CharacterSelect/创建蓝图父类完成人工审核与引用扫描后，才能标记这些旧角色方法为 Deprecated 并最终移除。
