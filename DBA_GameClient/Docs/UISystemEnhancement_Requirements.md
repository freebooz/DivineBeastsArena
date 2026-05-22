# UI系统完善需求规格

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.1.0
> 日期: 2026-05-05
> 状态: 待评审

---

## 1. 目标与范围

### 1.1 目标
完善UI系统的核心功能，包括血条/属性条显示、技能栏、UI状态管理。

### 1.2 范围
**包含:**
- 玩家单元框 (PlayerUnitFrame) 属性条显示
- 技能栏 (AbilityBar) 图标和冷却
- UI管理器 (DBAGameUIManager) 状态切换
- 头顶Overhead UI

**不包含:**
- Lobby UI (大厅界面)
- 具体美术资源/蓝图

---

## 2. 功能需求

### 2.1 玩家单元框 (PlayerUnitFrame)

#### 2.1.1 属性条组件
```cpp
// UDBAPlayerUnitFrameWidgetBase 扩展
public:
    /** 生命条 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UProgressBar* HealthBar;

    /** 能量条 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UProgressBar* EnergyBar;

    /** 经验条 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UProgressBar* XPBar;

    /** 终极能量条 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UProgressBar* UltimateEnergyBar;
```

#### 2.1.2 更新方法
```cpp
/** 更新生命值显示 */
UFUNCTION(BlueprintCallable, Category = "UI|PlayerUnitFrame")
void UpdateHealthBar(float CurrentHP, float MaxHP);

/** 更新能量显示 */
UFUNCTION(BlueprintCallable, Category = "UI|PlayerUnitFrame")
void UpdateEnergyBar(float CurrentEnergy, float MaxEnergy);

/** 更新经验显示 */
UFUNCTION(BlueprintCallable, Category = "UI|PlayerUnitFrame")
void UpdateXPBar(float CurrentXP, float MaxXP);

/** 更新终极能量 */
UFUNCTION(BlueprintCallable, Category = "UI|PlayerUnitFrame")
void UpdateUltimateEnergy(float Energy);
```

### 2.2 技能栏 (AbilityBar)

#### 2.2.1 技能槽组件
```cpp
// UDBAAbilityBarWidgetBase 扩展
public:
    /** 技能槽数组 (6个普通技能 + 1个终极技能) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    TArray<UDBAAbilitySlotWidget*> AbilitySlots;

    /** 被动技能槽 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UDBAAbilitySlotWidget* PassiveSlot;

    /** 共鸣技能槽 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UDBAAbilitySlotWidget* ResonanceSlot;
```

#### 2.2.2 技能槽Widget
```cpp
// UDBAAbilitySlotWidget (新创建)
public:
    /** 技能图标 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UImage* SkillIcon;

    /** 冷却遮罩 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UImage* CooldownOverlay;

    /** 快捷键文本 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    UTextBlock* HotkeyText;

    /** 设置技能数据 */
    UFUNCTION(BlueprintCallable, Category = "UI|AbilitySlot")
    void SetAbilityInfo(const FDBAAbilityInfo& AbilityInfo);

    /** 设置冷却状态 */
    UFUNCTION(BlueprintCallable, Category = "UI|AbilitySlot")
    void SetCooldown(float RemainingTime, float TotalTime);

    /** 设置是否可用 */
    UFUNCTION(BlueprintCallable, Category = "UI|AbilitySlot")
    void SetAvailable(bool bAvailable);
```

### 2.3 UI管理器 (DBAGameUIManager)

#### 2.3.1 状态切换
```cpp
// DBAGameUIManager 扩展
public:
    /** UI状态枚举 */
    UENUM(BlueprintType)
    enum class EDBAUIState : uint8
    {
        None,
        MainMenu,      // 主菜单
        Lobby,         // 大厅
        HeroSelect,    // 英雄选择
        Loading,      // 加载中
        InGame,        // 游戏中
        Pause          // 暂停
    };

    /** 获取当前状态 */
    UFUNCTION(BlueprintCallable, Category = "UI|Manager")
    EDBAUIState GetCurrentState() const { return CurrentState; }

    /** 切换UI状态 */
    UFUNCTION(BlueprintCallable, Category = "UI|Manager")
    void TransitionTo(EDBAUIState NewState);

    /** 注册状态改变回调 */
    UFUNCTION(BlueprintCallable, Category = "UI|Manager")
    void RegisterStateChangeCallback(FOnUIStateChanged Delegate);
```

#### 2.3.2 状态转换流程
```
MainMenu → Lobby → HeroSelect → Loading → InGame
                                          ↓
                                      Pause ← InGame
                                          ↓
                                        Resume → InGame
```

### 2.4 头顶Overhead UI

#### 2.4.1 单位头顶组件
```cpp
// UDBAOverheadWidgetComponent (新创建)
UCLASS()
class DIVINEBEASTSARENA_API UDBAOverheadWidgetComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /** 头顶Widget类 */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> OverheadWidgetClass;

    /** 显示血条 */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    bool bShowHealthBar = true;

    /** 显示名字 */
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    bool bShowName = true;

protected:
    UPROPERTY()
    UUserWidget* OverheadWidget;
};
```

---

## 3. 非功能需求

### 3.1 性能需求
- UI更新 < 16ms (60fps)
- Widget绑定延迟 < 5ms

### 3.2 网络同步
- 属性条通过属性复制同步
- 技能冷却通过GE同步

---

## 4. 技术方案

### 4.1 目录结构

```
GameDBA/UI/
├── Arena/
│   ├── AbilityBar/
│   │   ├── DBAAbilitySlotWidget.h/cpp     (新)
│   │   └── DBAAbilityBarWidgetBase.h/cpp (扩展)
│   ├── PlayerUnitFrame/
│   │   └── DBAPlayerUnitFrameWidgetBase.h/cpp (扩展)
│   └── Overhead/
│       └── DBAOverheadWidgetComponent.h/cpp (新)
└── DBAGameUIManager.h/cpp (扩展)
```

### 4.2 关键类设计

#### 4.2.1 FDBAAbilityInfo
```cpp
USTRUCT(BlueprintType)
struct FDBAAbilityInfo
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FText AbilityName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    UTexture2D* Icon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FKey Hotkey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float Cooldown;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    float CurrentCooldown;
};
```

---

## 5. 用户故事

| ID | 场景 | 预期行为 |
|----|------|----------|
| US-01 | 生命值变化 | 血条实时更新，数值同步 |
| US-02 | 能量变化 | 能量条实时更新 |
| US-03 | 技能冷却 | 冷却遮罩显示，冷却结束图标亮起 |
| US-04 | 状态切换 | 正确显示/隐藏对应UI状态 |
| US-05 | 单位头顶 | 头顶显示血条和名字 |

---

## 6. 验收标准

### 6.1 功能验收
- [ ] 血条实时显示当前HP/MaxHP
- [ ] 能量条实时显示当前Energy/MaxEnergy
- [ ] 技能冷却正确显示
- [ ] UI状态正确切换
- [ ] 头顶显示血条和名字

### 6.2 性能验收
- [ ] UI更新 < 16ms
- [ ] Widget绑定 < 5ms

---

## 7. 参考文档

- `DBAAbilityBarWidgetBase.h` - 技能栏基类
- `DBAPlayerUnitFrameWidgetBase.h` - 玩家单元框基类
- `DBAGameUIManager.h` - UI管理器

---

*文档生成时间: 2026-05-05*