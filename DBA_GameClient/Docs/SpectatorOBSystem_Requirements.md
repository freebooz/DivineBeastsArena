# 观战/OB系统需求规格

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.1.0
> 日期: 2026-05-05
> 状态: 待评审

---

## 1. 目标与范围

### 1.1 目标
为游戏提供专业级观战/OB系统，支持实时观战和比赛回放，满足电竞比赛和日常观战需求。

### 1.2 核心功能
- 专业OB视角切换
- 自由视角控制
- 实时信息显示 (技能冷却/装备/血量/能量)
- 观战控制 (暂停/速度调节)
- 战争迷雾视野保护

### 1.3 范围
**包含:**
- 观战者连接和管理
- 多视角切换系统
- 信息显示UI组件
- 观战控制功能
- 视野同步和保护

**不包含:**
- 录像存储和回放系统 (后续迭代)
- 导播自动切换AI
- 观众弹幕/互动

---

## 2. 功能需求

### 2.1 观战者连接

#### 2.1.1 连接方式
```cpp
// 观战者通过房间号或比赛ID连接
UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
void ConnectToSpectatorMode(FString MatchID);

UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
void DisconnectFromSpectatorMode();
```

#### 2.1.2 观战者类型
```
普通观战者 - 只能观看，有限控制
裁判观战者 - 可暂停/恢复，可踢人
主播观战者 - 有导播工具，可切换视角
```

---

### 2.2 视角切换系统

#### 2.2.1 视角类型
```
跟随视角 - 锁定跟随某个玩家，显示该玩家完整状态
自由视角 - 任意位置滑动，可随时切换
战术视角 - 俯视全场，适合分析
```

#### 2.2.2 切换方式
```
快捷键:
- Tab键: 切换到下一个玩家
- Shift+Tab: 切换到上一个玩家
- 数字键1-10: 直接切换到对应玩家
- Space: 切换到自由视角

小地图:
- 点击玩家头像: 切换到该玩家
- 右键拖拽: 自由视角移动

列表面板:
- 显示所有玩家列表
- 点击选择切换
```

#### 2.2.3 视角数据同步
```cpp
USTRUCT()
struct FDBAObserverViewTarget
{
    GENERATED_BODY()

    /** 观看的玩家Actor */
    UPROPERTY()
    TWeakObjectPtr<ADBAZodiacCharacterBase> TargetCharacter;

    /** 玩家名称 */
    UPROPERTY()
    FString PlayerName;

    /** 队伍ID */
    UPROPERTY()
    uint8 TeamID;

    /** 英雄类型 */
    UPROPERTY()
    FName HeroID;

    /** 当前HP */
    UPROPERTY()
    float CurrentHP;

    /** 最大HP */
    UPROPERTY()
    float MaxHP;

    /** 当前能量 */
    UPROPERTY()
    float CurrentEnergy;

    /** 最大能量 */
    UPROPERTY()
    float MaxEnergy;

    /** 技能冷却状态 */
    UPROPERTY()
    TArray<float> SkillCooldowns;

    /** 终极技能是否就绪 */
    UPROPERTY()
    bool bUltimateReady;
};
```

---

### 2.3 信息显示UI

#### 2.3.1 顶部状态栏
```
┌─────────────────────────────────────────────────────────────┐
│ [头像] 玩家名  HP ████████░░  Energy ████░░  [R] Ready    │
│ [头像] 玩家名  HP ██████████  Energy ██████  [R] 45s     │
└─────────────────────────────────────────────────────────────┘
```
- 显示当前观看的玩家队伍成员
- HP/Energy 条可视化
- 技能冷却图标+时间
- Ultimate就绪状态

#### 2.3.2 技能冷却显示
```cpp
// 技能槽组件 (复用 AbilitySlotWidget)
UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
TArray<UDBAAbilitySlotWidget*> SkillSlots;

// 每个技能显示:
- 图标
- 冷却进度环
- 冷却剩余秒数
- 是否可用 (高亮/灰暗)
```

#### 2.3.3 装备/属性显示
```cpp
// Hover显示面板
UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
UDBAAttributePanelWidget* AttributePanel;

// 显示:
- 装备栏 (6格)
- 攻击力/防御力
- 移速
- 暴击率
```

#### 2.3.4 小地图
```cpp
UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
UDBASpectatorMinimapWidget* Minimap;

// 功能:
- 显示所有玩家位置点 (颜色区分队伍)
- 点击头像切换视角
- 右键拖拽自由视角
```

---

### 2.4 视野保护

#### 2.4.1 战争迷雾规则
```
观战者视野规则:
1. 只显示当前观看的玩家视野范围内的单位
2. 不显示未被发现的敌人
3. 保持竞技公平性

例外 (可选配置):
- 裁判模式: 可开启全图视野
- 回放模式: 可开启全图视野
```

#### 2.4.2 视野同步
```cpp
// 服务端每帧同步观战者可见区域
void ADBARpcHandler::ServerSyncObserverVisibility_Implementation(
    FGameplayTagContainer VisibleTags,
    TArray<FVector> VisibleLocations
);
```

---

### 2.5 观战控制

#### 2.5.1 基础控制
```
播放控制面板:
[⏸️暂停] [▶️恢复] [⏪ -0.5x] [▶️ 1x] [⏩ +0.5x] [⏩ 2x]
```

#### 2.5.2 控制权限
```cpp
enum class EDBAObserverControlLevel : uint8
{
    None,       // 无控制权限
    ViewOnly,   // 只能观看
    Pause,     // 可暂停/恢复
    Kick,      // 可踢人
    Admin      // 完全控制
};
```

