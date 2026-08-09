# 01 依赖方向与迁移风险

## 目标依赖方向

`Widget/WidgetController/ViewModel -> Frontend Flow -> Domain Subsystem/Facade -> GameBackendClient -> HTTPS API`

`GameCore` 只提供通用类型、基础 Subsystem、存档接口和 Travel 基础能力；`GameMoba` 依赖 `GameCore`；`DivineBeastsArena` 依赖两个基础模块和 `GameBackendClient`。任何新前台对象不得令 `GameCore` 或 `GameMoba` 反向依赖 `DivineBeastsArena`。

## 风险登记

| 编号 | 风险 | 证据 | 等级 | 本步骤决定 |
| --- | --- | --- | --- | --- |
| DR-01 | 三个状态体系互相覆盖 | `EDBALoginFlowState`、`EDBAFrontendStep`、`EDBAFrontendSessionState` 同时存在 | 高 | 后续以 LoginFlowState 为前台唯一状态，另外两个分别作兼容 Mapper 和会话摘要 |
| DR-02 | UI 入口集中且绕过 Layer | `UDBAGameUIManager` 直接 CreateWidget、RemoveFromParent、切输入模式 | 高 | 保留为兼容实例管理，逐步抽到 UI Layer；不直接重写所有 Widget |
| DR-03 | 角色资产双域 | Frontend 与 Lobby 路径同名 WBP 同时存在，Registry 是二进制 DataAsset | 高 | 全部不可删除，必须在 Editor 使用 Asset Registry/Reference Viewer 确认后决定权威路径 |
| DR-04 | Token 本地明文持久化 | `UDBAAccountSaveGame` 声明 SessionToken 和 RefreshToken | 高 | 后续设计 `UDBASecureTokenStorage`；审计阶段不删除，防止破坏自动登录兼容 |
| DR-05 | AccountService 过宽 | 登录、注册、角色 CRUD、Profile、SaveGame、Token 集于一个基类 | 高 | 新业务不再扩展它；通过 Adapter 分出 Auth/Roster/Draft，保留兼容表面 |
| DR-06 | Preview Actor 不够轻量 | 一个 Actor 含 Mesh、Camera、多灯光、雾、天空、后处理及静态资源路径方法 | 中 | 后续审计 Dedicated Server 创建条件和异步加载；未证实前不拆、不删 |
| DR-07 | `GameCore` 直接依赖 UMG/Slate | `GameCore.Build.cs` 含 UMG、Slate、SlateCore | 中 | 不新增应用模块反向依赖；未来可独立拆 UI 基类，但不属于本步骤 |
| DR-08 | GameMode 同时承担大厅和对局后端运行时 | `ADBAGameModeBase` 含 Pawn、训练怪、心跳、比赛结果方法 | 中 | 保持现状；Boot/Frontend 专用 GameMode 应在后续地图契约阶段定义 |
| DR-09 | 默认客户端未经过 Boot 地图 | `GameDefaultMap` 为 FrontendMap | 中 | 先落实状态和资产契约，再用 Editor 创建/保存 Boot 地图并改 Config |
| DR-10 | 现有自动化/负载脚本与仓库策略冲突 | `DBA_GameBackend/load-tests/*.js` 存在 | 中 | 记录但不删除，因不属于本步骤前台链且删除范围需要专门审计 |
| DR-11 | 旧资料描述两套选创角资产仍未合并 | `docs/Architecture/...2026-07-14.md` 已标记 CNT-02 | 中 | 本审计承接其结论，不能仅靠文件名判定 Legacy |
| DR-12 | 硬编码表现参数 | `FDBACharacterPresentationStageSpec` 直接给出灯光和相机数值 | 中 | 后续迁往 DataAsset/DeveloperSettings，当前不触发二进制资产重配 |
| DR-13 | 已启用插件链含弃用依赖 | UBT 报告 `VibeUE -> StructUtils`，后者已于 UE 5.5 弃用 | 低 | 不为前台审计调整插件；后续升级/插件收敛阶段统一处理 |

## 编译与资产验证边界

本步骤的文档修改不会更改 C++ 代码。Editor Target 编译是工程检查，不代表前台业务验收。蓝图父类丢失、Redirector 和二进制 DataAsset 引用无法由纯文件名搜索可靠判定，必须在后续由 Unreal Editor 进行可见人工审核：打开 `DA_DBA_UIFlowRegistry`、Reference Viewer、相关地图和四个流程 Widget，记录真实父类和引用关系后再实施资产迁移。

## 后续最小化行动

1. 定义 `EDBALoginFlowState` 的目标状态和合法异步转换，建立 `EDBAFrontendStep` Mapper。
2. 用 Editor 读取 `DA_DBA_UIFlowRegistry`，确认 Frontend/Lobby 同名 WBP 的实际引用者、父类和 Redirector 状态。
3. 设计 Token 从 `UDBAAccountSaveGame` 至平台安全存储的兼容迁移；迁移完成前不可丢失自动登录。
4. 仅在以上三项完成后，提取 Roster/Draft、UI Layer 和 Travel Coordinator 的最小 C++ 增量。
