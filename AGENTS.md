# Repository Guidelines

## 指导开发纲领
- `docs/Development/ZodiacArena_UE5_8_Codex_总控提示词.md` 是本项目长期开发的纲领性文件，后续系统设计、代码生成、文档同步、验证与分阶段交付均应以该文档为指导。
- 执行具体任务时，优先级为：用户当次明确指令、本文件中的安全与 MCP 工作约束、总控提示词、各模块现有局部规范。
- 当总控提示词要求与当前仓库实际结构不一致时，先审查现有结构并按最小可验证增量推进，不盲目覆盖已有文件。

## 全局自主执行策略
- PolicyId: `DBA.Agent.DirectExecution`
- 本策略为全仓库、全模块、全后续任务的默认协作约束；后续 Codex、MCP、代码代理和自动化 Agent 不再反复问询常规执行细节，应在已有目标、当前仓库状态和安全边界内直接推进。
- 后续任务默认“直接执行”：除非触及破坏性操作、生产写入、密钥证书、项目全局设置、二进制资产直改或平台强制确认，不再先询问是否继续，不以等待确认替代可验证推进。
- 默认非交互执行：可验证的读取、文档同步、源码小步修改、测试/构建/脚本验证和看板更新均直接推进；不得把普通工程推进项升级为确认问题或等待用户回复。
- 除非用户明确要求暂停、只输出方案或等待确认，Agent 不再询问“是否继续”“是否进入下一阶段”或“是否执行常规验证”；默认直接进入下一可验证步骤，并在完成后用结果、证据和风险汇报。
- 默认持续推进：常规阶段切换、常规验证、文档同步、源码小步修改和本地证据补齐均不是确认点；Agent 必须直接执行并验证，不能将计划输出、等待确认或反复询问作为默认完成状态。
- 当用户已经给出方向、目标、阶段或长期任务时，默认理解为授权执行可验证的最小增量；Agent 应主动读取当前状态、制定内部步骤、实施改动、运行验证并汇报结果，而不是停留在方案确认或重复询问。
- 对不依赖外部输入的工程推进项，优先直接执行，包括文档同步、源码小步修复、测试契约补齐、构建/脚本验证、中文日志迁移、自动化证据加固和阶段看板更新。
- 外部发布输入暂缺时，不反复索要真实 CDN、包路径、签名证书、时间戳、生产密钥或正式环境凭据；应明确跳过该外部依赖项，继续推进本地可验证的工程、自动化、测试和文档闭环。
- 只有在更高优先级规则或运行平台强制要求时才中断执行并请求确认，例如破坏性删除/重置、密钥或证书处理、生产环境写入、付费外部操作、覆盖用户未确认成果、修改项目全局设置、直接编辑二进制资产，或当前信息不足会造成不可逆风险。
- 如果遇到可恢复的权限、临时文件、锁定、构建缓存或本地环境问题，优先诊断并直接采用安全的最小修复继续验证；不得把可自行处理的环境摩擦升级为反复问询。
- 汇报方式应以结果和证据为主：说明改了哪些文件、运行了哪些命令、哪些通过、哪些因外部输入缺失被跳过，以及下一步可继续推进的具体项。

## 全局 C++ 逻辑实现策略
- 本策略为全仓库、全模块、全后续任务的默认架构约束；除非用户在当次任务中明确书面豁免，否则所有逻辑相关需求都必须按 C++ 实现路线推进。
- 全项目所有逻辑相关实现均要求使用 C++，包括但不限于 Gameplay、GAS、网络复制与校验、AI、任务、经济、背包、商城、匹配、结算、UI 状态驱动、资源加载、运营活动、日志埋点、自动化流程和跨系统编排。
- Blueprint 仅作为参数配置、DataAsset/资源引用配置、表现层事件承接、动画/VFX/SFX/UI 绑定和设计师调参入口；不得承载权威 Gameplay 规则、伤害/治疗/护盾/控制/冷却/消耗结算、网络校验、状态机、业务流程或跨系统编排。
- 新增系统默认先设计 C++ 基类、接口、组件、Subsystem、数据结构和测试，再暴露必要的 `UPROPERTY`、`UFUNCTION`、DataAsset 或配置入口给 Blueprint 调参。
- 发现已有 Blueprint 承载逻辑时，不直接扩大蓝图逻辑；应按最小可验证增量迁移到 C++，保留蓝图的配置、表现和资源引用职责。
- 当后续任务要求创建 Blueprint、UMG、动画、VFX 或交互样例时，必须先判断是否包含逻辑；凡涉及状态流转、输入处理、校验、冷却、结算、网络、数据持久化或跨系统调用的部分，一律创建或复用 C++ 父类/组件/Subsystem 实现，Blueprint 仅继承和配置参数。
- 若当前 MCP 工具只能生成 Blueprint 图逻辑而无法生成对应 C++ 逻辑层，应标记该逻辑部分为“不可按项目策略自动执行”，输出 C++ 实现计划或源码改动方案，不得用 Blueprint 逻辑替代。

