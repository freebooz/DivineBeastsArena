# DivineBeastsArena Niagara 魔法粒子效果设计规范

> 项目: DBA_GameClient  
> 版本: 0.1.0  
> 日期: 2026-05-29  
> 状态: Niagara 制作草案

## 1. 目标

本文把法师、战士、盗贼、猎人、术士、圣骑士、萨满、德鲁伊、死亡骑士、武僧等技能粒子方案，整理成本项目可执行的 Niagara 制作规范。

当前客户端已经具备这些运行时入口：

| 模块 | 现状 | 设计要求 |
| --- | --- | --- |
| `FDBAPlayableSkillRuntimeSpec` | 支持 Cast / Projectile / Impact Niagara 与 SFX | 新技能优先通过技能规格或 DataAsset 配置，不硬编码到角色逻辑 |
| `ADBASkillProjectileBase` | 支持复制投射物、飞行 VFX、命中 VFX | 投射物类技能拆成释放、飞行、命中、状态四段 |
| `UDBAEffectPlayer` / `UDBAEffectTableManager` | 支持技能释放、命中特效表 | 通用技能反馈走表驱动 |
| `/Game/DBA/VFX` | 已有火球和部分生肖/通用 VFX | 新资产统一放到技能目录，旧资产保留兼容 |

设计重点：

- 统一暴露参数，方便蓝图和 GAS 设置范围、伤害、持续时间、方向、强度。
- 统一资产命名和目录，避免后续技能越做越散。
- 先打磨当前可玩技能，再扩展完整职业幻想库。
- 粒子只负责表现，伤害、治疗、减速、恐惧、Buff 计时仍由 GAS / 技能逻辑驱动。

## 2. 视觉原则

### 2.1 战斗可读性

每个特效在前 0.2 秒内要让玩家看懂三件事：

| 问题 | 表现方式 |
| --- | --- |
| 谁释放了技能 | 手部/武器发光、施法闪光、角色脚下光环 |
| 危险或治疗范围在哪里 | 地面圈、投射物轨迹、光束、目标光环 |
| 技能何时结算 | 命中爆发、周期脉冲、护盾破碎、消散效果 |

### 2.2 与 DBA 五行体系的映射

用户提供的职业幻想可以保留，但项目内的 UI、伤害数字、元素克制和技能分类应继续兼容 `EDBAElement`。

| 幻想系别 | DBA 元素建议 | 主色 | 辅色 | 高光 |
| --- | --- | --- | --- | --- |
| 火焰、怒气、猛虎掌 | Fire | `#FF4400` | `#FF8800` | `#FFFF00` |
| 冰霜、水、迷雾 | Water | `#00CCFF` | `#88DDFF` | `#FFFFFF` |
| 自然、治疗、毒药 | Wood | `#44FF44` | `#88FF88` | `#AAFFAA` |
| 神圣、闪电、金色奥术 | Gold | `#FFDD44` | `#FFFF88` | `#FFFFFF` |
| 暗影、鲜血、邪能、土尘 | Earth / Shadow Variant | `#6622AA` 或 `#886644` | `#220066` | `#AA44FF` |

规则：

- Niagara 内部可以使用职业幻想色。
- 伤害数字、技能图标描边、阵营提示仍以 DBA 元素色为准。
- 暗影、鲜血、邪能这类非五行幻想，运行时可先挂到 `Gold` 或 `Earth`，表现上用暗影变体色。

## 3. 资产目录与命名

### 3.1 新资产目录

```text
/Game/DBA/VFX/Skills/<ClassOrSchool>/<SkillName>/
  NS_<SkillName>_Cast
  NS_<SkillName>_Projectile
  NS_<SkillName>_Impact
  NS_<SkillName>_Area
  NS_<SkillName>_Status
  NS_<SkillName>_End
  M_<SkillName>_<Purpose>
  MI_<SkillName>_<Purpose>
  T_<SkillName>_<Purpose>
```

示例：

