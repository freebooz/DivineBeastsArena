# Divine Beasts Arena VFX/SFX 资源规范文档

## 一、概述

本文档定义游戏 VFX (视觉特效) 和 SFX (音效) 的资源目录结构、命名规范和配置方法，供设计师参考编辑。

---

## 二、资源目录结构

### 2.1 VFX 资源目录

```
Content/
├── VFX/
│   ├── Zodiac/                    # 生肖角色特效
│   │   ├── Rat/                   # 鼠
│   │   ├── Ox/                    # 牛
│   │   ├── Tiger/                 # 虎
│   │   ├── Rabbit/                # 兔
│   │   ├── Dragon/                # 龙
│   │   ├── Snake/                 # 蛇
│   │   ├── Horse/                 # 马
│   │   ├── Goat/                  # 羊
│   │   ├── Monkey/               # 猴
│   │   ├── Rooster/              # 鸡
│   │   ├── Dog/                  # 狗
│   │   └── Pig/                  # 猪
│   ├── Skills/                   # 技能特效
│   │   └── {Zodiac}/
│   │       ├── Passive/
│   │       ├── Q/
│   │       ├── W/
│   │       ├── E/
│   │       └── R/
│   ├── Elements/                 # 元素特效
│   │   ├── Fire/
│   │   ├── Water/
│   │   ├── Wood/
│   │   ├── Gold/
│   │   └── Earth/
│   ├── Common/                   # 通用特效
│   │   ├── HitEffects/
│   │   ├── BuffEffects/
│   │   └── DebuffEffects/
│   └── UI/                       # UI特效
```

### 2.2 SFX 资源目录

```
Content/
├── Audio/
│   ├── SFX/
│   │   ├── Zodiac/               # 生肖角色音效
│   │   │   └── {Zodiac}/
│   │   │       ├── Attack/
│   │   │       ├── Hit/
│   │   │       ├── Move/
│   │   │       ├── Death/
│   │   │       └── Voice/
│   │   ├── Skills/               # 技能音效
│   │   │   └── {Zodiac}/
│   │   │       ├── Passive/
│   │   │       ├── Q/
│   │   │       ├── W/
│   │   │       ├── E/
│   │   │       └── R/
│   │   ├── UI/                   # UI音效
│   │   └── Ambient/             # 环境音
│   └── Music/                    # 音乐
│       ├── Lobby/
│       ├── Battle/
│       └── Victory/
```

### 2.3 Animation 资源目录

```
Content/
├── Animation/
│   ├── Zodiac/                   # 生肖角色动画
│   │   └── {Zodiac}/
│   │       ├── ABPs/             # Animation Blueprint
│   │       ├── Montages/         # 动画蒙太奇
│   │       └── Notifies/         # 动画通知
│   └── Skills/                   # 技能动画
│       └── {Zodiac}/
│           ├── Passive/
│           ├── Q/
│           ├── W/
│           ├── E/
│           └── R/
```

---

## 三、命名规范

### 3.1 VFX 命名

| 类型 | 格式 | 示例 |
|------|------|------|
| 角色攻击 | P_{Zodiac}_Attack_{Type} | P_Dog_Attack_FX |
| 角色受击 | P_{Zodiac}_Hit_{Type} | P_Dog_Hit_FX |
| 角色移动 | P_{Zodiac}_Move_{Type} | P_Dog_Move_FX |
| 角色死亡 | P_{Zodiac}_Death_{Type} | P_Dog_Death_FX |
| 技能施法 | P_{Zodiac}_{Skill}_Cast | P_Dog_Q_Cast |
| 技能命中 | P_{Zodiac}_{Skill}_Impact | P_Dog_Q_Impact |
| 弹道飞行 | P_{Zodiac}_{Skill}_Projectile | P_Dog_Q_Projectile |
| 范围爆炸 | P_{Zodiac}_{Skill}_AOE | P_Dog_Q_AOE |
| 引导特效 | P_{Zodiac}_{Skill}_Channel | P_Dog_Q_Channel |
| 增益特效 | P_{Zodiac}_{Skill}_Buff | P_Dog_Q_Buff |
| 减益特效 | P_{Zodiac}_{Skill}_Debuff | P_Dog_Q_Debuff |

### 3.2 SFX 命名

| 类型 | 格式 | 示例 |
|------|------|------|
| 角色攻击 | S_{Zodiac}_Attack | S_Dog_Attack |
| 角色受击 | S_{Zodiac}_Hit | S_Dog_Hit |
| 角色移动 | S_{Zodiac}_Move | S_Dog_Move |
| 角色死亡 | S_{Zodiac}_Death | S_Dog_Death |
| 技能施法 | S_{Zodiac}_{Skill}_Cast | S_Dog_Q_Cast |
| 技能飞行 | S_{Zodiac}_{Skill}_Fly | S_Dog_Q_Fly |
| 技能命中 | S_{Zodiac}_{Skill}_Impact | S_Dog_Q_Impact |

### 3.3 Animation 命名

