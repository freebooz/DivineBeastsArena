# DivineBeastsArena GAS 系统审查报告（v1.9）

版本：v1.9（基于 v1.8 增量复核）
日期：2026-07-06
审查范围：`DBA_GameClient/Source/DivineBeastsArena/Public|Private/GameDBA/GAS/`、`DBA_GameClient/Source/GameMoba/`、`DBA_GameClient/Source/DivineBeastsArena/Public|Private/GameDBA/Character/`、`DBA_GameClient/Source/DivineBeastsArena/Public|Private/GameDBA/Player/`、`DBA_GameClient/Source/DivineBeastsArena/Public|Private/GameDBA/RPC/`

本次审查以当前源码为准，对 v1.8 文档结论进行复核，并重点标注 v1.8 之后的变化与此前未发现的风险。所有路径均为绝对路径。

---

## 1. 总体架构概览

当前 GAS 主干延续 v1.8 确定的方向：`ADBAPlayerState` 作为 OwnerActor 持有 `UDBAAbilitySystemComponent` 与两个 `UAttributeSet`，`ADBAZodiacCharacterBase` 作为 AvatarActor 消费 PlayerState 的 ASC。`UDBAAbilitySystemComponent` 权威管理终极能量、连锁等级、共鸣等级、技能授予、冷却事件同步与 GameplayCue 桥接。普通元素技能已通过 `CommitAbility` / `CheckCost` / `ApplyCost` / `CheckCooldown` / `ApplyCooldown` 接入 GAS 标准生命周期，并由 `UDBAFixedSkillGroupDataAsset` 的 `FDBAAbilityRuntimeConfig` 驱动消耗、冷却与展示数据。

继承层次（自顶向下）：
- `UAbilitySystemComponent`（UE）
  - `UDBAMobaAbilitySystemComponentBase`（GameMoba 模块，提供 `InitializeAbilities`）
    - `UDBAAbilitySystemComponent`（DivineBeastsArena 模块，项目层扩展）
- `UGameplayAbility`（UE）
  - `UDBAMobaGameplayAbilityBase`（GameMoba，通用基类，含 EnergyCost/CooldownDuration/AbilityIcon 字段）
    - `UDBAZodiacAbilityBase`（生肖技能基类，含 ZodiacType）
      - `UDBAZodiacPassiveAbility_Generic`（泛化被动）
      - `UDBAZodiacUltimateAbilityBase`（生肖大招基类，含终极能量校验/扣减）
        - `UDBAZodiacUltimateAbility_Generic`（泛化大招）
    - `UDBAElementAbilityBase`（元素主动技能基类，含元素类型/能量消耗/冷却标签/运行配置解析）
      - `UDBAElementSkillAbility_Generic`（泛化元素主动技能）
    - `UDBAResonanceAbilityBase`（共鸣被动基类）

ASC 复制模式为 `Mixed`，PlayerState 构造函数中设置。

---

## 2. ASC 归属与初始化

### 2.1 归属策略（已固化，与 v1.8 一致）

- `ADBAPlayerState` 实现了 `IAbilitySystemInterface`，在构造函数中创建 `UDBAAbilitySystemComponent`、`UDBABattleAttributeSet`、`UDBAHeroGrowthAttributeSet` 子对象。
  - 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Player\DBAPlayerState.cpp:26-31`
  - 复制模式：`SetReplicationMode(EGameplayEffectReplicationMode::Mixed)`
- `ADBAZodiacCharacterBase::GetDBAAbilitySystemComponent()` 优先从 `ADBAPlayerState` 获取强类型 ASC。
  - 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp:1081-1085`

### 2.2 InitAbilityActorInfo 调用点

- `ADBAZodiacCharacterBase::PossessedBy()` → `InitializeDBAAbilityActorInfo()`（`DBAZodiacCharacterBase.cpp:296-300`）
- `ADBAZodiacCharacterBase::OnRep_PlayerState()` → `InitializeDBAAbilityActorInfo()`（`DBAZodiacCharacterBase.cpp:302-306`）
- `ADBAZodiacCharacterBase::BeginPlay()` → `InitializeDBAAbilityActorInfo()`（`DBAZodiacCharacterBase.cpp:333`）
- `InitializeDBAAbilityActorInfo()` 调用 `DBAAbilitySystem->InitializeAbilities(DBAPlayerState, this)`，后者调用 `InitAbilityActorInfo(Owner, Avatar)`（`DBAMobaAbilitySystemComponentBase.cpp:27-35`）

### 2.3 GetDBAAvatarCharacter 解析路径（与 v1.8 一致）

