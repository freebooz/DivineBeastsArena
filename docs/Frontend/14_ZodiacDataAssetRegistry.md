# 步骤 14：十二生肖 DataAsset、AssetManager 与 Registry 收敛

## 结论

`UDBAZodiacHeroDataAsset` 是十二生肖静态配置的唯一目标类型。每个生肖对应一个 `ZodiacHero` Primary Asset；UI 通过 `UDBAZodiacRegistrySubsystem` 枚举和按需异步加载，不能再以 C++ 手写十二项列表或资产路径。

本轮未直接编辑或移动 `.uasset` / `.umap`。当前仓库没有位于 `Content/DBA/Data/Zodiac/` 的单生肖资产，因此 Registry 会明确报告“未扫描到 ZodiacHero Primary Asset”，而不会回退为隐式硬编码或同步加载十二生肖资源。

## 现状与迁移关系

| 现有实现 | 状态 | 目标实现 | 迁移办法 |
| --- | --- | --- | --- |
| `UDBAZodiacHeroDataAsset` 的多 DataTable 聚合模式 | ADAPT | 同类的单生肖静态配置模式 | 旧资产保留 `bLegacyTableCatalog=true`；新资产改为 `false`，才会以 `ZodiacHero` 注册。 |
| `DA_DBA_ZodiacCharacterSelection` | KEEP / ADAPT | 旧界面文本和技能表的只读兼容来源 | 现有选创角仍可读取其表数据；单生肖资产齐备后逐页迁移为 Registry 查询。 |
| `UDBAZodiacCharacterRegistry` / `DA_DBA_ZodiacCharacterRegistry` | ADAPT | 已配置角色类与展示资源的旧映射 Adapter | 不删除，避免已保存地图、GameMode 与外观设置失效；新 UI 不从它枚举十二生肖。 |
| `UDBAZodiacDataAsset`（`DBATypedDataAssets.h`） | DEPRECATE | `UDBAZodiacHeroDataAsset` | 搜索结果没有运行时引用；禁止新增此类型资产或业务依赖，后续清理前先在 Editor 复核无二进制资产使用它。 |
| `DT_HeroBalance`、生肖展示/技能 DataTable | KEEP / DERIVED | 派生文本、数值与技能说明 | 不再承担生肖预览资源主权威；其内容由单生肖资产关联或被 UI 作为过渡文本读取。 |
| `UDBAHeroSelectWidgetBase` 的 12 项数组 | MERGE | `UDBAZodiacRegistrySubsystem::GetAllZodiacTypes` | 已替换为 Registry 枚举。 |
| `UDBACharacterCreateFlowWidgetBase` 的 12 项数组 | MERGE | Registry 枚举，旧表只读兼容回退 | 已替换；未发现新资产时显示配置缺失，不会构造硬编码列表。 |
| `ADBACharacterPresentationActor` 与角色列表的 `Rat` 默认预览 | DEPRECATE | 无选择即不加载预览 | 已移除默认鼠生肖加载，避免无角色页面加载任意生肖资源。 |

## 单生肖资产契约

`UDBAZodiacHeroDataAsset` 继承的 `DisplayName`、`Description`、`Icon` 连同新增字段共同构成唯一静态入口：

- `ZodiacType`（可被 AssetRegistry 索引）、`Portrait`；
- `PreviewActorClass`、`GameplayCharacterClass`；
- `BodyMesh`、`HeadMesh`、`DefaultEquipmentAssets`；
- `AnimationBlueprintClass`、`IdleAnimation`、`SelectAnimation`；
- `PreviewVFX`、`PreviewSFX`、`CameraPreset`；
- `DefaultAppearanceOptionIds`、`AllowedAppearanceOptionIds`。

网格、动画、VFX、音效、角色类和装备均为软引用。Registry 初始化仅读取 `FAssetData` 标签，`LoadAsync` 只加载调用方选中的一个 Primary Asset，`Release` 取消未完成回调并卸载该资产；不会在 Boot 阶段预载十二生肖的高资源。

