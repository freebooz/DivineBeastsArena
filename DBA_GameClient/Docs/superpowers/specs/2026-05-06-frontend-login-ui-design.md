# 前端登录闭环 UI 设计

日期：2026-05-06

## 目标

本阶段只实现前端可玩闭环 UI：

```text
启动 -> 登录/游客登录 -> 角色列表 -> 创建角色 -> 主大厅
```

交付目标不是完成全游戏所有 UI，而是先让玩家能从客户端启动，完成登录，看到角色列表，创建或选择角色，并进入大厅。后续设置、组队、匹配、竞技场 HUD、观战 HUD、移动端布局和结算界面另行分阶段实现。

本阶段所有编译与运行验证统一使用：

```text
E:\UnrealEngine-5.7.1-release
```

## 范围

包含：

- 启动页
- 登录页
- 游客登录入口
- 角色列表页
- 角色创建页
- 主大厅页
- 通用错误提示和系统 Toast
- Dedicated Server 与客户端调试验证路径

不包含：

- 完整账号平台、支付、好友、实名、防沉迷、跨平台绑定
- 完整设置系统
- 完整组队、匹配和结算 UI
- 竞技场 HUD、观战 HUD、移动端 HUD
- 大规模 UI 资产目录重构

## 推荐方案

采用“C++ 控制器 + 现有 UMG 蓝图补齐布局”的方案。

C++ 负责状态流、账号服务、角色创建和大厅状态切换；UMG 蓝图负责布局、按钮、动画、提示和页面表现。这样能复用已经实现的登录流子系统和 Widget Controller，同时保留蓝图可视化扩展空间。

## 前端流程

启动后进入启动页。启动页调用登录控制器 `Start()`，由 `UDBALoginFlowSubsystem` 进入自动登录尝试。

状态流：

```text
Startup
  -> TryAutoLogin
  -> LoginScreen
  -> LoadCharacterList
  -> CharacterSelect 或 CharacterCreate
  -> MainLobby
```

页面职责：

- 启动页只展示启动状态，并触发登录流。
- 登录页提供账号登录和游客登录入口。
- 角色选择页展示账号下角色列表；如果角色列表为空，流程自动进入角色创建。
- 角色创建页输入名称，选择生肖、元素、阵营，并提交创建。
- 主大厅页展示第一版大厅骨架，提供后续组队、匹配、设置入口位置。

UI 不直接改业务状态，只监听 `EDBALoginFlowState` 并切换页面。登录、角色、Mock 兜底和大厅状态由 `GameCore` 负责。

## 蓝图资产主路径

本阶段使用 `Content/UI` 下资产作为主路径：

```text
Content/UI/Lobby/Startup/WBP_DBA_StartupScreen
Content/UI/Lobby/Login/WBP_DBA_Login
Content/UI/Lobby/Login/WBP_DBA_GuestLoginEntry
Content/UI/Lobby/Character/WBP_DBA_CharacterSelect
Content/UI/Lobby/Character/WBP_DBA_CharacterCreate
Content/UI/Lobby/MainLobby/WBP_DBA_MainLobby
Content/UI/Lobby/Common/WBP_DBA_ErrorBanner
Content/UI/Lobby/Common/WBP_DBA_SystemToast
Content/UI/Lobby/Common/WBP_DBA_ModalDialog
```

`Content/Blueprints/UI/DBA/` 中的重复旧资产暂时保留，不在本阶段移动或删除，避免破坏现有引用。

## 控制器绑定

蓝图页面绑定以下 C++ 控制器：

```text
WBP_DBA_Login              -> UDBALoginWidgetController
WBP_DBA_CharacterSelect    -> UDBACharacterSelectWidgetController
WBP_DBA_CharacterCreate    -> UDBACharacterCreateWidgetController
WBP_DBA_MainLobby          -> UDBAMainLobbyWidgetController
```

登录页监听：

- `OnLoginStateChanged`
- `OnLoginError`

角色选择页监听：

- `OnCharactersChanged`

角色创建页调用：

- `SetCharacterName`
- `SetZodiac`
- `SetElement`
- `SetFiveCamp`
- `Submit`