`UDBAAbilitySystemComponent::GetDBAAvatarCharacter()` 优先从 `AbilityActorInfo->AvatarActor` 解析 `ADBAZodiacCharacterBase`，仅在空时回退 `GetOwner()`。
- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp:117-129`
- 已被 `IsInputAbilityOnCooldown`、`SyncCooldownsToCharacter`、`IsValidTarget`、`TriggerGameplayCue`、`TryActivateAbilityByInputID` 使用。

---

## 3. Ability 类完整清单

| # | 类名 | 文件路径（Public 头） | 继承关系 | 类型 | ActivateAbility | CommitAbility | CheckCost | ApplyCost | CheckCooldown | ApplyCooldown | DataAsset 驱动 | GameplayCue 触发 |
|---|------|----------------------|----------|------|-----------------|---------------|-----------|-----------|---------------|---------------|----------------|------------------|
| 1 | `UDBAMobaGameplayAbilityBase` | `GameMoba/GAS/DBAMobaGameplayAbilityBase.h` | UGameplayAbility | 通用基类 | 重写(仅 Super) | 否 | 否 | 否 | 否 | 否 | 否（CDO 字段） | 否 |
| 2 | `UDBAZodiacAbilityBase` | `GAS/Abilities/DBAZodiacAbilityBase.h` | UDBAMobaGameplayAbilityBase | 生肖被动基类 | 否（继承） | 否 | 否 | 否 | 否 | 否 | 否 | 否 |
| 3 | `UDBAZodiacPassiveAbility_Generic` | `GAS/Abilities/DBAZodiacPassiveAbility_Generic.h` | UDBAZodiacAbilityBase | 泛化被动 | 重写(日志+扩展点+Super) | 否 | 否 | 否 | 否 | 否 | 否（PassiveSkillID+PassiveTable） | 否 |
| 4 | `UDBAZodiacUltimateAbilityBase` | `GAS/Abilities/DBAZodiacUltimateAbilityBase.h` | UDBAZodiacAbilityBase | 生肖大招基类 | 否（继承） | 仅重写 CommitAbilityCost | 否 | 否 | 否 | 否 | 否 | 否 |
| 5 | `UDBAZodiacUltimateAbility_Generic` | `GAS/Abilities/DBAZodiacUltimateAbility_Generic.h` | UDBAZodiacUltimateAbilityBase | 泛化大招 | 重写(Cue+扩展点+Super) | **未调用 CommitAbility** | 否 | 否 | 否 | 否 | 否（UltimateSkillID+UltimateTable） | 是(GameplayCue.DBA.Skill.Cast) |
| 6 | `UDBAElementAbilityBase` | `GAS/Abilities/DBAElementAbilityBase.h` | UDBAMobaGameplayAbilityBase | 元素主动基类 | 否（继承） | 否（提供 CommitAbilityCost 桥接） | 重写 | 重写 | 重写 | 重写 | 是(FDBAAbilityRuntimeConfig) | 否 |
| 7 | `UDBAElementSkillAbility_Generic` | `GAS/Abilities/DBAElementSkillAbility_Generic.h` | UDBAElementAbilityBase | 泛化元素主动 | 重写(CommitAbility+Cue+扩展点) | **是（显式调用）** | 继承 | 继承 | 继承 | 继承 | 是 | 是(GameplayCue.DBA.Skill.Cast) |
| 8 | `UDBAResonanceAbilityBase` | `GAS/Abilities/DBAResonanceAbilityBase.h` | UDBAMobaGameplayAbilityBase | 共鸣被动 | 重写(ApplyResonanceEffect) | 否 | 否 | 否 | 否 | 否 | 否 | 否 |

说明：
- `UDBAElementAbilityBase` 是唯一完整接入 GAS 消耗/冷却生命周期的基类，其 `ResolveRuntimeConfig` 通过 ASC 缓存的 `FDBAAbilityRuntimeConfig` 解析消耗、Cost GE、Cooldown GE、Cooldown Duration 与 Cooldown Tag。
- `UDBAElementSkillAbility_Generic::ActivateAbility` 在 `Super::ActivateAbility` 之后显式调用 `CommitAbility`，提交失败时输出中文日志并 `EndAbility(true, true)`。
- `UDBAZodiacUltimateAbility_Generic::ActivateAbility` **未调用 `CommitAbility`**，详见第 10 节 P0-2。

---

## 4. AttributeSet 字段清单

### 4.1 UDBABattleAttributeSet

文件：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Attributes\DBABattleAttributeSet.h`

| 属性字段 | 类别 | 复制规则 | 钳制逻辑 | 说明 |
|----------|------|----------|----------|------|
| MaxHealth | Health | COND_None, REPNOTIFY_Always | — | 最大生命值 |
| CurrentHealth | Health | COND_None, REPNOTIFY_Always | PreAttributeChange + PostGameplayEffectExecute: Clamp(0, MaxHealth) | 当前生命值 |
| AttackPower | Attack | COND_None, REPNOTIFY_Always | — | 攻击力 |
| Defense | Attack | COND_None, REPNOTIFY_Always | — | 防御力；DamageReduction = Defense/(Defense+100) |
| MoveSpeed | Movement | COND_None, REPNOTIFY_Always | — | 移动速度 |
| MaxEnergy | Energy | COND_None, REPNOTIFY_Always | — | 最大能量（GetMaxEnergy 读取此属性） |
| CurrentEnergy | Energy | COND_None, REPNOTIFY_Always | PreAttributeChange + PostGameplayEffectExecute: Clamp(0, MaxEnergy) | 当前能量（普通技能消耗源） |
| EnergyRegen | Energy | COND_None, REPNOTIFY_Always | — | 能量回复速度 |
| CriticalRate | Critical | COND_None, REPNOTIFY_Always | PreAttributeChange: Clamp(0, 1) | 暴击率 |
| CriticalMultiplier | Critical | COND_None, REPNOTIFY_Always | PreAttributeChange: Max(NewValue, 1.0) | 暴击倍率 |
| MaxShield | Shield | COND_None, REPNOTIFY_Always | — | 最大护盾 |
| CurrentShield | Shield | COND_None, REPNOTIFY_Always | PreAttributeChange + PostGameplayEffectExecute: Clamp(0, MaxShield) | 当前护盾 |

- 构造函数不再写入运行默认值，由 `ApplyDefaultAttributes(UDBABattleAttributeDefaultsDataAsset*)` 应用 DataAsset 初始值。
- v1.8 已移除 `UltimateEnergy` 重复字段，终极能量权威源固定为 ASC。

### 4.2 UDBAHeroGrowthAttributeSet

