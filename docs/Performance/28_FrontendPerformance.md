# 步骤 28：前台性能、资源释放与平台边界审计

## 本次结论

前台角色选择/创建链已经具备“当前生肖单项异步加载、快速切换以 generation 丢弃旧回调、离页释放预览资源”的主体机制。本步骤没有移动或删除任何 `.uasset` / `.umap`，而是在生命周期根部补上 Dedicated Server 子系统创建隔离，避免仅依赖各调用点的运行时分支。

本机没有可用的 UE 5.8 Engine / UnrealBuildTool，因而未运行 UE Insights、`stat` 或 Editor 人工审核；下述运行时项目为待人工审核清单，而非自动验收结论。

## 现状审计与所有权

| 范围 | 权威对象 | 静态证据 | 结论 |
| --- | --- | --- | --- |
| 十二生肖元数据 | `UDBAZodiacRegistrySubsystem` | Primary Asset 元数据枚举，按生肖 `LoadAsync` / `Release` | KEEP：启动阶段不要求装入十二个重资源 |
| 预览请求乱序 | `FDBACharacterPreviewRequestGate` / `UDBACharacterPreviewSubsystem` | 请求 generation 与弱引用回调 | KEEP：Rat→Tiger→Dragon 旧回调不能覆盖最后选择 |
| 展示资源生命周期 | `ADBACharacterPreviewActor` | 软加载 Mesh、Anim、VFX、SFX；`ReleasePreviewResources` 取消句柄、停音频/特效、清 Mesh | KEEP：离页释放由 Preview Stage / Flow 调用 |
| 角色选择 UI | `UDBACharacterSelectWidgetController` | 事件委托与弱引用，不含 `NativeTick` | KEEP：列表、详情和预览由事件驱动 |
| 根 UI | `UDBAUILayerManagerSubsystem` | 唯一 RootLayout 挂载入口，现已在创建阶段排除 DS | ADAPT：DS 隔离已收紧 |
| Preview 子系统 | `UDBACharacterPreviewSubsystem` | 现已覆盖 `ShouldCreateSubsystem`，Dedicated Server 返回 false | ADAPT：DS 不再实例化前台预览子系统 |
| Android 画质 | `Config/DefaultDeviceProfiles.ini` | `Android` / `Android_Low` 已配置分辨率、纹理池、Niagara 质量和帧率 | KEEP：需要真机人工复核实际预算 |

## 本次最小代码调整

1. `UDBACharacterPreviewSubsystem::ShouldCreateSubsystem` 在 Dedicated Server 进程返回 `false`。预览 Actor、舞台、相机与角色展示资源不再由 DS 子系统生命周期触及。
2. `UDBAUILayerManagerSubsystem::IsSupportedInCurrentEnvironment` 在 Dedicated Server 进程返回 `false`。这使用 `UDBAGameInstanceSubsystemBase` 的创建闸门，而非等待 World/LocalPlayer 就绪后的运行时判断。
3. 未改变客户端的 `IsClientUIRuntime` 二次检查；它仍负责防止无本地玩家的场景实际创建 Widget。

## 资源策略

```text
ZodiacRegistry Primary Asset 元数据
  -> 用户选中一个生肖
  -> PreviewSubsystem generation + LoadAsync
  -> PreviewActor 仅加载该生肖所需 Mesh / Anim / VFX / SFX
  -> 换生肖或离开角色页
  -> Registry.Release + PreviewActor.ReleasePreviewResources
```

- 启动页、登录页与选服页不得调用 `SelectZodiac`，因此不会预热十二套高清角色资源。
- 角色选择与创建页面允许保留当前预览，页面离开、切服、登出、Travel 前必须经过现有 Flow 清理入口释放。
- 本步骤不引入“相邻生肖预加载”。若后续在目标设备的停顿数据证明需要，应以可配置预算与可取消软加载单独实现，不能硬编码全量预热。

## UI 与异步审计

- 新的 CharacterSelection / CharacterCreate 前台链未发现 `NativeTick`。UI 更新应继续通过 Controller、ViewModel、Subsystem 委托与异步完成事件驱动。
- `StartupCoordinator`、`GameSessionSubsystem`、Flow 的定时器属于带取消/超时语义的基础设施；不等同于 Widget 的每帧轮询。
- 发现 Lobby 旧 HUD 仍存在 `NativeTick`，以及 Lobby 票据轮询。这两个对象不属于本次角色选择/创建链，标记为 **DEPRECATE-LATER / 后续专项审计**，不得复制到新前台页面。
- 部分历史 Flow Widget 注释仍提到由 UIManager `AddToViewport`。实际唯一挂载责任必须保持在 `UDBAUILayerManagerSubsystem`，后续 UI 清理应校正这些历史注释和直挂实现。

## 平台检查清单（人工审核）

### Windows

- 在 Editor 或独立客户端手动切换至少三种生肖并连续旋转、缩放、重置相机，确认最后选择的角色和主题稳定。
- 使用鼠标、键盘和手柄分别确认焦点、Hover Tooltip、Escape/Back 与 Modal 顺序。
- 用 Unreal Insights 或 `stat unit`、`stat memory` 手动记录进入角色选择、连续切换、离开页面后的 GameThread / RenderThread / GPU / 内存数据。

### Android

- 使用 SafeZone 设备和小屏设备手动走完登录→选服→选角→创建；确认触控热区、列表滚动、长按信息与系统 Back 可用。
- 分别以 `Android`、`Android_Low` Device Profile 审核 Preview VFX、背景、分辨率和纹理池是否符合目标机性能；低配机不应因前台特效导致掉帧或 OOM。
- Suspend 后恢复时，确认不会重复创建 UI Root，并重新校验 Token/Flow 有效性后才刷新页面。

### Linux Dedicated Server

- 启动 Dedicated Server 后人工检查日志和对象统计：不应创建 `UDBAUILayerManagerSubsystem` 或 `UDBACharacterPreviewSubsystem`。
- 不进入 `L_DBA_Frontend`，不生成 UMG/CommonUI Root、Preview Actor、Preview Stage、SceneCapture 或前台 Niagara 展示组件。
- 当前 `DivineBeastsArena.Build.cs` 仍因历史公开 C++ 类型保留 UMG/Slate/Niagara 公共依赖；这是**编译图风险**，不是本步骤允许的安全小改。后续需将纯前台公开类型拆至 Client-only 模块后，再从 Server 编译图移除这些模块。

## 内存回归人工记录模板

人工在可见客户端执行 100 次 CharacterSelect ↔ CharacterCreate 往返和多生肖切换后，记录：

| 项目 | 基线 | 第 100 次 | 人工结论 |
| --- | --- | --- | --- |
| UObject 数量 |  |  |  |
| SkeletalMesh / Anim / Niagara 常驻资源 |  |  |  |
| 预览 Actor 数量 |  |  |  |
| UI Root 数量 |  |  |  |
| 内存曲线是否持续增长 |  |  |  |

只有离页后对象与资源回落、且无持续增长趋势时，才能由人工标记通过。

## 已知后续风险

1. 当前主模块尚未拆分 Client-only 前台模块，Server Target 的模块依赖图仍包含历史公共 UI / Niagara 依赖；应在独立构建边界步骤处理。
2. `L_DBA_Boot`、`L_DBA_Frontend` 资产与默认地图配置尚未由本步骤创建或迁移，不能据此宣称完整冷启动人工验收通过。
3. 未在本机执行 UE Insights、Editor 编译或真机 Android / Linux DS 人工审核，原因是本机未发现可用 UE 5.8 工具链。
