# 观战系统 UE 编辑器配置指南

> 项目: DivineBeastsArena 神兽竞技场
> 日期: 2026-05-05

---

## 概览

观战系统需要以下 UE 编辑器配置:
1. Input Mapping Context (IMC_Spectator)
2. Input Action 资产
3. 观战HUD 蓝图实现

---

## 步骤 1: 创建 Input Actions

### 1.1 创建 Input Actions 文件夹
在 `Content/Input/` 下创建 `Spectator/` 子文件夹

### 1.2 创建 Input Actions (在 Content/Input/Spectator/ 下)

按以下顺序创建:

| 名称 | 类型 | 描述 |
|------|------|------|
| IA_Spectator_CycleNext | Input Action | 切换到下一个玩家 |
| IA_Spectator_CyclePrevious | Input Action | 切换到上一个玩家 |
| IA_Spectator_ToggleFreeView | Input Action | 切换自由视角 |
| IA_Spectator_TogglePause | Input Action | 暂停/恢复 |
| IA_Spectator_JumpToTarget_1 | Input Action | 跳转到玩家1 |
| IA_Spectator_JumpToTarget_2 | Input Action | 跳转到玩家2 |
| IA_Spectator_JumpToTarget_3 | Input Action | 跳转到玩家3 |
| IA_Spectator_JumpToTarget_4 | Input Action | 跳转到玩家4 |
| IA_Spectator_JumpToTarget_5 | Input Action | 跳转到玩家5 |
| IA_Spectator_JumpToTarget_6 | Input Action | 跳转到玩家6 |
| IA_Spectator_JumpToTarget_7 | Input Action | 跳转到玩家7 |
| IA_Spectator_JumpToTarget_8 | Input Action | 跳转到玩家8 |
| IA_Spectator_JumpToTarget_9 | Input Action | 跳转到玩家9 |

### 1.3 配置每个 Input Action

对于每个 Input Action:
1. **触发条件**: Pressed (按下)
2. **按住行为**: Not Held (不需要按住)

示例配置:
```
IA_Spectator_CycleNext:
  - Triggers: [When Key is Pressed]
  - Hold Timeout: 0.0
  - Modifier Keys: None
```

---

## 步骤 2: 创建 Input Mapping Context

### 2.1 创建 IMC_Spectator

在 `Content/Input/Spectator/` 下创建 `IMC_Spectator` (Input Mapping Context)

### 2.2 配置 IMC_Spectator

在 IMC_Spectator 中添加以下映射:

| Action | Key | Modifiers |
|--------|-----|-----------|
| IA_Spectator_CycleNext | Tab | - |
| IA_Spectator_CyclePrevious | Tab | Shift |
| IA_Spectator_ToggleFreeView | Space | - |
| IA_Spectator_TogglePause | P | - |
| IA_Spectator_JumpToTarget_1 | One | - |
| IA_Spectator_JumpToTarget_2 | Two | - |
| IA_Spectator_JumpToTarget_3 | Three | - |
| IA_Spectator_JumpToTarget_4 | Four | - |
| IA_Spectator_JumpToTarget_5 | Five | - |
| IA_Spectator_JumpToTarget_6 | Six | - |
| IA_Spectator_JumpToTarget_7 | Seven | - |
| IA_Spectator_JumpToTarget_8 | Eight | - |
| IA_Spectator_JumpToTarget_9 | Nine | - |

### 2.3 设置 IMC 优先级

- **IMC_Spectator Priority**: 100 (高于游戏默认 IMC)

---

## 步骤 3: 创建观战 HUD 蓝图

### 3.1 创建 Blueprint 类

1. 右键 -> Blueprint Class
2. 搜索 `DBASpectatorHUDWidgetBase`
3. 创建 `BP_SpectatorHUD`

### 3.2 在 Blueprint 中配置

#### 3.2.1 绑定 Widget

