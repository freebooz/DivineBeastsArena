# 步骤 16：3D Character Preview Stage、Camera 与 PreviewActor

## 本阶段的收敛结论

| 现有实现 | 状态 | 目标实现 | 迁移办法 |
| --- | --- | --- | --- |
| `ADBACharacterPresentationActor` | ADAPT | `ADBACharacterPreviewStage` + `ADBACharacterPreviewActor` + `ADBACharacterPreviewCameraRig` | 旧 `FrontendMap` 未放置新 Stage 前，由 `UDBACharacterPreviewSubsystem` 只读回退调用其展示接口；不再向它新增业务。 |
| Widget 对已放置展示 Actor 的直接旋转调用 | DEPRECATE | `UDBACharacterPreviewSubsystem::Rotate/Zoom/ResetCamera` | 后续 UI 迁移只调用 Subsystem；本阶段不直接编辑 UMG/地图二进制资产。 |
| 生肖展示资源的同步路径/硬编码候选 | DEPRECATE | `UDBAZodiacHeroDataAsset` + `UDBAZodiacRegistrySubsystem::LoadAsync` | 新 Preview Actor 只读取单个生肖 DataAsset 的软引用；旧舞台保留直至资产和地图人工迁移完成。 |
| `DBACharacterPreviewBridge` | UNKNOWN | 无 | 源码审计未发现该类，未创建同名平行实现。若外部插件或未纳入源码的蓝图仍引用它，需在资产迁移时提供窄 Adapter。 |

## 唯一运行链

`角色选择/创建 Controller` → `UDBACharacterPreviewSubsystem` → `UDBAZodiacRegistrySubsystem`（单生肖 Primary Asset 异步加载）→ `ADBACharacterPreviewStage` → `ADBACharacterPreviewActor` / `ADBACharacterPreviewCameraRig`。

`PreviewActor` 只拥有 Skeletal Mesh、共享 `UDBACharacterAppearanceComponent`、动画、可选 Niagara 和音频组件。它不继承 Gameplay Character，不创建 AbilitySystem，不复制，不包含 Combat 或 AI。

每次 `SelectZodiac` 都递增请求代次。Registry 回调、资源加载回调和离开页面的释放都校验同一代次；因此鼠→虎→龙快速选择时，仅最后的龙请求能应用资源。离开角色页面应调用 `ReleasePreview`，其会取消句柄、卸载当前生肖 Primary Asset，并销毁动态 PreviewActor/CameraRig。

## Camera 与场景规则

- 全屏选择使用持久 Frontend 世界中的真实 3D Stage 和 `UCameraComponent`，不引入 SceneCapture2D 或 RenderTarget。
- `FDBAZodiacPreviewCameraPreset` 扩展为 `Distance`、`Height`、`MinDistance`、`MaxDistance`；这些配置位于每个生肖 DataAsset，可覆盖体型差异。未配置字段才沿用 Stage Camera Rig 的可编辑场景默认值。
- `ADBACharacterPreviewStage` 提供 `SpawnPoint`、`CameraRigAnchor`、`KeyLight`、`FillLight`、`RimLight`、`BackgroundAnchor`、`VfxAnchor`。灯光、背景、位置和可选 Blueprint 外壳都由关卡/资产配置，不由 C++ 写入美术资源路径。
- Dedicated Server 侧不会创建 Stage、Actor、CameraRig 或加载 Preview 软资源；所有入口先拒绝 `NM_DedicatedServer`。

## L_DBA_Frontend 人工装配清单

本阶段遵守二进制资产边界，未直接编辑任何 `.umap`/`.uasset`。关卡设计师在持久前台地图中执行以下人工装配：

1. 放置一个 `ADBACharacterPreviewStage`（或仅配置表现的 Blueprint 子类），设定 Spawn/Camera/Background/VFX Anchor 与三点光。
2. 在 Stage 上配置可选的 PreviewActor 和 CameraRig Blueprint 类；它们必须继续以本阶段 C++ 类为父类，不能把流程逻辑写进 Blueprint。
3. 为 12 个 `UDBAZodiacHeroDataAsset` 建立单生肖 Primary Asset，填写 `BodyMesh`、动画、可选 VFX/SFX 与 CameraPreset；不得填入 GameplayCharacter 作为预览替身。
4. 手工进入角色选择和创建页，确认真实场景镜头、拖拽旋转、缩放、重置、选择动画、快速切生肖以及离页释放行为。

在新 Stage 与 12 个 DataAsset 配齐前，旧 `ADBACharacterPresentationActor` 仍在当前 `FrontendMap` 中提供可回滚的兼容展示；它位于不可删除列表。

## 验证契约

新增 `DBA.Frontend.Preview.AsyncRequestGeneration` Automation 测试代码，覆盖请求代次的鼠→虎→龙乱序以及离页失效规则。依据项目人工审核策略，本步骤未自动运行该测试或自动启动前端；编译仅作为工程检查，交互/画面由人工审核确认。
