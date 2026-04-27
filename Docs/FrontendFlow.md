# DivineBeastsArena 前台流程文档

> 大厅/匹配队列/英雄选择流程子系统详细说明。

---

## 1. 系统概览

### 1.1 前台流程架构图

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           前台流程系统                                            │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────┐    │
│  │                    UDBAMatchEntryCoordinator                               │    │
│  │                    (匹配入口协调器)                                         │    │
│  │  - 管理整个匹配流程状态机                                                 │    │
│  │  - 按顺序调用各个子系统                                                  │    │
│  └─────────────────────────────────────────────────────────────────────────┘    │
│                                      │                                            │
│         ┌────────────────────────────┼────────────────────────────┐             │
│         ▼                            ▼                            ▼             │
│  ┌─────────────┐           ┌─────────────┐           ┌─────────────┐        │
│  │UDBAQueueSubsystem│     │UDBAHeroSelectSubsystem│  │UDBAReadyCheckSubsystem│  │
│  │(排队子系统)    │        │(英雄选择子系统)       │  │(准备确认子系统)       │  │
│  └─────────────┘           └─────────────┘           └─────────────┘        │
│                                      │                            │             │
│         ┌────────────────────────────┼────────────────────────────┘             │
│         ▼                            ▼                                          │
│  ┌─────────────┐           ┌─────────────┐                                    │
│  │UDBAElementSelectSubsystem│ │UDBAFiveCampSelectSubsystem│                   │
│  │(元素选择子系统)           │  │(阵营选择子系统)           │                   │
│  └─────────────┘           └─────────────┘                                    │
│                                                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────┐    │
│  │                         UDBALobbySubsystem                                 │    │
│  │                         (大厅子系统)                                       │    │
│  │  - 管理大厅加载/卸载                                                     │    │
│  │  - 大厅状态追踪                                                          │    │
│  └─────────────────────────────────────────────────────────────────────────┘    │
│                                                                                  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. 匹配入口协调器 (MatchEntryCoordinator)

### 2.1 流程阶段

| 阶段 | 枚举值 | 说明 |
|------|--------|------|
| 无 | `None` | 初始状态 |
| 排队中 | `Queue` | 玩家正在匹配 |
| 匹配成功 | `MatchFound` | 找到对局 |
| 准备确认 | `ReadyCheck` | 确认准备状态 |
| 英雄选择 | `HeroSelect` | 选择生肖英雄 |
| 元素选择 | `ElementSelect` | 选择元素之力 |
| 阵营选择 | `FiveCampSelect` | 选择五大阵营 |
| 加载中 | `Loading` | 地图资源加载 |
| 比赛开始 | `MatchStart` | 进入对战 |

### 2.2 流程图

```
玩家点击 "开始匹配"
    ↓
MatchEntryCoordinator::StartMatchEntry
    ↓
┌─────────────────────────────────────────┐
│ Phase: Queue                             │
│ 调用 UDBAQueueSubsystem::JoinQueue        │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Phase: MatchFound                       │
│ 匹配成功回调 OnMatchFound               │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Phase: ReadyCheck                       │
│ 调用 UDBAReadyCheckSubsystem             │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Phase: HeroSelect                       │
│ 调用 UDBAHeroSelectSubsystem             │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Phase: ElementSelect                    │
│ 调用 UDBAElementSelectSubsystem          │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Phase: FiveCampSelect                  │
│ 调用 UDBAFiveCampSelectSubsystem         │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Phase: Loading                          │
│ 调用 UDALoadingPreparationSubsystem     │
└─────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────┐
│ Phase: MatchStart                       │
│ 广播 OnMatchEntryCompleted               │
│ 切换到对战地图                           │
└─────────────────────────────────────────┘
```

### 2.3 核心 API

