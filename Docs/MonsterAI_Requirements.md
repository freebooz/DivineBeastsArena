# 怪物AI系统需求规格

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.1.0
> 日期: 2026-05-05
> 状态: 待评审

---

## 1. 目标与范围

### 1.1 目标
为游戏中的怪物实现完整的AI行为系统，包括：巡逻、发现敌人、追击、攻击、返回等状态。

### 1.2 范围
**包含:**
- 怪物AI控制器 (ADBAMonsterAIController)
- C++状态机基础框架
- BehaviorTree决策系统
- 怪物攻击系统
- 仇恨/目标系统

**不包含:**
- NavMesh导航路径规划（下一版本）
- 怪物技能GAS绑定（下一版本）
- 怪物外观/动画（本版本只做逻辑）

---

## 2. 功能需求

### 2.1 AI行为状态机

| 状态 | 说明 | 入口条件 | 出口条件 |
|------|------|----------|----------|
| **Idle** | 原地待机 | 出生/战斗结束 | 发现敌人/超时 |
| **Patrol** | 沿路径巡逻 | Idle超时/战斗结束 | 发现敌人/到达终点 |
| **Chase** | 追击敌人 | 发现敌人 | 进入攻击范围/丢失目标 |
| **Attack** | 普通攻击 | 进入攻击范围 | 目标死亡/离开范围 |
| **Return** | 返回出生点 | 目标丢失/超出追击范围 | 到达出生点 |

### 2.2 状态转换流程

```
     ┌─────────┐
     │  Spawn  │
     └────┬────┘
          ↓
     ┌─────────┐     超时      ┌─────────┐
     │  Idle   │─────────────→│ Patrol  │
     └────┬─────┘              └────┬────┘
          │发现敌人                │到达终点
          ↓                        ↓
     ┌─────────┐              ┌─────────┐
     │  Chase  │←─────────────│  循环   │
     └────┬─────┘              └─────────┘
          │进入攻击范围
          ↓
     ┌─────────┐     目标死亡/离开
     │ Attack  │────────────────→ Return → Idle
     └─────────┘
```

### 2.3 核心功能

#### 2.3.1 巡逻系统
- 巡逻路径点存储在 `TArray<FVector> PatrolPoints`
- 沿路径点顺序移动，到达终点后循环
- 巡逻速度可配置（默认 200）
- 巡逻状态下每0.5秒检测范围内敌人

#### 2.3.2 敌人检测
- 检测范围可配置（默认 500）
- 通过 `UAbilitySystemComponent` 查找最近敌方角色
- 检测视线（可选，需LineOfSightTo）

#### 2.3.3 追击系统
- 追击速度可配置（默认 400）
- 最大追击距离可配置（默认 1000），超出后返回
- 追击时保持目标跟踪

#### 2.3.4 攻击系统
- 攻击范围可配置（默认 150）
- 攻击间隔可配置（默认 1.5秒）
- 攻击伤害通过 `UDBADamageCalculator` 计算
- 攻击时播放动画 `PlayAttackAnimation()`

#### 2.3.5 仇恨系统
- 记录当前攻击目标
- 目标死亡后切换最近敌人或返回

---

## 3. 非功能需求

### 3.1 性能需求
- AI Tick频率：每秒10次（0.1秒间隔）
- 同时支持100个怪物AI实例

### 3.2 网络同步
- AI状态复制到客户端用于表现
- 攻击判定仅在服务端执行

### 3.3 可配置性
- 所有AI参数可通过 Blueprint 调整
- 支持不同怪物类型不同AI配置

---

## 4. 技术方案

### 4.1 混合模式架构

```
C++ 层 (状态机基础)
├── ADBAMonsterAIController (AI控制器)
├── UDBAMonsterAIComponent (AI状态机组件)
│   ├── EMonsterAIState 枚举
│   ├── 状态转换逻辑
│   └── 属性配置
│
└── UDBAMonsterContextComponent (上下文组件)
    ├── TargetActor
    ├── PatrolIndex
    └── LastSeenTime

BehaviorTree 层 (决策)
├── DBAMonster_BT (行为树资源)
├── BB_DBA_Monster (黑板数据)
└── 条件检查Task节点

共享接口
├── IDBARpcInterface
└── DBADamageCalculator
```

### 4.2 关键类设计

#### ADBAMonsterAIController
```cpp
UCLASS()
class DIVINEBEASTSARENA_API ADBAMonsterAIController : public AAIController
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, Category = "AI")
    UBlackboardData* BlackboardAsset;

protected:
    UPROPERTY()
    UDBAMonsterAIComponent* AIComponent;
};
```

#### UDBAMonsterAIComponent
```cpp
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API UDBAMonsterAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // ===== 状态 =====
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI State")
    EMonsterAIState CurrentState = EMonsterAIState::Idle;

    // ===== 配置 =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float DetectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float ChaseMaxDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float ChaseSpeed = 400.0f;

    // ===== 巡逻路径配置 =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Patrol")
    TArray<FVector> PatrolPoints_CPP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Patrol")
    TSubclassOf<AActor> PatrolPointActorClass;

    // ===== 仇恨系统 =====
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI Aggro")
    TArray<FAggroInfo> AggroList;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI Aggro")
    AActor* CurrentTarget;

public:
    // 状态转换
    UFUNCTION(BlueprintCallable, Category = "AI")
    void TransitionTo(EMonsterAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void FindTarget();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void AttackTarget();

    // 仇恨管理
    UFUNCTION(BlueprintCallable, Category = "AI|Aggro")
    void AddAggro(AActor* Target, float Amount);

    UFUNCTION(BlueprintCallable, Category = "AI|Aggro")
    void RemoveAggro(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI|Aggro")
    AActor* GetTopAggroTarget();

protected:
    // 内部方法
    void UpdateAggroList();
    FVector GetPatrolPoint(int32 Index);
    bool HasLineOfSightTo(FVector Target);
};

// 仇恨信息结构
USTRUCT(BlueprintType)
struct FAggroInfo
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<AActor> Target;

    UPROPERTY()
    float Threat = 0.0f;

    UPROPERTY()
    float LastThreatTime = 0.0f;
};
```

