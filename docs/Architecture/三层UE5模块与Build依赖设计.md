# 三层 UE5 Module / Plugin 结构与 Build.cs 依赖设计

## 目的

本文把《总体架构设计与审查报告》中的三层架构要求落到当前 UE5.8 仓库的模块、目录、头文件和 `Build.cs` 依赖规则上。它是后续 C++ 类定义、GAS 基类落地、UI 分层、资源管理、Dedicated Server 剥离和自动化校验的工程基线。

当前仓库不做一次性破坏性重命名，先以现有模块稳定推进：

```text
GameCore -> Foundation / 通用基础层
GameMoba -> MobaCore / MOBA 通用层
DivineBeastsArena -> ArenaGame / 项目业务层
```

长期插件化命名可以逐步演进为：

```text
DBAFoundationRuntime -> DBAMobaRuntime -> DBAArenaGameRuntime
```

迁移必须以可编译、可测试、可回滚的小步完成，不允许为了贴合目标命名一次性移动大量源码或资源。

## 依赖方向

允许：

```text
DivineBeastsArena -> GameMoba
DivineBeastsArena -> GameCore
GameMoba -> GameCore
```

禁止：

```text
GameCore -> GameMoba
GameCore -> DivineBeastsArena
GameMoba -> DivineBeastsArena
任意循环依赖
任意模块 include 其他模块 Private 目录
任意源码使用 ../ 跨目录绕过 Public/Private 边界
```

`Build.cs` 应保持层级语义清晰：

| 模块 | 允许依赖 | 禁止依赖 | 说明 |
| --- | --- | --- | --- |
| `GameCore` | UE Core、Engine、基础输入、基础 UI、HTTP/Json、后端客户端抽象 | `GameMoba`、`DivineBeastsArena`、十二生肖业务模块 | Foundation 可以提供通用服务抽象，但不能知道 MOBA 或 DBA 规则。 |
| `GameMoba` | `GameCore`、GAS、GameplayTags、GameplayTasks、通用 UI/输入、AI 基础 | `DivineBeastsArena`、具体生肖、具体 FiveCamp、具体地图规则 | MobaCore 提供可复用 MOBA 骨架。 |
| `DivineBeastsArena` | `GameCore`、`GameMoba`、GAS、Niagara、项目后端客户端 | 无反向依赖要求，但客户端表现模块必须 server guard | ArenaGame 实现十二生肖、元素、FiveCamp、具体 UI 和产品业务。 |

## Public / Private 头文件规则

- `Public` 只能放跨模块可见的类型、接口、数据结构和最小依赖头。
- `Private` 只能放模块内部实现，不允许被其他模块 include。
- 跨模块访问优先通过接口、GameplayTag、DTO、DataAsset、WidgetController 或服务抽象。
- 禁止用 `../` include 绕过模块边界。
- 禁止在 Foundation 公开头里包含 MOBA 或 ArenaGame 类型。
- 禁止在 MobaCore 公开头里包含具体十二生肖、元素构建、FiveCamp 表现或具体 UI 类型。

推荐 include 方向：

```text
DivineBeastsArena/Public -> GameMoba/Public, GameCore/Public
DivineBeastsArena/Private -> DivineBeastsArena/Public, GameMoba/Public, GameCore/Public
GameMoba/Public -> GameCore/Public
GameMoba/Private -> GameMoba/Public, GameCore/Public
GameCore/Public -> UE / platform / backend abstraction only
GameCore/Private -> GameCore/Public
```

## 接口放置

| 接口类型 | 放置层 | 示例 |
| --- | --- | --- |
| 通用服务接口 | `GameCore/Public/.../Interfaces` | 认证、服务器状态、支付请求、日志、错误码、资源加载。 |
| MOBA 通用战斗接口 | `GameMoba/Public/.../Interfaces` | Team、Targetable、Damageable、AbilityOwner、CombatState。 |
| DBA 项目业务接口 | `DivineBeastsArena/Public/GameDBA/.../Interfaces` | Zodiac、Element、FiveCamp、冻结构建摘要、项目 UI 控制器。 |