## 全局 DataAsset 与禁止硬编码策略
- PolicyId: `DBA.DataAsset.NoHardcoding`
- 本策略为全仓库、全模块、全后续任务的默认架构约束；项目所有代码不写硬编码，运行数据、表现数据、资源引用、UI 文案、运营配置和平台差异均应通过数据资产或配置源驱动。
- Gameplay 数值、技能参数、冷却、消耗、伤害系数、治疗/护盾/控制参数、角色成长、掉落奖励、商城价格、任务条件、活动规则、匹配/结算参数、平台差异、UI 文案、图标、VFX/SFX/动画/材质/纹理引用、地图/模式配置和运营开关不得直接写死在 C++、Blueprint、脚本或前端代码中。
- UE 侧优先使用 `PrimaryDataAsset`、`DataAsset`、`DataTable`、`DeveloperSettings`、`GameplayTag`、软引用、Asset Manager 与本地化表；后台、前端、启动器与自动化脚本应使用配置文件、数据库、环境变量、模板输入或清单文件承载可变数据。
- C++ 负责读取、校验、缓存和应用数据，并提供缺失字段、非法范围、资源软引用失效和版本不兼容的中文错误日志；Blueprint 只配置数据资产和资源引用，不复制一份独立逻辑或硬编码表。
- 允许保留枚举、协议字段名、测试 fixture、常量键名、编译期安全边界和无法数据化的低层技术常量；但新增业务/表现/运营可变值时，必须优先设计数据资产或配置入口。
- 发现已有硬编码时，不继续扩散；应按最小可验证增量迁移为数据资产、DataTable、DeveloperSettings、配置文件或后端数据，并补充自动化校验。

## 全局 UI 事件更新与异步接口策略
- PolicyId: `DBA.UI.EventAsync`
- 本策略为全仓库、全模块、全后续任务的默认架构约束；所有用户界面更新必须使用事件驱动，不得依赖 Tick 轮询、定时扫表或蓝图临时逻辑刷新核心 UI 状态。
- UE UI 更新应优先通过 C++ Delegate、Multicast Delegate、ViewModel、FieldNotify、MVVM、OnRep、GameplayCue、ASC Attribute Delegate、Subsystem 事件、WidgetController 事件或显式数据变更事件驱动。
- 前端、管理后台、网站和启动器 UI 应通过状态管理、响应式数据流、事件总线、订阅回调或异步请求完成更新，不得用固定间隔轮询替代明确事件，除非该轮询是有超时、退避和退出条件的基础设施层降级方案。
- 所有外部服务、后端接口、平台 SDK、文件/网络 IO、资源加载、MCP/Editor 接口、数据库访问、支付、登录、匹配、商城、运营、遥测和自动化远程调用必须采用异步，不得阻塞 GameThread、UI 线程、HTTP 请求主路径或构建主流程。
- 异步接口必须具备完成、失败、超时、重试、取消、降级和中文错误上报路径；UI 只消费异步结果事件，不直接在表现层执行阻塞访问。
- 若当前工具只能生成同步接口或 Tick 刷新 UI，应标记为“不符合项目策略”，输出异步 C++/服务层实现计划，不得用同步阻塞或轮询逻辑替代。

## 全局中文日志与信息输出策略
- PolicyId: `DBA.Log.ChineseOutput`
- 本策略为全仓库、全模块、全后续任务的默认架构约束；项目所有日志、信息打印、错误提示、诊断报告、自动化脚本输出和开发者可见调试信息均使用中文输出。
- UE 侧 `UE_LOG`、`ensureMsgf`、`checkf`、屏幕调试信息、Automation Test 失败信息、Editor 自动化输出、MCP 执行报告和命令行诊断必须写中文说明，并使用 `TEXT("中文内容")` 包裹中文字符串。
- 后端、前端、启动器、脚本、CI、部署、运维、测试报告和 README/文档中的人类可读日志与错误信息应使用中文；不得新增英文占位日志、英文 TODO 式提示或只对开发者可见的英文失败说明。
- 对外协议字段名、枚举 Token、GameplayTag、资产路径、类名、文件名、第三方 SDK 原文错误码、HTTP 标准短语和必须保持英文的机器可读键名可以保留英文，但需要在上层日志、报告或 UI 错误提示中给出中文解释。
- 发现已有英文日志或信息打印时，不扩大使用范围；新增或触碰相关代码时应逐步替换为中文，并补充必要上下文，确保 Dedicated Server、客户端、后台和自动化流水线都能直接读懂失败原因。

