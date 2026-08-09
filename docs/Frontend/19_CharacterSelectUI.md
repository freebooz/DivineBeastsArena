# 步骤19：CharacterSelect UI、角色槽、详情与 3D 预览联动

## 审计与迁移

| 对象 | 现状 | 结论 | 本步骤处理 |
| --- | --- | --- | --- |
| `WBP_DBA_CharacterSelect` | Frontend 与 Lobby 各有一份二进制资产，父类为 `UDBACharacterSelectFlowWidgetBase` | ADAPT | 不移动、不删除资产；保留既有 C++ 父类，移除其直接全屏 Loading，并从 Roster 读取已选角色外观。 |
| `UDBACharacterSelectWidgetController` | 仅监听 Flow 列表并在点选时直接进服 | MERGE | 收敛为选择页唯一 Controller：领域快照、详情、Preview、删除确认、刷新、创建、进服和返回意图均由它编排。 |
| `UDBACharacterSelectViewModel` | 不存在 | ADD | 唯一可绑定显示状态，分离 Roster Loading 与 Preview Loading。 |
| `WBP_DBA_CharacterSlot` / `WBP_DBA_CharacterInfoPanel` | 未发现可确认的专用资产 | ADD-PARENT | 提供 C++ 父类，后续在 Editor 以布局/动画方式创建或将现有子控件改父类；不直接修改 `.uasset`。 |
| `UDBACharacterPreviewSubsystem` | 已具备异步生肖加载和请求代次保护 | KEEP | Controller 与旧选择页基类均传入 Roster 中的 `FDBACharacterAppearance`，不再一律传空外观。 |

## 最终链路

```text
Character Slot 点击
  -> CharacterSelectWidgetController.SelectCharacter
  -> CharacterRosterSubsystem.FindCachedCharacter
  -> CharacterSelectViewModel（详情、已选 ID、局部预览 Loading）
  -> CharacterPreviewSubsystem.SelectCharacter(Zodiac, Appearance)

Enter Game -> FrontendFlow.SubmitCharacterSelection -> Roster.SelectCharacter -> EnteringWorld
Delete -> Controller 确认状态 / WBP_DBA_ModalDialog -> Roster.DeleteCharacter -> 本地快照更新 + 刷新 + 安全选择下一项
```

角色槽展示 Name、Zodiac、Level、LastPlayed、Location 和 Portrait 绑定位；详情面板展示领域详情并独立显示预览加载。Portrait 资源仍应由 Zodiac DataAsset/Registry 按需供应，角色摘要不保存资产路径。

## 操作与输入

- `EnterGame` 仅在已选择角色且列表、预览均不在加载时发送 Flow 意图。
- `CreateCharacter` 只驱动 Flow 进入 `CharacterCreate_Zodiac`；空列表自动引导后，取消创建仍由状态机回到空的 CharacterSelect。
- `RequestDeleteSelectedCharacter` 只打开确认状态并广播当前摘要；`WBP_DBA_ModalDialog` 的确认/取消绑定到 Controller 的 `ConfirmDelete` / `CancelDelete`。删除请求仍由 Roster 执行。
- 既有选择页保留键盘/手柄前后切换、鼠标拖拽与触控拖拽预览；SafeZone 与布局继续由现有 Widget Blueprint 配置。
- 列表刷新和 Preview 资源加载状态分开，不再在点选角色时启动全屏 Lobby Loading。

## 验证边界

新增 ViewModel 契约测试覆盖初始安全选择、列表与预览加载状态独立、预览未完成时禁止进入游戏。该测试只验证工程契约，不替代人工在 Editor 中审核角色槽、Modal、手柄焦点、Android SafeZone 和真实 3D 场景。

## 后续资产前置条件

由于当前规则禁止直接编辑二进制 UMG 资产，下一次 Editor 人工操作需：

1. 将两个现有 `WBP_DBA_CharacterSelect` 的事件绑定到 `UDBACharacterSelectWidgetController`。
2. 使用 `UDBACharacterSlotWidgetBase` 与 `UDBACharacterInfoPanelWidgetBase` 创建/迁移对应的布局资产。
3. 将 `WBP_DBA_ModalDialog` 的确认、取消按钮绑定到 Controller，并以 UI Root 的 ModalLayer 挂载。
