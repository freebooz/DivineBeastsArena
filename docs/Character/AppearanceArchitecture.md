# 角色外观架构（步骤 15）

## 目标与边界

Preview Actor 与正式 `ADBAZodiacCharacterBase` 都挂载 `UDBACharacterAppearanceComponent`。两端接收相同的 `FDBACharacterAppearance`，因此创建角色阶段确认的外观可直接由正式角色恢复。

`FDBACharacterAppearance` 只保存稳定 ID：性别、体型、脸、发型、发色、肤色、瞳色、纹身、角、耳、尾、装备外观、武器外观与皮肤。它不包含、也不能反序列化任何 `/Game/...` 或其他客户端资源路径。

```text
数据库 / 服务 DTO（稳定外观 ID）
             ↓
FDBACharacterAppearance
             ↓
UDBACharacterAppearanceComponent
             ↓ 仅按需异步读取
UDBAAppearanceCatalogDataAsset（ID → 软引用资源）
             ↓
Preview Actor / Gameplay Character
```

## 当前实现与迁移

| 现有实现 | 标记 | 目标与迁移 |
| --- | --- | --- |
| 无 `CharacterAppearance` 类型或 Appearance Component | MERGE | 新增 `FDBACharacterAppearance` 与共享 `UDBACharacterAppearanceComponent`。 |
| `UDBAZodiacHeroDataAsset::DefaultAppearanceOptionIds` / `AllowedAppearanceOptionIds` | KEEP | 继续描述每生肖默认/允许 ID；实际 ID → 资源解析由 Appearance Catalog 负责。 |
| `UDBAZodiacCharacterRegistry` 的表现资源映射 | ADAPT | 旧生肖主体 Mesh/Anim 继续用于过渡；模块化部件、材质参数改由 Appearance Catalog 解析。 |
| `ADBACharacterPresentationActor` | KEEP | 新增 `ApplyPreviewAppearance`，复用共享组件。 |
| `ADBAZodiacCharacterBase` | KEEP | `CharacterAppearance` 仅复制稳定 ID；`OnRep_CharacterAppearance` 调用同一组件。 |
| 同步加载/直接资产路径持久化 | DEPRECATE | 禁止新增；Catalog 中的软引用只能由组件的异步请求读取。 |

## Appearance Catalog

`UDBAAppearanceCatalogDataAsset` 是唯一的 ID → 表现资源目录。每个 `FDBAAppearanceOptionDefinition` 包含：

- `OptionId`、槽位和生肖白名单；
- 可选的模块骨骼网格、Socket、材质替换和材质颜色参数；
- `bFallbackForSlot` 安全回退标记；
- 可选 CopyPose AnimBP。

UI 通过 `GetAvailableOptionIds(Zodiac, Slot)` 生成控件，不能自行维护十二生肖或外观部件列表。服务端应使用同一 Catalog/等价权威规则验证所有提交 ID。

当前没有可直接复用的 Appearance Catalog `.uasset`，本轮未直接创建或编辑二进制资产。应在 Editor 中创建并保存目录资产，然后在 Preview Actor 与正式角色 Blueprint/C++ 默认组件上配置其软引用。

## 模块化策略

- 主体与模块共享 Skeleton 时使用 UE5.8 `SetLeaderPoseComponent`。
- Skeleton 不一致时，只有显式配置 `CopyPoseAnimationClass` 才启用 AnimBP；否则跳过该部件并记录中文告警。
- 本阶段不使用 MeshMerge：现有生肖 Mesh 与 Skeleton 的共用性尚未经美术验证，盲目合并会破坏骨骼、材质槽与预览一致性。

## 异步与安全性

每次 `ApplyAppearance` 都递增请求代次并取消旧 `FStreamableHandle`。Catalog/资源回调必须匹配当前代次，角色切换、快速换发型或组件销毁后的旧回调都不会覆盖新状态。

非法、槽位不符或当前生肖不允许的 OptionId 会尝试该槽位 `bFallbackForSlot` 定义；没有回退定义时安全跳过该部件，保留主体模型。Dedicated Server 只持有稳定 ID，绝不加载 Mesh、Anim、材质或前台资源。

## 验证

新增 `DBA.Character.Appearance.SerializationRoundTrip` 工程契约，验证 ID 外观 JSON 序列化/反序列化的一致性及 JSON 中不含客户端资源路径。本轮遵守人工审核策略，未自动执行该测试。

人工审核前置条件：创建 Catalog 资产、为至少一个生肖配置主体兼容部件与 fallback，随后在可见 Preview 与正式角色中手动比较同一外观 ID 的结果。
