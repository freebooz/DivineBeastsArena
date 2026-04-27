# DivineBeastsArena 输入系统文档

> 详细说明输入路由组件与 Android 触屏输入桥接的架构设计、API 参考和使用指南。

---

## 1. 系统概览

### 1.1 架构图

```
┌─────────────────────────────────────────────────────────────────┐
│                      PlayerController                            │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │           UDBAInputRouterComponent                      │    │
│  │  ┌─────────────────┐  ┌────────────────────────────┐  │    │
│  │  │ InputMappingCtx │  │    EnhancedInputComponent    │  │    │
│  │  │  (Platform-based)│  │    BindAction() 绑定        │  │    │
│  │  └─────────────────┘  └────────────────────────────┘  │    │
│  │                                                           │    │
│  │  ┌─────────────────────────────────────────────────────┐│    │
│  │  │ UI 事件广播 (DECLARE_EVENT)                         ││    │
│  │  │  - OnPingTriggered     - OnScoreboardTriggered      ││    │
│  │  │  - OnMenuTriggered    - OnChatTriggered             ││    │
│  │  │  - OnMapTriggered     - OnLockTargetChanged         ││    │
│  │  └─────────────────────────────────────────────────────┘│    │
│  │                                                           │    │
│  │  ┌─────────────────────────────────────────────────────┐│    │
│  │  │ Server RPC (WithValidation)                         ││    │
│  │  │  - ServerPing           (频率限制 0.5s)             ││    │
│  │  │  - ServerLockTarget     (频率限制 0.3s)             ││    │
│  │  │  - ServerInteract       (频率限制 0.2s)             ││    │
│  │  └─────────────────────────────────────────────────────┘│    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │        UDBAAndroidTouchInputBridge (仅 Android)         │    │
│  │  ┌─────────────────────────────────────────────────┐  │    │
│  │  │ 技能轮盘事件                                       │  │    │
│  │  │  - OnSkillWheelShow    - OnSkillWheelHide        │  │    │
│  │  │  - OnSkillDirectionUpdate                       │  │    │
│  │  │  - OnSkillReleased                              │  │    │
│  │  └─────────────────────────────────────────────────┘  │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

### 1.2 组件职责

| 组件 | 平台 | 职责 |
|------|------|------|
| `UDBAInputRouterComponent` | PC / Android | 输入路由、动作绑定、RPC、UI 事件 |
| `UDBAAndroidTouchInputBridge` | Android Only | 技能轮盘、触屏手势 |

---

## 2. 输入路由组件 (DBAInputRouterComponent)

### 2.1 头文件

**文件路径**: `Public/Input/DBAInputRouterComponent.h`

**类声明**:
```cpp
UCLASS(ClassGroup = (Input), meta = (BlueprintSpawnableComponent, DisplayName = "输入路由组件"))
class DIVINEBEASTSARENA_API UDBAInputRouterComponent : public UActorComponent
```

### 2.2 公共 API

#### 初始化

```cpp
/**
 * 初始化输入路由
 * @param InputConfig 输入配置数据资产
 * @param PlatformPolicy 平台策略对象
 */
UFUNCTION(BlueprintCallable, Category = "Input Router")
void InitializeInputRouter(UDBAInputConfigDataAsset* InputConfig, UDBAInputPlatformPolicy* PlatformPolicy);
```

**使用示例**:
```cpp
// 在 PlayerController 或 Pawn 的 BeginPlay 中调用
UDBAInputRouterComponent* InputRouter = MyPC->FindComponentByClass<UDBAInputRouterComponent>();
if (InputRouter)
{
    InputRouter->InitializeInputRouter(InputConfig, PlatformPolicy);
}
```

#### 调试支持

```cpp
/**
 * 启用输入调试日志
 * @param bEnable 是否启用
 */