```text
/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_Projectile
/Game/DBA/VFX/Skills/Shaman/ChainHeal/NS_ChainHeal_Beam
/Game/DBA/VFX/Skills/Paladin/HolyLight/NS_HolyLight_Target
```

兼容规则：

- 已被 C++ 引用的旧路径不要直接删除。
- 例如当前火球仍引用 `/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Projectile` 和 `NS_DBA_Fireball_Impact`。
- 若要迁移到新目录，先更新 `FDBAPlayableSkillRuntimeSpec` 或 DataAsset，再清理旧资产。

### 3.2 命名前缀

| 前缀 | 含义 |
| --- | --- |
| `NS_` | Niagara System |
| `NE_` | Niagara Emitter 模板 |
| `M_` | Material |
| `MI_` | Material Instance |
| `T_` | Texture |
| `BP_` | Blueprint |
| `DT_` | DataTable |

## 4. 通用 Niagara 暴露参数

所有技能系统尽量统一使用这些 `User.*` 参数：

```cpp
User.EffectRadius      // float，效果范围，单位厘米
User.Damage            // float，表现用伤害值，权威伤害仍由 GAS 处理
User.Duration          // float，持续时间
User.TargetLocation    // FVector，目标点
User.Direction         // FVector，方向向量，要求归一化
User.Intensity         // float，强度缩放，建议 0.0-2.0
User.TeamTint          // LinearColor，可选，阵营提示色
User.ElementColorA     // LinearColor，主色
User.ElementColorB     // LinearColor，辅色
User.HighlightColor    // LinearColor，高光色
```

投射物额外参数：

```cpp
User.ProjectileSpeed
User.ProjectileRadius
User.TrailLength
```

范围和 Buff 额外参数：

```cpp
User.TickInterval
User.TickPulse
User.OwnerHeight
```

蓝图或 C++ 设置示例：

```cpp
NiagaraComponent->SetVariableFloat(TEXT("User.Damage"), Damage);
NiagaraComponent->SetVariableFloat(TEXT("User.EffectRadius"), Radius);
NiagaraComponent->SetVariableFloat(TEXT("User.Duration"), Duration);
NiagaraComponent->SetVariableFloat(TEXT("User.Intensity"), Intensity);
NiagaraComponent->SetVariableVec3(TEXT("User.TargetLocation"), TargetLocation);
NiagaraComponent->SetVariableVec3(TEXT("User.Direction"), Direction);
NiagaraComponent->SetVariableLinearColor(TEXT("User.ElementColorA"), ElementColorA);
NiagaraComponent->Activate(true);
```

## 5. 标准系统模板

### 5.1 投射物技能模板

适用：火球术、寒冰箭、暗影箭、死亡缠绕、月火术、毒蛇钉刺、自动射击。

| 发射器 | Renderer | 用途 |
| --- | --- | --- |
| `Emitter_Core` | Sprite 或 Mesh | 投射物主体，保证远距离可读 |
| `Emitter_Trail` | Ribbon | 飞行轨迹和元素识别 |
| `Emitter_Sparks` | Sprite | 火星、冰晶、暗影碎片、毒液滴落等 |
| `Emitter_Light` | Niagara Light | 近距离发光，高/史诗画质开启 |
| `Emitter_Impact` | Sprite / Mesh | 命中点爆发，独立命中特效系统 |
| `Emitter_Status` | Sprite / Ribbon | 可选 DoT、减速、感染等附着状态 |

运行时拆分：

- `CastNiagaraVFXAsset`: 释放闪光、手部/武器蓄力。
- `ProjectileNiagaraVFXAsset`: 核心 + 拖尾，挂在 `ADBASkillProjectileBase` 上。
- `ImpactNiagaraVFXAsset`: 命中点一次性爆发。
- DoT、燃烧、冰冻、恐惧等持续状态独立为 Buff/Debuff Niagara，不塞进飞行系统。

### 5.2 范围技能模板

适用：暴风雪、旋风斩、火焰图腾、召唤仪式、正义祝福。

