# DivineBeastsArena VFX/SFX 系统审查报告

版本：v1.0
日期：2026-07-06
审查范围：`DBA_GameClient/Source`（GameCore、GameMoba、DivineBeastsArena 三个模块）
策略约束：`DBA.DataAsset.NoHardcoding`、`DBA.UI.EventAsync`、`DBA.Log.ChineseOutput`、`DBA.Agent.DirectExecution`

---

## 0. 执行摘要

本次审查覆盖 VFX（特效）与 SFX（音效）两大表现层系统的代码实现、数据资产、加载方式、配置入口和日志合规性。共识别 **3 项 P0 风险**、**5 项 P1 风险**、**4 项 P2 风险**。

### P0 风险（必须立即处理）

| 编号 | 问题 | 影响 |
|------|------|------|
| VFX-P0-1 | 6 处 UI 音效同步加载 `LoadObject<USoundBase>` | 阻塞 GameThread，违反 `DBA.UI.EventAsync` |
| VFX-P0-2 | 6 个 Spell/Projectile 子类构造函数硬编码 VFX/SFX 路径 | 违反 `DBA.DataAsset.NoHardcoding`，无法配置驱动 |
| VFX-P0-3 | `ADBACue_Base` 硬编码 DataTable 路径（2 处候选） | GameplayCue 查表逻辑脆弱，路径错误无明确日志 |

### P1 风险（短期处理）

| 编号 | 问题 | 影响 |
|------|------|------|
| VFX-P1-1 | 双特效系统并存未收敛（ParticleSystem + Niagara） | 数据结构不统一，迁移无计划 |
| VFX-P1-2 | VFX/SFX 数据字段散落 12+ 处，无统一数据资产入口 | 违反 NoHardcoding 策略"统一数据资产入口"精神 |
| VFX-P1-3 | 12 个核心 VFX/SFX 文件 0 日志输出 | Dedicated Server 故障无法定位，违反 `DBA.Log.ChineseOutput` |
| VFX-P1-4 | 缺少 VFX/SFX 专用 DeveloperSettings 配置入口 | 无法集中配置默认音效、BGM、预加载策略 |
| VFX-P1-5 | 音效系统缺混音控制（无 USoundClass/SoundMix） | 无法做 BGM/SFX/UI 音量分类与推子 |

### P2 风险（中期处理）

| 编号 | 问题 | 影响 |
|------|------|------|
| VFX-P2-1 | GameplayCue 实现简陋，仅一个基类无派生 | 未充分利用 Cue Parameters 与 Instanced Actor 生命周期 |
| VFX-P2-2 | `DBASkillVFXManager` 使用 TMap 存储 VFX/SFX 软引用 | 键名为 FName，缺乏校验，易出现配置漂移 |
| VFX-P2-3 | 5 处 Resonance GE 同步加载 DataTable | 间接影响 VFX/SFX 触发时序 |
| VFX-P2-4 | Niagara Spawn 未检查返回值 | Spawn 失败时无日志，难以排查 |

---

## 1. 审查范围与方法

### 1.1 审查范围

- **GameCore 模块**：`Source/GameCore/`（UI 基类、常量、Session）
- **GameMoba 模块**：`Source/GameMoba/`（MOBA 通用 UI 基类）
- **DivineBeastsArena 模块**：`Source/DivineBeastsArena/`（VFX/SFX 核心实现）

### 1.2 审查方法

- 语义检索：Niagara、ParticleSystem、SoundBase、AudioComponent、GameplayCue 等关键词
- 模式检索：`LoadObject`、`LoadSynchronous`、`SpawnEmitterAtLocation`、`PlaySoundAtLocation`、`SpawnSystemAtLocation`
- 路径检索：`TEXT("/Game/` 硬编码资源路径
- 配置检索：`UDeveloperSettings` 派生类、`Config=Game` 标记
- 日志检索：`UE_LOG`、`ensure`、`checkf` 在 VFX/SFX 文件中的覆盖情况

### 1.3 VFX/SFX 核心目录

```
DivineBeastsArena/
├── Public/GameDBA/VFX/
│   ├── DBASkillVFXManager.h
│   ├── Structs/DBAVFXDataRow.h
│   └── Components/
│       ├── DBAZodiacVFXComponent_Generic.h
│       └── Skill/DBAZodiacSkillVFXComponent_Generic.h
├── Public/GameDBA/Combat/
│   ├── DBASkillProjectileBase.h
│   ├── DBAFrostShardProjectile.h
│   ├── DBAChainLightningSpell.h
│   ├── DBAHolyShieldSpell.h
│   ├── DBAPlayableSkillTypes.h
│   └── Feedback/
│       ├── DBAEffectPlayer.h
│       ├── DBAEffectTableManager.h
│       └── DBAFloatingDamageComponent.h
├── Public/GameDBA/GAS/Cues/
│   └── DBACue_Base.h
└── Public/GameDBA/Data/
    ├── DBASkillDataRow.h
    ├── DBAZodiacUltimateRow.h
    └── DBAZodiacHeroData.h
```

---

## 2. Niagara 特效系统

### 2.1 架构现状

Niagara 已被广泛使用于战斗反馈、飘字、Projectile、Spell 子类，但与旧 ParticleSystem 并存，未完成统一迁移。

### 2.2 核心使用位置

#### 2.2.1 飘字组件

[DBAFloatingDamageComponent.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/Feedback/DBAFloatingDamageComponent.h)
- L16-17：前置声明 `class UNiagaraComponent; class UNiagaraSystem;`
- L75：`TWeakObjectPtr<UNiagaraComponent> NiagaraComponent;`
- L102、L125：`SetDamageNumberSystem(UNiagaraSystem*)` 与 `TObjectPtr<UNiagaraSystem> DamageNumberSystem`

[DBAFloatingDamageComponent.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/Feedback/DBAFloatingDamageComponent.cpp)
- L13：`#include "NiagaraComponent.h"`
- L164：`UNiagaraFunctionLibrary::SpawnSystemAtLocation(...)`，含 `AutoRelease` 池

