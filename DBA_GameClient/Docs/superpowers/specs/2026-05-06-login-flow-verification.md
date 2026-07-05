# 登录闭环开发与验证说明

日期：2026-05-06

## 当前闭环

本分支已经接入第一版可玩前端闭环：

```text
启动 -> 自动登录尝试 -> 登录页 -> 角色列表 -> 角色选择/角色创建 -> 大厅
```

账号服务采用“真实后端优先，Mock 兜底”策略。客户端优先请求本地或配置的账号服务；当网络不可用、请求超时、接口未实现或服务不可用时，允许切换到本地 Mock 账号服务，保证开发阶段仍可进入角色创建和大厅。

## 默认后端协议

默认地址：

```text
http://127.0.0.1:8080
```

第一版客户端使用以下端点：

```text
POST /api/auth/login
POST /api/auth/register
POST /api/auth/refresh
GET  /api/account/characters
POST /api/account/characters
```

Mock 兜底只允许用于开发可恢复错误：

```text
NetworkUnavailable
Timeout
EndpointMissing
ServiceUnavailable
```

以下错误不会进入 Mock 兜底：

```text
InvalidCredentials
AccountUnavailable
ValidationFailed
MalformedResponse
```

## v4/v4.1 角色创建数据

角色创建请求和角色摘要已经承接玩法 v4/v4.1 的核心字段：

```text
Zodiac
PrimaryElement
FiveCamp
FixedSkillGroupId
CoreAttributes
```

Mock 创建角色时会根据 `Zodiac + PrimaryElement` 生成固定技能组 ID，例如：

```text
Rat + Water -> Rat_Water
```

核心属性使用 v4.1 的 8 个同步属性默认值，并叠加元素加成：

```text
MaxHealth = 1800
AttackPower = 100
Defense = 40
MoveSpeed = 380
MaxEnergy = 100
EnergyRegen = 10
CriticalRate = 5
CriticalMultiplier = 200
```

运行时状态 `UltimateEnergy`、`ChainLevel`、`ElementResonanceLevel` 不写入账号摘要，保留在战斗运行时系统中处理。

## Blueprint 绑定入口

第一版提供三个 Blueprint 可用控制器：

```text
UDBALoginWidgetController
UDBACharacterSelectWidgetController
UDBACharacterCreateWidgetController
```

推荐绑定方式：

1. 登录页创建或持有 `UDBALoginWidgetController`，调用 `Start()`。
2. 状态进入 `LoginScreen` 后，调用 `LoginWithEmail()` 或 `LoginAsGuest()`。
3. 角色列表页使用 `UDBACharacterSelectWidgetController::BindLoginFlow()` 接收角色列表。
4. 无角色时展示创建页，使用 `UDBACharacterCreateWidgetController` 设置名称、生肖、元素、阵营后调用 `Submit()`。
5. 创建或选择成功后，`UDBALoginFlowSubsystem` 会进入 `MainLobby`，并同步设置 `UDBAFrontendSessionSubsystem` 为大厅状态。

## 验证记录

已执行编辑器构建命令：

```powershell
& 'D:\UnrealEngine-5.8.0-release\Engine\Build\BatchFiles\Build.bat' DivineBeastsArenaEditor Win64 Development -Project='C:\Users\Administrator\.config\superpowers\worktrees\DivineBeastsArena\feature-login-flow\DivineBeastsArena.uproject' -WaitMutex -NoHotReloadFromIDE
```

验证结果：

```text
UHT 可处理新增登录控制器。
GameCore 登录流、JSON 协议、在线账号服务相关源文件可编译并链接到 UnrealEditor-GameCore.dll。
新增登录 UI 控制器进入编译队列。
GameMoba RPC 迁移后的导出宏错误已修复。
```

使用 `D:\UnrealEngine-5.8.0-release` 重新验证时，构建进程在 10 分钟工具超时内未返回完整日志；已清理遗留的 UBT/MSBuild 子进程。此前使用 UE 5.8 安装版得到的完整日志显示，当前完整项目构建仍被既有主模块问题阻断，阻断点不属于本次登录闭环新增代码：

```text
DBAGameplayAbility_Rat_E.cpp 缺少 DBAGameplayAbility_Rat_E.h
DBASpectatorManager.h 缺少 Engine/GameInstanceSubsystem.h
FAggroInfo.h 在结构体中直接调用 GetWorld()
多个投射物 InitializeProjectile 签名与父类不匹配
DBAClientPredictionComponent.h 错误前置声明 FVector
DBAEnumsCore 生成代码与当前枚举声明不一致
DBAZodiacAnimInstance.h 引用缺失的 GameDBA/Animation/AnimInstance.h
```