文件：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Attributes\DBAHeroGrowthAttributeSet.h`

| 属性字段 | 类别 | 复制规则 | 默认值（构造函数硬编码） | 说明 |
|----------|------|----------|--------------------------|------|
| HeroLevel | Growth | COND_None, REPNOTIFY_Always | 1.0 | 英雄等级 |
| Experience | Growth | COND_None, REPNOTIFY_Always | 0.0 | 当前经验 |
| ExperienceToNextLevel | Growth | COND_None, REPNOTIFY_Always | 100.0 | 下一级所需经验 |
| RespawnTime | State | COND_None, REPNOTIFY_Always | 10.0 | 复活时间 |
| GoldBounty | State | COND_None, REPNOTIFY_Always | 300.0 | 击杀奖励金币 |

- **注意**：构造函数硬编码默认值（`DBAHeroGrowthAttributeSet.cpp:17-21`），未走 DataAsset 驱动，与 BattleAttributeSet 的数据资产化路线不一致。详见第 10 节 P1-3。

---

## 5. GameplayEffect 清单

文件目录：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Effects\`

| GE 类名 | 继承 | 用途 | DataAsset 驱动 | 关键问题 |
|---------|------|------|----------------|----------|
| `UDBAGE_Base` | UGameplayEffect | GE 抽象基类 | 否 | 构造函数为空 |
| `UDBAGE_Cooldown` | UGameplayEffect | 冷却 GE（HasDuration） | 否（CooldownDuration 字段，CDO 默认 1.0） | 被 `UDBAElementAbilityBase::ApplyCooldown` 用作默认冷却 GE，运行时通过 `SetDuration` 覆盖时长 |
| `UDBAGE_Generic` | UGameplayEffect | 泛化技能 GE（SkillID+DataTable） | 是（SkillTable+SkillID） | 构造函数调用 `LoadAndApplyModifiers`；所有 Modifier 目标属性都是 CurrentHealth（伤害/治疗/护盾均加到生命），属占位/错误实现 |
| `UDBAGE_Fire_Resonance` | UDBAGE_Base | 火元素共鸣 | 是（DataTable） | **构造函数硬编码 LoadObject 同步加载**，路径疑似拼写错误 |
| `UDBAGE_Water_Resonance` | UDBAGE_Base | 水元素共鸣 | 同上 | 同上 |
| `UDBAGE_Wood_Resonance` | UDBAGE_Base | 木元素共鸣 | 同上 | 同上 |
| `UDBAGE_Gold_Resonance` | UDBAGE_Base | 金元素共鸣 | 同上 | 同上 |
| `UDBAGE_Earth_Resonance` | UDBAGE_Base | 土元素共鸣 | 同上 | 同上 |

**缺失的 GE 类**：未发现独立的伤害 GE、治疗 GE、护盾 GE、Buff/Debuff GE。当前伤害结算由 `UDBADamageCalculator::ApplyDamageToTargetWithCue`（非 GAS 路径）和 `UDBAGE_Generic`（占位）承担。

---

## 6. GameplayCue 清单

文件：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Cues\DBACue_Base.h`

| 类名 | 继承 | 职责 | 触发路径 |
|------|------|------|----------|
| `ADBACue_Base` | AGameplayCueNotify_Actor | GameplayCue 基类，按 SkillId 从 DataTable 查 VFX/SFX | `UDBAAbilitySystemComponent::TriggerGameplayCue` → `ExecuteGameplayCue` → `OnExecuteGameplayCue` |

- GameplayCue 标签定义在 `FDBAGameplayTags`：`GameplayCue.DBA.Skill.Cast` / `.Projectile` / `.Impact` / `.AOE` / `.Channel`。
- `ADBACue_Base::OnExecuteGameplayCue` 通过 `SkillId` 从技能 DataTable 查 `FDBASkillDataRow`，解析 VFX/SFX 软引用；未加载时发起异步预加载，已加载时直接 `SpawnEmitterAtLocation` / `PlaySoundAtLocation`。
- **硬编码路径**：`LoadSkillDataTableIfAvailable` 内部硬编码了两个候选路径 `/Game/DBA/Data/Skills/SkillDataTable` 与 `/Game/Data/Skills/SkillDataTable`。
- 当前目录下只有 `DBACue_Base` 一个 Cue 类，未见 5 元素或 12 生肖的具体 Cue 子类。

---

## 7. ASC 扩展能力

### 7.1 终极能量管理

- `UltimateEnergy`（float，ReplicatedUsing=OnRep_UltimateEnergy）权威存储于 ASC。
- `AddUltimateEnergy` / `ConsumeUltimateEnergy` / `HasEnoughUltimateEnergy` 均在服务端权威执行，Clamp 到 `[0, DBAConstants::MaxUltimateEnergy]`。
- 被动回复：`UltimateEnergyRegenTimerHandle`（1 秒间隔定时器）调用 `PassiveRegenUltimateEnergy`，每次 `+DBAConstants::UltimateEnergy_PassiveRegen`。
- 广播：`OnUltimateEnergyChanged`（CurrentEnergy, MaxEnergy）。

### 7.2 连锁等级管理

- `ChainLevel`（int32，ReplicatedUsing=OnRep_ChainLevel）权威存储于 ASC。
- `AddChainLevel` 更新 `LastHitTime` 并设置 `ChainResetTimerHandle`（`DBAConstants::ChainTimeout` = 6 秒后调用 `CheckChainReset`）。
- `ShouldTriggerChainFinisher` 在 `ChainLevel >= MaxChainLevel(10)` 时返回 true。

### 7.3 共鸣等级管理

- `ResonanceLevel`（int32，ReplicatedUsing=OnRep_ResonanceLevel）权威存储于 ASC。
- `CalculateResonanceLevel` 按同元素技能数量查 `DBAConstants::ResonanceLevel1~4_SkillCount` 阈值返回 0~4。
- `SetResonanceLevel` 由 `GrantAbilitiesFromFixedSkillGroup` 在授予技能后自动计算。

