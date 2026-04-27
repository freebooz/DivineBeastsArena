# DivineBeastsArena 数据资产系统文档

> 详细说明游戏数据资产架构、数据表配置、查表接口与数据验证机制。

---

## 1. 系统概览

### 1.1 数据资产架构图

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              数据资产系统                                        │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────┐    │
│  │                        UDBAStaticDataAsset                                │    │
│  │                    (主静态数据资产 - 游戏入口)                              │    │
│  │  - ZodiacStaticTable        生肖静态数据表                                │    │
│  │  - ElementDefinitionTable   元素定义数据表                                 │    │
│  │  - FiveCampDisplayTable     五大阵营显示数据表                             │    │
│  │  - MapDefinitionTable       地图定义数据表                                 │    │
│  │  - ModeDefinitionTable      游戏模式定义数据表                             │    │
│  └─────────────────────────────────────────────────────────────────────────┘    │
│                                      │                                          │
│         ┌────────────────────────────┼────────────────────────────┐             │
│         ▼                            ▼                            ▼             │
│  ┌─────────────────────┐  ┌─────────────────────┐  ┌─────────────────────┐      │
│  │UDBAZodiacHeroDataAsset│ │DBAResonanceBonusData│  │DBAFiveCampDisplayData│      │
│  │(生肖英雄数据资产)     │  │(共鸣加成数据)        │  │(阵营显示数据)        │      │
│  │  - 显示数据表        │  │  - 25 行 × 5×5     │  │  - 5 个阵营          │      │
│  │  - 配置数据表        │  │                     │  │                     │      │
│  │  - 技能组数据表       │  │                     │  │                     │      │
│  └─────────────────────┘  └─────────────────────┘  └─────────────────────┘      │
│                                                                                  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

### 1.2 数据资产类型

| 资产类型 | 基类 | 用途 |
|---------|------|------|
| `UDBAStaticDataAsset` | `UPrimaryDataAsset` | 全局静态数据入口 |
| `UDBAZodiacHeroDataAsset` | `UDataAsset` | 生肖英雄数据管理 |
| `UDBAResonanceBonusDataRow` | `FTableRowBase` | 共鸣加成数据行 |
| `FDBAFiveCampDisplayRow` | `FTableRowBase` | 阵营显示数据行 |

---

## 2. 主静态数据资产 (StaticDataAsset)

### 2.1 核心职责

`UDBAStaticDataAsset` 是游戏静态数据的统一入口，提供以下功能：
- 集中管理所有 `UDataTable` 软引用
- 统一的数据预加载接口
- 数据完整性验证

### 2.2 数据表引用

| 属性 | 类型 | 说明 |
|------|------|------|
| `ZodiacStaticTable` | `TSoftObjectPtr<UDataTable>` | 生肖静态数据表 |
| `ElementDefinitionTable` | `TSoftObjectPtr<UDataTable>` | 自然元素之力定义表 |
| `FiveCampDisplayTable` | `TSoftObjectPtr<UDataTable>` | 五大阵营显示数据表 |
| `MapDefinitionTable` | `TSoftObjectPtr<UDataTable>` | 地图定义数据表 |
| `ModeDefinitionTable` | `TSoftObjectPtr<UDataTable>` | 游戏模式定义数据表 |

### 2.3 核心 API

```cpp
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAStaticDataAsset : public UPrimaryDataAsset
{
    // 预加载所有静态数据表
    UFUNCTION(BlueprintCallable, Category = "StaticData")
    void PreloadAllTables();

    // 验证所有静态数据表是否有效
    UFUNCTION(BlueprintCallable, Category = "StaticData")
    bool ValidateAllTables() const;

    // 获取生肖静态数据表
    UFUNCTION(BlueprintCallable, Category = "StaticData|Zodiac")
    UDataTable* GetZodiacStaticTable() const;

    // 获取自然元素之力定义数据表
    UFUNCTION(BlueprintCallable, Category = "StaticData|Element")
    UDataTable* GetElementDefinitionTable() const;

    // 获取五大阵营显示数据表
    UFUNCTION(BlueprintCallable, Category = "StaticData|FiveCamp")
    UDataTable* GetFiveCampDisplayTable() const;

    // 获取地图定义数据表
    UFUNCTION(BlueprintCallable, Category = "StaticData|Map")
    UDataTable* GetMapDefinitionTable() const;

    // 获取游戏模式定义数据表
    UFUNCTION(BlueprintCallable, Category = "StaticData|Mode")
    UDataTable* GetModeDefinitionTable() const;

#if WITH_EDITOR
    // 编辑器下验证数据完整性
    virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};
```

