# Divine Beasts Arena 用户界面线框图与组件层级清单

## 概述

本文档描述游戏所有用户界面的线框图设计、组件类型及层级关系，供设计师参考编辑。

---

## 一、UI 架构总览

```
UUserWidget (UE基础类)
    │
    ├── DBAWidgetBase (项目Widget基类)
    │       ├── DBAUserWidgetBase (通用Widget基类)
    │       │       ├── MobaBase层 Widget
    │       │       │       ├── UDBAMobaUserWidgetBase
    │       │       │       │       └── Client层 Widget
    │       │       │       │               ├── Lobby Widget
    │       │       │       │               ├── Arena Widget
    │       │       │       │               └── ...
    │       │       │       │
    │       │       │       └── UDBAMobaHUDWidgetControllerBase
    │       │       │               └── Client层 Controller
    │       │       │
    │       │       └── (其他通用Widget)
    │       │
    │       └── DBAWidgetController (Widget控制器基类)
    │               └── MobaBase层 Controller
    │                       └── Client层 Controller
    │
    └── (第三方/插件Widget)
```

---

## 二、Common UI 框架层 (公共组件)

### 2.1 基础Widget

| Blueprint名称 | C++ 类 | 说明 | 层级 |
|--------------|--------|------|------|
| WBP_DBA_UIRootLayout | DBAWidgetBase | 5层叠加布局系统 | 根容器 |
| WBP_DBA_Tooltip | - | 工具提示框 | 覆盖层 |
| WBP_DBA_Notification | - | 通知提示 | 覆盖层 |
| WBP_DBA_ModalDialog | - | 模态对话框 | 覆盖层 |

### 2.2 单位框架组件

| Blueprint名称 | C++ 类 | 说明 | 层级 |
|--------------|--------|------|------|
| WBP_DBA_UnitFrame | - | 单位状态框架 | 底层 |
| WBP_DBA_PartyFrame | - | 队伍框架 | 底层 |
| WBP_DBA_RaidFrame | - | 团队框架 | 底层 |
| WBP_DBA_ActionBar | - | 技能快捷栏 | HUD层 |
| WBP_DBA_CastBar | - | 施法条 | HUD层 |

### 2.3 Buff/状态组件

| Blueprint名称 | C++ 类 | 说明 | 层级 |
|--------------|--------|------|------|
| WBP_DBA_BuffIcon | - | Buff图标 | HUD层 |
| WBP_DBA_BuffBar_Generic | - | Buff状态条 | HUD层 |
| WBP_DBA_CombatText | - | 战斗文字弹出 | 特效层 |

---

## 三、Frontend UI A (登录/注册模块)

### 3.1 启动与登录

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_StartupScreen | UDBAStartupVideoWidget | 启动视频界面 | 闪屏 |
| WBP_DBA_Login | - | 登录界面 | 表单 |
| WBP_DBA_Register | - | 注册界面 | 表单 |
| WBP_DBA_GuestLoginEntry | - | 游客入口 | 按钮 |

### 3.2 角色创建/选择

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_CharacterCreate | - | 角色创建 | 表单 |
| WBP_DBA_CharacterSelect | - | 角色选择 | 列表 |

### 3.3 通用提示组件

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_SystemToast | - | 系统提示 | Toast |
| WBP_DBA_ErrorBanner | - | 错误横幅 | Banner |
| WBP_DBA_FrontendTooltip | - | 前端工具提示 | Tooltip |
| WBP_DBA_MobileInfoSheet | - | 手机信息表 | 表单 |

---

## 四、Frontend UI B (大厅/匹配模块)

### 4.1 主大厅

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_MainLobby | UDBAMainLobbyWidgetBase | 主大厅界面 | 页面 |
| WBP_DBA_MainLobbyWidgetController | UDBAMainLobbyWidgetController | 大厅控制器 | Controller |

### 4.2 新手引导

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_NewbieVillageMain | UDBANewbieVillageMainWidgetBase | 新手村主界面 | 页面 |
| WBP_DBA_NewbieTaskTracker | UDBANewbieTaskTrackerWidgetBase | 新手任务追踪 | HUD |
| WBP_DBA_InteractionPrompt | UDBAInteractionPromptWidgetBase | 交互提示 | Prompt |