```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAMatchEntryCoordinator : public UGameInstanceSubsystem
{
    // 开始匹配流程
    UFUNCTION(BlueprintCallable)
    void StartMatchEntry(const FString& MatchId);

    // 取消匹配
    UFUNCTION(BlueprintCallable)
    void CancelMatchEntry();

    // 各阶段完成回调
    UFUNCTION(BlueprintCallable)
    void OnMatchFound(const FDBAMatchFoundInfo& MatchInfo);

    UFUNCTION(BlueprintCallable)
    void OnReadyCheckCompleted(bool bSuccess);

    UFUNCTION(BlueprintCallable)
    void OnHeroSelectCompleted(FName HeroId);

    UFUNCTION(BlueprintCallable)
    void OnElementSelectCompleted(EDBAElement Element);

    UFUNCTION(BlueprintCallable)
    void OnFiveCampSelectCompleted(EDBAFiveCamp FiveCamp);

    UFUNCTION(BlueprintCallable)
    void OnLoadingCompleted();

    // 查询当前状态
    UFUNCTION(BlueprintPure)
    EDBAMatchEntryPhase GetCurrentPhase() const;

    UFUNCTION(BlueprintPure)
    const FString& GetCurrentMatchId() const;

public:
    // 事件广播
    UPROPERTY(BlueprintAssignable)
    FDBAOnMatchEntryPhaseChanged OnMatchEntryPhaseChanged;

    UPROPERTY(BlueprintAssignable)
    FDBAOnMatchEntryFailed OnMatchEntryFailed;

    UPROPERTY(BlueprintAssignable)
    FDBAOnMatchEntryCompleted OnMatchEntryCompleted;
};
```

---

## 3. 排队子系统 (Queue)

### 3.1 队列模式

| 模式 | 枚举值 | 说明 |
|------|--------|------|
| 快速匹配 | `QuickMatch` | 随机对手 |
| 排位赛 | `Ranked` | 排位积分对战 |
| 自定义房间 | `Custom` | 房间密码进入 |

### 3.2 队列状态

| 状态 | 枚举值 | 说明 |
|------|--------|------|
| 未在队列中 | `NotInQueue` | 初始状态 |
| 正在排队 | `Queueing` | 匹配中 |
| 找到匹配 | `MatchFound` | 匹配成功 |
| 队列已取消 | `Cancelled` | 玩家取消 |
| 队列超时 | `Timeout` | 超时未匹配 |

### 3.3 核心 API

```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAQueueSubsystem : public UGameInstanceSubsystem
{
    // 加入队列
    UFUNCTION(BlueprintCallable)
    void JoinQueue(EDBAQueueSubsystemMode QueueMode);

    // 离开队列
    UFUNCTION(BlueprintCallable)
    void LeaveQueue();

    // 查询状态
    UFUNCTION(BlueprintPure)
    bool IsInQueue() const;

    UFUNCTION(BlueprintPure)
    FDBAQueueSubsystemInfo GetCurrentQueueInfo() const;

    UFUNCTION(BlueprintPure)
    EDBAQueueSubsystemState GetQueueState() const;

public:
    // 事件
    UPROPERTY(BlueprintAssignable)
    FDBAOnQueueStateChanged OnQueueStateChanged;

    UPROPERTY(BlueprintAssignable)
    FDBAOnMatchFound OnMatchFound;

    UPROPERTY(BlueprintAssignable)
    FDBAOnQueueCancelled OnQueueCancelled;
};
```

### 3.4 排队信息结构

```cpp
USTRUCT(BlueprintType)
struct FDBAQueueInfo
{
    FString QueueId;                    // 队列ID
    EDBAQueueMode QueueMode;           // 队列模式
    EDBAQueueState QueueState;         // 队列状态
    FDateTime QueueStartTime;          // 开始时间
    float EstimatedWaitTime;           // 预计等待
    float CurrentWaitTime;             // 当前等待
    bool bIsPartyQueue;               // 是否组队排队
    int32 PartyMemberCount;           // 队伍人数
};
```

---

## 4. 英雄选择子系统 (HeroSelect)

### 4.1 选择状态

| 状态 | 枚举值 | 说明 |
|------|--------|------|
| 未开始 | `None` | 初始状态 |
| 选择中 | `Selecting` | 正在选择 |
| 已确认 | `Confirmed` | 已确认选择 |
| 已锁定 | `LockedIn` | 等待确认 |

### 4.2 核心 API