### 2.4 使用流程

```
GameInstance 初始化
       │
       ▼
加载 UDBAStaticDataAsset
       │
       ▼
调用 PreloadAllTables()
       │
       ├── 同步加载所有 TSoftObjectPtr
       └── 返回加载完成的 DataTable
       │
       ▼
查询数据
├── GetZodiacStaticTable()->FindRow(...)
├── GetElementDefinitionTable()->FindRow(...)
└── ...
```

---

## 3. 生肖英雄数据资产 (ZodiacHeroDataAsset)

### 3.1 数据表结构

```
DBA_ZodiacHeroDataAsset
    ├── ZodiacHeroDisplayTable    (FDBAZodiacHeroDisplayRow × 12)
    ├── ZodiacHeroConfigTable      (FDBAZodiacHeroConfigRow × 12)
    ├── FixedSkillGroupTable       (FDBAZodiacElementFixedSkillGroupRow × 60)
    └── AbilitySetSummaryTable    (FDBAZodiacHeroAbilitySetSummaryRow × 12)
```

### 3.2 生肖英雄显示数据 (FDBAZodiacHeroDisplayRow)

**用途**：定义 12 生肖英雄的显示信息

| 字段 | 类型 | 说明 |
|------|------|------|
| `Zodiac` | `EDBAZodiac` | 生肖枚举值 |
| `DisplayName` | `FText` | 中文名称 |
| `EnglishName` | `FString` | 英文名称 |
| `ShortDescription` | `FText` | 简短描述 |
| `DetailedDescription` | `FText` | 详细描述 |
| `BackgroundStory` | `FText` | 背景故事 |
| `Icon` | `TSoftObjectPtr<UTexture2D>` | 图标 |
| `LargeIcon` | `TSoftObjectPtr<UTexture2D>` | 大图 |
| `SilhouetteIcon` | `TSoftObjectPtr<UTexture2D>` | 剪影图标 |
| `Portrait` | `TSoftObjectPtr<UTexture2D>` | 肖像 |
| `FullBodyArt` | `TSoftObjectPtr<UTexture2D>` | 全身立绘 |
| `SelectionBackground` | `TSoftObjectPtr<UTexture2D>` | 选择界面背景 |
| `ThemeColor` | `FLinearColor` | 主题色 |
| `SecondaryColor` | `FLinearColor` | 次要色 |
| `RoleTags` | `TArray<FName>` | 角色定位标签 |
| `DifficultyLevel` | `int32` | 难度等级 (1-5) |

**十二生肖名称映射**：

| 枚举 | 中文名 |
|------|--------|
| `Zodiac.Rat` | 夜隐灵鼠 |
| `Zodiac.Ox` | 镇岳神牛 |
| `Zodiac.Tiger` | 裂风虎君 |
| `Zodiac.Rabbit` | 月华灵兔 |
| `Zodiac.Dragon` | 云巡龙君 |
| `Zodiac.Snake` | 玄花灵蛇 |
| `Zodiac.Horse` | 踏风天驹 |
| `Zodiac.Goat` | 灵泽仙羊 |
| `Zodiac.Monkey` | 幻云灵猿 |
| `Zodiac.Rooster` | 曜鸣神鸡 |
| `Zodiac.Dog` | 镇魄灵犬 |
| `Zodiac.Pig` | 福岳灵猪 |

### 3.3 生肖英雄配置数据 (FDBAZodiacHeroConfigRow)

**用途**：定义生肖英雄的游戏配置

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `ZodiacUltimateSkillId` | `FName` | - | 生肖大招技能 ID |
| `UltimateName` | `FText` | - | 大招名称 |
| `UltimateDescription` | `FText` | - | 大招描述 |
| `UltimateIcon` | `TSoftObjectPtr<UTexture2D>` | - | 大招图标 |
| `UltimateEnergyCost` | `float` | 100.0 | 大招消耗能量 |
| `UltimateCooldown` | `float` | 120.0 | 大招冷却 (秒) |
| `BaseMaxHealth` | `float` | 1000.0 | 基础最大生命 |
| `BaseAttackPower` | `float` | 100.0 | 基础攻击力 |
| `BaseDefense` | `float` | 50.0 | 基础防御力 |
| `BaseMoveSpeed` | `float` | 600.0 | 基础移动速度 |
| `BaseMaxEnergy` | `float` | 100.0 | 基础最大能量 |
| `BaseEnergyRegen` | `float` | 5.0 | 基础能量回复 |
| `BaseCriticalRate` | `float` | 0.1 | 基础暴击率 |
| `BaseCriticalMultiplier` | `float` | 2.0 | 基础暴击倍率 |
| `CharacterClass` | `FSoftClassPath` | - | 角色类路径 |
| `SkeletalMesh` | `TSoftObjectPtr<USkeletalMesh>` | - | 骨骼网格体 |
| `AnimBlueprintClass` | `FSoftClassPath` | - | 动画蓝图类 |
| `VoiceSoundCue` | `TSoftObjectPtr<USoundCue>` | - | 音效 |
| `EffectSystem` | `TSoftObjectPtr<UParticleSystem>` | - | 特效 |

