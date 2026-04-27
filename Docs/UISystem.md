# DivineBeastsArena UI 系统文档

> Widget Controller 模式与 UI 事件广播机制详细说明。

---

## 1. 系统概览

### 1.1 UI 系统架构图

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              UI 系统架构                                            │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────┐    │
│  │                         GameInstanceSubsystem                               │    │
│  │  - 提供全局数据和业务逻辑                                                   │    │
│  │  - 持有 WidgetController 实例                                              │    │
│  └─────────────────────────────────────────────────────────────────────────┘    │
│                                    │                                              │
│                                    ▼                                              │
│  ┌─────────────────────────────────────────────────────────────────────────┐    │
│  │                          WidgetController                                 │    │
│  │  - 管理 Widget 的数据和业务逻辑                                            │    │
│  │  - 订阅 GameInstanceSubsystem 事件                                        │    │
│  │  - 通过 BlueprintAssignable 分发事件到 Widget                               │    │
│  └─────────────────────────────────────────────────────────────────────────┘    │
│                                    │                                              │
│                                    ▼                                              │
│  ┌─────────────────────────────────────────────────────────────────────────┐    │
│  │                            UserWidget                                      │    │
│  │  - 纯表现层，绑定 BlueprintAssignable 事件                                 │    │
│  │  - 通过 WidgetController 获取数据                                           │    │
│  └─────────────────────────────────────────────────────────────────────────┘    │
│                                                                                  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

### 1.2 UI 类层级

```cpp
UObject
└── UObject (NoExportTypes.h)
    └── UDBAWidgetController           // 控制器基类
        ├── UDBAWidgetBase            // 控件基类 (deprecated?)
        └── UDBAArenaHUDWidgetController
            └── UDBAPlayerUnitFrameWidgetController

UUserWidget
└── UDBAUserWidgetBase               // UserWidget 基类
    └── UDBAArenaHUDRootWidgetBase  // 竞技场 HUD 根 Widget
        ├── UDBAPlayerUnitFrameWidgetBase
        ├── UDBAAbilityBarWidgetBase
        ├── UDBABuffBarWidgetBase
        └── ... (其他 HUD 组件)
```

---

## 2. 核心类说明

### 2.1 UDBAWidgetController

**文件**: `Public/UI/DBAWidgetController.h`

Widget 控制器基类，负责管理 Widget 的数据和业务逻辑。

```cpp
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAWidgetController : public UObject
{
    GENERATED_BODY()

public:
    UDBAWidgetController(const FObjectInitializer& ObjectInitializer);

    // 初始化控制器
    UFUNCTION(BlueprintCallable)
    virtual void InitializeController();

    // 重置控制器状态
    UFUNCTION(BlueprintCallable)
    virtual void ResetController();

protected:
    // 控制器是否已初始化
    UPROPERTY(BlueprintReadOnly)
    bool bIsInitialized = false;
};
```

### 2.2 UDBAUserWidgetBase

**文件**: `Public/UI/DBAUserWidgetBase.h`

UMG UserWidget 基类，用于所有可在屏幕上显示的 UI 控件。

```cpp
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAUserWidgetBase : public UUserWidget
{
    GENERATED_BODY()

public:
    UDBAUserWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    // Widget 被激活时调用（显示时）
    UFUNCTION(BlueprintCallable)
    virtual void Activate();

    // Widget 被取消激活时调用（隐藏时）
    UFUNCTION(BlueprintCallable)
    virtual void Deactivate();

    // 检查 Widget 是否处于激活状态
    UFUNCTION(BlueprintPure)
    bool IsActive() const { return bIsActive; }

protected:
    UPROPERTY(BlueprintReadOnly)
    bool bIsActive = false;

    // Widget 的优先级
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WidgetPriority = 0;
};
```

---

## 3. WidgetController 模式

### 3.1 模式说明

WidgetController 模式将数据和表现分离：