```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAHeroSelectSubsystem : public UGameInstanceSubsystem
{
    // 开始英雄选择
    UFUNCTION(BlueprintCallable)
    void StartHeroSelect(float SelectTimeSeconds);

    // 选择英雄
    UFUNCTION(BlueprintCallable)
    void SelectHero(FName HeroId);

    // 确认选择
    UFUNCTION(BlueprintCallable)
    void ConfirmHeroSelection();

    // 取消选择
    UFUNCTION(BlueprintCallable)
    void CancelSelection();

    // 查询
    UFUNCTION(BlueprintPure)
    bool IsInHeroSelect() const;

    UFUNCTION(BlueprintPure)
    FDBAHeroSelectInfo GetCurrentSelectInfo() const;

    UFUNCTION(BlueprintPure)
    FName GetSelectedHeroId() const;

    UFUNCTION(BlueprintPure)
    bool HasConfirmed() const;

public:
    // 事件
    UPROPERTY(BlueprintAssignable)
    FDBAOnHeroSelectStateChanged OnHeroSelectStateChanged;

    UPROPERTY(BlueprintAssignable)
    FDBAOnHeroSelected OnHeroSelected;

    UPROPERTY(BlueprintAssignable)
    FDBAOnHeroConfirmChanged OnHeroConfirmChanged;

    UPROPERTY(BlueprintAssignable)
    FDBAOnHeroSelectTimeUpdate OnHeroSelectTimeUpdate;
};
```

### 4.3 选择信息结构

```cpp
USTRUCT(BlueprintType)
struct FDBAHeroSelectInfo
{
    FString SelectSessionId;           // 选择会话ID
    EDBAHeroSelectState State;        // 选择状态
    TArray<FName> AvailableHeroIds;   // 可选英雄列表
    FName SelectedHeroId;             // 已选英雄ID
    bool bHasConfirmed;               // 是否已确认
    float RemainingTime;               // 剩余时间
    float TotalTime;                  // 总时间
};
```

---

## 5. 元素选择子系统 (ElementSelect)

### 5.1 元素类型

基于五行相生相克：

| 元素 | 枚举值 | 说明 |
|------|--------|------|
| 无 | `None` | 未选择 |
| 金 | `Metal` | 金元素 |
| 木 | `Wood` | 木元素 |
| 水 | `Water` | 水元素 |
| 火 | `Fire` | 火元素 |
| 土 | `Earth` | 土元素 |

### 5.2 核心 API

```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAElementSelectSubsystem : public UGameInstanceSubsystem
{
    // 开始元素选择
    UFUNCTION(BlueprintCallable)
    void StartElementSelect(FName HeroId, float SelectTimeSeconds);

    // 选择元素
    UFUNCTION(BlueprintCallable)
    void SelectElement(EDBAElement Element);

    // 确认选择
    UFUNCTION(BlueprintCallable)
    void ConfirmElementSelection();

    // 取消选择
    UFUNCTION(BlueprintCallable)
    void CancelSelection();

    // 设置推荐元素
    UFUNCTION(BlueprintCallable)
    void SetSuggestedElement(EDBAElement Element);

public:
    UPROPERTY(BlueprintAssignable)
    FDBAOnElementSelectStateChanged OnElementSelectStateChanged;

    UPROPERTY(BlueprintAssignable)
    FDBAOnElementSelected OnElementSelected;

    UPROPERTY(BlueprintAssignable)
    FDBAOnElementSelectTimeUpdate OnElementSelectTimeUpdate;
};
```

---

## 6. 阵营选择子系统 (FiveCampSelect)

### 6.1 五大阵营

| 阵营 | 枚举值 | 说明 |
|------|--------|------|
| 无 | `None` | 未选择 |
| 白虎 | `Byakko` | 白虎阵营 |
| 青龙 | `Qinglong` | 青龙阵营 |
| 玄武 | `Xuanwu` | 玄武阵营 |
| 朱雀 | `Zhuque` | 朱雀阵营 |
| 麒麟 | `Kirin` | 麒麟阵营 |

### 6.2 核心 API

```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAFiveCampSelectSubsystem : public UGameInstanceSubsystem
{
    // 开始阵营选择
    UFUNCTION(BlueprintCallable)
    void StartFiveCampSelect(FName HeroId, EDBAElement Element, float SelectTimeSeconds);

    // 选择阵营
    UFUNCTION(BlueprintCallable)
    void SelectFiveCamp(EDBAFiveCamp FiveCamp);

    // 确认选择
    UFUNCTION(BlueprintCallable)
    void ConfirmFiveCampSelection();

    // 取消选择
    UFUNCTION(BlueprintCallable)
    void CancelSelection();

public:
    UPROPERTY(BlueprintAssignable)
    FDBAOnFiveCampSelectStateChanged OnFiveCampSelectStateChanged;

    UPROPERTY(BlueprintAssignable)
    FDBAOnFiveCampSelected OnFiveCampSelected;

    UPROPERTY(BlueprintAssignable)
    FDBAOnFiveCampSelectTimeUpdate OnFiveCampSelectTimeUpdate;
};
```