| 发射器 | Renderer | 用途 |
| --- | --- | --- |
| `Emitter_Ground` | Sprite / Mesh / Decal handoff | 显示半径、归属、危险区域 |
| `Emitter_Volume` | Sprite | 雾气、尘土、火焰、雪、能量 |
| `Emitter_TickPulse` | Ring / Sprite burst | 与伤害/治疗 Tick 同步 |
| `Emitter_Impact` | Sprite / Mesh | 周期性局部命中反馈 |
| `Emitter_End` | Burst | 结束、坍缩、消散 |

规则：

- Tick 节奏由技能逻辑或 GAS 控制。
- Niagara 可以同频率闪烁，但不能成为伤害/治疗权威来源。

### 5.3 Buff / Debuff 模板

适用：奥术智慧、潜行、毒药、野兽之心、恐惧术、回春、变形、圣盾。

| 发射器 | Renderer | 用途 |
| --- | --- | --- |
| `Emitter_Aura` | Sprite / Mesh ring | 地面或身体状态标识 |
| `Emitter_Body` | Sprite / Skeletal mesh location | 身体微光、火星、树叶、寒气、血雾 |
| `Emitter_Runes` | Sprite / Mesh | 环绕符文或职业符号 |
| `Emitter_TickPulse` | Burst | 周期治疗、伤害、控制反馈 |
| `Emitter_End` | Burst / dissolve | Buff 移除、护盾破碎、潜行进入/退出 |

规则：

- 持续系统挂在角色 root、胸口、武器或目标 socket。
- 移除时先 `Deactivate()`，再按需播放独立结束特效。

## 6. 当前可玩技能优先级

这些技能已经有对应运行时代码或默认配置，应优先做成高质量模板。

| 优先级 | SkillId | 显示名 | 形态 | 元素 | 目标 |
| --- | --- | --- | --- | --- | --- |
| P0 | `Lobby.Skill01.MageFireball` | Mage Fireball | Projectile | Fire | 升级当前火球飞行与命中效果 |
| P0 | `Lobby.Skill02.FrostShard` | Frost Shard | Projectile | Water | 将导入冰系占位替换成 DBA 寒冰箭语言 |
| P0 | `Lobby.Skill03.BloomHealing` | Bloom Healing | Area heal | Wood | 融合德鲁伊回春、治疗链的治疗粒子 |
| P0 | `Lobby.Skill04.ChainLightning` | Chain Lightning | Beam chain | Gold | 使用萨满闪电箭 + 治疗链跳跃逻辑 |
| P0 | `Lobby.Skill05.PriestShield` | Priest Shield | Buff shield | Wood / Gold | 融合圣盾、正义祝福、保护屏障 |
| P0 | `Lobby.Skill06.ShadowBolt` | Shadow Bolt | Projectile | Shadow | 替换当前毒系占位为真正暗影箭 |

## 7. P0 技能详细设计

### 7.1 火球术 Fireball

建议资产：

```text
/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_Cast
/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_Projectile
/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_Impact
/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_BurningStatus
```

| 发射器 | 设计 |
| --- | --- |
| `Emitter_Core` | 白黄核心到橙色外焰，尺寸 30 -> 5，自发光 3.0，带低半径光源 |
| `Emitter_Trail` | 橙红 Ribbon，粒子尺寸 5-15，向后速度 50，死亡时转暗 |
| `Emitter_Impact` | 橙黄爆发，向外 200-500，尺寸 50 -> 0，持续 0.3-0.5 秒 |
| `Emitter_BurningStatus` | 地面贴近火苗，持续 8 秒，每 2 秒视觉脉冲一次 |

参数：

| 参数 | 建议值 |
| --- | --- |
| `User.ProjectileSpeed` | 当前大厅 1580；慢速原型可用 500 |
| `User.ProjectileRadius` | 46 |
| `User.Duration` | 飞行 2-3 秒，燃烧 8 秒 |
| `ShakeScale` | 近距离命中 2.0 |

### 7.2 寒冰箭 / Frost Shard

