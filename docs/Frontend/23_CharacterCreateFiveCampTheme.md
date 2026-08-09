# 步骤23：角色创建第三步 FiveCamp 与前台主题

## 目标与边界

账号角色创建的第三步使用唯一的 `EDBAFiveCamp`：

`Zodiac(+Appearance) -> Element -> FiveCamp -> Confirm/Name`

FiveCamp 是创建角色的长期表现主题选择，只驱动前台背景、徽记、VFX 材质预设与主题音效。它不参与元素克制、属性/伤害、GAS 技能、网络复制或对局敌我；本步骤的代码没有读取、推导或修改 `TeamId`，也没有新增 `Faction` GameplayTag。

## 现有实现 → 目标实现 → 迁移方式

| 现有实现 | 结论 | 目标实现 | 迁移方式 |
| --- | --- | --- | --- |
| `EDBAFiveCamp`（`GameCore/Types/DBACommonEnums.h`） | KEEP | 账号创建和前台主题的唯一五营枚举 | 所有新 API 仅接受此枚举。 |
| `EDBAFiveCampType` 与 `DBAIdentityTypeAdapter` | ADAPT | 历史资产/旧流程的兼容边界 | 不全局改名；新创建流程不引用旧类型，遗留调用只能在既有 Adapter 处转换。 |
| `FDBAFiveCampDisplayRow` | KEEP | 五张卡片和预览主题的唯一展示数据定义 | 在 `UDBAFrontendSettings::CharacterCreateFiveCampDisplayTable` 配置软引用；表行由策划维护。 |
| `UDBAFiveCampDataAsset` | DEPRECATE（创建链路） | 不再作为角色创建数据源 | 其旧 GameplayTag/推荐角色字段不进入本步骤运行路径，待资产审计后单独迁移。 |
| `UDBAFiveCampSelectWidgetBase`、`WBP_DBA_FiveCampSelect`（Lobby） | ADAPT / ISOLATE | 赛前选择遗留 UI | 不复用到账号创建，以免与 Match Select 的超时/Team 语义混用。 |
| `ADBACharacterPreviewStage` | KEEP / EXTEND | FiveCamp 主题表现的唯一舞台入口 | 通过 C++ `ApplyFiveCampTheme` 接收已异步解析的主题；蓝图只将资源接到已有美术锚点。 |

本次审计未发现可作为新运行时入口的 `DivinePantheon` 类型或资产。若后续迁移发现该命名，只能经现有 `DBAIdentityTypeAdapter` 映射到 `EDBAFiveCamp`，不能向新创建页扩散。

## 运行时职责

| 对象 | 职责 | 禁止职责 |
| --- | --- | --- |
| `UDBACharacterCreateWidgetController` | 异步加载显示表、校验行、写入 Draft、发起主题预览 | 创建/销毁 Screen、修改 TeamId、承载 Faction 规则。 |
| `UDBACharacterCreateFiveCampViewModel` | 五营卡片、选中态、结构化校验信息的只读投影 | 读取 DataTable、加载资产、写 Draft。 |
| `UDBACharacterCreateFiveCampStepWidgetBase` | 依据 ViewModel 动态生成卡片，转发 Next/Back/选择意图 | `OpenLevel`、`AddToViewport`、直接访问 Subsystem。 |
| `UDBACharacterCreateFiveCampInfoPanelWidgetBase` | 将当前卡片投影交给 UMG 布局 | 推导业务属性或可用性。 |
| `UDBACharacterPreviewSubsystem` | 前台唯一预览入口，将主题转交给 Stage | 加载 GameplayCharacter 或 DS 资源。 |
| `ADBACharacterPreviewStage` | 保存主题表现投影、播放主题音效、通知表现蓝图 | 覆盖 `AppearanceComponent` 或处理对局队伍。 |

## 数据表契约

`FDBAFiveCampDisplayRow` 已提供以下配置：`FiveCampEnum`、名称/描述、图标、徽记、背景、主题/辅助颜色、`EffectMaterial`、`ThemeSound`、`SelectionMusic`、可用性和解锁等级。

Controller 从配置表动态构建卡片，拒绝以下坏数据：无效枚举、`None`、重复的 FiveCamp 枚举、错误行结构和空表。未开放或有解锁等级的行会保留在 UI 中但禁用，并提供中文原因。由于账号创建阶段没有角色等级上下文，`UnlockLevel > 0` 当前一律不可选，避免客户端猜测解锁资格。

所有主题大资源继续使用软引用；只在选中当前五营后异步加载背景、徽记、VFX 材质和主题音效。主题请求与数据表请求分别有代次和可取消句柄，快速切换卡片或关闭页面时，旧回调不能覆盖当前舞台。

## PreviewStage 表现接线

`ADBACharacterPreviewStage` 新增 `BP_OnFiveCampThemeApplied` 与 `BP_OnFiveCampThemeCleared`。在不修改业务逻辑的前提下，`L_DBA_Frontend` 中的舞台蓝图应在 Editor 手工完成下列绑定：

1. 将 `BackgroundTexture` 应用于 `BackgroundAnchor` 对应的背景组件或 UI 材质实例。
2. 将 `EmblemTexture` 应用于徽记组件。
3. 将 `VfxMaterial` 应用于 `VfxAnchor` 下已有 Niagara/材质实例的主题参数。
4. 使用 `ThemeColor` / `SecondaryColor` 驱动现有灯光、材质和 UI 高亮的表现参数。
5. 不在蓝图中更改 Draft、Flow、PreviewActor 外观或任何 Team/Match 字段。

本步骤遵守二进制资产不直接编辑约束，未移动或修改 `.uasset/.umap`；因此必须在 `Project Settings -> DBA Frontend` 配置 `CharacterCreateFiveCampDisplayTable`，并在 UMG/Stage 蓝图把上述 C++ 父类和事件接入后再进行人工运行审核。

## 关键流程

```text
WBP_DBA_CharacterCreate_FiveCampStep
  -> UDBACharacterCreateFiveCampStepWidgetBase
  -> UDBACharacterCreateWidgetController::SelectFiveCamp
  -> UDBACharacterCreateDraftSubsystem::SetFiveCamp
  -> DraftChanged / FiveCampViewModel
  -> 异步解析当前 FDBAFiveCampDisplayRow 的主题软资源
  -> UDBACharacterPreviewSubsystem
  -> ADBACharacterPreviewStage::ApplyFiveCampTheme
  -> BP_OnFiveCampThemeApplied（仅美术表现）

Next / Back -> UDBAFrontendFlowSubsystem -> Draft 状态机
```

## 人工审核准备

工程检查和自动化测试不能替代人工验收。完成数据表和蓝图绑定后，人工应在可见客户端中验证：

1. 五张卡片均来自数据表，禁用行显示中文原因。
2. 选择任意卡片时，生肖外观不变，而背景/徽记/VFX 主题/音效切换。
3. 快速切换多个卡片后，最终停留的卡片主题不会被旧异步请求覆盖。
4. Back 回 Element 后 Draft 保留；Next 后进入 Confirm/Name。
5. 对局 TeamId 和赛前 Lobby 五营选择不随本页操作变化。