### 7.4 冷却同步机制

- `BeginPlay` 绑定 `OnActiveGameplayEffectAddedDelegateToSelf` 与 `OnAnyGameplayEffectRemovedDelegate`。
- 添加/移除带有 `Cooldown` 标签的 GE 时触发 `SyncCooldownsToCharacter`。
- `SyncCooldownsToCharacter` 通过 `GetDBAAvatarCharacter()` 推送冷却数组到 Character，并广播 `OnSkillCooldownUpdated` / `OnAllSkillCooldownsUpdated`。
- `GetSkillCooldowns` 通过 `GetCooldownTimeRemainingAndDuration(SpecHandle, ...)` 查询 GAS 冷却，保证同一泛型 Ability Class 的不同输入槽冷却独立。

### 7.5 固定技能组运行配置缓存

- `AbilityRuntimeConfigsByInputID`（TMap<int32, FDBAAbilityRuntimeConfig>）在 `GrantAbilitiesFromFixedSkillGroup` 时按 InputID 缓存。
- `FindAbilityRuntimeConfigByInputID` 供 `UDBAElementAbilityBase` 解析消耗/冷却/展示数据。

---

## 8. RPC 校验层

文件：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\RPC\DBARpcHandler.h`、`Private\GameDBA\RPC\DBARpcHandler.cpp`

### 8.1 ServerTryActivateAbility

- `_Validate`：校验 AbilityHandle 有效 → `ValidateServerCharacterContext`（角色未死亡、ASC 可用）→ `ValidateTarget` → `ValidateAbilityInputSemantics(bRequireUltimate=false)` → `ValidateAbilityCooldown`。
- `_Implementation`：重复执行 Validate 链后 `ASC->TryActivateAbility`，成功时 `ClientAbilityActivated`。
- **问题**：`_Validate` 与 `_Implementation` 存在校验重复执行（Validate 函数在 `_Implementation` 内又被调用一遍）。

### 8.2 ServerUltimateAbility

- `_Validate`：校验 AbilityHandle → 角色上下文 → 输入语义（`bRequireUltimate=true`）→ 冷却 → **`CharacterRef->GetUltimateEnergy() >= DBAConstants::MaxUltimateEnergy`**。
- `_Implementation`：同样校验能量后 `ASC->TryActivateAbility`。
- **能量校验读取路径**：`CharacterRef->GetUltimateEnergy()` → `ADBAZodiacCharacterBase::GetUltimateEnergy()` → `ASC->GetUltimateEnergy()`，最终读取 ASC，与 v1.8 单源策略一致。

### 8.3 ServerCancelAbility

- `_Validate`：校验 Handle 有效 + 角色上下文。
- `_Implementation`：`ASC->CancelAbilityHandle(Handle)`。

### 8.4 其他校验

- `ValidateAbilityCooldown`：通过 `DBAAbilitySystem->IsInputAbilityOnCooldown(Spec->InputID)` 校验，读取 Character 的 SkillCooldowns 缓存（详见 P1-2 风险）。
- `ValidateAbilityInputSemantics`：校验 Spec->InputID 与 `bRequireUltimate` 匹配，防止普通技能走大招 RPC。
- `ServerMoveTo_Validate`：地图边界 + 移动距离上限校验。
- `ServerRequestAttack_Validate`：攻击频率下限校验（`DBAConstants::MinAttackInterval`）。

---

## 9. 数据驱动情况

### 9.1 UDBABattleAttributeDefaultsDataAsset

- 文件：`Public\GameDBA\GAS\Attributes\DBABattleAttributeDefaultsDataAsset.h`
- 结构 `FDBABattleAttributeDefaults`：12 个字段（MaxHealth/CurrentHealth/AttackPower/Defense/MoveSpeed/MaxEnergy/CurrentEnergy/EnergyRegen/CriticalRate/CriticalMultiplier/MaxShield/CurrentShield）。
- `ValidateData_Implementation` 提供中文校验（最大值>0、当前值不超最大值、暴击率范围、暴击倍率下限、护盾范围）。
- 接入路径：`UDBABattleAttributeDeveloperSettings`（`Config=Game`，DisplayName="DBA 战斗属性设置"）的 `DefaultBattleAttributeDefaults` 软引用 → `ADBAPlayerState::RequestDefaultBattleAttributeDefaultsAsync` 异步加载 → `BattleAttributeSet->ApplyDefaultAttributes`。
- **资产状态**：Content 目录下**未发现**对应的 `.uasset`。未配置时输出中文缺配置警告。

### 9.2 UDBAFixedSkillGroupDataAsset（固定技能组）

- 文件：`Public\GameDBA\GAS\DBAAbilitySetLibrary.h`
- 继承 `UPrimaryDataAsset`，包含 PassiveAbilityClass、Skill01~04Class、ZodiacUltimateClass、ResonanceAbilityClass、Skill01~04RuntimeConfig、输入键配置。
- `FDBAAbilityRuntimeConfig`：DisplayName、Icon、EnergyCost、CostGameplayEffectClass、CooldownDuration、CooldownGameplayEffectClass、CooldownTag。
- `UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById`：已加载缓存读取 → 未加载时 `RequestAsyncLoad` → 兜底创建 Transient 资产。
- **资产状态**：Content 目录下**已存在** `DT_FixedSkillGroups.uasset` 和 5 个 `DA_FSG_Standard_5v5_1~5.uasset`（v1.8 说尚未创建，现已补齐）。

### 9.3 UDBAAbilitySetDataAsset（技能组汇总数据表）

- 文件：`Public\GameDBA\Data\DBAAbilitySetDataAsset.h`
- 继承 `UDataAsset`，管理 7 张 DataTable 软引用（ElementPassiveTable、ElementActiveAbilityTable、ElementUltimateTemplateTable、ElementResonanceTable、ZodiacUltimateTable、SkillDataTable、AbilitySetSummaryTable）。
- `LoadDataTable` 已迁移为异步边界：已加载返回 `DataTablePtr.Get()`，未加载调用 `RequestDataTableAsync` 返回 nullptr。
- `PreloadAllDataTablesAsync` 批量预热。
- **注意**：该资产与 `UDBAFixedSkillGroupDataAsset` 并存，前者是 DataTable 汇总入口，后者是技能授予主入口。v1.8 文档主要描述前者，但实际授予路径使用后者。

### 9.4 UDBAPlayableSkillCatalogDataAsset

- 文件：`Public\GameDBA\Combat\DBAPlayableSkillCatalogDataAsset.h`
- 包含 `CatalogId` 与 `TArray<FDBAPlayableSkillRuntimeSpec> SkillSpecs`。
- `UDBAPlayableSkillComponent` 通过 `DefaultSkillCatalog` 软引用异步加载。
- **资产状态**：Content 目录下**未发现**对应的 `.uasset`。

### 9.5 UDBAZodiacHeroDataAsset / UDBAStaticDataAsset

- 两者均已在 v1.7/v1.8 完成 DataTable 异步加载迁移，`LoadSynchronous` 已移除。
- **资产状态**：Content 目录下未发现对应 `.uasset`。

### 9.6 UDBAHeroBalanceConfig

- 文件：`Public\GameDBA\GAS\Balance\DBAAbilityBalance.h`
- 继承 `UDataAsset`，包含 12 个 `FDBAHeroBalanceData`（Rat_ShadowFang 等），用于 UI 展示和匹配。
- 当前作为 CDO 字段，未见运行时加载路径。

### 9.7 DBAConstants 硬编码常量

文件：`Public\GameDBA\Core\DBAConstants.h`，关键硬编码值：
- `MaxUltimateEnergy = 100.0f`、`UltimateEnergy_PassiveRegen = 1.0f`
- `MaxChainLevel = 10`、`ChainTimeout = 6.0f`
- `MaxResonanceLevel = 4`、`ResonanceLevel1~4_SkillCount = 2/3/4/5`
- `ResonanceLevel1~4_CCDuration = 0.25/0.50/0.75/1.0`、`ResonanceLevel1~4_ShieldBonus = 0.05~0.20`、`ResonanceLevel1~4_DamageBonus = 0.05~0.20`
- `DefenseReductionConstant = 100.0f`
- `ActiveSkillCount = 4`、`ArenaCombatSkillSlotCount = 5`

---

## 10. 问题清单（按优先级分级）

### P0（阻断性，必须优先处理）

#### P0-1：共鸣 GameplayEffect 构造函数硬编码同步 LoadObject（v1.8 未发现）

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Effects\DBAGE_Fire_Resonance.cpp:18`（及 Water/Wood/Gold/Earth 共 5 处）
- 代码：`UDataTable* ResonanceTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Elements/DBAElementResonanceRowe.DBAElementResonanceRowe'"));`
- 问题：
  1. 硬编码资产路径，违反 `DBA.DataAsset.NoHardcoding` 策略。
  2. `LoadObject` 同步阻塞加载，在 GE 构造函数中执行，违反 `DBA.UI.EventAsync` / 异步资源访问策略。
  3. 路径 `DBAElementResonanceRowe` 疑似拼写错误（正常应为 `DBAElementResonanceRow`），且该路径下大概率不存在资产。
  4. 构造函数中读取 DataTable 并构造 Modifier，但 Modifier 目标属性全部错误地设为 `CurrentHealth`（共鸣应修改控制时间、护盾、伤害等，而非生命值）。