建议资产：

```text
/Game/DBA/VFX/Skills/Mage/FrostBolt/NS_FrostBolt_Cast
/Game/DBA/VFX/Skills/Mage/FrostBolt/NS_FrostBolt_Projectile
/Game/DBA/VFX/Skills/Mage/FrostBolt/NS_FrostBolt_Impact
/Game/DBA/VFX/Skills/Mage/FrostBolt/NS_FrostBolt_SlowStatus
```

| 发射器 | 设计 |
| --- | --- |
| `Emitter_Core` | 青蓝球体，白色核心，尺寸 25，自发光 2.5 |
| `Emitter_Trail` | 蓝色细 Ribbon，冰晶粒子 3-8，透明淡出 |
| `Emitter_FreezeImpact` | 白/青爆裂，范围 100，可后续转地面 Decal |
| `Emitter_FrostStatus` | 冷雾缓慢上升，持续 3-6 秒 |

同步规则：

- 减速状态开始时挂 `SlowStatus`。
- 减速结束或目标死亡时播放小型冰晶碎裂消散。

### 7.3 闪电链 / Chain Lightning

建议资产：

```text
/Game/DBA/VFX/Skills/Shaman/ChainLightning/NS_ChainLightning_Cast
/Game/DBA/VFX/Skills/Shaman/ChainLightning/NS_ChainLightning_Beam
/Game/DBA/VFX/Skills/Shaman/ChainLightning/NS_ChainLightning_Impact
```

| 发射器 | 设计 |
| --- | --- |
| `Emitter_Precursor` | 起手叉状小闪电，主闪电前 0.1 秒 |
| `Emitter_LightningArc` | Ribbon 锯齿闪电，白色边缘、黄色核心、蓝紫光晕 |
| `Emitter_Branches` | 30% 分支概率，短生命周期 |
| `Emitter_ImpactSparks` | 白色火花四散，附淡绿色臭氧雾 |
| `Emitter_TargetTingle` | 目标身上 0.5 秒电光闪烁 |

实现建议：

- 用专用 Chain actor/component 更新每段 Beam 端点。
- 不建议把每次跳跃做成独立投射物，否则网络和命中反馈会变复杂。

### 7.4 绽放治疗 / Bloom Healing

建议资产：

```text
/Game/DBA/VFX/Skills/Druid/BloomHealing/NS_BloomHealing_Area
/Game/DBA/VFX/Skills/Druid/BloomHealing/NS_BloomHealing_Target
/Game/DBA/VFX/Skills/Druid/BloomHealing/NS_BloomHealing_Tick
```

| 发射器 | 设计 |
| --- | --- |
| `Emitter_HealingGlow` | 绿色旋转光环，半径由 `User.EffectRadius` 控制 |
| `Emitter_RisingLeaves` | 绿色/金色叶片上升，带轻微旋转和 Curl Noise |
| `Emitter_Droplets` | 绿色光滴落向目标，在接触时消失 |
| `Emitter_TickBurst` | 每 3 秒绿色治疗爆发 |

运行时：

- 当前大厅技能可做即时治疗 + 短持续区域。
- 若作为回春，使用 12 秒、每 3 秒一次 Tick、共 4 次治疗。

### 7.5 神圣护盾 / Priest Shield

建议资产：

```text
/Game/DBA/VFX/Skills/Paladin/Sanctuary/NS_Sanctuary_Cast
/Game/DBA/VFX/Skills/Paladin/Sanctuary/NS_Sanctuary_Shield
/Game/DBA/VFX/Skills/Paladin/Sanctuary/NS_Sanctuary_Break
```

| 发射器 | 设计 |
| --- | --- |
| `Emitter_SanctuaryAura` | 金色地面光环，半径 150-200，Alpha 脉冲 |
| `Emitter_RisingRunes` | 金色符文上升，环绕角色，尺寸 15 |
| `Emitter_Shield` | 半透明金色护盾，覆盖全身，用 Fresnel 材质 |
| `Emitter_Break` | 护盾破碎时金色碎片向外飞散 |