#### 2.2.2 EffectPlayer

[DBAEffectPlayer.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/Feedback/DBAEffectPlayer.cpp)
- L20-21：`#include "NiagaraComponent.h"`、`#include "NiagaraFunctionLibrary.h"`
- L127-141：`SafeSpawnNiagaraEffect` 封装 `SpawnSystemAtLocation`，使用 `ENCPoolMethod::AutoRelease`

#### 2.2.3 Projectile / Spell 子类

| 类 | 文件 | Niagara 字段数 |
|----|------|----------------|
| `ADBAFrostShardProjectile` | [DBAFrostShardProjectile.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBAFrostShardProjectile.h) | 3 个 `TObjectPtr<UNiagaraComponent>` + 5 个 `TSoftObjectPtr<UNiagaraSystem>` |
| `ADBAHolyShieldSpell` | [DBAHolyShieldSpell.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBAHolyShieldSpell.h) | 4 个 `TSoftObjectPtr<UNiagaraSystem>` + 1 个 `TObjectPtr<UNiagaraComponent>` |
| `ADBAChainLightningSpell` | [DBAChainLightningSpell.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBAChainLightningSpell.h) | 3 个 `TSoftObjectPtr<UNiagaraSystem>` |
| `ADBAShadowBoltProjectile` | [DBAShadowBoltProjectile.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBAShadowBoltProjectile.h) | `ShadowWake`、`WakeVFXAsset`、`SecondaryImpactVFXAsset` |
| `ADBSkillProjectileBase` | [DBASkillProjectileBase.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBASkillProjectileBase.h) | `ProjectileNiagaraVFX` + `ProjectileNiagaraVFXAsset` + `ImpactNiagaraVFXAsset` |

#### 2.2.4 可玩技能配置

[DBAPlayableSkillTypes.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBAPlayableSkillTypes.h)
- L106-112：`CastNiagaraVFXAsset`、`ProjectileNiagaraVFXAsset`、`ImpactNiagaraVFXAsset`

[DBAPlayableSkillComponent.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBAPlayableSkillComponent.h)
- L87-89：`AddNiagaraWarmupPath`、`WarmUpNiagaraSystem`（预热机制）

### 2.3 问题

- **VFX-P2-4**：所有 `SpawnSystemAtLocation` 调用未检查返回值，Spawn 失败时无日志
- Niagara 与 ParticleSystem 在同一数据结构中混合使用，缺乏统一抽象

---

## 3. ParticleSystem 旧特效系统

### 3.1 架构现状

ParticleSystem 仍为系统主流，被 VFX 组件、Cue、Projectile、Skill 等大量使用，未启动向 Niagara 的统一迁移。

### 3.2 核心数据结构

#### FDBAVFXDataRow（VFX 数据表行）

[DBAVFXDataRow.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/VFX/Structs/DBAVFXDataRow.h)
- L45-69：7 个 `TSoftObjectPtr<UParticleSystem>` 字段
  - `CastingVFX`、`ImpactVFX`、`ProjectileVFX`、`AOEVFX`、`ChannelVFX`、`BuffVFX`、`DebuffVFX`

#### UDBASkillVFXManager（技能 VFX 管理器）

[DBASkillVFXManager.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/VFX/DBASkillVFXManager.h)
- L81、L85：`TMap<FName, TSoftObjectPtr<UParticleSystem>> SkillCastingVFX` 与 `SkillImpactVFX`
- L89、L93：`TMap<FName, TSoftObjectPtr<USoundBase>> SkillCastingSFX` 与 `SkillImpactSFX`

#### UDBAZodiacVFXComponent_Generic（生肖 VFX 组件）

[DBAZodiacVFXComponent_Generic.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/VFX/Components/DBAZodiacVFXComponent_Generic.h)
- L127-143：5 个 `TSoftObjectPtr<UParticleSystem>`（Attack/Hit/Move/Death/Respawn VFX）
- L149-161：4 个 `TSoftObjectPtr<USoundBase>`（对应 SFX）

### 3.3 Spawn 调用位置

| 文件 | 行号 | 调用 |
|------|------|------|
| [DBASkillVFXManager.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/DBASkillVFXManager.cpp) | 40 | `SpawnEmitterAtLocation` |
| [DBAZodiacVFXComponent_Generic.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/DBAZodiacVFXComponent_Generic.cpp) | 66, 76, 86, 96, 106 | `SpawnEmitterAtLocation` × 5 |
| [DBAZodiacSkillVFXComponent_Generic.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Generic.cpp) | 196, 230, 270, 287, 315 | `SpawnEmitterAtLocation` × 5 |
| [DBACue_Base.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Cues/DBACue_Base.cpp) | 84, 136 | `SpawnEmitterAtLocation` × 2 |
| [DBASkillProjectileBase.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBASkillProjectileBase.cpp) | 230 | `CreateDefaultSubobject<UParticleSystemComponent>` |

### 3.4 问题

- **VFX-P1-1**：ParticleSystem 与 NiagaraSystem 在 7+ 数据结构中混合使用，无统一迁移计划
  - `FDBAVFXDataRow` 全用 ParticleSystem
  - `FDBASkillEffectRow` 全用 NiagaraSystem
  - `FDBASkillDataRow` 又用 ParticleSystem

---

## 4. SoundBase / SoundCue 音效系统

### 4.1 架构现状

仅使用 `USoundBase`（含 SoundCue 间接引用），**未使用 `USoundClass`、`USoundConcurrency`、`USoundMix` 进行混音/并发控制**；ProjectSettings 入口缺失。

### 4.2 核心播放位置

#### 4.2.1 VFX 管理器

[DBASkillVFXManager.cpp:66-70](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/DBASkillVFXManager.cpp)
- `PlaySkillSFX` 调用 `UGameplayStatics::PlaySoundAtLocation`