UFUNCTION(BlueprintCallable, Category = "Input Router|Debug")
void EnableInputDebugLog(bool bEnable);
```

---

## 3. 数据资产 (DBAInputConfigDataAsset)

### 3.1 输入动作集结构

```cpp
// 核心战斗输入动作集 (6个)
USTRUCT FDBACombatInputActionSet
{
    TObjectPtr<UInputAction> BasicAttack;  // 基础攻击
    TObjectPtr<UInputAction> Skill01;      // 技能1
    TObjectPtr<UInputAction> Skill02;      // 技能2
    TObjectPtr<UInputAction> Skill03;      // 技能3
    TObjectPtr<UInputAction> Skill04;      // 技能4
    TObjectPtr<UInputAction> Ultimate;     // 生肖大招
};

// 系统输入动作集 (7个)
USTRUCT FDBASystemInputActionSet
{
    TObjectPtr<UInputAction> LockTarget;   // 锁定目标
    TObjectPtr<UInputAction> Ping;         // Ping
    TObjectPtr<UInputAction> Scoreboard;  // 计分板
    TObjectPtr<UInputAction> Menu;        // 菜单
    TObjectPtr<UInputAction> Chat;         // 聊天
    TObjectPtr<UInputAction> Map;          // 地图
    TObjectPtr<UInputAction> Interact;     // 交互
};

// 移动输入动作集 (3个)
USTRUCT FDBAMovementInputActionSet
{
    TObjectPtr<UInputAction> Move;          // 移动 (2D向量)
    TObjectPtr<UInputAction> Look;          // 摄像机旋转 (2D向量)
    TObjectPtr<UInputAction> Jump;          // 跳跃
};
```

### 3.2 平台映射配置

```cpp
// 平台类型枚举
enum class EDBAInputPlatform : uint8
{
    PC,        // PC平台（键鼠）
    Android,  // Android平台（触屏）
    Universal // 通用平台
};

// 映射上下文配置
USTRUCT FDBAInputMappingContextConfig
{
    EDBAInputPlatform Platform;              // 平台类型
    TSoftObjectPtr<UInputMappingContext> MappingContext; // 映射上下文
    int32 Priority;                          // 优先级
};
```

### 3.3 配置示例 (Blueprint)

```
DBAInputConfigDataAsset
├── CombatActions
│   ├── BasicAttack → IA_BasicAttack
│   ├── Skill01 → IA_Skill01
│   ├── Skill02 → IA_Skill02
│   ├── Skill03 → IA_Skill03
│   ├── Skill04 → IA_Skill04
│   └── Ultimate → IA_Ultimate
├── SystemActions
│   ├── LockTarget → IA_LockTarget
│   ├── Ping → IA_Ping
│   ├── Scoreboard → IA_Scoreboard
│   ├── Menu → IA_Menu
│   ├── Chat → IA_Chat
│   ├── Map → IA_Map
│   └── Interact → IA_Interact
├── MovementActions
│   ├── Move → IA_Move
│   ├── Look → IA_Look
│   └── Jump → IA_Jump
└── MappingContexts
    ├── (PC, IMC_PC, Priority=0)
    └── (Android, IMC_Touch, Priority=0)
```

---

## 4. UI 事件广播

### 4.1 事件类型

| 事件 | 签名 | 说明 |
|------|------|------|
| `OnPingTriggered` | `FDBAOnPingTriggered(FVector)` | Ping 位置 |
| `OnScoreboardTriggered` | `FDBAOnScoreboardTriggered()` | 计分板切换 |
| `OnMenuTriggered` | `FDBAOnMenuTriggered()` | 菜单开关 |
| `OnChatTriggered` | `FDBAOnChatTriggered()` | 聊天打开 |
| `OnMapTriggered` | `FDBAOnMapTriggered()` | 地图开关 |
| `OnLockTargetChanged` | `FDBAOnLockTargetChanged(AActor*)` | 锁定目标变更 |

### 4.2 订阅示例 (C++)

```cpp
// 在 UI Widget 或 Controller 中
class UMyHUDWidget : public UUserWidget
{
    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();

        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            UDBAInputRouterComponent* InputRouter =
                PC->FindComponentByClass<UDBAInputRouterComponent>();