实现建议：

- 护盾主体优先用 Mesh + Material，Niagara 负责符文、碎片和闪光。
- `Emitter_Break` 由护盾值耗尽或 Buff 移除触发。

### 7.6 暗影箭 / Shadow Bolt

建议资产：

```text
/Game/DBA/VFX/Skills/Warlock/ShadowBolt/NS_ShadowBolt_Cast
/Game/DBA/VFX/Skills/Warlock/ShadowBolt/NS_ShadowBolt_Projectile
/Game/DBA/VFX/Skills/Warlock/ShadowBolt/NS_ShadowBolt_Impact
```

| 发射器 | 设计 |
| --- | --- |
| `Emitter_Core` | 紫黑能量球，尺寸 25，自发光 2.0，内部烟雾翻涌 |
| `Emitter_Trail` | 紫色暗影 Ribbon，宽度 10-20 |
| `Emitter_Impact` | 紫色爆发，尺寸 60，持续 0.3 秒 |
| `Emitter_Shards` | 紫色碎片飞出；若后续有灵魂资源，可向施法者回收 |

参数：

| 参数 | 建议值 |
| --- | --- |
| `User.ProjectileSpeed` | 当前大厅 1580；职业原型 800 |
| `User.ProjectileRadius` | 40 |
| `User.Intensity` | 暴击时提升碎片数量和亮度 |

## 8. 完整魔法粒子清单

| 职业 | 技能 | 系统组成 | 时间 | 重点 |
| --- | --- | --- | --- | --- |
| 法师 | 火球术 | Cast / Projectile / Impact / BurningStatus | 飞行 2-3s，燃烧 8s | 白热核心、橙红拖尾、暗色余烬 |
| 法师 | 寒冰箭 | Cast / Projectile / FreezeImpact / FrostStatus | 飞行 1.5-2s，减速 3s | 蓝白核心、冰晶拖尾、冷雾 |
| 法师 | 暴风雪 | Ground / FallingIce / Impact / Mist | 8s，Tick 0.5s | 半径 800，中心到边缘伤害衰减表现 |
| 法师 | 奥术智慧 | Runes / Particles / Aura / Ring | 长 Buff | 紫粉符文环绕，低成本持续系统 |
| 法师 | 传送门 | Vortex / Sparks / Glow / Startup / Close | 完成或取消 | 蓝紫内旋漩涡，关闭时外爆 |
| 战士 | 冲锋 | Dust / Trail / Indicator / Impact | 1s | 尘土、红色残影、黄色路径提示 |
| 战士 | 顺劈斩/致死打击 | Swing / WeaponGlow / Hit / Blood | 0.3-0.4s | 弧形 Ribbon 与武器动画同步 |
| 战士 | 旋风斩 | BladeVortex / DustVortex / WeaponTrail / Tick | 4s，Tick 0.5s | 150 半径刀刃旋转 |
| 盗贼 | 潜行 | FadeOut / ShadowRipple / CloakShimmer / FootFade | 进入 1s，持续 | 低透明斗篷微光，紫色脚印 |
| 盗贼 | 疾跑 | SpeedLines / DustTrail / Afterimage | Buff 持续 | 速度线和角色残影 |
| 盗贼 | 毒药 | Drip / WeaponGlow / Infection / Spread | 感染 12s，Tick 3s | 武器绿光、目标毒雾 |
| 猎人 | 自动射击 | MuzzleFlash / Projectile / Trail / ImpactDust | 速度 2000 | 箭矢 Mesh 更清晰 |
| 猎人 | 毒蛇钉刺 | PoisonArrow / DripTrail / DoT | 6s，Tick 2s | 命中后箭矢附着目标 |
| 猎人 | 野兽之心 | BeastAura / EyesGlow / Rage | Buff 持续 | 宠物红橙狂暴光环 |
| 术士 | 暗影箭 | Core / Trail / Impact / Shards | 速度 800 | 紫黑球体、灵魂碎片 |
| 术士 | 恐惧术 | Wisp / FearAura / TargetEffect | 3s | 紫色波纹和头顶控制标识 |
| 术士 | 召唤仪式 | Circle / RisingEnergy / Portal / Complete | 5s + 1s | 绿色召唤阵、中心传送门 |
| 圣骑士 | 圣光术 | LightRay / Burst / Healing / WarmGlow | 2s | 金白光柱和上升治疗粒子 |
| 圣骑士 | 十字军打击 | HammerArc / HolyImpact / Aura | 1s | 金色挥砍与命中爆发 |
| 圣骑士 | 正义祝福 | Aura / RisingRunes / Shield / Break | 30s | 金色护盾、符文、破碎 |
| 萨满 | 闪电箭 | Arc / Precursor / ImpactSparks / TargetTingle | 0.1s 前兆 | 锯齿 Ribbon，30% 分支 |
| 萨满 | 元素图腾 | Placement / Ambient / Activation | 每 3s 脉冲 | 一个系统用元素参数切换火水地风 |
| 萨满 | 治疗链 | HealingWave / JumpFlash / Healing | 3 次跳跃 | 目标 1/2/3 强度递减 |
| 德鲁伊 | 月火术 | Projectile / TargetGlow / Sparks | 12s，Tick 2s | 金绿色星形投射物 |
| 德鲁伊 | 回春/野性成长 | HealingGlow / RisingLeaves / Droplets / TickBurst | 12s，Tick 3s | 叶片旋转上升 |
| 德鲁伊 | 变形术 | TransformSwirl / LightBurst / FormAura | 1s | 形态颜色由参数控制 |
| 死亡骑士 | 死亡缠绕 | Projectile / Trail / Impact / Horror | 速度 600 | 绿黑死灵球体 |
| 死亡骑士 | 冰霜打击 | WeaponFrost / IceTrail / FrostBurst / Shatter | 命中 | 武器寒霜和碎冰 |
| 死亡骑士 | 鲜血打击 | BloodSpray / BloodDrip / BloodPool | 命中 | 血液重力和短时血泊 |
| 武僧 | 猛虎掌 | PalmImpact / ChiBlast / Sparks | 一次性 | 金橙掌击和气功扩散 |
| 武僧 | 翻滚 | DustCloud / MotionTrail / SpeedLines | 位移持续 | 路径残影和速度线 |
| 武僧 | 活血术 | Mist / HealingWave / TickHeal | 引导 8s，Tick 1s | 绿蓝迷雾和治疗 Ribbon |