#### 4.2.2 生肖 VFX 组件

[DBAZodiacVFXComponent_Generic.cpp:110-138](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/DBAZodiacVFXComponent_Generic.cpp)
- 4 个 `PlayXxxSFX` 函数全部使用 `PlaySoundAtLocation`

#### 4.2.3 技能 VFX 组件

[DBAZodiacSkillVFXComponent_Generic.cpp:199-356](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Generic.cpp)
- 8 处 `PlaySoundAtLocation`

#### 4.2.4 EffectPlayer

[DBAEffectPlayer.cpp:62-75, 144-157](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/Feedback/DBAEffectPlayer.cpp)
- `PlaySound` 通过 `EffectData.CastSound/HitSound` 取出 `USoundBase*`
- `SafePlaySound` 支持 3D（`SpawnSoundAtLocation`）与 2D（`SpawnSound2D`）双路径

#### 4.2.5 UI 背景音乐与点击音效

| 类 | 文件 | 字段 |
|----|------|------|
| `UDBAGameUIManager` | [DBAGameUIManager.h:568,571](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/UI/DBAGameUIManager.h) | `LoginFlowBackgroundMusicComponent`、`LoginFlowBackgroundMusicSound` |
| `UDBASplashVideoWidget` | [UDBASplashVideoWidget.h:100](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/UI/Splash/UDBASplashVideoWidget.h) | `FallbackAudioComponent` |
| `UDBALoginFlowWidgetBase` | [UDBALoginFlowWidgetBase.h:251,254,257](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h) | `BackgroundMusicSound`、`ButtonClickSound`、`BackgroundMusicComponent` |

#### 4.2.6 Spell/Projectile 子类

| 类 | 文件 | 异步回退 |
|----|------|----------|
| `ADBAChainLightningSpell` | [DBAChainLightningSpell.cpp:467-490](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAChainLightningSpell.cpp) | `PlaySFXAtLocation` 含异步回退路径 |
| `ADBAHolyShieldSpell` | [DBAHolyShieldSpell.cpp:435-457](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAHolyShieldSpell.cpp) | `PlaySFX` 含异步回退路径 |
| `ADBABloomHealingSpell` | [DBABloomHealingSpell.cpp:449](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBABloomHealingSpell.cpp) | `PlaySFX` 同上模式 |
| `ADBAFireballProjectile` | [DBAFireballProjectile.cpp:59](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAFireballProjectile.cpp) | `FireballLoopAudio = CreateDefaultSubobject<UAudioComponent>` |

### 4.3 音效数据字段散落位置

| 结构 / 类 | 文件 | SFX 字段 |
|-----------|------|----------|
| `FDBAVFXDataRow` | [DBAVFXDataRow.h:75,79,83](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/VFX/Structs/DBAVFXDataRow.h) | `CastingSFX`、`ProjectileSFX`、`ImpactSFX` |
| `FDBASkillEffectRow` | [DBAEffectTableManager.h:50,54](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/Feedback/DBAEffectTableManager.h) | `CastSound`、`HitSound` |
| `FDBASkillDataRow` | [DBASkillDataRow.h:137](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBASkillDataRow.h) | `SFXAsset` |
| `FDBAZodiacUltimateRow` | [DBAZodiacUltimateRow.h:117](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBAZodiacUltimateRow.h) | `SFXAsset` |
| `FDBAZodiacHeroConfigRow` | [DBAZodiacHeroData.h:240](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBAZodiacHeroData.h) | `VoiceSoundCue` |
| `FDBAFiveCampDisplayData` | [DBAFiveCampDisplayData.h:70,74](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBAFiveCampDisplayData.h) | `ThemeSound`、`SelectionMusic` |

### 4.4 问题

- **VFX-P1-5**：未发现 `USoundClass`、`USoundConcurrency`、`USoundMix` 的使用，无法做 BGM/SFX/UI 音量分类与推子控制
- **VFX-P1-2**：音效字段散落 6+ 处数据结构，无统一数据资产入口

---

## 5. GameplayCue 系统

### 5.1 架构现状

GAS 钩子已搭建基础框架，但实现较为简陋，Cue 内部仍直接调用 `SpawnEmitterAtLocation` / `PlaySoundAtLocation`，未充分利用 Cue Parameters 与 Instanced Actor 生命周期。

### 5.2 核心实现

#### ADBACue_Base

[DBACue_Base.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/GAS/Cues/DBACue_Base.h)
- L21：`class ADBACue_Base : public AGameplayCueNotify_Actor`
- L27-29：`OnExecuteGameplayCue`、`OnActiveGameplayCue`、`OnRemoveGameplayCue`
- L36-39：`PlayVFX`、`PlaySFX` 辅助函数
- L52-55：`DefaultVFX`、`DefaultSFX` 软引用

[DBACue_Base.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Cues/DBACue_Base.cpp)
- L22-30：`ResolveCueLocation` 私有辅助
- L32-59：`LoadSkillDataTableIfAvailable` —— **硬编码两个 DataTable 路径**（详见第 7 节）
- L66-115：`OnExecuteGameplayCue` —— 查表后取 `SkillData->VFXAsset` / `SkillData->SFXAsset`，未加载则异步预加载
- L130-144：`PlayVFX` 直接 `SpawnEmitterAtLocation`
- L146-157：`PlaySFX` 直接 `PlaySoundAtLocation`

#### GameplayCue Tag 定义

[DBAGameplayTags.h:202-206](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Core/DBAGameplayTags.h)
- 5 个 GameplayCue Tag 字段：`Cast`、`Projectile`、`Impact`、`AOE`、`Channel`

[DBAGameplayTags.cpp:242-263](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Core/DBAGameplayTags.cpp)
- 5 个 Tag 初始化，使用 `AddTag` 注册 `GameplayCue.DBA.Skill.*`

#### TriggerGameplayCue 调用链