- 影响：共鸣 GE 在 CDO 初始化时同步加载失败资产，Modifier 不会被添加，共鸣效果完全失效；若路径碰巧存在则会阻塞 GameThread。

#### P0-2：生肖大招未调用 CommitAbility（v1.8 未发现）

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAZodiacUltimateAbility_Generic.cpp:20-41`
- 问题：`UDBAZodiacUltimateAbility_Generic::ActivateAbility` 仅触发 GameplayCue、调用扩展点和 `Super::ActivateAbility`，**未调用 `CommitAbility`**。对比 `UDBAElementSkillAbility_Generic::ActivateAbility` 明确调用了 `CommitAbility`。
- 基类 `UDBAZodiacUltimateAbilityBase` 重写了 `CommitAbilityCost`（扣除终极能量），但 `CommitAbility` 未被调用意味着 `CommitAbilityCost` 也不会被触发。
- 影响：大招释放后终极能量不会被扣除，冷却也不会被应用（`ApplyCooldown` 未重写也未调用），导致大招可无限释放。RPC 层 `ServerUltimateAbility_Validate` 虽然校验了满能量，但实际消耗依赖 Ability 的 Commit 链，校验通过后能量不扣减。

#### P0-3：Character 仍保留 Replicated 终极能量/连锁/共鸣字段（死复制字段）

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h:366-375`、`Private\GameDBA\Character\DBAZodiacCharacterBase.cpp:1712-1714`
- 代码：`UPROPERTY(Replicated) float UltimateEnergy`、`int32 ChainLevel`、`int32 ResonanceLevel`，`DOREPLIFETIME_CONDITION(..., COND_OwnerOnly)`
- 问题：v1.8 声称"Character 写源已收敛到 ASC"，但 Character 仍保留这三个 Replicated 字段。当前 `SetUltimateEnergy`/`AddUltimateEnergy`/`AddChainLevel`/`ResetChainLevel` 已委托 ASC，`GetUltimateEnergy`/`GetChainLevel`/`GetResonanceLevel` 也读 ASC，因此这三个 Character 字段**永远不会被写入**，但仍参与网络复制（COND_OwnerOnly），浪费带宽并容易误导后续开发者直接读写 Character 字段绕过 ASC。
- 影响：观战/重连流程若读取 Character 字段会得到 0 值；后续开发者可能误以为 Character 是权威源。

