# 导航系统需求规格

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.1.0
> 日期: 2026-05-05
> 状态: 待评审

---

## 1. 目标与范围

### 1.1 目标
实现完整的导航系统，为玩家角色和怪物AI提供可靠的路径导航能力。

### 1.2 范围
**包含:**
- 玩家角色 NavMesh 导航
- 怪物 AI PathFollowing 导航
- 移动速度配置
- 移动同步（服务端权威）

**不包含:**
- NavMesh 烘焙（运行时生成或手动烘焙）
- 复杂寻路算法（A*变体等）
- 跳跃/攀爬系统

---

## 2. 功能需求

### 2.1 NavMesh 配置

#### 2.1.1 引擎配置
```ini
[/Script/NavigationSystem.NavigationSystemV1]
bAutoCreateNavigationData=True
bAllowClientSideNavigation=False
DataGatheringMode=Instant
DefaultAgentRadius=34.0
DefaultAgentHeight=144.0

[/Script/AIModule.AISystem]
bEnableAISystem=True
PerceptionSystemUpdateInterval=0.1
```

#### 2.1.2 NavigationAgent 配置
```cpp
struct FDBA NavigationAgent
{
    float AgentRadius = 34.0f;
    float AgentHeight = 144.0f;
    float WalkableFloorZ = 0.7f;
};
```

### 2.2 玩家角色导航

#### 2.2.1 移动配置
| 属性 | 默认值 | 说明 |
|------|--------|------|
| MaxWalkSpeed | 600.0f | 最大行走速度 |
| MaxRunSpeed | 900.0f | 最大奔跑速度 |
| BrakingDeceleration | 2048.0f | 停止减速度 |

#### 2.2.2 ServerMoveTo 实现
```cpp
void ADBARpcHandler::ServerMoveTo_Implementation(FVector_NetQuantize10 Location)
{
    if (!HasAuthority()) return;

    ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
    if (!Character) return;

    // 使用 CharacterMovement 的 PathFollowing
    if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
    {
        Movement->SetMovementMode(MOVE_Walking);
        // 调用 MoveToLocation
    }
}
```

#### 2.2.3 客户端预测
- 客户端先本地预测移动
- 同时发送 ServerMoveTo RPC
- 服务端校正客户端位置

### 2.3 怪物AI导航

#### 2.3.1 路径跟随配置
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
float AcceptanceRadius = 50.0f;  // 到达目标判定半径

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
float NavmeshSamplingCellSize = 50.0f;
```

#### 2.3.2 DBAMonsterAIComponent 导航方法
```cpp
// 移动到目标位置
UFUNCTION(BlueprintCallable, Category = "AI|Movement")
void MoveToLocation(FVector Destination);

// 移动到目标Actor
UFUNCTION(BlueprintCallable, Category = "AI|Movement")
void MoveToActor(AActor* Target);

// 停止移动
UFUNCTION(BlueprintCallable, Category = "AI|Movement")
void StopMovement();

// 获取当前路径状态
UFUNCTION(BlueprintCallable, Category = "AI|Movement")
bool IsMoving() const;
```

#### 2.3.3 怪物移动速度
| 怪物类型 | 巡逻速度 | 追击速度 |
|----------|----------|----------|
| Slime | 150 | 300 |
| Ghost | 200 | 400 |
| Golem | 100 | 200 |
| Imp | 250 | 500 |
| Skeleton | 180 | 360 |

### 2.4 移动同步

#### 2.4.1 服务端权威模型
```
客户端输入 → 本地预测 → ServerMoveTo RPC → 服务端执行 → 位置校正
```

#### 2.4.2 客户端预测流程
```cpp
// 1. 客户端收到移动输入
void UDBAClientPredictionComponent::TryPredictMove(FVector TargetLocation)
{
    // 本地立即预测移动
    ADBAZodiacCharacterBase* Character = GetPawn<ADBAZodiacCharacterBase>();
    if (Character)
    {
        PredictedLocation = TargetLocation;
        Character->SetActorLocation(TargetLocation); // 本地预测

        // 发送RPC请求
        if (IDBARpcServer* RpcServer = GetRpcServer())
        {
            RpcServer->ServerMoveTo(TargetLocation);
        }
    }
}
```

#### 2.4.3 服务端校正流程
```cpp
// 服务端执行移动
void ADBARpcHandler::ServerMoveTo_Implementation(FVector_NetQuantize10 Location)
{
    // ... 执行移动 ...

    // 如果偏差过大，校正客户端
    float Distance = FVector::Dist(Character->GetActorLocation(), Location);
    if (Distance > 50.0f) // 阈值
    {
        // 发送校正RPC
        if (IDBARpcClient* ClientInterface = Cast<IDBARpcClient>(this))
        {
            ClientInterface->ClientMoveCorrection(Location, GetWorld()->GetTimeSeconds());
        }
    }
}
```

#### 2.4.4 客户端校正执行
```cpp
// 客户端收到校正
void ADBARpcHandler::ClientMoveCorrection_Implementation(FVector_NetQuantize10 ServerLocation, float ServerTime)
{
    // 平滑移动到服务端位置
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->Velocity = FVector::ZeroVector;
        Movement->SetMovementMode(MOVE_Walking);
    }

    // 平滑插值（延迟补偿）
    FVector StartLocation = GetActorLocation();
    float Duration = 0.1f; // 100ms
    // 使用 Timeline 或 Tick 平滑过渡到 StartLocation
}
```

#### 2.4.5 校正触发条件
| 条件 | 阈值 | 说明 |
|------|------|------|
| 位置偏差 | > 50 单位 | 触发校正 |
| 校正冷却 | 100ms | 避免频繁校正 |
| 速度异常 | > 2000 | 异常速度检测 |

---

## 3. 非功能需求

### 3.1 性能需求
- NavMesh 查询 < 1ms
- PathFollowing 更新 < 2ms 每帧
- 支持同时 100 个移动单位

### 3.2 网络同步
- 移动同步频率 10-30Hz
- 延迟容忍度 < 100ms
- 位置校正平滑过渡

---

## 4. 技术方案

### 4.1 目录结构

```
GameDBA/
├── Navigation/
│   ├── DBANavigationAgent.h/cpp        (导航代理配置)
│   └── DBANavMeshComponent.h/cpp      (导航网格组件)
├── Character/
│   ├── Zodiac/
│   │   └── DBAZodiacCharacterBase.h/cpp (添加移动配置)
│   └── Monster/
│       └── AI/
│           ├── DBAMonsterAIComponent.h/cpp (扩展导航方法)
│           └── DBAMonsterAIController.h/cpp (PathFollowing)
```

### 4.2 关键类设计

#### 4.2.1 FDBA NavigationAgent
```cpp
USTRUCT(BlueprintType)
struct FDBA NavigationAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AgentRadius = 34.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AgentHeight = 144.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WalkableFloorZ = 0.7f;
};
```

#### 4.2.2 DBAMonsterAIComponent 扩展
```cpp
// 导航相关
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
float AcceptanceRadius = 50.0f;