在 `BP_SpectatorHUD` 的 Designer 中:
1. 添加 `StatusBar` 组件 -> 绑定到 `WBP_SpectatorStatusBar`
2. 添加 `Minimap` 组件 -> 绑定到 `WBP_SpectatorMinimap`
3. 添加 `CurrentPlayerNameText` -> Text Block
4. 添加 `PauseOverlay` -> Overlay

#### 3.2.2 配置技能槽

在 `BP_SpectatorHUD` 中创建技能槽组件:
1. 添加 4 个 `DBAAbilitySlotWidget` 作为子组件
2. 命名为 `SkillSlot_Q`, `SkillSlot_W`, `SkillSlot_E`, `SkillSlot_R`

### 3.3 创建 WBP_SpectatorStatusBar

1. 继承自 `DBASpectatorStatusBarWidgetBase`
2. 设计 UI:
   - 横向排列 5 个玩家状态卡片
   - 每个卡片包含: 头像、名字、HP条、Energy条、Ultimate指示

### 3.4 创建 WBP_SpectatorMinimap

1. 继承自 `DBASpectatorMinimapWidgetBase`
2. 设计 UI:
   - 200x200 小地图底图
   - 玩家位置点 (红色/蓝色)
   - 点击区域用于切换玩家

---

## 步骤 4: 配置输入组件

### 4.1 在 PlayerController 中添加组件

创建 `BP_SpectatorPlayerController`:
1. 继承自 `PlayerController`
2. 添加 `DBASpectatorComponent`
3. 添加 `BP_SpectatorHUD` 作为 HUD Widget

### 4.2 配置 Auto Receive Input

在 `BP_SpectatorPlayerController`:
- **Auto Receive Input**: Spectator (或其他合适的玩家索引)

---

## 步骤 5: 观战模式启动流程

### 5.1 进入观战模式

在 GameInstance 或 GameMode 中:
```
1. Spawn BP_SpectatorPlayerController
2. 调用 SpectatorComponent->JoinSpectatorMode(MatchID)
3. 启用 IMC_Spectator
4. 显示 BP_SpectatorHUD
```

### 5.2 退出观战模式

```
1. 调用 SpectatorComponent->LeaveSpectatorMode()
2. 停用 IMC_Spectator
3. 隐藏 BP_SpectatorHUD
4. 销毁或归还 SpectatorPlayerController
```

---

## 步骤 6: 技能冷却同步配置

### 6.1 创建技能冷却同步组件

在角色蓝图中:
1. 添加 `DBAAbilityComponent` (如果还没有)
2. 配置技能槽绑定到 `SkillCooldowns` 数组

### 6.2 同步冷却数据

在技能施放时:
```
1. 更新 SkillCooldowns 数组
2. 调用 Replicate 以同步到观战者
```

---

## 快速检查清单

- [ ] 创建了 13 个 Input Actions
- [ ] 创建了 IMC_Spectator 并配置了所有映射
- [ ] IMC 优先级设为 100
- [ ] 创建了 BP_SpectatorHUD
- [ ] 创建了 WBP_SpectatorStatusBar
- [ ] 创建了 WBP_SpectatorMinimap
- [ ] 创建了 BP_SpectatorPlayerController
- [ ] 配置了技能冷却同步

---

## 常见问题

### Q: 输入没有响应
**A**: 检查:
1. IMC 是否正确加载
2. PlayerController 的 Auto Receive Input 是否设置
3. SpectatorComponent 的 SetupInputComponent 是否被调用

### Q: 视角切换没有效果
**A**: 检查:
1. DBASpectatorManager 是否正确初始化
2. MatchPlayers 列表是否包含正确的角色
3. 是否有权限问题

### Q: 技能冷却不显示
**A**: 检查:
1. SkillCooldowns 数组是否正确更新
2. 技能槽 Widget 是否正确绑定
3. Replicate 属性是否正确设置

---

*文档生成时间: 2026-05-05*