            if (InputRouter)
            {
                // 绑定 Ping 事件
                PingHandle = InputRouter->OnPingTriggeredEvent.AddUObject(
                    this, &UMyHUDWidget::OnPingReceived);

                // 绑定计分板事件
                ScoreboardHandle = InputRouter->OnScoreboardTriggeredEvent.AddUObject(
                    this, &UMyHUDWidget::OnScoreboardToggled);
            }
        }
    }

    virtual void NativeDestruct() override
    {
        // 解除绑定
        if (InputRouter)
        {
            InputRouter->OnPingTriggeredEvent.Remove(PingHandle);
            InputRouter->OnScoreboardTriggeredEvent.Remove(ScoreboardHandle);
        }

        Super::NativeDestruct();
    }

    void OnPingReceived(FVector PingLocation)
    {
        // 显示 Ping 标记
    }

    void OnScoreboardToggled()
    {
        // 切换计分板显示
    }
};
```

---

## 5. 网络安全 (RPC)

### 5.1 RPC 列表

| RPC | 频率限制 | 验证内容 |
|-----|----------|----------|
| `ServerPing` | 0.5s | Ping 位置距离检查 (≤50000) |
| `ServerLockTarget` | 0.3s | 目标距离检查 (≤3000) |
| `ServerInteract` | 0.2s | 交互对象距离检查 (≤500) |

### 5.2 RPC 签名

```cpp
/**
 * 发送 Ping 到服务端（带验证）
 * @param Location Ping 位置（世界坐标）
 */
UFUNCTION(Server, Reliable, WithValidation)
void ServerPing(FVector_NetQuantize10 Location);

/**
 * 锁定目标切换请求（带验证）
 * @param TargetActor 目标 Actor（nullptr 表示解除锁定）
 */
UFUNCTION(Server, Reliable, WithValidation)
void ServerLockTarget(AActor* TargetActor);

/**
 * 交互请求（带验证）
 * @param InteractableActor 可交互对象
 */
UFUNCTION(Server, Reliable, WithValidation)
void ServerInteract(AActor* InteractableActor);
```

### 5.3 验证实现

```cpp
// ServerPing 验证
bool UDBAInputRouterComponent::ServerPing_Validate(FVector_NetQuantize10 Location)
{
    const float MaxPingDistance = 50000.0f; // 500米
    FVector OwnerLocation = GetPawnLocation();
    return FVector::Dist(OwnerLocation, Location) <= MaxPingDistance;
}

// ServerLockTarget 验证
bool UDBAInputRouterComponent::ServerLockTarget_Validate(AActor* TargetActor)
{
    if (!TargetActor) return true; // nullptr 允许

    const float MaxLockTargetDistance = 3000.0f; // 30米
    return FVector::Dist(GetPawnLocation(), TargetActor->GetActorLocation())
        <= MaxLockTargetDistance;
}

// ServerInteract 验证
bool UDBAInputRouterComponent::ServerInteract_Validate(AActor* InteractableActor)
{
    if (!InteractableActor) return false; // 交互对象不能为空

    const float MaxInteractDistance = 500.0f; // 5米
    return FVector::Dist(GetPawnLocation(), InteractableActor->GetActorLocation())
        <= MaxInteractDistance;
}
```

---

## 6. Android 触屏输入桥接

### 6.1 头文件

**文件路径**: `Public/Input/DBAAndroidTouchInputBridge.h`

**类声明**:
```cpp
UCLASS(ClassGroup = (Input), meta = (BlueprintSpawnableComponent, DisplayName = "Android 触屏输入桥接"))
class DIVINEBEASTSARENA_API UDBAAndroidTouchInputBridge : public UActorComponent
```

### 6.2 技能轮盘事件

| 事件 | 签名 | 说明 |
|------|------|------|
| `OnSkillWheelShow` | `FDBAOnSkillWheelShow(int32)` | 显示技能轮盘 |
| `OnSkillWheelHide` | `FDBAOnSkillWheelHide()` | 隐藏技能轮盘 |
| `OnSkillDirectionUpdate` | `FDBAOnSkillDirectionUpdate(FVector2D)` | 更新技能方向 |
| `OnSkillReleased` | `FDBAOnSkillReleased(int32)` | 技能释放 |

### 6.3 公共 API

```cpp
/**
 * 处理技能按钮长按开始
 * @param SkillIndex 技能索引（0=BasicAttack, 1~4=Skill01~04, 5=Ultimate）
 * @param TouchLocation 触摸屏幕位置
 */