| 分层 | 职责 | 类 |
|------|------|-----|
| 数据层 | GameInstanceSubsystem 管理全局数据 | `XXXSubsystem` |
| 逻辑层 | WidgetController 处理业务逻辑 | `XXXWidgetController` |
| 表现层 | UserWidget 仅负责显示 | `XXXWidgetBase` |

### 3.2 数据流向

```
GameInstanceSubsystem (数据源)
    │
    │ 1. 订阅事件
    ▼
WidgetController (业务逻辑)
    │
    │ 2. 处理数据
    ▼
    │ 3. BlueprintAssignable 广播
    ▼
UserWidget (UI 表现)
```

### 3.3 事件类型选择

| 宏 | 用途 | 说明 |
|-----|------|------|
| `DECLARE_EVENT` | 单播事件 | Controller → Widget (1对1) |
| `DECLARE_EVENT_OneParam` | 单播事件(带参数) | Controller → Widget (1对1) |
| `DECLARE_MULTICAST_DELEGATE` | 多播事件 | Controller → Widget (1对多) |
| `DECLARE_DYNAMIC_MULTICAST_DELEGATE` | 动态多播事件 | 蓝图可用 |

---

## 4. 输入系统 UI 事件

### 4.1 DBAInputRouterComponent 事件

**文件**: `Public/Input/DBAInputRouterComponent.h`

```cpp
// ========================================
// UI 事件广播（供 UI 层订阅）
// ========================================

// Ping 事件
DECLARE_EVENT_OneParam(UDBAInputRouterComponent, FDBAOnPingTriggered, FVector)
FDBAOnPingTriggered OnPingTriggeredEvent;

// 计分板切换事件
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnScoreboardTriggered)
FDBAOnScoreboardTriggered OnScoreboardTriggeredEvent;

// 菜单切换事件
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnMenuTriggered)
FDBAOnMenuTriggered OnMenuTriggeredEvent;

// 聊天触发事件
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnChatTriggered)
FDBAOnChatTriggered OnChatTriggeredEvent;

// 地图切换事件
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnMapTriggered)
FDBAOnMapTriggered OnMapTriggeredEvent;

// 锁定目标切换事件
DECLARE_EVENT_OneParam(UDBAInputRouterComponent, FDBAOnLockTargetChanged, AActor*)
FDBAOnLockTargetChanged OnLockTargetChangedEvent;
```

### 4.2 Android 触屏输入事件

**文件**: `Public/Input/DBAAndroidTouchInputBridge.h`

```cpp
// ========================================
// 技能轮盘事件
// ========================================

// 技能轮盘展开事件
DECLARE_EVENT_OneParam(UDBAAndroidTouchInputBridge, FDBAOnSkillWheelShow, int32)
FDBAOnSkillWheelShow OnSkillWheelShowEvent;

// 技能轮盘隐藏事件
DECLARE_EVENT(UDBAAndroidTouchInputBridge, FDBAOnSkillWheelHide)
FDBAOnSkillWheelHide OnSkillWheelHideEvent;

// 技能方向更新事件
DECLARE_EVENT_OneParam(UDBAAndroidTouchInputBridge, FDBAOnSkillDirectionUpdate, FVector2D)
FDBAOnSkillDirectionUpdate OnSkillDirectionUpdateEvent;

// 技能释放事件
DECLARE_EVENT_OneParam(UDBAAndroidTouchInputBridge, FDBAOnSkillReleased, int32)
FDBAOnSkillReleased OnSkillReleasedEvent;
```

---

## 5. 竞技场 HUD WidgetController

### 5.1 ArenaHUDWidgetController

**文件**: `Public/UI/Arena/DBAArenaHUDWidgetController.h`