| 文件 | 行号 | 场景 |
|------|------|------|
| [DBAAbilitySystemComponent.cpp:570-582](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/DBAAbilitySystemComponent.cpp) | `TriggerGameplayCue` 实现 |
| [DBAZodiacSkillVFXComponent_Generic.cpp:215,260,278,294,324,418,467-492](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Generic.cpp) | `ExecuteSkillGameplayCue` 触发路径 |
| [DBADamageCalculator.cpp:115-148,372,409](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBADamageCalculator.cpp) | `ExecuteDamageGameplayCue` |
| [DBASkillProjectileBase.cpp:140-170,275,532](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBASkillProjectileBase.cpp) | 发射与命中时触发 Cue |
| [DBAElementSkillAbility_Generic.cpp:41-42](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Abilities/DBAElementSkillAbility_Generic.cpp) | 技能激活时触发 |
| [DBAZodiacUltimateAbility_Generic.cpp:32-33](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Abilities/DBAZodiacUltimateAbility_Generic.cpp) | 大招激活时触发 |
| [DBARpcHandler.cpp:320](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/RPC/DBARpcHandler.cpp) | RPC 中触发 Impact Cue |

### 5.3 问题

- **VFX-P2-1**：仅一个 `ADBACue_Base` 基类，未派生子类化
- 未发现 `UGameplayCueManager` 自定义子类、`GameplayCueDefinition` 配置
- 未发现 `GameplayCueNotify_Static`、`GameplayCueNotify_Looping` 等其它 Notify 类型
- Cue 内部硬编码 DataTable 路径（详见第 7 节）

---

## 6. VFX/SFX 数据资产现状

### 6.1 关键发现

**没有专门的 `UDBAAbilityVFXDataAsset`、`UDBASoundConfigDataAsset`、`UDBAEffectDataAsset` 等集中式数据资产类**（grep `UDBA.*VFXDataAsset|UDBA.*SFXDataAsset|UDBA.*SoundConfig|UDBA.*VFXConfig|UDBA.*EffectDataAsset|UDBA.*AudioDataAsset` 返回 0 匹配）。

### 6.2 VFX/SFX 软引用字段散落清单

| 结构 / 类 | 文件 | VFX 字段 | SFX 字段 |
|-----------|------|----------|----------|
| `FDBAVFXDataRow` | [DBAVFXDataRow.h:45-83](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/VFX/Structs/DBAVFXDataRow.h) | 7 个 `TSoftObjectPtr<UParticleSystem>` | 3 个 `TSoftObjectPtr<USoundBase>` |
| `FDBASkillEffectRow` | [DBAEffectTableManager.h:41-54](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/Feedback/DBAEffectTableManager.h) | 2 个 `TSoftObjectPtr<UNiagaraSystem>` | 2 个 `TSoftObjectPtr<USoundBase>` |
| `FDBASkillDataRow` | [DBASkillDataRow.h:128-137](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBASkillDataRow.h) | `VFXAsset`（ParticleSystem） | `SFXAsset` |
| `FDBAZodiacUltimateRow` | [DBAZodiacUltimateRow.h:108-117](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBAZodiacUltimateRow.h) | `VFXAsset`（ParticleSystem） | `SFXAsset` |
| `FDBAZodiacHeroConfigRow` | [DBAZodiacHeroData.h:240-244](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBAZodiacHeroData.h) | `EffectSystem`（ParticleSystem） | `VoiceSoundCue` |
| `FDBAFiveCampDisplayData` | [DBAFiveCampDisplayData.h:70-74](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBAFiveCampDisplayData.h) | - | `ThemeSound`、`SelectionMusic` |
| `UDBASkillVFXManager` | [DBASkillVFXManager.h:81-93](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/VFX/DBASkillVFXManager.h) | 2 个 `TMap<FName, TSoftObjectPtr<UParticleSystem>>` | 2 个 `TMap<FName, TSoftObjectPtr<USoundBase>>` |
| `UDBAZodiacVFXComponent_Generic` | [DBAZodiacVFXComponent_Generic.h:127-161](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/VFX/Components/DBAZodiacVFXComponent_Generic.h) | 5 个 `TSoftObjectPtr<UParticleSystem>` | 4 个 `TSoftObjectPtr<USoundBase>` |
| `ADBAFrostShardProjectile` 等 Spell 子类 | 见第 2 节 | 大量直接成员 | 大量直接成员 |
| `ADBACue_Base` | [DBACue_Base.h:52-55](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/GAS/Cues/DBACue_Base.h) | `DefaultVFX` | `DefaultSFX` |
| `FDBAPlayableSkillConfig` | [DBAPlayableSkillTypes.h:106-112](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBAPlayableSkillTypes.h) | 3 个 Niagara 软引用 | - |

### 6.3 问题

- **VFX-P1-2**：数据结构不统一
  - `FDBAVFXDataRow` 用 ParticleSystem
  - `FDBASkillEffectRow` 用 NiagaraSystem
  - `FDBASkillDataRow` 又用 ParticleSystem
- 同一份"技能 VFX/SFX"概念在 3 处独立表/结构中重复定义
- 违反 `DBA.DataAsset.NoHardcoding` 策略关于"统一数据资产入口"的精神

---

## 7. 同步加载问题（P0）

### 7.1 VFX-P0-1：UI 音效同步加载

**严重程度**：高。违反 `DBA.UI.EventAsync`，阻塞 GameThread。

| 文件 | 行号 | 代码 |
|------|------|------|
| [UDBAStartupVideoWidget.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Startup/UDBAStartupVideoWidget.cpp) | 30 | `LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"))` |
| [UDBASplashVideoWidget.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Splash/UDBASplashVideoWidget.cpp) | 41 | 同上路径 |
| [UDBACharacterCreateFlowWidgetBase.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.cpp) | 1103 | `LoadObject<USoundBase>(..., TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"))` |
| [UDBACharacterCreateFlowWidgetBase.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.cpp) | 1112 | `LoadObject<USoundBase>(..., TEXT("/Game/DBA/Audio/UI/BGM/BGM_CharacterCreate_Loop.BGM_CharacterCreate_Loop"))` |
| [UDBACharacterSelectFlowWidgetBase.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.cpp) | 953 | 同 1103 路径 |
| [UDBACharacterSelectFlowWidgetBase.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.cpp) | 962 | `LoadObject<USoundBase>(..., TEXT("/Game/DBA/Audio/UI/BGM/BGM_CharacterSelect_Loop.BGM_CharacterSelect_Loop"))` |

