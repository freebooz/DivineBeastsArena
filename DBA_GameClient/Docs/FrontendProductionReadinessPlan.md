# 游戏前端生产实践缺失分析与执行计划

本文面向 `DBA_GameClient`，目标是把当前 UE 客户端从“功能可演示”推进到“可预生产验收、可发布、可持续运营”的工程状态。

## 当前基线

- 已具备登录、游客登录、开发账号登录、角色列表、角色创建、角色选择和进入大厅的流程骨架。
- 已接入 `GameBackendClient` 插件，包含 Auth、Player、Config、Room、Match、Session、Runtime、Crash、Telemetry 等客户端服务封装。
- 已具备大厅 UI 管理器、登录/选角/创角 UI、主大厅控制器、设置面板、背包面板、匹配/房间/ReadyCheck 等基础界面类。
- 已具备 WASD/方向键移动、鼠标左右键视角、滚轮镜头距离、ESC/B 面板切换、技能快捷键和技能栏点击入口。
- 已具备 GAS、生肖角色、技能数据、投射物、伤害计算、GameplayCue、Niagara/Cascade VFX、怪物血条和大厅训练怪。
- 已具备 Android 运行时配置、触摸输入桥、移动端渲染配置和部分 cook 目录配置。

## 主要缺失

### 1. 生产登录与账号安全

- `SubmitDebugLogin` 仍是可见前端入口，生产包需要构建期开关隐藏，并禁止访问后端 `dev-login`。
- 客户端仍保留 Mock 账号服务和 Mock 兜底说明；虽然默认 `bAllowMockFallback=false`，但生产构建需要自动校验不允许开启。
- Refresh Token 当前落在 SaveGame 明文字段，生产环境需要改为平台安全存储或至少做本机加密封装。
- 登录错误提示需要建立统一中文错误码映射，避免直接展示后端技术错误。
- 缺少封禁、维护、最低版本、服务器满载、网络超时、Token 过期等完整 UI 状态。

### 2. 前端状态机与网络容错

- 登录状态、前台会话、主大厅、匹配、进服 Travel 分散在多个类中，缺少统一“前端会话状态图”和失败恢复策略。
- Room/Match/Session 当前以轮询和局部状态为主，缺少 SignalR 或长连接事件对接。
- 断线重连、进服失败重试、Dedicated Server 不可达、Session 过期、玩家被踢出等流程未形成完整闭环。
- HTTP 客户端有超时和重试，但缺少指数退避、请求取消、离线队列、幂等请求追踪和网络质量提示。
- 客户端 TraceId 已发送，但缺少和本地日志、崩溃日志、用户反馈的一键关联。

### 3. UI 完整度与蓝图资源治理

- C++ UI 基类较完整，但蓝图布局、纹理、字体、声音、输入焦点和适配规则需要做资产级清单验收。
- 登录、选角、创角、加载、主大厅、设置、背包、匹配、战斗 HUD 还缺少统一 UI Design Token 和响应式规则。
- 部分文本存在历史编码痕迹或临时文案风险，需要全量本地化表治理。
- 缺少通用弹窗、Toast、二次确认、全屏遮罩、进度条、错误页、维护公告、版本更新提示、隐私协议确认等生产常用 UI。
- 技能栏、Buff/Debuff、单位框、目标框、战斗播报等 Arena HUD 已有类，但需要和真实 ASC 属性、冷却、资源、目标状态绑定验收。

### 4. 输入、相机与可访问性

- 当前仍绑定方向键移动，之前需求提到方向键不应控制角色选择，生产需要明确键位矩阵并按模式隔离输入上下文。
- 项目配置启用 Enhanced Input，但实际大厅控制器仍以旧 `InputComponent->BindKey/BindAxis` 为主，需要迁移到 Enhanced Input Mapping Context。
- 鼠标视角逻辑已经多次调节，生产需要提供用户可配置灵敏度、反转 Y 轴、镜头距离、镜头碰撞、自动跟随和保存设置。
- 移动端触摸仅有基础桥接，缺少虚拟摇杆/技能按钮/镜头区域/误触保护/安全区适配的完整体验。
- 需要键鼠、手柄、触摸三套输入方案和可重绑定设置。

### 5. GAS、技能表现与战斗一致性

- `DBADamageCalculator` 仍有直接修改 AttributeSet 的临时实现，应改为权威 GameplayEffect/ExecutionCalculation。
- 技能投射物表现已支持 VFX/SFX 和 GameplayCue，但需要把十二生肖每个技能的飞行、命中、施法、持续、消散效果完全数据化。
- `DBASkillVFXManager::GetSkillVFXPath/GetSkillSFXPath` 当前为空，需要改为 DataTable/Primary Asset 驱动。
- 需要将技能施法动画、Montage Section、Cast Point、Cancel Window、Projectile Spawn Socket、Hit Cue 统一进技能配置。
- 客户端预测、服务端权威命中、回滚校正、重复命中保护、AOE 查询、碰撞通道矩阵尚未生产化。

### 6. 资源、打包与性能

- `DefaultGame.ini` 仍使用 Development 打包配置，生产需要 Shipping/Distribution profile。
- `DefaultEngine.ini` 和 `DefaultGame.ini` 都配置了 AlwaysCook，存在重复和过宽 cook 风险，需要按 PrimaryAssetLabel/Chunk 重整。
- Android 配置已存在，但缺少真实机型性能预算、纹理尺寸、Niagara LOD、材质复杂度、内存池和帧率目标。
- 需要建立客户端包体体积、首屏加载时间、进入大厅时间、进服时间、CPU/GPU 帧耗、网络流量的验收指标。
- 缺少自动化资源审计：未引用资源、重定向器、超大贴图、未压缩音频、同步加载热点、Shader 编译风险。