```cpp
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaHUDWidgetController : public UDBAWidgetController
{
    GENERATED_BODY()

public:
    // 更新玩家 HP
    UFUNCTION(BlueprintCallable)
    void UpdatePlayerHP(float CurrentHP, float MaxHP);

    // 更新玩家能量
    UFUNCTION(BlueprintCallable)
    void UpdatePlayerEnergy(float CurrentEnergy, float MaxEnergy);

    // 更新终极能量
    UFUNCTION(BlueprintCallable)
    void UpdateUltimateEnergy(float CurrentEnergy, float MaxEnergy);

public:
    // HP 变化事件
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerHPChanged, float, CurrentHP, float, MaxHP);
    UPROPERTY(BlueprintAssignable)
    FOnPlayerHPChanged OnPlayerHPChanged;

    // 能量变化事件
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
    UPROPERTY(BlueprintAssignable)
    FOnPlayerEnergyChanged OnPlayerEnergyChanged;

    // 终极能量变化事件
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUltimateEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
    UPROPERTY(BlueprintAssignable)
    FOnUltimateEnergyChanged OnUltimateEnergyChanged;

protected:
    UPROPERTY(BlueprintReadOnly)
    float CurrentHP;

    UPROPERTY(BlueprintReadOnly)
    float MaxHP;

    UPROPERTY(BlueprintReadOnly)
    float CurrentEnergy;

    UPROPERTY(BlueprintReadOnly)
    float MaxEnergy;
};
```

### 5.2 PlayerUnitFrameWidgetController

**文件**: `Public/UI/Arena/DBAPlayerUnitFrameWidgetController.h`

```cpp
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAPlayerUnitFrameWidgetController : public UDBAWidgetController
{
    GENERATED_BODY()

public:
    // 获取当前 HP
    UFUNCTION(BlueprintCallable)
    float GetCurrentHP() const;

    // 获取最大 HP
    UFUNCTION(BlueprintCallable)
    float GetMaxHP() const;

    // 获取当前能量
    UFUNCTION(BlueprintCallable)
    float GetCurrentEnergy() const;

    // 获取最大能量
    UFUNCTION(BlueprintCallable)
    float GetMaxEnergy() const;

    // 获取当前等级
    UFUNCTION(BlueprintCallable)
    int32 GetCurrentLevel() const;

public:
    // HP 更新事件
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPUpdated, float, CurrentHP, float, MaxHP);
    UPROPERTY(BlueprintAssignable)
    FOnHPUpdated OnHPUpdated;

    // 能量更新事件
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnergyUpdated, float, CurrentEnergy, float, MaxEnergy);
    UPROPERTY(BlueprintAssignable)
    FOnEnergyUpdated OnEnergyUpdated;

    // 等级更新事件
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpdated, int32, Level);
    UPROPERTY(BlueprintAssignable)
    FOnLevelUpdated OnLevelUpdated;
};
```

---

## 6. 竞技场 HUD 组件

### 6.1 HUD 根 Widget

**文件**: `Public/UI/Arena/DBAArenaHUDRootWidgetBase.h`

```cpp
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaHUDRootWidgetBase : public UDBAUserWidgetBase
{
    GENERATED_BODY()

public:
    // 设置 Widget Controller
    UFUNCTION(BlueprintCallable)
    void SetWidgetController(UDBAArenaHUDWidgetController* InController);

    // 设置 HUD 可见性
    UFUNCTION(BlueprintCallable)
    void SetHUDVisible(bool bVisible);

    // 设置 HUD 编辑模式
    UFUNCTION(BlueprintCallable)
    void SetHUDEditMode(bool bEditMode);

    // 应用五大阵营主题
    UFUNCTION(BlueprintCallable)
    void ApplyFiveCampTheme(uint8 FiveCamp);

protected:
    // 绑定的子 Widget
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UDBAPlayerUnitFrameWidgetBase> PlayerUnitFrame;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UDBAAbilityBarWidgetBase> AbilityBar;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UDBAPassiveAndResonancePanelWidgetBase> PassiveAndResonancePanel;

    // ... 其他 HUD 组件

    // Controller 引用
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UDBAArenaHUDWidgetController> WidgetController;
};
```

### 6.2 HUD 组件列表

