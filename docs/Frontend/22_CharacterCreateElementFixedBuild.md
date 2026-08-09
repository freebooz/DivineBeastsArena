# 步骤22：角色创建第二步（元素与固定构筑）

## 审计与迁移

| 对象 | 结论 | 本步骤处理 |
| --- | --- | --- |
| `EDBAElement` | KEEP | 唯一元素枚举。前台创建与服务端请求继续使用该类型。 |
| `UDBASkillGroupGeneratorSubsystem` + `DT_FixedSkillGroups` | KEEP / EXTEND | 唯一 Zodiac×Element 固定构筑规则入口；新增主表异步就绪事件，仅驱动前台展示刷新。 |
| `FDBAZodiacElementFixedSkillGroupRow` | KEEP | 唯一构筑行；提供 RowId、技能稳定 ID、共鸣与属性展示字段。 |
| Lobby `UDBAElementSelectWidgetBase` / `UDBAFixedSkillGroupPreviewWidgetBase` | ADAPT / ISOLATE | 属于赛前选择；保留但不接入账号角色创建。其硬编码元素与占位技能文本不再供新前台业务使用。 |
| `UDBACharacterCreateDraftSubsystem` | KEEP / EXTEND | 元素变化时清空旧构筑；只接受规则生成的展示摘要。 |

## 唯一数据流

```text
FixedSkillGroupGenerator --异步就绪事件--> CharacterCreateWidgetController
                                                     |
Draft(Zodiac, Element) ------------------------------+
                                                     v
                                   Element ViewModel -> ElementStep / Info / Build / Attribute WBP
```

- 元素卡由 `GetAllElementTypes()` 生成；实际可选性由当前生肖对应的固定技能组行 `bEnabled && !bIsInDevelopment` 决定。
- 选择元素只写 Draft。Controller 随后查询固定技能组，生成 `FixedSkillBuildRowId` 与 `PreviewSummary` 的显示缓存。
- 技能列表是配置行的稳定 ID 投影，不存在添加、移除、换位或自由 SkillLoadout API。
- 共鸣控制时间与护盾加成只用于创建页属性预览；GAS 授予和战斗结算保持独立，服务端仍为最终权威。
- 本阶段未发现元素专用 Preview Material/VFX 配置，因此元素切换不会销毁或重建 PreviewActor，也不会伪造表现资源；以后仅在数据资产新增对应软引用时按需更新。

## Blueprint 资产绑定清单

仓库禁止直接编辑 `.uasset`，因此以下操作需在 Unreal Editor 的 Undo Transaction 中人工完成并 Compile/Save：

- 创建 `WBP_DBA_CharacterCreate_ElementStep`，父类为 `UDBACharacterCreateElementStepWidgetBase`，设置 `ElementCardContainer` 和 `ElementCardClass`。
- 创建 `WBP_DBA_ElementCard`，父类为 `UDBACharacterCreateElementCardWidgetBase`。
- 创建 `WBP_DBA_ElementInfoPanel`、`WBP_DBA_FixedSkillBuildPreview`、`WBP_DBA_AttributePreviewPanel`，分别使用对应的 `DBACharacterCreate*WidgetBase` C++ 父类。
- 在根 `WBP_DBA_CharacterCreate` 的 Construct 胶水中复用 `GetOrCreateWidgetController()`，将同一实例注入四个子 Widget；不得创建第二个 Controller。

## 人工审核建议

在可见 PIE 中，从生肖页进入元素页，确认五个元素卡片由配置生成；选择每个可用元素后观察固定技能、共鸣与属性摘要同步更新；Back 回生肖后再返回，确认草稿元素仍在；验证未配置或开发中的构筑不可选，Next 仅在有效元素已写入 Draft 时进入 FiveCamp。不得将本文件的自动化测试源码作为业务验收结论。