低层接口不得引用高层枚举或类。如果确实需要跨层传递项目数据，使用上层适配器把项目类型转换为低层 DTO 或 GameplayTag。

## DataAsset 与资源放置

| 数据类型 | 放置层 | 说明 |
| --- | --- | --- |
| DataAsset 基类、PrimaryAsset 基类、验证基类 | `GameCore` | 不包含 MOBA 或十二生肖字段。 |
| MOBA 通用规则资产 | `GameMoba` | 技能槽、通用属性、目标选择、通用战斗规则。 |
| DBA 项目资产 | `DivineBeastsArena` | Zodiac、Element、FiveCamp、FixedSkillBuild、具体技能、地图、皮肤、UI 资源。 |

资源引用原则：

- 表现资源使用软引用。
- 高配 VFX/SFX/材质通过 Bundle 或平台变体加载。
- Dedicated Server 不加载前台 UI、角色展示、媒体、音频、纯 VFX 资源。
- DataAsset 不得硬引用其他层 Private 资源或具体实现类。

## GAS 模块依赖

`GameMoba` 提供通用 GAS 骨架：

- AbilitySystemComponent 基类。
- GameplayAbility 基类。
- AttributeSet 基类。
- DamageExecution 基类。
- 技能槽与目标选择抽象。
- 通用 Buff / Debuff / Status 桥接。

`DivineBeastsArena` 扩展项目规则：

- Zodiac Ability。
- Element Ability。
- FixedSkillBuild。
- ElementCounter。
- ElementResonance。
- UltimateEnergy。
- ChainLevel。
- FiveCamp 表现包选择。

禁止把十二生肖技能名称、元素克制表、FiveCamp 表现包和具体技能 DataAsset 写入 `GameMoba` 或 `GameCore`。

## Dedicated Server 编译与表现剥离

`DivineBeastsArena.Build.cs` 中所有仅客户端需要的依赖必须放入：

```csharp
if (Target.Type != TargetType.Server)
{
    // RenderCore / RHI / AudioMixer / MediaAssets 等客户端表现依赖
}
```

Dedicated Server 路径禁止：

- 创建 Widget。
- 播放音频。
- 触发纯客户端媒体播放。
- 加载角色展示材质、皮肤预览、高配 VFX。

服务端可以触发 GameplayCue Tag 或复制状态，由客户端表现层消费。

## 自动化验证

当前仓库应至少持续运行：

```powershell
.\scripts\validate-unreal-module-boundaries.ps1
.\scripts\validate-unreal-source-guardrails.ps1
.\scripts\test-unreal-source-guardrails.ps1
.\scripts\validate-unreal-baseline-entrypoints.ps1
.\scripts\production-preflight.ps1 -SkipUnrealOnlineValidation
```

验证目标：

- `GameCore` 不依赖 `GameMoba` / `DivineBeastsArena`。
- `GameMoba` 依赖 `GameCore`，但不依赖 `DivineBeastsArena`。
- `DivineBeastsArena` 依赖 `GameCore` 与 `GameMoba`。
- 源码没有跨模块 `Private` include。
- 源码没有 `../` 相对父目录 include。
- 源码没有 `LogTemp`。
- `RenderCore`、`RHI`、`AudioMixer`、`MediaAssets` 等客户端表现依赖受 `Target.Type != TargetType.Server` 保护。
- source guardrail 有 fixture 自测，能证明未保护依赖失败、正确 guard 通过。

## 迁移策略

短期：

1. 保持 `GameCore / GameMoba / DivineBeastsArena` 模块稳定。
2. 用验证脚本防止新增反向依赖。
3. 优先补齐 Foundation/MobaCore 的日志、Tag、DataAsset、GAS 基线。

中期：

1. 把 Foundation 与 MobaCore 中已经稳定的抽象整理为插件候选。
2. 为 `DBAFoundationRuntime`、`DBAMobaRuntime`、`DBAArenaGameRuntime` 建立迁移计划。
3. 按模块逐步迁移，不在同一 PR 内移动大量源码和资源。

长期：

1. 形成插件化三层结构。
2. CI 强制 Build.cs 和源码边界检查。
3. Dedicated Server、客户端、Android 和资源包体都按层级和平台变体可验证。