| 组件 | 类名 | 说明 |
|------|------|------|
| 玩家信息框 | `UDBAPlayerUnitFrameWidgetBase` | 显示头像/HP/能量/等级 |
| 技能栏 | `UDBAAbilityBarWidgetBase` | 技能 1~4 + 大招 |
| 被动/共鸣面板 | `UDBAPassiveAndResonancePanelWidgetBase` | 被动技能/共鸣效果 |
| Buff 栏 | `UDBABuffBarWidgetBase` | Buff 显示 |
| Debuff 栏 | `UDBADebuffBarWidgetBase` | Debuff 显示 |
| 控制效果栏 | `UDBACCBarWidgetBase` | 眩晕/沉默等 |
| 自施法栏 | `UDBASelfCastBarWidgetBase` | 自我施法指示器 |
| 动能面板 | `UDBAMomentumPanelWidgetBase` | 连击数显示 |
| 连锁终结面板 | `UDBAChainUltimatePanelWidgetBase` | 连锁终结指示 |
| 战斗公告 | `UDBACombatAnnouncementWidgetBase` | 战斗文字提示 |
| 危险提示 | `UDBACriticalStateHintWidgetBase` | 低血量警告 |
| 能量就绪提示 | `UDBAUltimateReadyPromptWidgetBase` | 大招充能完成 |
| 连接警告 | `UDBAConnectionWarningWidgetBase` | 网络断开提示 |
| 目标追踪 | `UDBAArenaObjectiveTrackerWidgetBase` | 对线/野怪目标 |

---

## 7. 事件订阅示例

### 7.1 C++ 订阅输入事件

```cpp
// 在 HUD Widget 的 NativeConstruct 中
void UMyArenaHUD::NativeConstruct()
{
    Super::NativeConstruct();

    // 获取 InputRouter
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UDBAInputRouterComponent* InputRouter =
        PC->FindComponentByClass<UDBAInputRouterComponent>();

    if (InputRouter)
    {
        // 绑定 Ping 事件
        PingDelegateHandle = InputRouter->OnPingTriggeredEvent.AddUObject(
            this, &UMyArenaHUD::OnPingReceived);

        // 绑定计分板事件
        ScoreboardDelegateHandle = InputRouter->OnScoreboardTriggeredEvent.AddUObject(
            this, &UMyArenaHUD::OnScoreboardToggled);
    }
}

// 在 NativeDestruct 中解除绑定
void UMyArenaHUD::NativeDestruct()
{
    if (InputRouter)
    {
        InputRouter->OnPingTriggeredEvent.Remove(PingDelegateHandle);
        InputRouter->OnScoreboardTriggeredEvent.Remove(ScoreboardDelegateHandle);
    }

    Super::NativeDestruct();
}

// Ping 事件处理
void UMyArenaHUD::OnPingReceived(FVector PingLocation)
{
    // 在屏幕上显示 Ping 标记
    ShowPingMarker(PingLocation);
}

// 计分板事件处理
void UMyArenaHUD::OnScoreboardToggled()
{
    // 切换计分板可见性
    ToggleScoreboardWidget();
}
```

### 7.2 C++ 订阅 WidgetController 事件

```cpp
// 在 Widget 的 NativeConstruct 中
void UMyPlayerUnitFrame::NativeConstruct()
{
    Super::NativeConstruct();

    // 获取 Controller
    UDBAPlayerUnitFrameWidgetController* Controller =
        Cast<UDBAPlayerUnitFrameWidgetController>(GetWidgetController());

    if (Controller)
    {
        // 绑定 HP 变化事件
        HPChangedDelegateHandle = Controller->OnHPUpdated.AddUObject(
            this, &UMyPlayerUnitFrame::OnHPChanged);

        // 绑定能量变化事件
        EnergyChangedDelegateHandle = Controller->OnEnergyUpdated.AddUObject(
            this, &UMyPlayerUnitFrame::OnEnergyChanged);
    }
}

void UMyPlayerUnitFrame::NativeDestruct()
{
    if (Controller)
    {
        Controller->OnHPUpdated.Remove(HPChangedDelegateHandle);
        Controller->OnEnergyUpdated.Remove(EnergyChangedDelegateHandle);
    }

    Super::NativeDestruct();
}

void UMyPlayerUnitFrame::OnHPChanged(float CurrentHP, float MaxHP)
{
    // 更新 HP 条
    HPProgressBar->SetPercent(CurrentHP / MaxHP);
    HPText->SetText(FText::AsNumber(CurrentHP));
}
```