## 9. Niagara 模块链

### 9.1 投射物标准链

```text
Emitter Spawn
  Spawn Burst Instantaneous: 1 个核心粒子
  Spawn Rate: 拖尾 20-80，随画质缩放
Particle Spawn
  Initialize Particle: 尺寸、颜色、Sprite/Mesh
  Initial Velocity: User.Direction * User.ProjectileSpeed
  Add Velocity: 火星/碎片随机锥形速度
Particle Update
  Solve Forces and Velocity
  Drag: 0.05-0.15
  Curl Noise: 0.2-0.6
  Scale Color: 按生命周期淡出
  Scale Sprite Size: 曲线缩放
Render
  Sprite Renderer 或 Mesh Renderer
  Ribbon Renderer 负责拖尾
  Niagara Light Renderer 仅高画质或英雄技能开启
```

### 9.2 爆炸标准链

```text
Emitter Spawn
  Spawn Burst Instantaneous: 24-80
Particle Spawn
  Initialize Particle: size 20-60, alpha 1
  Shape Location: sphere 或 disk
  Add Velocity: outward 200-500
Particle Update
  Solve Forces and Velocity
  Drag: 0.5
  Gravity: 仅碎冰、岩石、鲜血、碎片使用
  Scale Sprite Size: 大 -> 0
  Scale Color: alpha 1 -> 0
Render
  Additive: 魔法能量
  Translucent: 雾气、烟、血液
```

### 9.3 地面/光环标准链

