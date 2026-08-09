# 步骤20｜CharacterCreateDraft 与四步创建状态机

## 审计与迁移

| 现有实现 | 审计结论 | 目标实现 | 迁移办法 |
| --- | --- | --- | --- |
| `UDBACharacterCreateFlowWidgetBase` 的 `CharacterName/SelectedZodiac/SelectedElement/SelectedFiveCamp` | ADAPT：Widget 内临时字段会在页面重建时丢失，且不含 Appearance | `UDBACharacterCreateDraftSubsystem` 的 `FDBACharacterCreateDraft` | Widget 字段仅作兼容显示镜像，写入统一 Draft。 |
| `UDBACharacterCreateWidgetController::PendingRequest` | MERGE：第二套创建请求缓存 | Draft 的 `BuildCreateRequest` | Controller 不再保存请求，只写 Draft 并驱动 Flow。 |
| `UDBAFrontendFlowSubsystem` 四个 `CharacterCreate_*` 状态 | KEEP：已具备前台权威路由状态 | Draft 的四步业务状态与 Flow 同步推进 | Flow 每次推进调用 Draft 校验，提交只允许 Confirm/Name。 |
| Lobby Hero/Element/FiveCamp Select 控件和 `DBAArenaConfig` 的倒计时 | KEEP、隔离 | 赛前 Match Select | 不依赖、不迁移、不读取；账号角色创建无 30 秒强制超时。 |

## 唯一职责

`UDBACharacterCreateDraftSubsystem` 是未创建角色的唯一业务草稿：保存生肖、外观稳定 ID、元素、五营、名称和仅用于展示的构筑摘要。它不保存 Token、密码、角色 ID，也不代表服务端已创建的角色。

`UDBAFrontendFlowSubsystem` 仍是唯一 Screen/状态路由：

`CharacterCreate_Zodiac → CharacterCreate_Element → CharacterCreate_FiveCamp → CharacterCreate_Confirm`

每次 `Next` 都由 Draft 的 `CanLeave/Validate` 守卫；`Back` 回退一个步骤，第一步 Back 才取消并回到 CharacterSelect。取消与创建成功均清空 Draft。没有角色时，角色列表加载完成会初始化空 Draft 后进入第一步。

## 外观与恢复

- 生肖默认外观从 `UDBAZodiacHeroDataAsset::DefaultAppearanceOptionIds` 异步读取；只记录 OptionId，不记录资产路径。
- 随机外观只在 `UDBAAppearanceCatalogDataAsset` 已通过 `UDBAFrontendSettings::CharacterAppearanceCatalog` 配置并加载后执行，候选项会同时经过当前生肖和 `AllowedAppearanceOptionIds` 过滤；目录不可用时拒绝随机，绝不生成未经校验的组合。
- `SerializeRecovery/RestoreRecovery` 只处理本地临时 JSON；调用者可选择通过 SaveGame 保存。恢复不会请求创建 API、不会生成角色 ID、不会修改服务端权威数据。

## 提交边界

Flow 在 Confirm/Name 步骤从 Draft 生成 `FDBACharacterCreateRequest`，并将 Draft.Appearance 交给 `UDBACharacterRosterSubsystem::CreateCharacter`。服务端仍校验名称唯一性、槽位、生肖、元素、五营和外观合法性；客户端仅表达意图。

## 不可删除与后续资产工作

- 现有 `WBP_DBA_CharacterCreate` 及其兼容 C++ 父类仍被 UI 流程注册表/既有资产引用，不能删除。
- 不直接编辑 `.uasset`。后续在 Editor 将步骤页面的 Next/Back/随机外观事件绑定到 Controller 的 `Next/Back/RandomizeAppearance`，并为 `CharacterAppearanceCatalog` 配置正式目录资产。