### 7.3 Blueprint 订阅事件

```
1. 创建 UDBAUserWidgetBase 子类 Widget
2. 在 Widget 的 Event Graph 中：
   - 获取 WidgetController 引用
   - 拖出 Controller 的 "On HP Updated" 事件
   - 连接到 Update HP Bar 函数
3. 事件会自动通过 BlueprintAssignable 属性暴露
```

---

## 8. 事件广播示例

### 8.1 WidgetController 广播事件

```cpp
void UDBAArenaHUDWidgetController::UpdatePlayerHP(float NewCurrentHP, float NewMaxHP)
{
    CurrentHP = NewCurrentHP;
    MaxHP = NewMaxHP;

    // 广播 HP 变化事件
    OnPlayerHPChanged.Broadcast(CurrentHP, MaxHP);
}
```

### 8.2 BlueprintAssignable vs BlueprintCallable

| 宏 | 用途 | 触发方式 |
|-----|------|----------|
| `BlueprintAssignable` | 事件源 | `Delegate.Broadcast(...)` |
| `BlueprintCallable` | 方法调用 | `FunctionName(...)` |

---

## 9. 最佳实践

### 9.1 生命周期管理

```
1. Widget NativeConstruct:
   - 获取 Controller 引用
   - 订阅 Controller 事件

2. Widget NativeDestruct:
   - 解除订阅 Controller 事件
   - 清理定时器/计时器

3. Widget Activate/Deactivate:
   - 显示/隐藏时暂停/恢复更新
```

### 9.2 事件解绑

**重要**: 在 Widget 销毁前必须解除所有事件绑定，否则可能导致崩溃。

```cpp
void UMyWidget::NativeDestruct()
{
    // 解除所有事件绑定
    if (Controller)
    {
        Controller->OnHPUpdated.Remove(HPHandle);
        Controller->OnEnergyUpdated.Remove(EnergyHandle);
    }

    Super::NativeDestruct();
}
```

### 9.3 多语言文本

使用 `FText` 而非 `FString`：

```cpp
// 推荐
UFUNCTION(BlueprintCallable)
void ShowMessage(const FText& Message);

// 避免
UFUNCTION(BlueprintCallable)
void ShowMessage(const FString& Message);
```

---

## 10. DECLARE_EVENT 宏说明

### 10.1 宏变体

```cpp
// 无参数事件
DECLARE_EVENT(OwnerClass, EventName)
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnScoreboardTriggered)

// 单参数事件
DECLARE_EVENT_OneParam(OwnerClass, EventName, ParamType)
DECLARE_EVENT_OneParam(UDBAInputRouterComponent, FDBAOnPingTriggered, FVector)

// 多参数事件
DECLARE_EVENT_TwoParams(OwnerClass, EventName, Param1Type, Param2Type)
DECLARE_EVENT_ThreeParams(...)

// 动态多播委托 (蓝图可用)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(...)
```

### 10.2 事件声明与调用

```cpp
// 声明 (头文件)
DECLARE_EVENT_OneParam(UDBAInputRouterComponent, FDBAOnPingTriggered, FVector)
FDBAOnPingTriggered OnPingTriggeredEvent;

// 调用 (cpp)
OnPingTriggeredEvent.Broadcast(PingLocation);

// 绑定 (C++)
OnPingTriggeredEvent.AddUObject(this, &MyClass::OnPingHandler);

// 绑定 (Blueprint)
// 直接拖拽 "On Ping Triggered Event" 到 Event Graph

// 解绑 (C++)
OnPingTriggeredEvent.Remove(Handle);
```

---

*文档版本: 1.0*
*生成时间: 2026-04-27*
*引擎版本: UE 5.7.1*