| 类型 | 格式 | 示例 |
|------|------|------|
| 动画蓝图 | ABP_{Zodiac} | ABP_Dog |
| 攻击动画 | AM_{Zodiac}_Attack | AM_Dog_Attack |
| 受击动画 | AM_{Zodiac}_Hit | AM_Dog_Hit |
| 移动动画 | AM_{Zodiac}_Move | AM_Dog_Move |
| 死亡动画 | AM_{Zodiac}_Death | AM_Dog_Death |
| 技能动画 | AM_{Zodiac}_{Skill}_Cast | AM_Dog_Q_Cast |

---

## 四、特效类型说明

### 4.1 VFX 特效类型

| 类型 | 说明 | 使用场景 |
|------|------|----------|
| Cast | 施法特效 | 技能释放瞬间 |
| Impact | 命中特效 | 技能击中目标 |
| Projectile | 弹道特效 | 飞行中的投射物 |
| AOE | 范围特效 | 范围技能爆发 |
| Channel | 引导特效 | 持续施法过程中 |
| Buff | 增益特效 | 获得正面状态 |
| Debuff | 减益特效 | 获得负面状态 |
| Ambient | 环境特效 | 持续环境效果 |

### 4.2 SFX 音效类型

| 类型 | 说明 | 使用场景 |
|------|------|----------|
| Cast | 施法音效 | 技能释放 |
| Fly | 飞行音效 | 弹道飞行中 |
| Impact | 命中音效 | 技能命中 |
| Equip | 装备音效 | 武器切换 |
| UI | 界面音效 | 按钮点击等 |

---

## 五、资源配置方法

### 5.1 在 C++ 中引用资源

```cpp
// 方法1: 使用 TSoftObjectPtr (延迟加载)
UPROPERTY(EditDefaultsOnly, Category = "VFX")
TSoftObjectPtr<UParticleSystem> AttackVFX;

// 方法2: 使用路径加载
UParticleSystem* VFX = LoadObject<UParticleSystem>(nullptr, TEXT("ParticleSystem'/Game/VFX/Zodiac/Dog/P_Dog_Attack_FX.P_Dog_Attack_FX'"));
```

### 5.2 在 DataTable 中配置资源

在 `SkillDataTable` 中配置 VFX/SFX:

| 列名 | 类型 | 说明 |
|------|------|------|
| VFXAsset | TSoftObjectPtr<UParticleSystem> | 技能特效 |
| SFXAsset | TSoftObjectPtr<USoundCue> | 技能音效 |

### 5.3 在 Blueprints 中配置

1. 打开对应角色蓝图
2. 添加 `DBAZodiacVFXComponent` 组件
3. 在组件属性中拖拽资源

---

## 六、十二生肖资源清单

### 6.1 Rat (鼠) - 夜隐灵鼠

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Rat/P_Rat_Attack_FX |
| Hit | /Game/VFX/Zodiac/Rat/P_Rat_Hit_FX |
| Move | /Game/VFX/Zodiac/Rat/P_Rat_Move_FX |
| Death | /Game/VFX/Zodiac/Rat/P_Rat_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Rat/S_Rat_Voice |

### 6.2 Ox (牛) - 镇岳神牛

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Ox/P_Ox_Attack_FX |
| Hit | /Game/VFX/Zodiac/Ox/P_Ox_Hit_FX |
| Move | /Game/VFX/Zodiac/Ox/P_Ox_Move_FX |
| Death | /Game/VFX/Zodiac/Ox/P_Ox_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Ox/S_Ox_Voice |

### 6.3 Tiger (虎) - 裂风虎君

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Tiger/P_Tiger_Attack_FX |
| Hit | /Game/VFX/Zodiac/Tiger/P_Tiger_Hit_FX |
| Move | /Game/VFX/Zodiac/Tiger/P_Tiger_Move_FX |
| Death | /Game/VFX/Zodiac/Tiger/P_Tiger_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Tiger/S_Tiger_Voice |

### 6.4 Rabbit (兔) - 月华灵兔

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Rabbit/P_Rabbit_Attack_FX |
| Hit | /Game/VFX/Zodiac/Rabbit/P_Rabbit_Hit_FX |
| Move | /Game/VFX/Zodiac/Rabbit/P_Rabbit_Move_FX |
| Death | /Game/VFX/Zodiac/Rabbit/P_Rabbit_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Rabbit/S_Rabbit_Voice |

### 6.5 Dragon (龙) - 云巡龙君

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Dragon/P_Dragon_Attack_FX |
| Hit | /Game/VFX/Zodiac/Dragon/P_Dragon_Hit_FX |
| Move | /Game/VFX/Zodiac/Dragon/P_Dragon_Move_FX |
| Death | /Game/VFX/Zodiac/Dragon/P_Dragon_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Dragon/S_Dragon_Voice |