### 3.4 固定技能组数据 (FDBAZodiacElementFixedSkillGroupRow)

**用途**：定义生肖 × 元素 的固定技能组

**重要约束**：
- 12 生肖 × 5 元素 = 60 条固定技能组
- 玩家不自由选择技能，由系统查表生成
- FiveCamp 只改变表现，不改变技能组

| 字段 | 类型 | 说明 |
|------|------|------|
| `RowId` | `FName` | 行 ID |
| `ZodiacType` | `EDBAZodiac` | 生肖类型 |
| `ElementType` | `EDBAElement` | 元素类型 |
| `ElementPassiveSkillId` | `FName` | 被动技能 ID |
| `ElementSkill1Id` | `FName` | 元素技能 1 ID |
| `ElementSkill2Id` | `FName` | 元素技能 2 ID |
| `ElementSkill3Id` | `FName` | 元素技能 3 ID |
| `ElementSkill4Id` | `FName` | 元素技能 4 ID |
| `ZodiacUltimateSkillId` | `FName` | 生肖大招 ID |
| `ElementResonanceLevel` | `int32` | 元素共鸣等级 (0-4) |
| `ResonanceElement` | `EDBAElement` | 共鸣元素类型 |
| `ResonanceControlTimeBonus` | `float` | 共鸣控制时间加成 |
| `ResonanceShieldBonus` | `float` | 共鸣护盾加成 |
| `AbilitySetAsset` | `TSoftObjectPtr<UDataAsset>` | AbilitySet 资产引用 |

### 3.5 技能组汇总数据 (FDBAZodiacHeroAbilitySetSummaryRow)

**用途**：汇总单个生肖英雄的所有技能组引用

| 字段 | 类型 | 说明 |
|------|------|------|
| `Zodiac` | `EDBAZodiac` | 生肖类型 |
| `MetalSkillGroupRowId` | `FName` | 金系技能组行 ID |
| `WoodSkillGroupRowId` | `FName` | 木系技能组行 ID |
| `WaterSkillGroupRowId` | `FName` | 水系技能组行 ID |
| `FireSkillGroupRowId` | `FName` | 火系技能组行 ID |
| `EarthSkillGroupRowId` | `FName` | 土系技能组行 ID |
| `bAllSkillGroupsConfigured` | `bool` | 所有技能组是否已配置 |
| `ConfigurationCompleteness` | `float` | 配置完成度 (0.0-1.0) |

### 3.6 核心 API

```cpp
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacHeroDataAsset : public UDataAsset
{
    // 获取生肖英雄显示数据
    UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
    bool GetZodiacHeroDisplayData(EDBAZodiac Zodiac, FDBAZodiacHeroDisplayRow& OutRow) const;

    // 获取生肖英雄配置数据
    UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
    bool GetZodiacHeroConfigData(EDBAZodiac Zodiac, FDBAZodiacHeroConfigRow& OutRow) const;

    // 获取固定技能组数据
    UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
    bool GetFixedSkillGroupData(EDBAZodiac Zodiac, EDBAElement Element,
        FDBAZodiacElementFixedSkillGroupRow& OutRow) const;

    // 获取技能组汇总数据
    UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
    bool GetAbilitySetSummaryData(EDBAZodiac Zodiac,
        FDBAZodiacHeroAbilitySetSummaryRow& OutRow) const;

    // 获取所有可用的生肖英雄
    UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
    void GetAllAvailableZodiacs(TArray<EDBAZodiac>& OutZodiacs) const;

    // 检查生肖英雄是否可用
    UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
    bool IsZodiacAvailable(EDBAZodiac Zodiac) const;

    // 检查技能组是否可用
    UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
    bool IsSkillGroupAvailable(EDBAZodiac Zodiac, EDBAElement Element) const;

    // 验证数据完整性
    UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
    bool ValidateDataIntegrity(TArray<FString>& OutErrors) const;
};
```