#### 2.5.3 观战者管理 (裁判用)
```cpp
// 踢出观战者
UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|Admin")
void KickSpectator(FUniqueNetIdRepl SpectatorID);

// 获取观战者列表
UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|Admin")
TArray<FDBAObserverInfo> GetSpectatorList();
```

---

### 2.6 音效和评论

#### 2.6.1 观战专属音效
```
- 击杀播报
- 大龙/小龙击杀提示
- 团战警报
- 胜利/失败音效
```

#### 2.6.2 静音控制
```
观战者可选择静音:
- 全局静音
- 只听比赛音效
- 只听解说 (如有)
```

---

## 3. 非功能需求

### 3.1 性能需求
- 观战延迟 < 100ms
- 支持同时100+观战者
- 视角切换响应 < 50ms
- UI更新 < 16ms (60fps)

### 3.2 网络同步
- 视角数据同步频率: 10Hz
- 技能冷却同步频率: 1Hz (或变更时)
- 位置同步使用现有Replicate机制

### 3.3 扩展性
- 易于添加新的观战视角
- 易于添加观战统计
- 支持后续弹幕系统集成

---

## 4. 目录结构

```
GameDBA/
├── Spectator/
│   ├── Component/
│   │   └── DBASpectatorComponent.h/cpp        (新 - 观战者组件)
│   ├── Controller/
│   │   └── DBASpectatorController.h/cpp        (新 - 观战控制器)
│   └── UI/
│       ├── DBASpectatorHUDWidget.h/cpp        (新 - 观战HUD)
│       ├── DBASpectatorMinimapWidget.h/cpp    (新 - 观战小地图)
│       └── DBASpectatorStatusBarWidget.h/cpp  (新 - 状态栏)
├── Character/
│   └── DBAZodiacCharacterBase.h/cpp           (修改 - 添加观战数据复制)
└── System/
    └── DBAMatch spectatorManager.h/cpp         (新 - 观战管理器)
```

---

## 5. 用户故事

| ID | 场景 | 预期行为 |
|----|------|----------|
| OBS-01 | 观战者加入 | 输入房间号进入观战，显示观战HUD |
| OBS-02 | 切换视角 | 按Tab键切换到下一个玩家，状态栏更新 |
| OBS-03 | 查看技能冷却 | 观看的玩家技能进入冷却，显示冷却动画 |
| OBS-04 | Hover查看装备 | 鼠标悬停玩家显示装备/属性面板 |
| OBS-05 | 裁判暂停 | 裁判点击暂停，比赛暂停，观战者看到提示 |
| OBS-06 | 战争迷雾 | 观战者只能看到跟随玩家的视野 |

---

## 6. 验收标准

### 6.1 功能验收
- [ ] 观战者可通过房间号连接
- [ ] Tab键正确切换玩家视角
- [ ] 技能冷却正确显示
- [ ] HP/Energy条实时更新
- [ ] 裁判可暂停/恢复比赛
- [ ] 视野保护正确生效

### 6.2 性能验收
- [ ] 视角切换 < 50ms
- [ ] UI更新 60fps流畅
- [ ] 100观战者同时在线

### 6.3 体验验收
- [ ] 观战HUD清晰易读
- [ ] 小地图交互直观
- [ ] 快捷键响应及时

---

## 7. 依赖关系

```
观战系统依赖:
[DBAMatch spectatorManager] - 管理观战者连接
       ↓
[DBASpectatorComponent] - 挂在观战者Pawn上
       ↓
[DBASpectatorHUD] - 显示观战UI
       ↓
[DBAZodiacCharacterBase] - 提供观战数据 (技能冷却/HP/Energy)
       ↓
[DBAAbilitySlotWidget] - 复用显示技能冷却
```

---

## 8. 参考文档

- `Docs/SkillFeedbackSystem_Requirements.md` - 技能反馈系统 (复用AbilitySlotWidget)
- `Docs/UISystemEnhancement_Requirements.md` - UI系统完善
- `Public/GameDBA/Character/DBAZodiacCharacterBase.h` - 角色基类
- `Public/GameDBA/UI/Arena/AbilityBar/DBAAbilitySlotWidget.h` - 技能槽组件

---

## 9. 开放问题

1. **全图视野**: 是否提供"全图观战模式"选项？(默认OFF保持公平)
2. **回放功能**: 是否需要录像存储？(本次可不实现)
3. **主播工具**: 导播AI自动切换是否需要？

---

## 10. 实现状态

### 已完成
- [x] DBAObserverTypes.h - 核心数据结构
- [x] DBASpectatorManager.h/cpp - 观战管理器
- [x] DBASpectatorComponent.h/cpp - 观战组件 (输入处理)
- [x] DBASpectatorHUDWidgetBase.h/cpp - HUD基类
- [x] DBASpectatorStatusBarWidgetBase.h/cpp - 状态栏
- [x] DBASpectatorMinimapWidgetBase.h/cpp - 小地图
- [x] DBAZodiacCharacterBase - 添加TeamID和GetSpectatorData()
- [x] DBAAbilitySystemComponent - 添加冷却同步功能
- [x] 技能冷却同步逻辑 (SkillCooldowns 数组更新与 Replicate 同步)

### 待完成 (需要UE编辑器操作)
- [ ] 在 UE 编辑器中创建 IMC_Spectator Input Mapping Context
- [ ] 配置 Spectator_CycleNext 等输入 Action 和 Mapping
- [ ] 在蓝图实现状态栏和小地图的视觉表现

### 配置文件
- [x] Config/DefaultSpectatorInput.ini - 输入配置说明

---

*文档生成时间: 2026-05-05*
*最后更新: 2026-05-05*