### 7. 观测、崩溃与运营反馈

- 客户端已有 Telemetry/Crash 服务封装，但需要接入真实上传端点和隐私开关。
- 需要统一本地日志目录、日志滚动、TraceId、玩家 ID、SessionId、地图、版本号、设备信息。
- 需要游戏内反馈入口，自动附加日志片段、TraceId、截图开关和玩家授权。
- 缺少客户端运营开关：维护公告、灰度配置、热修复配置、强更提示、活动入口、公告弹窗。

### 8. 自动化测试与发布流程

- 当前机器未验证 UE C++ 编译，生产流程需要 UBT Build、Cook、Package、Run Automation Tests。
- 已有若干 Python/Automation 测试脚本，但缺少统一入口和 CI 工作流。
- 缺少前端端到端脚本：启动客户端、登录、创建角色、匹配、进服、释放 Session、断线重连。
- 缺少截图回归、UI 层级检查、关键资产存在性检查、Cook 后启动检查。
- 需要客户端版本、资源 manifest、Launcher manifest、后端 minClientVersion 的发布一致性校验。

## 生产化执行计划

### P0：发布阻塞项

1. 建立 `Shipping` 客户端配置 Profile，关闭 DebugLogin、MockFallback、开发者工具和不必要日志。
2. 把 Refresh Token 从明文 SaveGame 迁移到安全存储封装，并提供旧存档迁移和清理。
3. 完成真实后端登录、角色列表、创建角色、选择角色、进入大厅、匹配、获取连接、进服的端到端验收。
4. 完成真实 Dedicated Server 下的 Runtime register/ready/heartbeat/player-joined/player-left/match-ended 验收。
5. 修正伤害应用为 GameplayEffect 权威流程，移除直接改 AttributeSet 的临时路径。
6. 为十二生肖技能建立完整数据表：施法动画、投射物、飞行 VFX、命中 VFX、CUE、音效、伤害参数、冷却、资源消耗。

### P1：UI 与体验闭环

1. 整理 UI Design Token：字体、字号、颜色、按钮、面板、背景、进度条、错误提示、禁用态、加载态。
2. 补齐生产 UI：维护公告、版本过低、服务器繁忙、网络重试、断线重连、进服失败、通用确认框、Toast。
3. 将大厅、背包、设置、匹配、ReadyCheck、战斗 HUD 与真实数据绑定，清理 Stub 数据。
4. 输入系统迁移到 Enhanced Input，并按 Login/Lobby/Arena/Menu/Mobile 五类 Mapping Context 隔离。
5. 提供玩家设置持久化：鼠标灵敏度、Y 轴反转、镜头距离、画质、音量、语言、按键绑定。

### P2：性能与移动端适配

1. 建立 Android/Windows 两套 Scalability Profile 和 DeviceProfile，定义 30/60 FPS 目标。
2. 对 Niagara、材质、贴图、音频、骨骼动画建立 LOD 和预算规则。
3. 整理 AlwaysCook，改为 PrimaryAssetLabel/Chunk 管理，降低包体和同步加载风险。
4. 建立资源审计脚本：大贴图、未压缩音频、无 LOD SkeletalMesh、重定向器、未引用资产。
5. 用真实设备记录首屏、登录、进大厅、进服、团战技能释放的性能基线。

### P3：观测与运营能力

1. 接入真实 Telemetry/Crash/Feedback API，默认尊重隐私设置。
2. 本地日志加入 TraceId、PlayerId、SessionId、Build、Map、NetworkState，并支持一键打包。
3. 客户端读取后端 Config Bundle，支持维护公告、灰度开关、活动入口、技能配置版本。
4. 增加用户反馈 UI，支持分类、描述、邮箱、日志附加和提交状态。
5. 建立线上问题定位流程：玩家反馈 ID -> TraceId -> 后端日志 -> Dedicated Server 日志 -> 客户端日志。

### P4：自动化验收与发布

1. 增加 `Scripts/BuildClient.ps1`、`Scripts/CookClient.ps1`、`Scripts/PackageClient.ps1` 和统一参数。
2. 增加 UBT 编译、Cook、Package、Automation Tests、资产审计、配置审计的 CI 工作流。
3. 增加客户端 smoke test：启动、登录、创建角色、选择角色、匹配、进服、退出。
4. 增加 UI 截图回归和关键 Widget 层级验证。
5. 增加版本一致性检查：客户端版本、Launcher manifest、后端 minClientVersion、CDN SHA256。

## 建议执行顺序

1. 先做 P0：生产配置隔离、真实后端闭环、真实 Dedicated Server 闭环、GAS 伤害权威化。
2. 再做 P1：UI 补全、Stub 清理、输入系统正规化、设置持久化。
3. 同步推进 P2：打包资源治理、移动端预算、性能基线。
4. 完成 P3：观测、崩溃、反馈和运营配置。
5. 最后固化 P4：CI、Cook、Package、自动化验收和发布一致性。

## 验收标准

- Shipping 包不能出现 DebugLogin、MockFallback、开发账号入口和明文敏感 Token。
- 无后端或后端异常时，客户端必须显示可理解错误和重试路径，不能静默进大厅。
- 十二生肖所有技能均有施法动画、飞行/命中粒子、音效、GameplayCue 和服务端权威伤害。
- Windows 客户端可以通过 Launcher 下载、校验、修复、启动，并能完成登录到进服闭环。
- Android 客户端在目标机型达到稳定帧率、UI 安全区正确、触摸操作可用。
- CI 能执行编译、Cook、Package、自动化测试、资源审计和配置审计。