### 7.2 VFX-P2-3：Resonance GE 同步加载 DataTable

**严重程度**：中。间接影响 VFX/SFX 触发时序。

| 文件 | 行号 | 路径 |
|------|------|------|
| [DBAGE_Earth_Resonance.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Effects/DBAGE_Earth_Resonance.cpp) | 18 | `DataTable'/Game/Data/Elements/DBAElementResonanceRowe.DBAElementResonanceRowe'` |
| [DBAGE_Gold_Resonance.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Effects/DBAGE_Gold_Resonance.cpp) | 18 | 同上 |
| [DBAGE_Wood_Resonance.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Effects/DBAGE_Wood_Resonance.cpp) | 18 | 同上 |
| [DBAGE_Fire_Resonance.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Effects/DBAGE_Fire_Resonance.cpp) | 18 | 同上 |
| [DBAGE_Water_Resonance.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Effects/DBAGE_Water_Resonance.cpp) | 18 | 同上 |

### 7.3 测试中的 LoadObject（可豁免）

- [DBAAIShowcaseTests.cpp:55](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/Tests/DBAAIShowcaseTests.cpp)：测试模板 `LoadRequiredAsset<TObjectType>` 内部使用 `LoadObject`，仅用于编辑器测试
- [DBAFixedSkillGroupDataTests.cpp:144](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/Tests/DBAFixedSkillGroupDataTests.cpp)：测试加载 DataTable

### 7.4 LoadSynchronous 使用情况

- [UDBAGameSettingsWidgetBase.cpp:409](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/UDBAGameSettingsWidgetBase.cpp)
- [UDBAInventoryWidgetBase.cpp:612](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/UDBAInventoryWidgetBase.cpp)
- [UDBAMobaUserWidgetBase.cpp:193](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/GameMoba/Private/GameMoba/UI/UDBAMobaUserWidgetBase.cpp)

均为 `UTexture2D` 加载，**不涉及 VFX/SFX**，但同样违反异步原则。

### 7.5 异步加载基础设施

[DBAAsyncAssetLoader.h](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Utilities/DBAAsyncAssetLoader.h) 提供了完整的异步加载模板：
- `RequestAsyncAsset<AssetType>`（L12-40）：单个资产异步加载，回调式
- `AddPreloadPath`（L42-49）：批量收集软引用路径
- `RequestAsyncPreload`（L51-63）：批量预加载

**问题**：基础工具已就绪，但 UI 层（特别是登录流程）未使用，仍走 `LoadObject`。`DBASkillProjectileBase.cpp:319,639` 与 `DBAChainLightningSpell.cpp:481`、`DBAHolyShieldSpell.cpp:449` 等战斗类已正确使用异步路径作为兜底。

---

## 8. 硬编码资源路径（P0）

### 8.1 VFX-P0-2：Spell/Projectile 子类构造函数硬编码

**严重程度**：高。违反 `DBA.DataAsset.NoHardcoding`，无法配置驱动。

#### Niagara 资产路径硬编码

[DBAFrostShardProjectile.cpp:132-138](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAFrostShardProjectile.cpp)（7 处）：
```cpp
ProjectileNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_IceDart.NS_IceDart")));
CrystalWakeVFXAsset       = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_IceCrystal.NS_IceCrystal")));
MistWakeVFXAsset          = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Iceicle3D.NS_Iceicle3D")));
SpiralWakeVFXAsset        = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_MagicLanceShuriken.NS_MagicLanceShuriken")));
ImpactNiagaraVFXAsset     = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Ice_01.NS_Hit_Ice_01")));
SecondaryImpactVFXAsset   = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_ColdBlood.NS_Hit_ColdBlood")));
RingImpactVFXAsset        = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_IceCrystal.NS_IceCrystal")));
```

[DBAChainLightningSpell.cpp:67-69](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAChainLightningSpell.cpp)（3 处）：
```cpp
ArcVFXAsset    = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_ThunderBolt.NS_ThunderBolt")));
BranchVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Eletric_01.NS_Hit_Eletric_01")));
ImpactVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Thunder.NS_Hit_Thunder")));
```

#### SFX 资产路径硬编码

| 文件 | 行号 | 路径 |
|------|------|------|
| [DBAFrostShardProjectile.cpp:139-140](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAFrostShardProjectile.cpp) | 139-140 | `/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_Flight`、`SFX_FrostShard_Impact` |
| [DBAFireballProjectile.cpp:78-79](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAFireballProjectile.cpp) | 78-79 | `/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_Flight`、`SFX_MageFireball_Impact` |
| [DBAShadowBoltProjectile.cpp:82-83](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAShadowBoltProjectile.cpp) | 82-83 | `/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_Flight`、`SFX_ShadowBolt_Impact` |
| [DBAChainLightningSpell.cpp:70-72](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAChainLightningSpell.cpp) | 70-72 | `/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_PreCast/Flight/Impact` |
| [DBAHolyShieldSpell.cpp:88-90](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAHolyShieldSpell.cpp) | 88-90 | `/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_PreCast/Flight/Impact` |
| [DBABloomHealingSpell.cpp:95-97](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBABloomHealingSpell.cpp) | 95-97 | `/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_PreCast/Flight/Impact` |

### 8.2 VFX-P0-3：GameplayCue DataTable 路径硬编码

[DBACue_Base.cpp:34-37](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Cues/DBACue_Base.cpp)：
```cpp
const TCHAR* CandidatePaths[] = {
    TEXT("/Game/DBA/Data/Skills/SkillDataTable.SkillDataTable"),
    TEXT("/Game/Data/Skills/SkillDataTable.SkillDataTable")
};
```