### P1（重要，应尽快处理）

#### P1-1：UDBAElementAbilityBase::ApplyCost 直接 SetCurrentEnergy 绕过 GAS Modifier

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAElementAbilityBase.cpp:127-136`
- 问题：默认能量扣减路径直接 `AttrSet->SetCurrentEnergy(NewEnergy)`，而非通过 Cost GameplayEffect 的 Modifier。虽然 `FDBAAbilityRuntimeConfig` 支持 `CostGameplayEffectClass`，但未配置时走直接 Set 路径，绕过了 GAS 的 Modifier 聚合、Tag 条件、Execution 计算。
- v1.8 承认此点并建议迁移到 Cost GE。

#### P1-2：IsInputAbilityOnCooldown 读取 Character 缓存而非 GAS 冷却

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp:474-492`
- 问题：`IsInputAbilityOnCooldown` 通过 `GetDBAAvatarCharacter()->IsAbilityOnCooldown(SkillSpec.SkillId)` 读取 Character 的 `SkillCooldowns` 复制缓存，而非直接查询 ASC 的 GAS 冷却 GE。`TryActivateAbilityByInputID` 和 RPC `ValidateAbilityCooldown` 都依赖此函数。
- 风险：存在同步时序窗口——GAS 冷却 GE 已应用但 `SyncCooldownsToCharacter` 尚未执行时，冷却门禁会误判为未冷却，导致短时间内重复激活。服务端权威路径下此窗口较小但非零。

#### P1-3：UDBAHeroGrowthAttributeSet 构造函数硬编码默认值

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Attributes\DBAHeroGrowthAttributeSet.cpp:17-21`
- 代码：`InitHeroLevel(1.0f)`、`InitExperienceToNextLevel(100.0f)`、`InitRespawnTime(10.0f)`、`InitGoldBounty(300.0f)`
- 问题：与 `UDBABattleAttributeSet` 已完成的数据资产化路线不一致，成长属性默认值仍硬编码在构造函数中。

#### P1-4：UDBAGE_Generic 所有 Modifier 目标属性都是 CurrentHealth

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Effects\DBAGE_Generic.cpp:42-66`
- 问题：伤害、治疗、护盾三种 Modifier 都 `Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute()`，应分别为伤害（扣 CurrentHealth）、治疗（加 CurrentHealth）、护盾（加 CurrentShield）。当前实现是占位/错误。

#### P1-5：UDBACue_Base 硬编码技能表路径

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Cues\DBACue_Base.cpp:34-37`
- 代码：`static const TCHAR* CandidatePaths[] = { TEXT("/Game/DBA/Data/Skills/SkillDataTable.SkillDataTable"), TEXT("/Game/Data/Skills/SkillDataTable.SkillDataTable") };`
- 问题：硬编码资产路径，应通过 DataAsset 软引用或 DeveloperSettings 配置。

#### P1-6：ADBAZodiacCharacterBase::ApplyLobbyVisuals 同步 LoadObject 加载 SkeletalMesh

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp:394-406`
- 问题：`LoadObject<USkeletalMesh>` 和 `LoadObject<USkeleton>` 同步加载，且路径来自 `GetLobbyDisplayMeshCandidatePathsForZodiac` 硬编码候选。违反异步资源访问策略。