`TeamId` 不在该资产中，不能从生肖、元素或五大阵营推导。该类型也没有 Faction 业务字段；仅保留 `DeprecatedLegacyClassificationId` 作为迁移哨兵，任何非空值都会被数据校验阻断。

## AssetManager 与依赖边界

`DefaultEngine.ini` 新增：

```ini
PrimaryAssetType=ZodiacHero
AssetBaseClass=/Script/DivineBeastsArena.DBAZodiacHeroDataAsset
Directory=/Game/DBA/Data/Zodiac
```

依赖方向为：

```text
DataAsset（静态软引用）
        ↓ AssetRegistry / AssetManager 元数据
UDBAZodiacRegistrySubsystem
        ↓ 按需 LoadAsync
Frontend UI / Preview Adapter
```

Registry 位于 `Character/Data`，不依赖 UMG、CommonUI、Niagara 的运行时对象或前台地图；Dedicated Server 可以读取静态身份配置，但不应请求前台展示资源。旧 `UDBAZodiacCharacterRegistry` 仍由预览 Adapter 使用，属于下一次 Preview 层迁移的兼容边界，而不是新的枚举入口。

## 数据校验

每个单生肖资产的 Editor Data Validation 会：

- 阻断缺失 `ZodiacType`；
- 阻断 `DeprecatedLegacyClassificationId`（旧 Faction/分类）残留；
- 对 Portrait、预览 Actor、对局角色类、BodyMesh、AnimBP、Idle 资源缺失给出警告。

`UDBAZodiacRegistrySubsystem::ValidateConfiguration` 用于全局人工校验：检查十二生肖完整性、重复 `ZodiacType`、重复 `PrimaryAssetId` 与旧分类哨兵。它不会自动运行或自动判定验收。

## 十二资产创建/迁移清单（需在 Editor 中人工创建并保存）

目标目录为 `Content/DBA/Data/Zodiac/`，每项均创建为 `UDBAZodiacHeroDataAsset`，设置 `bLegacyTableCatalog=false` 后保存，使 AssetRegistry 写入 `ZodiacType` 标签：

| 目标资产名 | ZodiacType | 当前状态 |
| --- | --- | --- |
| `DA_DBA_Zodiac_Rat` | `Rat` | 待创建/迁移 |
| `DA_DBA_Zodiac_Ox` | `Ox` | 待创建/迁移 |
| `DA_DBA_Zodiac_Tiger` | `Tiger` | 待创建/迁移 |
| `DA_DBA_Zodiac_Rabbit` | `Rabbit` | 待创建/迁移 |
| `DA_DBA_Zodiac_Dragon` | `Dragon` | 待创建/迁移 |
| `DA_DBA_Zodiac_Snake` | `Snake` | 待创建/迁移 |
| `DA_DBA_Zodiac_Horse` | `Horse` | 待创建/迁移 |
| `DA_DBA_Zodiac_Goat` | `Goat` | 待创建/迁移 |
| `DA_DBA_Zodiac_Monkey` | `Monkey` | 待创建/迁移 |
| `DA_DBA_Zodiac_Rooster` | `Rooster` | 待创建/迁移 |
| `DA_DBA_Zodiac_Dog` | `Dog` | 待创建/迁移 |
| `DA_DBA_Zodiac_Pig` | `Pig` | 待创建/迁移 |

创建后在 Editor 执行 Asset Registry 重扫描和 Data Validation；确认十二项通过前，不移动旧 `DA_DBA_ZodiacCharacterSelection` / `DA_DBA_ZodiacCharacterRegistry`，也不执行 Fix Redirectors。

## 本轮未扩大处理的遗留项

`UDBALobbyPlayerHUDWidgetBase` 仍含旧大厅 HUD 的生肖文字和颜色 switch，属于进入游戏后的 HUD 表现而非本步骤的选角/创角枚举入口；其迁移需要先将异步 Registry 数据投影到 HUD ViewModel，避免在 HUD 静态函数中同步加载资产。本轮不以删除或降级该表现为代价修改它，已将其列为后续 UI 数据绑定前置项。