### 4.3 组队与社交

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_PortalConfirm | UDBAPortalConfirmWidgetBase | 传送确认 | Modal |
| WBP_DBA_PartyPanel | UDBAPartyPanelWidgetBase | 队伍面板 | Panel |
| WBP_DBA_InvitePanel | UDBAInvitePanelWidgetBase | 邀请面板 | Panel |

### 4.4 匹配系统

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_QueueModeSelect | UDBAQueueModeSelectWidgetBase | 匹配模式选择 | 页面 |
| WBP_DBA_QueueStatus | UDBAQueueStatusWidgetBase | 匹配状态 | HUD |
| WBP_DBA_MatchFound | UDBAMatchFoundWidgetBase | 匹配成功 | Modal |
| WBP_DBA_ReadyCheck | UDBAReadyCheckWidgetBase | 准备确认 | Modal |

---

## 五、Frontend UI C (英雄/元素/阵营选择模块)

### 5.1 英雄选择

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_HeroSelect | UDBAHeroSelectWidgetBase | 英雄选择界面 | 页面 |
| WBP_DBA_HeroSelectWidgetController | UDBAHeroSelectWidgetController | 英雄选择控制器 | Controller |
| WBP_DBA_HeroInfoPanel | UDBAHeroInfoPanelWidgetBase | 英雄信息面板 | Panel |
| WBP_DBA_FixedSkillGroupPreview | UDBAFixedSkillGroupPreviewWidgetBase | 固定技能组预览 | Preview |

### 5.2 元素选择

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_ElementSelect | UDBAElementSelectWidgetBase | 元素选择界面 | 页面 |
| WBP_DBA_ElementSelectWidgetController | UDBAElementSelectWidgetController | 元素选择控制器 | Controller |
| WBP_DBA_ElementInfoPanel | UDBAElementInfoPanelWidgetBase | 元素信息面板 | Panel |

### 5.3 阵营选择

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_FiveCampSelect | UDBAFiveCampSelectWidgetBase | 五阵营选择界面 | 页面 |
| WBP_DBA_FiveCampSelectWidgetController | UDBAFiveCampSelectWidgetController | 阵营选择控制器 | Controller |
| WBP_DBA_FiveCampInfoPanel | UDBAFiveCampInfoPanelWidgetBase | 阵营信息面板 | Panel |

### 5.4 验证提示

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_BuildValidationHint | UDBABuildValidationHintWidgetBase | 构建验证提示 | Hint |
| WBP_DBA_QueueRuleTooltip | UDBAQueueRuleTooltipWidgetBase | 匹配规则提示 | Tooltip |

---

## 六、Loading/结果/设置模块

### 6.1 加载界面

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_LoadingScreen | UDBALoadingScreenWidgetBase | 加载等待界面 | 页面 |
| WBP_DBA_LoadingWidgetController | UDBALoadingWidgetController | 加载控制器 | Controller |

### 6.2 结算界面

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_Result | - | 比赛结果 | 页面 |
| WBP_DBA_PostMatch | - | 赛后统计 | 页面 |

### 6.3 设置界面

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_SettingsRoot | - | 设置根页面 | 页面 |
| WBP_DBA_SettingsGraphics | - | 画面设置 | Tab |
| WBP_DBA_SettingsAudio | - | 声音设置 | Tab |
| WBP_DBA_SettingsInput | - | 输入设置 | Tab |
| WBP_DBA_SettingsGameplay | - | 游戏设置 | Tab |
| WBP_DBA_SettingsAndroid | - | 手机设置 | Tab |
| WBP_DBA_SettingsNetwork | - | 网络设置 | Tab |
| WBP_DBA_SettingsAccessibility | - | 辅助功能 | Tab |

---

## 七、Arena HUD (战斗内界面)

### 7.1 HUD根容器

| Blueprint名称 | C++ 类 | 说明 | 层级 |
|--------------|--------|------|------|
| WBP_DBA_ArenaHUDRoot | UDBAArenaHUDRootWidgetBase | HUD根容器 | Root |
| WBP_DBA_ArenaHUDWidgetController | UDBAArenaHUDWidgetController | HUD控制器 | Controller |