---

## 7. 大厅子系统 (Lobby)

### 7.1 大厅类型

| 类型 | 枚举值 | 说明 |
|------|--------|------|
| 未知 | `None` | 未在大厅 |
| 新手村 | `NewbieVillage` | 新手引导区域 |
| 主大厅 | `MainLobby` | 正式大厅 |

### 7.2 大厅状态

| 状态 | 枚举值 | 说明 |
|------|--------|------|
| 未加载 | `Unloaded` | 未进入 |
| 正在加载 | `Loading` | 加载中 |
| 准备就绪 | `Ready` | 可用状态 |
| 加载失败 | `LoadFailed` | 加载失败 |

### 7.3 核心 API

```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAIobbySubsystem : public UGameInstanceSubsystem
{
    // 进入默认大厅
    UFUNCTION(BlueprintCallable)
    void EnterLobby();

    // 进入指定大厅
    UFUNCTION(BlueprintCallable)
    void EnterSpecificLobby(EDBALobbyType LobbyType);

    // 离开大厅
    UFUNCTION(BlueprintCallable)
    void LeaveLobby();

    // 应用阵营主题
    UFUNCTION(BlueprintCallable)
    void ApplyCampTheme(EDBAFiveCampType CampType);

    // 查询
    UFUNCTION(BlueprintPure)
    EDBALobbyType GetCurrentLobbyType() const;

    UFUNCTION(BlueprintPure)
    EDBALobbyState GetLobbyState() const;

    UFUNCTION(BlueprintPure)
    bool IsInLobby() const;

public:
    UPROPERTY(BlueprintAssignable)
    FDBAOnLobbyStateChanged OnLobbyStateChanged;

    UPROPERTY(BlueprintAssignable)
    FDBAOnLobbyLoaded OnLobbyLoaded;
};
```

### 7.4 大厅配置

```cpp
USTRUCT(BlueprintType)
struct FDBALobbyConfig
{
    EDBALobbyType LobbyType;          // 大厅类型
    FString LevelPath;                 // 关卡路径
    FText DisplayName;                 // 显示名称
    bool bEnableParty;               // 允许组队
    bool bEnablePractice;             // 允许练习
    bool bEnableMatchmaking;          // 允许匹配
    bool bShowNewbieGuide;            // 显示新手引导
    EDBAFiveCampType ThemeCamp;       // 主题阵营
};
```

---

## 8. 选择流程时序图

### 8.1 完整选择流程

```
玩家                    客户端                      服务端
 │                       │                          │
 │  StartMatchEntry()    │                          │
 │──────────────────────>│                          │
 │                       │                          │
 │                       │     JoinQueue()         │
 │                       │─────────────────────────>│
 │                       │                          │
 │                       │     MatchFound()         │
 │                       │<─────────────────────────│
 │                       │                          │
 │                       │  OnMatchFound Event      │
 │  OnMatchFound Event   │<─────────────────────────│
 │<──────────────────────│                          │
 │                       │                          │
 │  ReadyCheck()         │                          │
 │──────────────────────>│                          │
 │                       │                          │
 │                       │     ReadyCheck Completed  │
 │                       │─────────────────────────>│
 │                       │                          │
 │                       │     HeroSelect Started   │
 │                       │<─────────────────────────│
 │                       │                          │
 │  OnHeroSelectStateChanged │                        │
 │<──────────────────────│                          │
 │                       │                          │
 │  SelectHero(HeroId)   │                          │
 │──────────────────────>│                          │
 │                       │                          │
 │  ConfirmHero()        │                          │
 │──────────────────────>│                          │
 │                       │                          │
 │                       │     HeroSelect Completed  │
 │                       │─────────────────────────>│
 │                       │                          │
 │                       │     ElementSelect Started │
 │                       │<─────────────────────────│
 │                       │                          │
 │  OnElementSelectStateChanged │                   │
 │<──────────────────────│                          │
 │                       │                          │
 │  SelectElement(Element) │                        │
 │──────────────────────>│                          │
 │                       │                          │
 │  ConfirmElement()      │                          │
 │──────────────────────>│                          │
 │                       │                          │
 │                       │     ElementSelect Completed│
 │                       │─────────────────────────>│
 │                       │                          │
 │                       │     FiveCampSelect Started│
 │                       │<─────────────────────────│
 │                       │                          │
 │  OnFiveCampSelectStateChanged │                   │
 │<──────────────────────│                          │
 │                       │                          │
 │  SelectFiveCamp(Camp) │                          │
 │──────────────────────>│                          │
 │                       │                          │
 │  ConfirmFiveCamp()     │                          │
 │──────────────────────>│                          │
 │                       │                          │
 │                       │     FiveCampSelect Completed│
 │                       │─────────────────────────>│
 │                       │                          │
 │                       │     Loading Started      │
 │                       │<─────────────────────────│
 │                       │                          │
 │  OnLoadingComplete     │                          │
 │<──────────────────────│                          │
 │                       │                          │
 │                       │     Match Start          │
 │                       │<─────────────────────────│
 │                       │                          │
 │  Travel to Arena      │                          │
 │<──────────────────────│                          │
```

