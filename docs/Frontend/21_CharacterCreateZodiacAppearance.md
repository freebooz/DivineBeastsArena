# 步骤21：角色创建第一步（生肖与外观）

## 审计结论与迁移

| 现有对象 | 结论 | 迁移方式 |
| --- | --- | --- |
| `UDBACharacterCreateDraftSubsystem` | KEEP / EXTEND | 继续作为账号角色创建四步链唯一草稿；补充外观选项写入、默认恢复、合法化与候选项查询。 |
| `UDBAZodiacRegistrySubsystem` | KEEP | 唯一生肖枚举及异步 DataAsset 查询入口；创建页不再写死十二个按钮或资产路径。 |
| `UDBAZodiacHeroDataAsset` | KEEP / EXTEND | 补充创建页定位与难度文本；名称、简介、头像及预览配置仍由此静态配置主权威提供。 |
| `UDBACharacterPreviewSubsystem` | KEEP | 沿用既有 `RequestGeneration` 异步门控；创建页仅调用其选中、旋转、缩放和复位意图。 |
| 已有 `WBP_DBA_CharacterCreate`（Lobby/Frontend 两份） | ADAPT | 本步骤不直接编辑二进制资产；后续在 Editor 中将需要保留的布局绑定到新的 C++ 父类和 Controller。 |
| `WBP_DBA_CharacterCreate_ZodiacStep`、`WBP_DBA_ZodiacItem`、`WBP_DBA_AppearancePanel`、`WBP_DBA_CharacterPreviewControls` | CREATE (Blueprint 外壳待 Editor 创建) | C++ 父类已提供。Blueprint 仅负责绑定控件、列表行样式、动画和触控表现。 |

未发现第二套可复用的“账号角色创建生肖+外观”业务入口。赛前 Hero/Element 选择保持隔离，不能接入本 Draft 或页面 Controller。

## 唯一数据流

```text
ZodiacRegistry --异步加载--> CharacterCreateWidgetController --> Zodiac ViewModel --> WBP
                                      |                                  ^
                                      v                                  |
                                CharacterCreateDraft --变更事件----------+
                                      |
                         CharacterPreviewSubsystem（请求代次门控）
```

1. Controller 从 Registry 的实际枚举建立生肖列表；无 UI 资产路径和十二生肖 `if`。
2. 点击生肖只写入 Draft。Draft 异步取回该生肖默认配置，保存允许外观 ID 集合后显式归一化旧外观。
3. 所有不可用槽位从 Appearance Catalog 过滤后不生成显示组；可用选项以稳定 `OptionId` 写入 Draft。
4. Draft 变更事件驱动 ViewModel，并将生肖与合法外观交给 PreviewSubsystem。其既有请求代次防止 Rat → Tiger → Dragon 快速切换时旧资源覆盖新预览。
5. 下一步仍使用 Draft 的 `Next/CanLeave`；它验证生肖和外观，不引入第五个主创建步骤。

## 外观安全规则

- 外观持久化只使用稳定 ID，绝不写入客户端资源路径。
- 换生肖后不兼容 ID 会显式替换为 DataAsset 默认项、Catalog fallback 或空值，并通过 Draft 变更更新界面；不会残留非法组合。
- Randomize 只从当前生肖已加载 Catalog 的候选项抽取；Catalog 不可用时安全拒绝并记录中文日志。
- Reset 使用当前生肖的默认外观后再执行同一合法化流程。
- Widget 不访问 HTTP、Token、原始 JSON 或 AssetManager；Controller 只编排，Draft/Registry/Preview 分别保留业务、配置与资源生命周期职责。

## Editor 绑定清单（人工资产步骤）

由于仓库禁止直接编辑 `.uasset`，以下工作必须在 Unreal Editor 的 Undo Transaction 中完成并 Compile/Save：

- 为 `WBP_DBA_CharacterCreate` 保留/设置父类 `UDBACharacterCreateFlowWidgetBase`；在 Construct 胶水中调用 `GetOrCreateWidgetController` 一次，并把同一返回值注入全部下列子 Widget。
- 为 `WBP_DBA_CharacterCreate_ZodiacStep` 设置父类 `UDBACharacterCreateZodiacStepWidgetBase`，配置 `ZodiacItemClass` 为 `UDBAZodiacItemWidgetBase` 子类，并绑定 `ZodiacListContainer`。
- 为 `WBP_DBA_AppearancePanel` 设置父类 `UDBAAppearancePanelWidgetBase`；用 `BP_OnAppearanceGroupsChanged` 按数据生成实际支持的行与选项。
- 为 `WBP_DBA_CharacterPreviewControls` 设置父类 `UDBACharacterPreviewControlsWidgetBase`；将鼠标拖动、手柄/触控手势转换为 Rotate、Zoom、Reset 调用。
- 保留一份权威 `WBP_DBA_CharacterCreate`，使其持有同一个 `UDBACharacterCreateWidgetController`；另一份现有同名资产只在实际引用确认后再迁移或废弃。
- 为 12 个生肖 DataAsset 配置创建页“定位”和“难度”，并确认默认外观、允许 OptionId 和预览软引用完整。

## 人工审核建议

在可见 PIE/客户端中人工执行：连续选择鼠、虎、龙；观察只显示最后一个生肖的预览；切换到不支持角/尾的生肖后确认选项回退；测试随机、重置、拖动旋转、缩放与复位；最后验证 Next 仅在生肖和外观合法时进入元素步骤。此文档不将任何自动化测试视为验收。