创建或选择成功后，`UDBALoginFlowSubsystem` 进入 `MainLobby`，并设置 `UDBAFrontendSessionSubsystem` 为主大厅状态。

## 布局设计

### 启动页

全屏启动页，显示项目名、加载进度或状态文案。启动完成后不提供多余交互，自动进入登录流。

### 登录页

全屏布局，左侧为登录表单和游客入口，右侧为五行天命主题视觉区。

登录表单包含：

- 账号输入
- 密码输入
- 登录按钮
- 游客登录按钮
- 错误提示区

登录页不做营销式落地页，不放无关功能介绍。

### 角色选择页

主体为角色卡列表。每张卡展示：

- 角色名
- 生肖
- 元素
- 阵营
- 固定技能组 ID

角色为空时不展示空态说明页面，而是自动跳转到角色创建页。仍保留“创建新角色”入口，供后续多角色账号使用。

### 角色创建页

采用紧凑四步布局：

```text
名称 -> 生肖 -> 元素 -> 阵营 -> 确认
```

选择区域使用卡片或按钮组，右侧或下方显示预览：

- 选择摘要
- 固定技能组 ID
- v4.1 核心属性预览
- 元素共鸣提示

第一版不要求完整 12 生肖美术资产，但必须让玩家完成选择并提交。

### 主大厅页

主大厅先做功能骨架：

- 当前角色摘要
- 开始匹配入口
- 队伍区域
- 设置入口
- 退出登录入口

尚未实现的功能入口可以显示禁用态或占位提示，但不能阻断登录闭环。

## Dedicated Server 与客户端验证

验证分两层。

第一层：前端闭环验证。

```text
客户端启动
进入启动页
触发登录流
游客登录或 Mock 兜底登录
加载角色列表
无角色时创建角色
创建成功后进入主大厅
```

第二层：Dedicated Server + 客户端调试验证。

编译命令：

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaServer Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
& 'E:\UnrealEngine-5.7.1-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaEditor Win64 Development -Project="$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

运行命令：

```powershell
& 'E:\UnrealEngine-5.7.1-release\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "$PWD\DivineBeastsArena.uproject" LobbyMap -server -log
& 'E:\UnrealEngine-5.7.1-release\Engine\Binaries\Win64\UnrealEditor.exe' "$PWD\DivineBeastsArena.uproject" 127.0.0.1 -game -log
```

如果 `LobbyMap` 不适合当前前端启动流，则改用项目内实际承载前端的地图，例如 `FrontendMap`。地图选择以能进入启动页并触发登录流为准。

登录服务仍优先请求：

```text
http://127.0.0.1:8080
```

后端不可用时使用 Mock 兜底。账号密码错误、账号封禁、校验失败等业务错误不进入 Mock 兜底。

## 编译阻断处理

如果当前项目主模块已有编译错误阻断 UI 验证，则实施计划必须先包含“第 0 阶段：编译清障”。清障只修复阻断登录闭环验证的编译问题，不顺手重构无关系统。

当前已知可能阻断项包括：

- 缺失技能头文件
- 枚举重复定义
- `FVector` 前置声明冲突
- `GameInstanceSubsystem` include 路径错误
- AI/投射物函数签名不匹配
- 动画头文件路径缺失

这些问题修到项目能编译后，再启动 Dedicated Server 和客户端验证。

## 成功标准

- 使用指定引擎路径完成 Server 和 Editor 构建，或明确记录仍阻断的首个编译错误。
- 客户端能显示启动页和登录页。
- 游客登录或 Mock 兜底能成功返回账号状态。
- 角色列表能加载。
- 无角色时能创建角色。
- 创建后能自动进入主大厅。
- Dedicated Server 能启动，客户端能启动并连接到本地服务端。
- 验证日志记录到项目文档中。

## 自检

- 范围只覆盖前端登录闭环，没有混入全量 HUD 或设置系统。
- 蓝图资产主路径明确，旧重复资产不在本阶段移动。
- C++ 与蓝图职责边界明确。
- Dedicated Server 验证使用指定引擎路径。
- 编译阻断被明确列为第 0 阶段，不假设当前项目已经可完整构建。