### 6.6 Snake (蛇) - 玄花灵蛇

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Snake/P_Snake_Attack_FX |
| Hit | /Game/VFX/Zodiac/Snake/P_Snake_Hit_FX |
| Move | /Game/VFX/Zodiac/Snake/P_Snake_Move_FX |
| Death | /Game/VFX/Zodiac/Snake/P_Snake_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Snake/S_Snake_Voice |

### 6.7 Horse (马) - 踏风天驹

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Horse/P_Horse_Attack_FX |
| Hit | /Game/VFX/Zodiac/Horse/P_Horse_Hit_FX |
| Move | /Game/VFX/Zodiac/Horse/P_Horse_Move_FX |
| Death | /Game/VFX/Zodiac/Horse/P_Horse_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Horse/S_Horse_Voice |

### 6.8 Goat (羊) - 灵泽仙羊

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Goat/P_Goat_Attack_FX |
| Hit | /Game/VFX/Zodiac/Goat/P_Goat_Hit_FX |
| Move | /Game/VFX/Zodiac/Goat/P_Goat_Move_FX |
| Death | /Game/VFX/Zodiac/Goat/P_Goat_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Goat/S_Goat_Voice |

### 6.9 Monkey (猴) - 幻云灵猿

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Monkey/P_Monkey_Attack_FX |
| Hit | /Game/VFX/Zodiac/Monkey/P_Monkey_Hit_FX |
| Move | /Game/VFX/Zodiac/Monkey/P_Monkey_Move_FX |
| Death | /Game/VFX/Zodiac/Monkey/P_Monkey_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Monkey/S_Monkey_Voice |

### 6.10 Rooster (鸡) - 曜鸣神鸡

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Rooster/P_Rooster_Attack_FX |
| Hit | /Game/VFX/Zodiac/Rooster/P_Rooster_Hit_FX |
| Move | /Game/VFX/Zodiac/Rooster/P_Rooster_Move_FX |
| Death | /Game/VFX/Zodiac/Rooster/P_Rooster_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Rooster/S_Rooster_Voice |

### 6.11 Dog (狗) - 镇魄灵犬

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Dog/P_Dog_Attack_FX |
| Hit | /Game/VFX/Zodiac/Dog/P_Dog_Hit_FX |
| Move | /Game/VFX/Zodiac/Dog/P_Dog_Move_FX |
| Death | /Game/VFX/Zodiac/Dog/P_Dog_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Dog/S_Dog_Voice |

### 6.12 Pig (猪) - 福岳灵猪

| 特效类型 | 资源路径 |
|----------|----------|
| Attack | /Game/VFX/Zodiac/Pig/P_Pig_Attack_FX |
| Hit | /Game/VFX/Zodiac/Pig/P_Pig_Hit_FX |
| Move | /Game/VFX/Zodiac/Pig/P_Pig_Move_FX |
| Death | /Game/VFX/Zodiac/Pig/P_Pig_Death_FX |
| Voice | /Game/Audio/SFX/Zodiac/Pig/S_Pig_Voice |

---

## 七、技能资源清单 (60个技能)

### 7.1 技能特效命名

| 技能 | 施法特效 | 命中特效 | 弹道特效 |
|------|----------|----------|----------|
| Rat_Passive | P_Rat_Passive_Cast | P_Rat_Passive_Impact | - |
| Rat_Q | P_Rat_Q_Cast | P_Rat_Q_Impact | P_Rat_Q_Projectile |
| Rat_W | P_Rat_W_Cast | P_Rat_W_Impact | P_Rat_W_Projectile |
| Rat_E | P_Rat_E_Cast | P_Rat_E_Impact | P_Rat_E_Projectile |
| Rat_R | P_Rat_R_Cast | P_Rat_R_Impact | P_Rat_R_AOE |

(其他生肖同理)

---

## 八、元素共鸣特效

| 元素 | 共鸣特效路径 |
|------|--------------|
| Fire | /Game/VFX/Elements/Fire/P_Fire_Resonance |
| Water | /Game/VFX/Elements/Water/P_Water_Resonance |
| Wood | /Game/VFX/Elements/Wood/P_Wood_Resonance |
| Gold | /Game/VFX/Elements/Gold/P_Gold_Resonance |
| Earth | /Game/VFX/Elements/Earth/P_Earth_Resonance |

---

## 九、设计师工作流程

1. **创建资源**: 按照上述目录结构创建 VFX/SFX/Animation 资源
2. **命名资源**: 按照命名规范命名所有资源
3. **配置组件**: 在角色蓝图中添加 `DBAZodiacVFXComponent` 并配置资源
4. **配置技能**: 在 `SkillDataTable` 中配置技能的 VFX/SFX 资源路径
5. **测试验证**: 在游戏中测试所有特效和音效是否正常播放

---

## 十、常用工具和快捷键

| 功能 | 快捷键 |
|------|--------|
| 预览特效 | Space (在 Persona 中) |
| 播放音效 | Space (在 Content Browser 中) |
| 刷新资源 | F5 |
| 搜索资源 | Ctrl+F |

---

*本文档由脚本自动生成，最后更新: 2026-05-01*