---

## 4. 共鸣加成数据 (ResonanceBonusData)

### 4.1 数据结构

`FDBAResonanceBonusDataRow` 用于定义共鸣等级对应的 GameplayEffect 配置。

**共鸣等级定义**：

| 等级 | 同元素技能数 | 说明 |
|------|-------------|------|
| Lv.0 | < 2 | 无共鸣 |
| Lv.1 | 2 | 初级共鸣 |
| Lv.2 | 3 | 中级共鸣 |
| Lv.3 | 4 | 高级共鸣 |
| Lv.4 | 5+ | 超级共鸣 |

### 4.2 数据字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `ResonanceLevel` | `int32` | 共鸣等级 (0-4) |
| `DamageBonusPercent` | `float` | 伤害加成 (%) |
| `DefenseBonusPercent` | `float` | 防御加成 (%) |
| `HealthBonusPercent` | `float` | 生命值加成 (%) |
| `EnergyRegenBonusPercent` | `float` | 能量恢复加成 (%) |
| `MoveSpeedBonusPercent` | `float` | 移动速度加成 (%) |
| `ControlTimeBonusSeconds` | `float` | 控制时间加成 (秒) |
| `ShieldBonusPercent` | `float` | 护盾值加成 (%) |
| `GameplayEffectAsset` | `TSoftObjectPtr<UDataAsset>` | GE 资产路径 |

### 4.3 查表方法

```cpp
// 静态查表方法
static FDBAResonanceBonusDataRow* FindRow(UDataTable* DataTable,
    EDBAElement Element, int32 Level)
{
    if (!DataTable) return nullptr;

    // 行命名格式：{Element}_{Level}
    FString RowName = FString::Printf(TEXT("%s_%d"),
        *UEnum::GetValueAsString(Element), Level);

    return DataTable->FindRow<FDBAResonanceBonusDataRow>(FName(RowName), TEXT(""));
}
```

**数据行数**：25 行 (5 元素 × 5 共鸣等级)

---

## 5. 五大阵营显示数据 (FiveCampDisplayData)

### 5.1 核心概念

五大阵营**只影响表现层**，不影响：
- 自然元素之力克制
- 核心属性
- 伤害公式
- 技能组生成
- TeamId

### 5.2 五大阵营

| 枚举 | 中文名 | 英文名 |
|------|--------|--------|
| `Byakko` | 白虎 | White Tiger |
| `Qinglong` | 青龙 | Azure Dragon |
| `Xuanwu` | 玄武 | Black Tortoise |
| `Zhuque` | 朱雀 | Vermilion Bird |
| `Kirin` | 麒麟 | Qilin |

### 5.3 数据字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `FiveCampEnum` | `uint8` | 阵营枚举值 |
| `DisplayNameCN` | `FText` | 中文显示名称 |
| `DisplayNameEN` | `FText` | 英文显示名称 |
| `Description` | `FText` | 描述 |
| `IconTexture` | `TSoftObjectPtr<UTexture2D>` | 图标 |
| `EmblemTexture` | `TSoftObjectPtr<UTexture2D>` | 徽记 |
| `BackgroundTexture` | `TSoftObjectPtr<UTexture2D>` | 法相背景 |
| `ThemeColor` | `FLinearColor` | 主题色 |
| `SecondaryColor` | `FLinearColor` | 次要色 |
| `EffectMaterial` | `TSoftObjectPtr<UMaterialInterface>` | 特效材质 |
| `ThemeSound` | `TSoftObjectPtr<USoundCue>` | 音效主题 |
| `SelectionMusic` | `TSoftObjectPtr<USoundCue>` | 选择界面背景音乐 |
| `SkinThemeTag` | `FName` | 皮肤主题标签 |
| `bIsAvailable` | `bool` | 是否可用 |
| `UnlockLevel` | `int32` | 解锁等级要求 |

---

## 6. 生肖静态数据 (ZodiacStaticData)

### 6.1 数据字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `ZodiacEnum` | `uint8` | 生肖枚举值 |
| `DisplayNameCN` | `FText` | 中文名称 |
| `DisplayNameEN` | `FText` | 英文名称 |
| `Description` | `FText` | 描述 |
| `IconTexture` | `TSoftObjectPtr<UTexture2D>` | 图标 |
| `SilhouetteTexture` | `TSoftObjectPtr<UTexture2D>` | 剪影 |
| `UltimateAbilityName` | `FText` | 大招名称 |
| `UltimateAbilityDescription` | `FText` | 大招描述 |
| `UltimateAbilityIcon` | `TSoftObjectPtr<UTexture2D>` | 大招图标 |
| `ThemeColor` | `FLinearColor` | 主题色 |
| `bIsAvailable` | `bool` | 是否可用 |
| `UnlockLevel` | `int32` | 解锁等级要求 |