UFUNCTION(BlueprintCallable, Category = "AI|Movement")
void MoveToLocation(FVector Destination);

UFUNCTION(BlueprintCallable, Category = "AI|Movement")
void MoveToActor(AActor* Target);

UFUNCTION(BlueprintCallable, Category = "AI|Movement")
void StopMovement();

UFUNCTION(BlueprintCallable, Category = "AI|Movement")
bool IsMoving() const;

// 内部
void UpdatePathFollowing(float DeltaTime);
void OnPathComplete();
void OnPathFailed();
```

### 4.3 ServerMoveTo 实现

```cpp
void ADBARpcHandler::ServerMoveTo_Implementation(FVector_NetQuantize10 Location)
{
    if (!HasAuthority()) return;

    ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
    if (!Character) return;

    APawn* Pawn = Cast<APawn>(Character);
    if (!Pawn) return;

    // 使用 AIController 的 PathFollowingComponent
    if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
    {
        if (UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent())
        {
            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalLocation(Location);
            MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
            PathFollowing->RequestMove(MoveRequest);
        }
    }
    else
    {
        // 玩家角色使用 CharacterMovement
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
            if (NavSys)
            {
                FNavLocation NavLocation;
                if (NavSys->GetRandomPointInNavigableRadius(Location, 50.0f, NavLocation))
                {
                    Movement->RequestDirectMove(NavLocation.Location, true);
                }
            }
        }
    }
}
```

---

## 5. 用户故事

| ID | 场景 | 预期行为 |
|----|------|----------|
| US-01 | 玩家点击移动 | 角色移动到目标位置，NavMesh导航绕障 |
| US-02 | 怪物巡逻 | 怪物沿路径点移动，使用NavMesh |
| US-03 | 怪物追击 | 怪物发现目标后NavMesh路径追踪 |
| US-04 | 位置校正 | 客户端与服务端位置偏差时平滑校正 |
| US-05 | 停止移动 | 到达目标或手动停止时角色静止 |

---

## 6. 验收标准

### 6.1 功能验收
- [ ] 玩家角色可以点击移动到任意可达位置
- [ ] 怪物可以沿路径点巡逻
- [ ] 怪物可以追踪玩家
- [ ] 移动中障碍物可以被绕行
- [ ] 客户端与服务端位置同步
- [ ] 位置偏差超过阈值时平滑校正

### 6.2 性能验收
- [ ] NavMesh 查询 < 1ms
- [ ] PathFollowing 更新 < 2ms
- [ ] 100个单位同时移动无卡顿

### 6.3 网络验收
- [ ] 客户端本地预测移动响应及时
- [ ] 服务端校正平滑无抖动
- [ ] 移动RPC延迟 < 50ms

---

## 7. 开放问题

| 问题 | 说明 | 优先级 | 决策 |
|------|------|--------|------|
| Q-02 | 怪物是否需要奔跑/行走速度切换？ | 中 | 默认追击时使用ChaseSpeed，巡逻时使用PatrolSpeed |
| Q-03 | 移动是否有动画融合需求？ | 低 | 动画层处理，不在导航系统范围内 |

---

## 8. 参考文档

- `DefaultEngine.ini` - NavigationSystem 配置
- `DBAZodiacCharacterBase.h/cpp` - 玩家角色
- `DBAMonsterBase.h/cpp` - 怪物基类
- `DBAMonsterAIController.h/cpp` - AI控制器
- `DBARpcHandler.cpp` - ServerMoveTo 实现

---

*文档生成时间: 2026-05-05*