**问题**：
- 两个候选路径均未通过 DeveloperSettings 配置
- 路径不存在时无明确日志说明回退逻辑
- `DBAElementResonanceRowe` 路径名疑似拼写错误（Rowe → Row）

### 8.3 UI 音效路径硬编码

| 文件 | 行号 | 路径 |
|------|------|------|
| [UDBAStartupVideoWidget.cpp:30](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Startup/UDBAStartupVideoWidget.cpp) | 30 | `/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick` |
| [UDBASplashVideoWidget.cpp:41](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Splash/UDBASplashVideoWidget.cpp) | 41 | 同上 |
| [UDBACharacterCreateFlowWidgetBase.cpp:1103,1112](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.cpp) | 1103, 1112 | 同上 + `/Game/DBA/Audio/UI/BGM/BGM_CharacterCreate_Loop` |
| [UDBACharacterSelectFlowWidgetBase.cpp:953,962](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.cpp) | 953, 962 | 同上 + `/Game/DBA/Audio/UI/BGM/BGM_CharacterSelect_Loop` |
| [DBAUserWidgetBase.cpp:30](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/GameCore/Private/GameCore/UI/DBAUserWidgetBase.cpp) | 30 | `/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick` |
| [UDBAMobaUserWidgetBase.cpp:32](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/GameMoba/Private/GameMoba/UI/UDBAMobaUserWidgetBase.cpp) | 32 | 同上 |

### 8.4 DBAConstants 路径常量表现状

[DBAConstants.h:602-645](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Core/DBAConstants.h) 已定义 `CoreRoot`、`DBARoot`、`DT_Skills` 等路径常量，但 **VFX/SFX 路径未纳入此常量表**。

### 8.5 测试中的硬编码（可豁免）

[DBAAIShowcaseTests.cpp:41-50](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/Tests/DBAAIShowcaseTests.cpp)、[DBACharacterPresentationStageTests.cpp:40-417](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/Tests/DBACharacterPresentationStageTests.cpp) 等测试文件中的硬编码路径仅用于断言验证，可豁免。

---

## 9. DeveloperSettings 配置缺失

### 9.1 VFX-P1-4：缺少 VFX/SFX 专用 DeveloperSettings

**关键发现**：**项目完全没有 VFX/SFX 专用的 DeveloperSettings 配置入口**。

现有 4 个 DeveloperSettings 类：

| 类 | 文件 | 用途 |
|----|------|------|
| `UDBASkillNameDeveloperSettings` | [DBASkillNameDeveloperSettings.h:30](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Data/DBASkillNameDeveloperSettings.h) | 技能名称配置 |
| `UDBABattleAttributeDeveloperSettings` | [DBABattleAttributeDeveloperSettings.h:17](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/GAS/Attributes/DBABattleAttributeDeveloperSettings.h) | 战斗属性设置 |
| `UDBAHeroBalanceDeveloperSettings` | [DBAHeroBalanceDeveloperSettings.h:29](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/GAS/Balance/DBAHeroBalanceDeveloperSettings.h) | 英雄数值平衡设置 |
| `UDBAPlayableSkillDeveloperSettings` | [DBAPlayableSkillDeveloperSettings.h:17](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Public/GameDBA/Combat/DBAPlayableSkillDeveloperSettings.h) | 可玩技能设置（仅含 `DefaultSkillCatalog` 一字段） |

### 9.2 缺失的配置入口

没有 `UDBAVFXSettings`、`UDBASoundSettings`、`UDBAAudioSettings`、`UDBAEffectSettings` 类，无法集中配置：
- 默认 UI 点击音效路径
- 默认 BGM 路径
- 默认 SkillDataTable 路径（当前在 `DBACue_Base.cpp` 硬编码两路径试探）
- 默认 Niagara/Particle 预加载策略
- 音量分类（缺少 `USoundClass` 配置）

---

## 10. 中文日志合规性

### 10.1 VFX-P1-3：核心 VFX/SFX 文件 0 日志

**严重程度**：中。违反 `DBA.Log.ChineseOutput`，Dedicated Server 与客户端表现层故障无法定位。

以下 VFX/SFX 核心文件 grep `UE_LOG|ensure|checkf` 返回 **0 匹配**：

| 文件 | 职责 |
|------|------|
| [DBASkillVFXManager.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/DBASkillVFXManager.cpp) | 技能 VFX/SFX 管理器 |
| [DBAEffectPlayer.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/Feedback/DBAEffectPlayer.cpp) | 效果播放器 |
| [DBAEffectTableManager.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/Feedback/DBAEffectTableManager.cpp) | 效果表管理器 |
| [DBAFloatingDamageComponent.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/Feedback/DBAFloatingDamageComponent.cpp) | 飘字组件 |
| [DBACue_Base.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/Cues/DBACue_Base.cpp) | GameplayCue 基类 |
| [DBASkillProjectileBase.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBASkillProjectileBase.cpp) | 投射物基类 |
| [DBAChainLightningSpell.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAChainLightningSpell.cpp) | 连锁闪电法术 |
| [DBAHolyShieldSpell.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAHolyShieldSpell.cpp) | 神圣护盾法术 |
| [DBABloomHealingSpell.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBABloomHealingSpell.cpp) | 绽放治疗法术 |
| [DBAFireballProjectile.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAFireballProjectile.cpp) | 火球投射物 |
| [DBAShadowBoltProjectile.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAShadowBoltProjectile.cpp) | 暗影箭投射物 |
| [DBAFrostShardProjectile.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/Combat/DBAFrostShardProjectile.cpp) | 冰霜碎片投射物 |

### 10.2 已合规文件