---

## 7. 数据验证机制

### 7.1 编辑器验证

```cpp
#if WITH_EDITOR
virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
```

### 7.2 运行时验证

```cpp
// UDBAStaticDataAsset
bool ValidateAllTables() const;

// UDBAZodiacHeroDataAsset
bool ValidateDataIntegrity(TArray<FString>& OutErrors) const;
```

### 7.3 验证检查项

| 检查项 | 说明 |
|--------|------|
| 表引用有效性 | 检查 `TSoftObjectPtr` 是否有效 |
| 行数据完整性 | 检查必填字段是否有值 |
| 枚举值合法性 | 检查枚举值是否在有效范围内 |
| 命名规范 | 检查行命名是否符合规范 |
| 依赖资源 | 检查资源路径是否可加载 |

---

## 8. 查表流程

### 8.1 技能组查询流程

```
玩家选择 Zodiac + Element
         │
         ▼
UDBAZodiacHeroDataAsset::GetFixedSkillGroupData(Zodiac, Element)
         │
         ▼
构建行名称: Zodiac_{ZodiacName}_Element_{ElementName}
         │
         ▼
FixedSkillGroupTable->FindRow<FDBAZodiacElementFixedSkillGroupRow>(RowName)
         │
         ▼
返回技能组数据
├── ElementPassiveSkillId
├── ElementSkill1Id ~ ElementSkill4Id
├── ZodiacUltimateSkillId
└── ElementResonanceLevel
```

### 8.2 共鸣加成查询流程

```
确定 Element + ResonanceLevel
         │
         ▼
FDBAResonanceBonusDataRow::FindRow(DataTable, Element, Level)
         │
         ▼
构建行名称: {Element}_{Level}
         │
         ▼
DataTable->FindRow<FDBAResonanceBonusDataRow>(RowName)
         │
         ▼
返回共鸣加成
├── DamageBonusPercent
├── DefenseBonusPercent
├── ControlTimeBonusSeconds
└── ...
```

---

## 9. 数据表命名规范

### 9.1 行命名格式

| 数据表 | 行命名格式 | 示例 |
|--------|-----------|------|
| 生肖静态表 | `Zodiac_{ZodiacName}` | `Zodiac_Rat` |
| 元素定义表 | `Element_{ElementName}` | `Element_Metal` |
| 技能组表 | `Zodiac_{Zodiac}_Element_{Element}` | `Zodiac_Rat_Element_Metal` |
| 共鸣加成表 | `{Element}_{Level}` | `Metal_1` |
| 五大阵营表 | `FiveCamp_{CampName}` | `FiveCamp_Byakko` |

### 9.2 表命名建议

| 用途 | 表名建议 |
|------|---------|
| 生肖静态数据 | `DBA_ZodiacStaticDataTable` |
| 元素定义数据 | `DBA_ElementDefinitionDataTable` |
| 生肖英雄显示数据 | `DBA_ZodiacHeroDisplayDataTable` |
| 生肖英雄配置数据 | `DBA_ZodiacHeroConfigDataTable` |
| 固定技能组数据 | `DBA_FixedSkillGroupDataTable` |
| 共鸣加成数据 | `DBA_ResonanceBonusDataTable` |
| 五大阵营显示数据 | `DBA_FiveCampDisplayDataTable` |

---

## 10. 常量配置

| 配置项 | 常量名 | 值 | 说明 |
|--------|--------|-----|------|
| 生肖数量 | `MaxZodiacCount` | 12 | 生肖总数 |
| 元素数量 | `MaxElementCount` | 5 | 元素总数 |
| 技能组数量 | `MaxSkillGroupCount` | 60 | 12×5 技能组 |
| 共鸣加成行数 | `MaxResonanceRows` | 25 | 5×5 共鸣表 |
| 大招默认能量 | `DefaultUltimateEnergyCost` | 100.0 | 大招消耗 |
| 大招默认冷却 | `DefaultUltimateCooldown` | 120.0 | 大招冷却 (秒) |

---

*文档版本: 1.0*
*生成时间: 2026-04-27*
*引擎版本: UE 5.7.1*