#### P1-7：DBAConstants 中大量平衡数值未数据资产化

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Core\DBAConstants.h:283-317`
- 问题：共鸣各级别的控制时间加成、护盾加成、伤害加成、终极能量阈值、被动回复量等均为编译期常量。这些属于运行平衡数据，应迁移到 DataAsset/DataTable/服务器配置。

#### P1-8：大招能量阈值/消耗量未数据资产化

- 证据：`e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAZodiacUltimateAbilityBase.cpp:36,56`
- 问题：`CanActivateAbility` 和 `CommitAbilityCost` 都使用 `DBAConstants::MaxUltimateEnergy` 作为阈值和消耗量，意味着大招必须满能量（100）才能释放且消耗全部能量。阈值和消耗量应可配置。

### P2（次要/改善项）

#### P2-1：UltimateEnergy 被动回复使用 Timer 而非 GE

- 证据：`DBAAbilitySystemComponent.cpp:91`
- 说明：1 秒间隔定时器调用 `PassiveRegenUltimateEnergy`。可接受但不够 GAS 原生，建议改为周期性 GE 或 Attribute-Based 回复。

#### P2-2：DBARpcHandler::CalculateAttackDamage 不走 GAS 属性

- 证据：`DBARpcHandler.cpp:744-775`
- 问题：伤害公式基于 `TargetMaxHealth * DBAConstants::BaseDamagePercentOfMaxHealth`，暴击用 `FMath::RandRange`，未读取攻击者 `AttackPower`/`CriticalRate`/`CriticalMultiplier` 属性。属 RPC 层占位伤害逻辑，应迁移到 GAS ExecutionCalculation 或 DamageGE。

#### P2-3：ResonanceAbilityBase::ApplyResonanceEffect 仅为日志占位

- 证据：`DBAResonanceAbilityBase.cpp:36-37`
- 问题：只输出 `UE_LOG`，未实际应用共鸣 Buff/Debuff。

#### P2-4：ZodiacPassiveAbility_Generic::ActivateAbility 无效果逻辑

- 证据：`DBAZodiacPassiveAbility_Generic.cpp:18-30`
- 问题：仅日志和扩展点调用，无实际被动效果应用。

#### P2-5：RPC _Validate 与 _Implementation 校验重复执行

- 证据：`DBARpcHandler.cpp:45-85`（ServerTryActivateAbility）、`DBARpcHandler.cpp:350-392`（ServerUltimateAbility）
- 问题：`_Implementation` 内部再次调用了 `_Validate` 链中的校验函数（`ValidateServerCharacterContext`、`ValidateAbilityInputSemantics`、`ValidateAbilityCooldown`），与 UE 的 `_Validate` → `_Implementation` 调用约定重复。

#### P2-6：缺少独立伤害/治疗/护盾 GE 类

- 说明：当前仅有 `UDBAGE_Generic`（占位）和共鸣 GE，未见 `UDBAGE_Damage`、`UDBAGE_Heal`、`UDBAGE_Shield` 等专用 GE。伤害结算主要走 `UDBADamageCalculator` 非 GAS 路径。

---

## 11. 改进建议

1. **P0-1 共鸣 GE 重构**：移除 5 个共鸣 GE 构造函数中的 `LoadObject`，改为通过 `UDBAAbilitySetDataAsset` 的 `ElementResonanceTable` 软引用异步加载；修正 Modifier 目标属性（控制时间→自定义属性或 ExecutionCalculation，护盾→CurrentShield，伤害→CurrentHealth 负向）；修正资产路径拼写。

2. **P0-2 大招 CommitAbility**：在 `UDBAZodiacUltimateAbility_Generic::ActivateAbility` 中、`Super::ActivateAbility` 之前显式调用 `CommitAbility(Handle, ActorInfo, ActivationInfo)`，提交失败时输出中文日志并 `EndAbility`，与 `UDBAElementSkillAbility_Generic` 保持一致。

3. **P0-3 移除 Character 死复制字段**：删除 `ADBAZodiacCharacterBase` 的 `UltimateEnergy`/`ChainLevel`/`ResonanceLevel` Replicated 字段及对应 `DOREPLIFETIME_CONDITION`，所有读取统一走 ASC Getter 或观战 DTO。

4. **P1-1 能量扣减迁移 Cost GE**：将 `UDBAElementAbilityBase::ApplyCost` 默认路径从直接 `SetCurrentEnergy` 迁移为通过 `UDBAGE_Cost`（新增）或 `FDBAAbilityRuntimeConfig::CostGameplayEffectClass` 应用 Cost GE。

5. **P1-2 冷却校验直查 GAS**：将 `IsInputAbilityOnCooldown` 改为直接通过 `GetCooldownTimeRemainingAndDuration` 查询 GAS 冷却，不再依赖 Character 缓存。

6. **P1-3 HeroGrowth 数据资产化**：新增 `UDBAHeroGrowthDefaultsDataAsset`，将 HeroLevel/Experience/RespawnTime/GoldBounty 默认值迁移到 DataAsset，与 BattleAttributeSet 路线一致。

7. **P1-7 常量数据资产化**：将 `DBAConstants` 中的共鸣数值、终极能量阈值、被动回复量迁移到 `UDBACombatBalanceDataAsset` 或 DataTable。

8. **资产补齐**：在 Editor 中创建真实的 `UDBABattleAttributeDefaultsDataAsset`、`UDBAPlayableSkillCatalogDataAsset`、`UDBAAbilitySetDataAsset` 及共鸣 DataTable，并配置到对应 DeveloperSettings。

---

## 12. 与 v1.8 文档的差异说明

### 12.1 v1.8 之后已落实的变化

| 变化项 | v1.8 状态 | 当前状态 | 证据 |
|--------|-----------|----------|------|
| 固定技能组 DataTable 资产 | 尚未导入/保存 | **已存在** `DT_FixedSkillGroups.uasset` + 5 个 `DA_FSG_Standard_5v5_*.uasset` | `Content/DBA/Data/Tables/DT_FixedSkillGroups.uasset`、`Content/DBA/Data/SkillGroups/` |
| `UDBAFixedSkillGroupDataAsset` 运行配置 | v1.8 描述为 `UDBAAbilitySetDataAsset` 的子结构 | 已独立为 `UDBAFixedSkillGroupDataAsset`（`GAS/DBAAbilitySetLibrary.h`），与 `UDBAAbilitySetDataAsset`（`Data/DBAAbilitySetDataAsset.h`）并存 | 两个独立类 |
| 共鸣 GE LoadObject | v1.8 未提及 | **新发现** 5 个共鸣 GE 构造函数硬编码同步加载 | 见 P0-1 |
| 大招 CommitAbility | v1.8 未提及 | **新发现** 未调用 CommitAbility | 见 P0-2 |

### 12.2 v1.8 已确认且当前仍成立的内容

- ASC 归属策略（PlayerState 持有、Character 作为 Avatar）。
- `GetDBAAvatarCharacter()` 统一 AvatarActor 解析路径。
- 终极能量/连锁/共鸣 ASC 权威管理（Character 写源已委托 ASC）。
- 普通元素技能 `CommitAbility` + `CheckCost`/`ApplyCost`/`CheckCooldown`/`ApplyCooldown` 接入。
- ASC 冷却事件驱动同步（ActiveGameplayEffect 增删事件）。
- Character Tick 不再递减冷却。
- `UDBABattleAttributeSet` 构造函数不再写运行默认值，由 DataAsset 驱动。
- `UDBAAbilitySetDataAsset` / `UDBAStaticDataAsset` / `UDBAZodiacHeroDataAsset` 的 DataTable 异步加载边界。

### 12.3 v1.8 声称已解决但当前仍有残留的内容

- v1.8 声称"Character 中仍存在历史兼容复制字段，后续新增代码只能通过 ASC/Getter/Delegate 读取"——当前仍保留 `UltimateEnergy`/`ChainLevel`/`ResonanceLevel` 三个 Replicated 死字段（P0-3）。
- v1.8 声称"普通技能消耗从当前直接扣 CurrentEnergy 进一步数据化为 Cost GE"——当前 `ApplyCost` 默认路径仍直接 Set（P1-1）。

### 12.4 v1.8 未覆盖的新风险

- P0-1（共鸣 GE 硬编码同步 LoadObject）、P0-2（大招未 Commit）、P1-2（冷却校验读 Character 缓存）、P1-3（HeroGrowth 硬编码）、P1-4（GE_Generic Modifier 错误）、P1-5（Cue 硬编码路径）、P1-6（ApplyLobbyVisuals 同步加载）均为本次审查新发现。

---

## 十三、2026-07-10 元素共鸣 GameplayEffect CDO 风险修复

本轮复核确认，五个元素共鸣 GameplayEffect（火、水、木、金、土）曾在构造函数/CDO 阶段通过 `LoadObject<UDataTable>` 同步加载共鸣表，并把 `ControlTimeBonus` 与 `ShieldBonus` 都作为 `CurrentHealth` 的加法 Modifier 写入。这会造成资源访问绕过异步边界，并把控制时长/护盾配置错误结算为生命值。

已完成的源码修复：

- 五个元素共鸣 GE 的构造函数不再同步加载 DataTable。
- 静态 GE 不再向 `CurrentHealth` 写入控制时长或护盾 Modifier，错误生命值结算已消除。
- 共鸣数值只能由 C++ 运行时路径和固定技能组/共鸣数据资产解析；在缺少专用控制时长 Attribute 与正式护盾应用协议前，不允许使用无关 Attribute 代替。

本轮验证：

```powershell
& 'D:\UnrealEngine-5.8.0-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaEditor Win64 Development -Project='E:\work\Game\DivineBeastsArena\DBA_GameClient\DivineBeastsArena.uproject' -WaitMutex -NoHotReloadFromIDE
```

验证结论：Editor 目标编译通过，五个共鸣 GE 均已重新编译并链接进 `UnrealEditor-DivineBeastsArena.dll`。

当前剩余风险：

- `UDBAResonanceAbilityBase::ApplyResonanceEffect` 目前只记录共鸣等级；它尚未从固定技能组数据资产读取护盾百分比并生成正确的 `CurrentShield` 运行时效果。
- `ControlTimeBonus` 需要独立、可复制且由控制效果消费的运行时属性或 SetByCaller 通道；在此之前不得映射到生命、护盾或其他无关 Attribute。
- 正式实现必须由服务端 C++ 在数据资产异步就绪后应用，并覆盖护盾、控制时长、重复授予/移除及网络复制自动化测试。

---

## 附：审查涉及的关键文件路径清单

**ASC 与 Ability 基类**
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\GameMoba\Public\GameMoba\GAS\DBAMobaAbilitySystemComponentBase.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\GameMoba\Private\GameMoba\GAS\DBAMobaAbilitySystemComponentBase.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\GameMoba\Public\GameMoba\GAS\DBAMobaGameplayAbilityBase.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\GameMoba\Private\GameMoba\GAS\DBAMobaGameplayAbilityBase.cpp`