### 4.3 仇恨系统设计

#### 仇恨机制
- 每个目标有 Threat 值，初始100点
- 造成伤害增加 Threat，被攻击也增加 Threat
- 每5秒自然衰减10%
- 超过30秒无互动则移除记录
- 当前目标 = Threat 最高者

#### 仇恨行为
| 行为 | Threat变化 | 说明 |
|------|------------|------|
| 首次攻击 | +100 | 建立仇恨 |
| 造成伤害 | +伤害值×0.5 | 仇恨奖励 |
| 受到伤害 | +50 | 反向仇恨 |
| 目标死亡 | -100% | 移除记录 |
| 长时间无互动 | -10%/5秒 | 自然衰减 |

### 4.4 巡逻路径配置

#### 代码配置 (PatrolPoints_CPP)
```cpp
// 在子类或蓝图实例中配置
UPROPERTY(EditDefaultsOnly, Category = "AI Patrol")
TArray<FVector> PatrolPoints_CPP = {
    FVector(0, 0, 0),
    FVector(500, 0, 0),
    FVector(500, 500, 0),
    FVector(0, 500, 0)
};
```

#### 编辑器放置 (PatrolPointActorClass)
1. 创建路径点Actor子类 `DBAPatrolPointActor`
2. 在地图上放置多个路径点，按顺序编号
3. 怪物AI组件引用 `PatrolPointActorClass`
4. AI自动查找场景中所有该类型Actor并按距离排序

### 4.3 目录结构

```
GameDBA/Character/Monster/
├── AI/
│   ├── DBAMonsterAIController.h/cpp
│   ├── DBAMonsterAIComponent.h/cpp
│   ├── DBAMonsterContextComponent.h/cpp
│   └── EMonsterAIState.h (枚举)
├── BehaviorTree/
│   ├── DBAMonster_BT.uasset
│   └── BB_DBA_Monster.uasset
├── DBAMonsterBase.h/cpp
├── DBAMonster_Slime.h/cpp
├── DBAMonster_Ghost.h/cpp
├── DBAMonster_Golem.h/cpp
├── DBAMonster_Imp.h/cpp
└── DBAMonster_Skeleton.h/cpp
```

### 4.4 仇恨系统设计

#### 仇恨机制
- 每个目标有 Threat 值，初始100点
- 造成伤害增加 Threat，被攻击也增加 Threat
- 每5秒自然衰减10%
- 超过30秒无互动则移除记录
- 当前目标 = Threat 最高者

#### 仇恨行为
| 行为 | Threat变化 | 说明 |
|------|------------|------|
| 首次攻击 | +100 | 建立仇恨 |
| 造成伤害 | +伤害值×0.5 | 仇恨奖励 |
| 受到伤害 | +50 | 反向仇恨 |
| 目标死亡 | -100% | 移除记录 |
| 长时间无互动 | -10%/5秒 | 自然衰减 |

### 4.5 巡逻路径配置

#### 代码配置 (PatrolPoints_CPP)

---

## 5. 用户故事

| ID | 场景 | 预期行为 |
|----|------|----------|
| US-01 | 怪物出生 | 进入Idle状态，3秒后开始巡逻 |
| US-02 | 巡逻中 | 沿路径点移动，检测到敌人后切换Chase |
| US-03 | 追击敌人 | 朝敌人移动，进入攻击范围后Attack |
| US-04 | 攻击敌人 | 周期性造成伤害，播放攻击动画 |
| US-05 | 目标死亡 | 切换最近敌人或返回出生点 |
| US-06 | 目标丢失 | 追击超过最大距离，返回出生点 |
| US-07 | 返回出生点 | 回到原点后进入Idle状态 |

---

## 6. 验收标准

### 6.1 功能验收
- [ ] 怪物可以在地图上巡逻移动
- [ ] 怪物可以发现并追击玩家角色
- [ ] 怪物可以攻击玩家造成伤害
- [ ] 怪物目标死亡后可以切换目标或返回
- [ ] 怪物超出追击范围可以返回出生点
- [ ] 怪物可以记录多个目标的仇恨值
- [ ] 怪物可以基于仇恨值选择攻击目标
- [ ] 仇恨值会随时间和行为动态变化

### 6.2 性能验收
- [ ] 100个怪物同时AI运算无明显卡顿
- [ ] AI Tick不超过0.5ms/实例

### 6.3 网络验收
- [ ] 客户端可以显示怪物AI状态
- [ ] 攻击判定在服务端执行

---

## 7. 开放问题

| 问题 | 说明 | 优先级 |
|------|------|--------|
| Q-03 | 怪物死亡后如何处理AI状态？ | 低 |

---

## 8. 参考文档

- `CharacterSystem_Architecture.md` - 角色系统架构
- `DBAMonsterBase.h/cpp` - 怪物基类
- `DBADamageCalculator.h/cpp` - 伤害计算器

---

*文档生成时间: 2026-05-05*