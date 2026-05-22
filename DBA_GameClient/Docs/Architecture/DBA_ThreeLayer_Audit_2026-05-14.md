# DBA 三层架构目录审计（2026-05-14）

## 1. 目标三层架构

1. 游戏核心通用基础层：`Source/GameCore`
2. MOBA 类型游戏层：`Source/GameMoba`
3. 神兽竞技场扩展层：`Source/DivineBeastsArena` + `Content/DBA`

## 2. 结论

当前代码模块分层总体是**合理且满足三层架构**的：

1. `GameCore` 承载账户、会话、队列、对象池、基础 UI 基类与通用类型。
2. `GameMoba` 承载 MOBA 通用能力系统、RPC 接口、HUD 基类。
3. `DivineBeastsArena` 承载生肖角色、DBA 专用 UI/VFX/战斗扩展逻辑。

但目录规划上存在可维护性问题，已完成一轮低风险优化（见第 4 节）。

## 3. 发现的问题

1. `Source/DivineBeastsArena/Public/.../Login` 下存在 `.cpp`，违反 UE C++ 常规规范（`Public` 放 `.h`，`Private` 放 `.cpp`）。
2. `Content/DBA/Zodiacs` 中语义混杂：
   - `DA_Zodiac_Aquarius...Virgo` 是西方黄道十二宫命名。
   - 新增的 `Rat/Ox/.../Pig` 是中国十二生肖视觉资产。
3. `Zodiacs` 中部分 `DA_Zodiac_*` 资产类加载失败（日志提示类 `DBADataAsset` 不存在），导致无法通过 EditorAssetLibrary 安全重命名这些资产。

## 4. 本次已执行优化

1. 代码目录修复（已执行）：
   - 将以下实现文件从 `Public` 移到 `Private`：
     - `UDBACharacterCreateFlowWidgetBase.cpp`
     - `UDBACharacterSelectFlowWidgetBase.cpp`
     - `UDBALoginFlowWidgetBase.cpp`

2. 资产目录分层（已执行，且不破引用）：
   - 新增：
     - `/Game/DBA/Zodiacs/Chinese/Visuals/Materials`
     - `/Game/DBA/Zodiacs/Chinese/Visuals/Meshes`
     - `/Game/DBA/Zodiacs/Western`
   - 已将十二生肖可视资源归档到 `Chinese/Visuals` 下（材质与网格已在该路径）。

## 5. 尚未自动完成的项（原因与建议）

1. `DA_Zodiac_Aquarius...Virgo` 未迁移到 `Western/DataAssets`：
   - 原因：资产可见但无法加载，重命名 API 返回失败。
   - 建议处理顺序：
     1. 修复 `DBADataAsset` 类可加载问题（确认模块/类名重构历史、重定向配置、蓝图父类）。
     2. 在编辑器内执行 `Fix Up Redirectors`。
     3. 再用编辑器资产迁移（不是文件系统直接移动）迁到 `/Game/DBA/Zodiacs/Western/DataAssets`。

2. `Content` 根目录仍有 `UI/Blueprints/Models/VFX/...` 并行目录，和 `Content/DBA` 双轨并存：
   - 建议逐步收敛 DBA 专属资产到 `Content/DBA`，公共资产保留在共享根目录。

## 6. 推荐最终目录（DBA 资产层）

```text
Content/DBA
├─ Core            # 公共基础资源（如通用材质、通用UI样式、共享DataAsset）
├─ Moba            # MOBA通用资源（地图规则、模式、通用战斗反馈）
├─ Arena           # 神兽竞技场扩展资源（生肖、阵营、元素、特效、专用UI）
│  ├─ Characters
│  ├─ Heroes
│  ├─ Zodiacs
│  │  ├─ Chinese
│  │  │  ├─ DataAssets
│  │  │  └─ Visuals
│  │  └─ Western
│  │     └─ DataAssets
│  ├─ UI
│  ├─ VFX
│  └─ Audio
└─ Data
```

## 7. 风险说明

1. `.uasset` 不建议用文件系统直接移动，必须通过编辑器资产工具迁移，否则引用容易断裂。
2. 当前 `DA_Zodiac_*` 的类加载异常，是后续目录优化的主要阻塞项。