---

## 9. 子系统依赖关系

```
UGameInstanceSubsystem
├── UDBAMatchEntryCoordinator
│   ├── UDBAQueueSubsystem
│   ├── UDBAReadyCheckSubsystem
│   ├── UDBAHeroSelectSubsystem
│   ├── UDBAElementSelectSubsystem
│   ├── UDBAFiveCampSelectSubsystem
│   └── UDALoadingPreparationSubsystem
│
├── UDBALobbySubsystem
│   ├── UDBACharacterRosterSubsystem
│   └── UDBANewbieGuideGateSubsystem
│
├── UDBAQueueSubsystem
│   └── UDBAIartySubsystem
│
└── (其他全局子系统)
```

---

## 10. 事件使用示例

### 10.1 监听匹配流程

```cpp
// 在 UI Widget 或 Controller 中
void UMyMatchEntryWidget::BindMatchEntryEvents()
{
    UDBAMatchEntryCoordinator* Coordinator =
        GetGameInstance()->GetSubsystem<UDBAMatchEntryCoordinator>();

    if (Coordinator)
    {
        // 监听阶段变化
        Coordinator->OnMatchEntryPhaseChanged.AddUObject(
            this, &UMyMatchEntryWidget::OnPhaseChanged);

        // 监听匹配失败
        Coordinator->OnMatchEntryFailed.AddUObject(
            this, &UMyMatchEntryWidget::OnMatchFailed);

        // 监听匹配完成
        Coordinator->OnMatchEntryCompleted.AddUObject(
            this, &UMyMatchEntryWidget::OnMatchCompleted);
    }
}

void UMyMatchEntryWidget::OnPhaseChanged(EDBAMatchEntryPhase NewPhase)
{
    switch (NewPhase)
    {
    case EDBAMatchEntryPhase::Queue:
        ShowQueueUI();
        break;
    case EDBAMatchEntryPhase::MatchFound:
        ShowMatchFoundUI();
        break;
    case EDBAMatchEntryPhase::HeroSelect:
        ShowHeroSelectUI();
        break;
    // ...
    }
}
```

### 10.2 监听英雄选择

```cpp
void UMyHeroSelectWidget::BindHeroSelectEvents()
{
    UDBAHeroSelectSubsystem* HeroSelectSubsystem =
        GetGameInstance()->GetSubsystem<UDBAHeroSelectSubsystem>();

    if (HeroSelectSubsystem)
    {
        HeroSelectSubsystem->OnHeroSelected.AddUObject(
            this, &UMyHeroSelectWidget::OnHeroSelected);

        HeroSelectSubsystem->OnHeroSelectTimeUpdate.AddUObject(
            this, &UMyHeroSelectWidget::OnTimeUpdate);
    }
}

void UMyHeroSelectWidget::OnHeroSelected(FName HeroId)
{
    // 更新选中的英雄显示
    SelectedHeroId = HeroId;
    UpdateHeroDisplay(HeroId);
}
```

---

## 11. 常量配置

| 配置项 | 常量名 | 值 | 说明 |
|--------|--------|-----|------|
| 选择超时 | HeroSelectDuration | 30s | 英雄选择时长 |
| 元素选择超时 | ElementSelectDuration | 20s | 元素选择时长 |
| 阵营选择超时 | FiveCampSelectDuration | 15s | 阵营选择时长 |
| 准备确认超时 | ReadyCheckTimeout | 30s | 准备确认超时 |
| 最大排队时间 | MaxQueueWaitTime | 300s | 排队最大等待 |

---

*文档版本: 1.0*
*生成时间: 2026-04-27*
*引擎版本: UE 5.7.1*