```text
Emitter Spawn
  Spawn Rate: 10-40 continuous
Particle Spawn
  Shape Location: disk radius User.EffectRadius
  Initialize Particle: 低 alpha
Particle Update
  Curl Noise: 横向漂移
  Velocity: 上升 20-50
  Alpha Pulse: TickPulse > 0 时按 TickInterval 脉冲
Render
  Sprite Renderer: 粒子
  Mesh ring 或 Decal handoff: 清晰范围
```

## 10. GAS / 技能系统接入

### 10.1 激活流程

```text
Ability Activate
  读取 FDBAPlayableSkillRuntimeSpec 或 FDBASkillEffectRow
  在施法者 socket 播放 Cast Niagara
  设置 User 参数
  生成 Projectile / Beam / Area / Buff Actor
  提交消耗与冷却
```

### 10.2 命中流程

```text
Projectile 或 Trace 命中
  Server 验证命中并应用 GameplayEffect
  Multicast impact feedback
  在 HitResult.ImpactPoint 播放 Impact Niagara
  设置 Damage / EffectRadius / TargetLocation / Direction / Intensity
  播放音效、屏幕震动、伤害数字
```

### 10.3 Buff / Debuff 流程

```text
GameplayEffect Applied
  将 Status Niagara attach 到目标
  设置 Duration / TickInterval / ElementColorA/B

GameplayEffect Tick
  触发 TickPulse 或播放 TickBurst Niagara

GameplayEffect Removed
  Deactivate 持续 Niagara
  按需播放 End / Break Niagara
```

## 11. 性能预算

| 类型 | 同屏上限 | 粒子预算 | 说明 |
| --- | --- | --- | --- |
| 普通投射物 | 20 | 每个 20-60 | 非本地玩家关闭光源 |
| 重点投射物 | 8 | 每个 60-120 | 固定 Bounds，提前 Warmup |
| 命中爆发 | 20 | 每次 24-80 | 生命周期低于 0.8 秒 |
| 范围技能 | 4 | 总 100-250 | 远距离降低 Spawn Rate |
| 持续 Buff | 20 | 每个 5-40 | 避免光源和碰撞 |
| 仪式/传送门 | 2 | 150-300 | 竞技场内限制数量 |

画质缩放：

- Low: 关闭 Niagara Light，关闭二级碎片，拖尾生成率 40%。
- Medium: 本地玩家投射物保留 1 个光源，拖尾生成率 70%。
- High/Epic: 完整发射器、屏幕震动、更高 Ribbon 细分。

## 12. 制作检查清单

每个新技能完成前检查：

1. 资产放入规范目录。
2. 添加统一 `User.*` 参数。
3. 投射物和范围技能设置 Fixed Bounds。
4. 明亮地图和暗色地图都验证材质可读性。
5. 更新 `FDBAPlayableSkillRuntimeSpec`、DataAsset 或 `FDBASkillEffectRow`。
6. 测试释放、飞行、命中、持续状态、移除、冷却后再次释放。
7. 测试 Low / Medium / High 画质。
8. 验证网络路径：服务端命中、Multicast Impact、本地预测回退。

## 13. 建议实施顺序

| 阶段 | 范围 | 产出 |
| --- | --- | --- |
| Phase 1 | 火球、寒冰箭、暗影箭 | 三套高质量投射物模板 |
| Phase 2 | 闪电链、神圣护盾、绽放治疗 | 光束链、护盾 Buff、治疗区域模板 |
| Phase 3 | 暴风雪、传送门、召唤仪式 | 大范围和仪式类技能模板 |
| Phase 4 | 毒药、潜行、回春、正义祝福 | 持续状态模板库 |
| Phase 5 | 战士、盗贼、猎人、武僧物理混合技能 | 武器拖尾、残影、尘土、血液 |

Phase 1 和 Phase 2 会直接提升当前大厅可玩技能体验；后续阶段在对应 GameplayAbility 或技能类建立后，再通过数据表或 DataAsset 接入。
