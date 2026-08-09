# 步骤24：角色创建确认、名称与提交

## 现有实现 → 目标实现 → 迁移方法

| 现有实现 | 状态 | 目标实现 | 迁移方法 |
| --- | --- | --- | --- |
| `UDBACharacterCreateDraftSubsystem` | KEEP | 确认页唯一输入与摘要来源 | Confirm ViewModel 只投影 Draft，不保留第二份业务数据。 |
| `UDBACharacterRosterSubsystem::CreateCharacter` | KEEP / EXTEND | UE 唯一创建 HTTP 入口 | 增加由 Flow 持有的 Idempotency-Key；Widget 不接触 ApiClient。 |
| `UDBAFrontendFlowSubsystem::SubmitCharacterCreation` | KEEP / FIX | 创建单飞、完成回调、返回 CharacterSelect | 成功后不再自动进入世界，改为设置新角色前台选中态并回角色选择。 |
| 旧 `UDBAOnlineAccountService::CreateCharacter` | DEPRECATE-LATER | 保留兼容账户服务 | 新 Confirm 运行时不调用。 |
| `WBP_DBA_CharacterCreate_ConfirmStep`、`WBP_DBA_CharacterCreateSummary` | ADAPT | 分别继承新的 C++ Step/Summary 父类 | 本步骤不直接修改二进制 Widget；在 Editor 手工绑定控件和事件。 |

## 运行时链路

```text
Confirm Widget
  -> UDBACharacterCreateWidgetController
  -> UDBAFrontendFlowSubsystem（单飞门闩 / 幂等键）
  -> UDBACharacterRosterSubsystem
  -> POST /api/v1/characters
  -> CharacterService（最终校验）
  -> Roster 缓存 + FrontendSession.SelectedCharacterId
  -> Flow: CharacterSelect
  -> CharacterSelect Controller: 新角色 3D 预览
```

`Create` 双击由 Flow 的 `bCharacterCreateRequestInFlight` 拒绝。相同 Draft 的网络取消后，下一次提交复用同一 `Idempotency-Key`，避免服务端已收到首个请求时创建重复角色。幂等键不写日志、不暴露给 Widget。

## 确认页显示与边界

`UDBACharacterCreateConfirmViewModel` 提供角色名、生肖、外观项数量摘要、元素、五营、固定构筑和属性预览。最终 3D 预览继续使用现有 `UDBACharacterPreviewSubsystem` 与 Draft 的生肖/外观，不创建 GameplayCharacter。

本地校验仅用于即时 UX；名字唯一性、槽位、外观选项和构筑合法性必须等待服务端结果。错误映射覆盖目标码及当前服务码：

- `NAME_EXISTS` / `CHARACTER_NAME_DUPLICATE`
- `INVALID_NAME`
- `SLOT_LIMIT` / `CHARACTER_SLOT_LIMIT`
- `INVALID_APPEARANCE` / `APPEARANCE_OPTION_INVALID`
- `INVALID_BUILD` / `CHARACTER_BUILD_INVALID`
- `SERVER_MAINTENANCE`
- 网络连接、超时和服务不可用

失败时 Draft 保留，用户可返回各步骤修正。已知不可重试校验失败会生成新的幂等键；网络、超时或维护等可重试结果保留当前键。取消仅停止本地等待、失效旧回调并保留 Draft/键，不假定服务端未收到请求。

## 成功语义

服务端成功后：

1. Roster 将新详情加入缓存并刷新列表。
2. Flow 写入 `SelectedCharacterId` 与摘要，回到 `CharacterSelect`。
3. Draft 被清空。
4. CharacterSelect 接收缓存/状态事件后选中新角色并请求其 3D 预览。

创建成功不再自动 `EnteringWorld` 或 `ClientTravel`；进入游戏仍由角色选择页的显式 `EnterGame` 意图驱动。

## Editor 手工绑定与人工审核

由于不得直接编辑 `.uasset/.umap`，需在 Editor 中手工完成：

1. `WBP_DBA_CharacterCreate_ConfirmStep` 继承 `UDBACharacterCreateConfirmStepWidgetBase`，绑定 `CharacterNameInput`、`CreateButton`、`BackButton`、可选 `CancelButton`。
2. `WBP_DBA_CharacterCreateSummary` 继承 `UDBACharacterCreateSummaryWidgetBase`，在蓝图事件中绑定 ViewModel 文本、错误横幅与 Loading。
3. 根创建页将既有唯一 `UDBACharacterCreateWidgetController` 注入这两个子 Widget。
4. 在可见客户端人工验证：双击 Create、重复名字保留 Draft、网络取消后重试、成功回角色选择并预览新角色。

自动化测试不能替代人工审核；本步骤只新增 ViewModel 契约测试，默认不执行。