| 文件 | 行号 | 日志内容 |
|------|------|----------|
| [DBAZodiacVFXComponent_Generic.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/DBAZodiacVFXComponent_Generic.cpp) | 35 | `加载默认资源: ZodiacType=%d` |
| [DBAZodiacVFXComponent_Generic.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/DBAZodiacVFXComponent_Generic.cpp) | 144 | `播放技能音效: %s` |
| [DBAZodiacSkillVFXComponent_Generic.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Generic.cpp) | 140 | `无效的配置: ZodiacType=%d, SkillSlot=%s` |
| [DBAZodiacSkillVFXComponent_Generic.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Generic.cpp) | 153 | `加载VFX配置: %s` |
| [DBAZodiacSkillVFXComponent_Generic.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Generic.cpp) | 157 | `未找到VFX配置: %s` |
| [DBAAbilitySystemComponent.cpp](file:///e:/work/Game/DivineBeastsArena/DBA_GameClient/Source/DivineBeastsArena/Private/GameDBA/GAS/DBAAbilitySystemComponent.cpp) | 574 | `GameplayCue 标签无效。` |

### 10.3 主要缺失场景

按 `DBA.Log.ChineseOutput` 策略要求，以下场景必须补中文日志但当前缺失：

1. **资产加载失败/兜底**：`DBACue_Base.cpp:87-91,101-105` 异步预加载触发条件无任何日志
2. **同步加载失败**：`UDBAStartupVideoWidget.cpp:30`、`UDBASplashVideoWidget.cpp:41` 等 `LoadObject` 调用未检查返回值，未输出失败原因
3. **DataTable 查找失败**：`DBACue_Base.cpp:71-75` `FindRow` 失败无日志
4. **Niagara/Particle Spawn 失败**：所有 `SpawnEmitterAtLocation`、`SpawnSystemAtLocation` 调用未检查返回值
5. **SoundBase 为空**：`PlaySoundAtLocation` 调用前未 ensure 资产有效性
6. **DataTable 路径不存在**：`DBACue_Base.cpp:34-37` 候选路径无日志说明回退逻辑
7. **`DBASkillVFXManager.cpp` PlaySkillVFX/PlaySkillSFX** L26-77：Owner 为空、SkillId 为 None、VFXPath 为空、资产未加载等所有早退路径均无日志

---

## 11. 综合架构问题

### 11.1 双特效系统并存未收敛

ParticleSystem（旧）与 NiagaraSystem（新）在 7+ 数据结构中混合使用，无统一迁移计划：
- `FDBAVFXDataRow` 全用 ParticleSystem
- `FDBASkillEffectRow` 全用 NiagaraSystem
- `FDBASkillDataRow` 又用 ParticleSystem

### 11.2 VFX/SFX 数据无统一入口

12+ 处分散的 `TSoftObjectPtr<USoundBase>` / `TSoftObjectPtr<UParticleSystem>` / `TSoftObjectPtr<UNiagaraSystem>` 字段散落在 DataRow、Struct、Component、Spell 子类、Widget 中，没有 `UDBAVFXDataAsset` 类，也没有 `UDBAVFXDeveloperSettings`。

### 11.3 Spell 子类构造函数硬编码

6 个 Spell/Projectile 子类（FrostShard、Fireball、ShadowBolt、ChainLightning、HolyShield、BloomHealing）在构造函数中硬编码所有 VFX/SFX 路径，违反 `DBA.DataAsset.NoHardcoding`。

### 11.4 UI 音效同步加载

6 处 `LoadObject<USoundBase>` 同步加载，且都集中在登录/选人流程，违反 `DBA.UI.EventAsync`。

### 11.5 GameplayCue 实现简陋

`ADBACue_Base` 仅一个基类，未派生子类化，Cue 内部硬编码 DataTable 路径，未利用 `UGameplayCueManager` 的 Tag-Definition 集中注册。

### 11.6 音效系统缺混音控制

未发现 `USoundClass`、`USoundConcurrency`、`USoundMix` 的使用，无法做 BGM/SFX/UI 音量分类与推子控制。

### 11.7 核心 VFX/SFX 文件无日志

12 个关键文件 0 日志输出，无法在 Dedicated Server 与客户端定位表现层故障，违反 `DBA.Log.ChineseOutput`。

---

## 12. 风险分级与完善建议

### 12.1 P0 风险完善建议

#### VFX-P0-1：UI 音效同步加载修复

**建议**：
1. 新建 `UDBAAudioDeveloperSettings`，暴露 `DefaultUIClickSound`、`DefaultLoginBGM`、`DefaultCharacterCreateBGM`、`DefaultCharacterSelectBGM` 等软引用字段
2. 将 6 处 `LoadObject<USoundBase>` 改为通过 `StreamableManager` 异步预加载 + 缓存
3. 借鉴已完成的 P0-3（Widget LoadSynchronous 修复）模式

**涉及文件**：
- 新建：`Source/DivineBeastsArena/Public/GameDBA/Audio/DBAAudioDeveloperSettings.h`
- 修改：6 个 UI Widget cpp 文件

#### VFX-P0-2：Spell/Projectile 子类硬编码路径迁移

**建议**：
1. 新建 `UDBASpellVFXDataAsset`（或复用 `FDBAVFXDataRow` 扩展），承载 Spell/Projectile 的 VFX/SFX 软引用
2. 6 个 Spell/Projectile 子类构造函数改为从 DataAsset 读取软引用
3. 在 `DefaultGame.ini` 配置各 Spell 对应的 DataAsset

**涉及文件**：
- 新建：`Source/DivineBeastsArena/Public/GameDBA/Combat/DBASpellVFXDataAsset.h`
- 修改：6 个 Spell/Projectile 子类的 .h 和 .cpp

#### VFX-P0-3：GameplayCue DataTable 路径配置化

**建议**：
1. 在 `UDBAAudioDeveloperSettings` 或新建 `UDBAGameplayCueSettings` 中暴露 `SkillDataTable` 软引用
2. `ADBACue_Base::LoadSkillDataTableIfAvailable` 改为从 Settings 读取，移除候选路径试探
3. 补充路径不存在、DataTable 未加载的中文日志

**涉及文件**：
- 修改：`DBACue_Base.h` / `DBACue_Base.cpp`
- 新建或修改：DeveloperSettings

### 12.2 P1 风险完善建议

#### VFX-P1-1：特效系统统一收敛

**建议**：
1. 制定 Niagara 迁移计划，将 `FDBAVFXDataRow` 的 7 个 ParticleSystem 字段逐步迁移为 NiagaraSystem
2. 新增数据统一使用 Niagara，旧 ParticleSystem 标记 `@deprecated`
3. 长期目标：统一为 `TSoftObjectPtr<UObject>` 或自定义 `FDBAVFXAssetRef` 支持 Particle/Niagara 双类型

#### VFX-P1-2：VFX/SFX 数据统一入口

**建议**：
1. 设计 `UDBAVFXDataAsset` 基类，承载技能/英雄/阵营的 VFX/SFX 软引用
2. 现有 `FDBAVFXDataRow`、`FDBASkillEffectRow`、`FDBASkillDataRow.VFXAsset/SFXAsset` 统一迁移到 DataAsset
3. 短期保留 DataTable 行结构兼容，新增数据走 DataAsset

#### VFX-P1-3：核心文件中文日志补齐

**建议**：
1. 12 个 0 日志文件按 `DBA.Log.ChineseOutput` 补齐关键路径日志
2. 重点补齐：资产加载失败、Spawn 失败、DataTable 查找失败、早退路径
3. 使用 `LogDBACombat` / `LogDBAData` 日志通道

#### VFX-P1-4：VFX/SFX DeveloperSettings 配置入口

**建议**：新建 `UDBAAudioDeveloperSettings`，集中配置：
- 默认 UI 点击音效
- 默认 BGM（登录、选人、大厅、对局）
- SkillDataTable 路径
- 默认预加载策略
- SoundClass 引用（配合 VFX-P1-5）

#### VFX-P1-5：音效混音控制

**建议**：
1. 在 Content 目录创建 `SoundClass` 层级：`Master` → `BGM` / `SFX` / `UI` / `Voice`
2. 创建 `SoundMix` 用于推子控制
3. 在 `UDBAAudioDeveloperSettings` 暴露 SoundMix 引用
4. UI 音量设置 Widget 通过 `UGameplayStatics::SetSoundMixClassOverride` 控制音量

### 12.3 P2 风险完善建议

#### VFX-P2-1：GameplayCue 子类化

**建议**：
1. 为 5 个 GameplayCue Tag（Cast/Projectile/Impact/AOE/Channel）派生 `ADBACue_Cast`、`ADBACue_Projectile` 等子类
2. 利用 `FGameplayCueParameters` 传递更多上下文
3. 考虑实现 `UGameplayCueManager` 子类集中管理 Tag-Definition 映射

#### VFX-P2-2：DBASkillVFXManager TMap 校验

**建议**：
1. 为 `TMap<FName, TSoftObjectPtr<...>>` 添加 `ValidateDataIntegrity` 方法
2. 检查键名是否匹配已知技能 ID
3. 检查软引用是否有效

#### VFX-P2-3：Resonance GE 同步加载修复

**建议**：
1. 5 处 `LoadObject<UDataTable>` 改为通过 `UDBAStaticDataAsset` 或 Subsystem 异步加载
2. 修正 `DBAElementResonanceRowe` 路径名疑似拼写错误（Rowe → Row）

#### VFX-P2-4：Niagara Spawn 返回值检查

**建议**：
1. 所有 `SpawnSystemAtLocation` / `SpawnEmitterAtLocation` 调用检查返回值
2. 返回 nullptr 时输出中文 Warning 日志

---

## 13. 完善优先级排序

| 优先级 | 任务 | 类型 | 可代码执行 |
|--------|------|------|------------|
| 立即 | VFX-P0-1 UI 音效同步加载修复 | 源码修改 + DeveloperSettings | ✅ |
| 立即 | VFX-P0-3 GameplayCue DataTable 路径配置化 | 源码修改 + DeveloperSettings | ✅ |
| 短期 | VFX-P1-3 核心文件中文日志补齐 | 源码修改 | ✅ |
| 短期 | VFX-P1-4 VFX/SFX DeveloperSettings 配置入口 | 源码修改 | ✅ |
| 中期 | VFX-P0-2 Spell/Projectile 硬编码路径迁移 | 源码 + DataAsset | ⚠️ 需配合 .uasset |
| 中期 | VFX-P1-1 特效系统统一收敛 | 源码修改 | ⚠️ 影响面大，建议单独 PR |
| 中期 | VFX-P1-2 VFX/SFX 数据统一入口 | 源码 + DataAsset | ⚠️ 需配合 .uasset |
| 中期 | VFX-P1-5 音效混音控制 | 源码 + .uasset | ⚠️ 需 UE 编辑器创建 SoundClass |
| 中期 | VFX-P2-1 GameplayCue 子类化 | 源码修改 | ⚠️ 影响面大 |
| 中期 | VFX-P2-3 Resonance GE 同步加载修复 | 源码修改 | ✅ |
| 中期 | VFX-P2-4 Niagara Spawn 返回值检查 | 源码修改 | ✅ |
| 中期 | VFX-P2-2 DBASkillVFXManager TMap 校验 | 源码修改 | ✅ |

---

## 14. 与其他审查报告的关联

本报告与以下审查报告存在交叉关联：
- **《数据资产审查报告 v1.0》**：VFX/SFX 数据字段散落问题与数据资产体系统一相关
- **《GAS系统审查报告 v1.9》**：GameplayCue 实现与 GAS 共振 GE 同步加载问题相关
- **《UI系统审查报告》**：UI 音效同步加载与 Widget 异步加载原则相关
- **《角色与动画系统审查报告》**：Projectile/Spell 子类的 VFX/SFX 与动画系统相关

---

审查结束。