UFUNCTION(BlueprintCallable, Category = "Android Touch Input")
void OnSkillButtonLongPressStart(int32 SkillIndex, FVector2D TouchLocation);

/**
 * 处理技能按钮拖拽
 * @param SkillIndex 技能索引
 * @param DragDelta 拖拽增量
 */
UFUNCTION(BlueprintCallable, Category = "Android Touch Input")
void OnSkillButtonDrag(int32 SkillIndex, FVector2D DragDelta);

/**
 * 处理技能按钮松手释放
 * @param SkillIndex 技能索引
 */
UFUNCTION(BlueprintCallable, Category = "Android Touch Input")
void OnSkillButtonRelease(int32 SkillIndex);

/**
 * 更新 Ultimate 按钮状态
 * @param UltimateEnergy 当前终极能量值（0~100）
 * @param bIsReady 是否可释放
 */
UFUNCTION(BlueprintCallable, Category = "Android Touch Input")
void UpdateUltimateButtonState(float UltimateEnergy, bool bIsReady);
```

### 6.4 技能轮盘交互流程

```
用户长按技能按钮
    ↓
OnSkillButtonLongPressStart(SkillIndex, TouchLocation)
    ↓
┌─────────────────────────────────────────┐
│ 1. 记录长按状态                          │
│ 2. 广播 OnSkillWheelShowEvent           │
│    (UI 显示技能轮盘)                      │
└─────────────────────────────────────────┘
    ↓
用户拖拽手指选择方向
    ↓
OnSkillButtonDrag(SkillIndex, DragDelta)
    ↓
┌─────────────────────────────────────────┐
│ 1. 计算技能方向                           │
│ 2. 广播 OnSkillDirectionUpdateEvent      │
│    (UI 更新方向指示器)                     │
└─────────────────────────────────────────┘
    ↓
用户松手
    ↓
OnSkillButtonRelease(SkillIndex)
    ↓
┌─────────────────────────────────────────┐
│ 1. 广播 OnSkillReleasedEvent             │
│    (GAS 层触发技能释放)                    │
│ 2. 广播 OnSkillWheelHideEvent            │
│    (UI 隐藏技能轮盘)                       │
└─────────────────────────────────────────┘
```

---

## 7. 平台策略 (DBAInputPlatformPolicy)

### 7.1 接口定义

```cpp
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAInputPlatformPolicy : public UObject
{
    /**
     * 获取当前平台类型
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    EDBAInputPlatform GetCurrentPlatform() const;

    /**
     * 判断是否为触屏平台
     */
    UFUNCTION(BlueprintCallable)
    bool IsTouchPlatform() const;
};
```

### 7.2 实现示例

```cpp
UCLASS()
class UDBAAndroidPlatformPolicy : public UDBAInputPlatformPolicy
{
    virtual EDBAInputPlatform GetCurrentPlatform_Implementation() const override
    {
#if PLATFORM_ANDROID
        return EDBAInputPlatform::Android;
#else
        return EDBAInputPlatform::PC;
#endif
    }