## 项目结构与模块组织
- `DBA_GameClient/`：UE5 客户端与 Dedicated Server。源码在 `Source/`，测试位于 `Source/**/Tests/`（如 `DBA_GameClient/Source/GameCore/Private/Tests/`）。
- `DBA_GameBackend/`：.NET 解决方案 `GameBackend.sln`，包含 API、Worker、ServerManagement 与对应测试项目（`Game.*.Tests`）。
- `DBA_GameAdmin/`：Angular 18 管理后台。
- `DBA_GameWebsite/`：Next.js 前端网站。
- `DBA_GameLauncher/`：Tauri + React 启动器，Rust 后端在 `src-tauri/`。
- `docs/`：方案、架构和发布文档；`scripts/`：预检与冒烟检查脚本。

## 构建、测试与开发命令
- 后端：
  - `cd DBA_GameBackend; dotnet build GameBackend.sln`
  - `dotnet test GameBackend.sln --no-build`
- Admin：
  - `cd DBA_GameAdmin; npm ci; npm run build`
- Website：
  - `cd DBA_GameWebsite; npm install; npm run build`
- Launcher：
  - `cd DBA_GameLauncher; npm install; npm run build`
  - `cargo check --manifest-path src-tauri/Cargo.toml`
- 全量发布前（按需）：
  - `.\scripts\production-preflight.ps1`
  - `.\scripts\production-smoke-backend.ps1 -BaseUrl "https://api.example.com"`

## Unreal 与 MCP 工作约束
- 不要直接修改 `.uasset` / `.umap` 二进制资产文件。
- 全项目所有 Gameplay、GAS、网络、AI、任务、经济、背包、商城、匹配、结算、UI 状态驱动、资源加载、运营与自动化等逻辑相关实现必须使用 C++；Blueprint 只允许作为参数配置、DataAsset/资源引用配置、表现层事件承接、动画/VFX/SFX/UI 绑定和设计师调参入口。
- 禁止在 Blueprint 中实现权威 Gameplay 规则、伤害/治疗/护盾/控制/冷却/消耗结算、网络校验、状态机、业务流程或跨系统编排；若发现已有蓝图承载逻辑，应逐步迁移为 C++，蓝图保留配置与表现。
- MCP 自动化创建 UI、VFX、Actor、交互物、技能或样例资产时，逻辑层必须落在 C++；Blueprint 资产只能作为 C++ 类的配置、资源引用、表现绑定或设计师调参外壳。
- UMG 修改统一使用 Motion Graphics MCP（`UmgMcp`）。
- Blueprint 修改统一优先使用 Monolith。
- Editor 控制统一使用 Unreal MCP。
- 常规可验证增量默认直接执行；破坏性操作、生产写入、密钥证书处理、项目全局设置修改、直接二进制资产编辑或运行平台强制确认的操作，必须遵守更高优先级安全规则。
- 任何 Blueprint 修改后必须执行 Compile。
- 任何资产修改后必须执行 Save。
- 所有 Editor、UMG、Blueprint、资产修改必须保持 Undo Transaction。

## Unreal 修改完成检查
- Compile。
- Save。
- PIE Smoke Test。
- Git Diff。

## 代码风格与命名规范
- 统一使用空格缩进，项目中推荐 2 空格；确保文件末尾换行、去除行尾空格（`DBA_GameAdmin/.editorconfig` 中有该约定，其他模块按同等标准执行）。
- C++/UE：类与结构体使用 `PascalCase`，函数与变量采用语义化命名；优先复用已有模块名 `DBA_*`。
- C#：遵循 .NET 命名约定（`PascalCase` 类型与方法，`camelCase` 局部变量）。
- TypeScript/React/Angular：`PascalCase` 组件，`camelCase` 函数/变量，路径命名与文件导出保持一致。
- 目录与文件命名建议与模块一致，如 `Game.Api.Tests`、`game-list.tsx`、`*Tests.cs`。

## 测试规范
- 测试框架以 .NET `dotnet test` 为主，优先覆盖新增 API、服务、集成与后端业务逻辑。
- 测试命名建议：`XxxTests` 项目名，测试类采用 `XxxFeatureTests`，方法使用行为描述如 `Should_Return_Fallback_When...` 或中文语义对应英文。
- UE 与客户端测试文件命名保持 `*Tests.cpp`，放在对应模块 `Tests` 文件夹。
- 提交前必须在涉及模块上至少运行一次对应构建 + 测试命令，若因环境限制无法跑，请在 PR 中明确说明。

## 提交与 PR 规范
- 当前历史记录偏向简短英文摘要（如 `Fix ...`、`Add ...`），建议继续使用可读且祼句式的祈使式标题，并可采用 `type(scope): 短描述` 进行统一。
- PR 需包含：变更范围、验证命令输出、涉及配置项、以及 UI 改动截图（Admin/Website/Launcher）。
- 涉及环境变量、服务地址、UE 引擎路径等配置，不得提交密钥；优先使用本地环境文件或 CI Secret。
- 代码和资源改动应保持与各模块职责清晰分离：客户端、后台、管理后台、网站、启动器分步评审，避免一次 PR 跨越多个子系统。