### 7.2 玩家状态

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_PlayerUnitFrame | UDBAPlayerUnitFrameWidgetBase | 玩家单位框架 | Frame |
| WBP_DBA_PlayerUnitFrameWidgetController | UDBAPlayerUnitFrameWidgetController | 单位框架控制器 | Controller |
| WBP_DBA_SelfCastBar | UDBASelfCastBarWidgetBase | 自我施法条 | CastBar |

### 7.3 技能栏

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_AbilityBar | UDBAAbilityBarWidgetBase | 技能快捷栏 | Bar |

### 7.4 Buff/Debuff栏

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_PassiveAndResonancePanel | UDBAPassiveAndResonancePanelWidgetBase | 被动与共鸣面板 | Panel |
| WBP_DBA_BuffBar | UDBABuffBarWidgetBase | Buff显示条 | Bar |
| WBP_DBA_DebuffBar | UDBADebuffBarWidgetBase | Debuff显示条 | Bar |
| WBP_DBA_CCBar | UDBACCBarWidgetBase | 控制效果条 | Bar |

### 7.5 能量与状态

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_MomentumPanel | UDBAMomentumPanelWidgetBase | 动量面板 | Panel |
| WBP_DBA_ChainUltimatePanel | UDBAChainUltimatePanelWidgetBase | 连锁终极技能面板 | Panel |

### 7.6 战斗反馈

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_CombatAnnouncement | UDBACombatAnnouncementWidgetBase | 战斗播报 | Announcement |
| WBP_DBA_CriticalStateHint | UDBACriticalStateHintWidgetBase | 危险状态提示 | Hint |
| WBP_DBA_UltimateReadyPrompt | UDBAUltimateReadyPromptWidgetBase | 终极技能就绪提示 | Prompt |
| WBP_DBA_AuraSummaryPanel | UDBAAuraSummaryPanelWidgetBase | 光环总结面板 | Panel |

### 7.7 连接与目标

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_ConnectionWarning | UDBAConnectionWarningWidgetBase | 连接警告 | Warning |
| WBP_DBA_ArenaObjectiveTracker | UDBAArenaObjectiveTrackerWidgetBase | 目标追踪器 | Tracker |

---

## 八、Arena UI B (目标/团队/地图/计分板)

### 8.1 目标框架

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_TargetFrame | - | 目标框架 | Frame |
| WBP_DBA_TargetInfo | - | 目标信息 | Info |
| WBP_DBA_TargetCastBar | - | 目标施法条 | CastBar |
| WBP_DBA_FocusFrame | - | 焦点框架 | Frame |

### 8.2 团队框架

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_AllyTeamFrame | - | 同伴团队框架 | Frame |
| WBP_DBA_PartyRaidStyleMemberSlot | - | 队伍/团队成员槽位 | Slot |

### 8.3 击杀与计分

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_KillFeed | - | 击杀信息流 | Feed |
| WBP_DBA_Scoreboard | - | 计分板 | Board |

### 8.4 地图

| Blueprint名称 | C++ 类 | 说明 | 组件类型 |
|--------------|--------|------|----------|
| WBP_DBA_Minimap | - | 小地图 | Minimap |
| WBP_DBA_ExpandedMap | - | 扩展地图 | Map |

---

## 九、WidgetController 层级关系

### 9.1 控制器继承链

```
WidgetController (基类 - Common层)
    │
    ├── MobaBase层
    │       ├── UDBAMobaWidgetControllerBase
    │       │       │
    │       │       └── Client层
    │       │               ├── UDBAMainLobbyWidgetController
    │       │               ├── UDBAHeroSelectWidgetController
    │       │               ├── UDBAElementSelectWidgetController
    │       │               ├── UDBAFiveCampSelectWidgetController
    │       │               ├── UDBALoadingWidgetController
    │       │               └── UDBAQueueWidgetController
    │       │
    │       └── UDBAMobaHUDWidgetControllerBase
    │               │
    │               └── UDBAArenaHUDWidgetController
    │                       └── UDBAPlayerUnitFrameWidgetController
    │
    └── (其他专用控制器)
```