    virtual bool IsTouchPlatform() const override
    {
        return GetCurrentPlatform() == EDBAInputPlatform::Android;
    }
};
```

---

## 8. 输入绑定流程

### 8.1 初始化流程

```cpp
void UDBAInputRouterComponent::InitializeInputRouter(
    UDBAInputConfigDataAsset* InputConfig,
    UDBAInputPlatformPolicy* PlatformPolicy)
{
    if (!InputConfig || !PlatformPolicy) return;

    InputConfigAsset = InputConfig;
    PlatformPolicyObject = PlatformPolicy;

    // 1. 加载并应用映射上下文
    LoadAndApplyMappingContexts();

    // 2. 绑定输入动作
    BindInputActions();
}
```

### 8.2 映射上下文应用

```cpp
void UDBAInputRouterComponent::LoadAndApplyMappingContexts()
{
    EDBAInputPlatform CurrentPlatform = PlatformPolicyObject->GetCurrentPlatform();

    // 获取平台对应的配置
    TArray<FDBAInputMappingContextConfig> PlatformConfigs;
    InputConfigAsset->GetMappingContextsForPlatform(CurrentPlatform, PlatformConfigs);

    // 清空现有映射
    Subsystem->ClearAllMappings();

    // 应用新映射
    for (const FDBAInputMappingContextConfig& Config : PlatformConfigs)
    {
        if (UInputMappingContext* MappingContext = Config.MappingContext.LoadSynchronous())
        {
            Subsystem->AddMappingContext(MappingContext, Config.Priority);
        }
    }
}
```

### 8.3 动作绑定

```cpp
void UDBAInputRouterComponent::BindInputActions()
{
    UEnhancedInputComponent* EnhancedInputComponent = ...;

    // 战斗输入
    EnhancedInputComponent->BindAction(
        CombatActions.BasicAttack,
        ETriggerEvent::Triggered,
        this,
        &UDBAInputRouterComponent::OnBasicAttackTriggered);

    EnhancedInputComponent->BindAction(
        CombatActions.Skill01,
        ETriggerEvent::Triggered,
        this,
        &UDBAInputRouterComponent::OnSkill01Triggered);
    // ... 其他技能

    // 系统输入
    EnhancedInputComponent->BindAction(
        SystemActions.Ping,
        ETriggerEvent::Triggered,
        this,
        &UDBAInputRouterComponent::OnPingTriggered);
    // ... 其他系统输入

    // 移动输入
    EnhancedInputComponent->BindAction(
        MovementActions.Move,
        ETriggerEvent::Triggered,
        this,
        &UDBAInputRouterComponent::OnMoveTriggered);

    EnhancedInputComponent->BindAction(
        MovementActions.Look,
        ETriggerEvent::Triggered,
        this,
        &UDBAInputRouterComponent::OnLookTriggered);

    EnhancedInputComponent->BindAction(
        MovementActions.Jump,
        ETriggerEvent::Triggered,
        this,
        &UDBAInputRouterComponent::OnJumpTriggered);
}
```

---

## 9. 调试支持

### 9.1 调试日志

```cpp
void UDBAInputRouterComponent::LogInputDebug(const FString& Message) const
{
    if (bInputDebugLogEnabled)
    {
        UE_LOG(LogDBAUI, Log, TEXT("[DBAInputRouter] %s"), *Message);
    }
}
```

### 9.2 典型日志输出

```
[DBAInputRouter] 输入路由初始化完成
[DBAInputRouter] 当前平台：0 (PC)
[DBAInputRouter] 已加载映射上下文：IMC_PC，优先级：0
[DBAInputRouter] 输入动作绑定完成
[DBAInputRouter] Ping 输入触发
[DBAInputRouter] Ping 触发: X=100.00, Y=200.00, Z=0.00
[DBAInputRouter] 锁定目标输入触发
[DBAInputRouter] 锁定目标切换
```

---

## 10. 注意事项

### 10.1 Dedicated Server

组件在 Dedicated Server 上会自动禁用：

```cpp
#if UE_SERVER
    bAutoActivate = false;
#endif
```

### 10.2 Android 平台检测

```cpp
#if PLATFORM_ANDROID
    bAutoActivate = true;
#else
    bAutoActivate = false;
#endif
```

### 10.3 频率限制时间戳更新

频率限制时间戳在**发送 RPC 后**更新，而非发送前检查：

```cpp
void UDBAInputRouterComponent::OnPingTriggered(const FInputActionValue& Value)
{
    if (!CanSendPingRPC()) return; // 检查

    ServerPing(PingLocation);
    UpdatePingTimestamp(); // 更新（在 RPC 发送后）
}
```

---

*文档版本: 1.0*
*生成时间: 2026-04-27*
*引擎版本: UE 5.7.1*