**Ability 子类**
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAElementAbilityBase.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAElementAbilityBase.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAElementSkillAbility_Generic.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAElementSkillAbility_Generic.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAZodiacAbilityBase.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAZodiacUltimateAbilityBase.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAZodiacUltimateAbilityBase.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAZodiacUltimateAbility_Generic.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAZodiacUltimateAbility_Generic.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAZodiacPassiveAbility_Generic.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAZodiacPassiveAbility_Generic.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAResonanceAbilityBase.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAResonanceAbilityBase.cpp`

**AttributeSet 与 DataAsset**
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Attributes\DBABattleAttributeSet.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Attributes\DBABattleAttributeSet.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Attributes\DBAHeroGrowthAttributeSet.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Attributes\DBAHeroGrowthAttributeSet.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Attributes\DBABattleAttributeDefaultsDataAsset.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Attributes\DBABattleAttributeDeveloperSettings.h`

**GameplayEffect / GameplayCue / Balance**
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Effects\DBAGE_Base.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Effects\DBAGE_Cooldown.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Effects\DBAGE_Cooldown.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Effects\DBAGE_Generic.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Effects\DBAGE_Generic.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Effects\DBAGE_Fire_Resonance.cpp`（及 Water/Wood/Gold/Earth）
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Cues\DBACue_Base.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Cues\DBACue_Base.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Balance\DBAAbilityBalance.h`

**PlayerState / Character / RPC**
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Player\DBAPlayerState.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Player\DBAPlayerState.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\RPC\DBARpcHandler.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp`

**数据资产与常量**
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySetLibrary.h`（含 `UDBAFixedSkillGroupDataAsset`）
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Data\DBAAbilitySetDataAsset.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBAPlayableSkillCatalogDataAsset.h`
- `e:\work\Game\DivineBeastsArena\DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Core\DBAConstants.h`

---

**报告结束**。本报告基于 v1.9 增量复核，覆盖 ASC 归属、8 个 Ability 类、2 个 AttributeSet、8 个 GameplayEffect、1 个 GameplayCue 基类、ASC 扩展能力（终极能量/连锁/共鸣/冷却同步/运行配置缓存）、RPC 校验层、数据驱动情况，共发现 3 项 P0 严重问题、8 项 P1 架构性问题、6 项 P2 改进建议。审查未对任何文件进行修改操作，可作为后续 GAS 系统重构和资产补齐的依据。