### 9.2 控制器与Widget对应关系

| Controller | 控制的Widget | 说明 |
|------------|-------------|------|
| UDBAMainLobbyWidgetController | WBP_DBA_MainLobby | 大厅主控制器 |
| UDBAHeroSelectWidgetController | WBP_DBA_HeroSelect | 英雄选择控制器 |
| UDBAElementSelectWidgetController | WBP_DBA_ElementSelect | 元素选择控制器 |
| UDBAFiveCampSelectWidgetController | WBP_DBA_FiveCampSelect | 阵营选择控制器 |
| UDBALoadingWidgetController | WBP_DBA_LoadingScreen | 加载界面控制器 |
| UDBAQueueWidgetController | WBP_DBA_QueueStatus | 匹配状态控制器 |
| UDBAArenaHUDWidgetController | WBP_DBA_ArenaHUDRoot | 战斗HUD控制器 |
| UDBAPlayerUnitFrameWidgetController | WBP_DBA_PlayerUnitFrame | 单位框架控制器 |

---

## 十、命名规范

### 10.1 Blueprint命名规则

| 类型 | 前缀 | 示例 |
|------|------|------|
| Blueprint Widget | WBP_DBA_ | WBP_DBA_MainLobby |
| Blueprint Controller | WBP_DBA_ | WBP_DBA_MainLobbyWidgetController |
| 变量 (C++) | - | AbilityBar, PlayerUnitFrame |
| BindWidget | 同变量名 | Slot_PlayerUnitFrame |

### 10.2 目录结构

```
Content/UI/
├── Lobby/                  (大厅相关)
├── Arena/                 (战斗相关)
├── Common/                (通用组件)
└── Settings/              (设置相关)
```

---

## 十一、设计师编辑指南

### 11.1 创建新Widget流程
1. 在 UE 编辑器中创建 Blueprint 类
2. 选择对应的 C++ 基类（如 `WBP_DBA_MainLobbyWidgetBase`）
3. 绑定对应的 WidgetController
4. 使用 BindWidget 绑定子组件变量

### 11.2 修改现有Widget
1. 在 UE 编辑器中打开对应的 Blueprint
2. 调整布局、外观、动画
3. 重要：保持 BindWidget 名称与 C++ 变量名一致
4. 如需新增 BindWidget，在 C++ 中添加对应成员变量

### 11.3 创建新Controller流程
1. 创建 C++ 类继承自 `UDBAMobaWidgetControllerBase`
2. 在 `.h` 中声明需要的数据成员
3. 在 `.cpp` 中实现数据绑定逻辑
4. 在 Blueprint 中设置 Controller 引用

### 11.4 添加新Buff类型
1. 在 `DBABattleAttributeSet` 中定义新属性
2. 在 `DBAGE_{Zodiac}_{Skill}` 中添加 GE 修饰符
3. 在 `WBP_DBA_BuffBar` 中添加新图标槽位

---

## 十二、文件路径索引

### 12.1 C++ 头文件路径

```
Source/DivineBeastsArena/Public/
├── Common/UI/
│       ├── DBAWidgetBase.h
│       ├── DBAUserWidgetBase.h
│       └── DBAWidgetController.h
├── MobaBase/UI/
│       ├── UDBAMobaUserWidgetBase.h
│       └── UDBAMobaWidgetControllerBase.h
├── Client/UI/Lobby/
│       ├── MainLobby/
│       ├── HeroSelect/
│       ├── ElementSelect/
│       ├── FiveCampSelect/
│       └── ...
└── Client/UI/Arena/
        ├── UDBAAbilityBarWidgetBase.h
        ├── UDBAPlayerUnitFrameWidgetBase.h
        └── ...
```

### 12.2 Blueprint 资源路径

```
Content/UI/
├── WBP_DBA_MainLobby.uasset
├── WBP_DBA_HeroSelect.uasset
├── WBP_DBA_ArenaHUDRoot.uasset
└── ...
```

---

*本文档由代码分析和设计规范整理，最后更新: 2026-04-30*