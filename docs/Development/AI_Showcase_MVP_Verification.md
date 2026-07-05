# AI_Showcase MVP 验证记录

版本：v0.5
日期：2026-06-30
范围：`/Game/MCP_Generated/AI_Showcase`

## 目的

本记录用于沉淀 AI_Showcase 可交互 UI/VFX 样例的当前工程状态，避免后续重复进行无证据的编辑器尝试。后续迭代仍以 `docs/Development/ZodiacArena_UE5_8_Codex_总控提示词.md` 和仓库 `AGENTS.md` 的 Unreal / MCP 约束为准。

## 已创建资产

- `/Game/MCP_Generated/AI_Showcase/UI/WBP_MainMenu`
- `/Game/MCP_Generated/AI_Showcase/UI/WBP_GameHUD`
- `/Game/MCP_Generated/AI_Showcase/UI/WBP_InteractionPrompt`
- `/Game/MCP_Generated/AI_Showcase/BP_InteractiveProp`
- `/Game/MCP_Generated/AI_Showcase/Materials/M_InteractiveGlow`
- `/Game/MCP_Generated/AI_Showcase/Materials/MI_InteractiveGlow`
- `/Game/MCP_Generated/AI_Showcase/VFX/NS_InteractionBurst`
- `/Game/MCP_Generated/AI_Showcase/VFX/NS_InteractionBurst_V2`
- `/Game/MCP_Generated/AI_Showcase/VFX/NS_InteractionBurst_V3`
- `/Game/MCP_Generated/AI_Showcase/VFX/NSE_InteractionSpark`
- `/Game/MCP_Generated/AI_Showcase/Maps/L_AI_Showcase_Test`

## 当前状态摘要

- `BP_InteractiveProp` 已切换到 `NS_InteractionBurst_V3`。
- `L_AI_Showcase_Test` 中存在 `BP_InteractiveProp` 实例，位置为 `X=0, Y=0, Z=100`。
- 关键 Widget / Blueprint / Niagara 在此前已通过编译或验证。
- 当前新增了一组原生自动化回归测试，用于替代“只靠 PIE 记忆确认”的方式。
- `WBP_MainMenu` 与 `WBP_GameHUD` 的关键 Widget Tree 控件已进入 AI_Showcase 自动化回归，避免资产存在但主菜单/HUD 骨架丢失时误判通过。

## 已完成验证

### 编辑器内验证

- `BP_InteractiveProp` 编译通过。
- `WBP_MainMenu` 编译通过。
- `WBP_GameHUD` 编译通过。
- `WBP_InteractionPrompt` 编译通过。
- `NS_InteractionBurst_V3` 验证通过。
- `L_AI_Showcase_Test` 的 PIE smoke 已通过。

### 原生自动化回归

新增测试文件：

- `DBA_GameClient/Source/DivineBeastsArena/Private/Tests/DBAAIShowcaseTests.cpp`

覆盖项：

- `AssetsExist`
  - 校验 AI_Showcase 关键 Widget、Material、Niagara、Map 资产存在。
- `WidgetTreeContract`
  - 校验 `WBP_MainMenu` 暴露 `AIShowcaseMenu_TitleText`、`AIShowcaseMenu_StartButton`、`AIShowcaseMenu_OptionsButton`、`AIShowcaseMenu_QuitButton`。
  - 校验 `WBP_GameHUD` 暴露 `AIShowcaseHUD_HealthBar`、`AIShowcaseHUD_EnergyBar`、`AIShowcaseHUD_ScoreText`、`AIShowcaseHUD_MinimapRoot`、`AIShowcaseHUD_EventFeedBox`、`AIShowcaseHUD_SkillButton_0`。
  - 测试读取 `UWidgetBlueprintGeneratedClass::GetWidgetTreeArchetype()`，不修改 UMG 资产。
- `InteractivePropDefaults`
  - 校验 `BP_InteractiveProp` 默认变量值。
  - 校验组件模板存在。
  - 校验 `FX_Interact` 默认引用 `NS_InteractionBurst_V3`。
- `InteractionContract`
  - 校验 `Interact`、`ServerInteract`、`ResetInteractionCooldown` 函数存在。
  - 校验 `Interact` 和 `ResetInteractionCooldown` 保持无参调用契约。
  - 在临时自动化世界中生成 `BP_InteractiveProp`，验证交互入口可被安全调用。
- `MapPlacement`
  - 校验 `L_AI_Showcase_Test` 中存在 `BP_InteractiveProp`。
  - 校验其位置保持在 `X=0, Y=0, Z=100`。

### 本轮根因修正

- `NSE_InteractionSpark` 的实际资产类型是 `NiagaraStatelessEmitter`，不是 `NiagaraEmitter`。
- 自动化测试已改为：
  - 校验该资产可加载。
  - 校验其运行时类名为 `NiagaraStatelessEmitter`。

## 验证命令

### 本地构建

```powershell
Build.bat DivineBeastsArenaEditor Win64 Development `
  -Project=E:\work\Game\DivineBeastsArena\DBA_GameClient\DivineBeastsArena.uproject `
  -NoHotReloadFromIDE `
  -WaitMutex
```

结果：`Result: Succeeded`

### 自动化测试

```powershell
.\scripts\run-ai-showcase-automation.ps1
```

结果：

- `5 Test(s) Requested`
- `5 Test(s) Passed`
- `ExitCode=0`
- `logErrorCount=0`（统计窗口限定在 AI_Showcase 自动化测试会话内）

### 仓库内标准入口

为避免后续依赖临时命令历史，本仓库已新增：

- `scripts/run-ai-showcase-automation.ps1`
  - 作为 AI_Showcase Unreal 自动化回归的标准运行入口。
  - 使用 `UnrealEditor-Cmd.exe -ExecCmds="Automation RunTests DivineBeastsArena.Showcase.AIShowcase; Quit"` 直跑项目自动化，避免 UE 5.8 启动阶段 Core low-level smoke 日志污染项目测试结果。
  - 生成证据时会从 `Found N automation tests based on ...` 到 `TEST COMPLETE` 的项目自动化窗口统计 `logErrorCount` / `logWarningCount`。
- `scripts/test-ai-showcase-automation-runner.ps1`
  - 作为该入口的轻量 fixture 自检。
- `scripts/test-ai-showcase-widget-tree-contract.ps1`
  - 作为 AI_Showcase Widget Tree 自动化契约的轻量源码自检。
  - 要求 runner fixture 统计 `requestedTestCount=5` 与 `passedTestCount=5`，确保新增 `WidgetTreeContract` 不会被后续证据脚本漏计。
- `scripts/test-production-evidence-automation.ps1`
  - 已接入上述 fixture，自检总入口会覆盖该 runner。
- `scripts/run-unreal-evidence.ps1`
  - 已默认接入 AI_Showcase Unreal 自动化回归，使该演示样例进入 UE production evidence 链路。
  - 如遇特殊 runner 需要只跑联机证据，可显式传入 `-SkipAIShowcaseAutomation`。
  - 运行时会把 AI_Showcase 回归结果写入 `unreal/ai-showcase-automation-<RunId>.json`，供生产证据 manifest 收集。
- `.github/workflows/unreal-evidence.yml`
  - 已暴露 `run_ai_showcase_automation` 手动输入，默认开启。
  - 当该输入为 `false` 时，workflow 会向 `run-unreal-evidence.ps1` 传入 `-SkipAIShowcaseAutomation`。
- `scripts/collect-production-evidence.ps1`
  - 已识别 `unreal.ai_showcase_automation` 证据类别，匹配 `ai-showcase-automation*.json`。
  - 当前生产门禁要求 `automationReady=true`、`logErrorCount=0`、`requestedTestCount=5`、`passedTestCount=5`；旧的 4/4 AI_Showcase 证据会被判定为 `incomplete`。
- `scripts/diagnose-release-blockers.ps1`
  - 当 `unreal.ai_showcase_automation` 缺失或不完整时，会生成指向 `scripts/run-ai-showcase-automation.ps1` 的可执行 blocker action。
  - blocker action 会列出 `automationReady`、`logErrorCount`、`requestedTestCount`、`passedTestCount` 等 observed reasons，并要求重新生成 `requestedTestCount=5`、`passedTestCount=5` 的当前自动化证据。
- `scripts/write-release-input-template.ps1`
  - 会把 AI_Showcase blocker action 中的 `release run id` 写入输入模板，并生成 `run-ai-showcase-automation` 建议命令。
  - 当 release 没有 blocker 时，不会生成带未声明占位符的 suggested command，避免全绿发布路径被派生模板误拦截。
  - 当某个建议命令所需输入不齐备时，会跳过该命令，避免生成引用未声明占位符的无效命令。
- `scripts/validate-release-input-template.ps1`
  - 会校验 `suggestedCommands.usesInputs` 中的每个输入名都存在于 `inputs` 行，避免建议命令元数据漂移。

推荐日常使用命令：

```powershell
.\scripts\run-ai-showcase-automation.ps1
```

推荐生产证据链使用命令：

```powershell
.\scripts\run-unreal-evidence.ps1 -BaseUrl "http://localhost:8080" -UsePackagedServer -EvidenceRoot .\Artifacts\ProductionEvidence -RunId local-ue-evidence
```

### 真实证据链路验证

本地已执行独立 AI_Showcase 证据生成：

```powershell
.\scripts\run-ai-showcase-automation.ps1 -EvidenceDir .tmp\ai-showcase-real-evidence -RunId ai-showcase-real-20260629
```

产物：

- `.tmp\ai-showcase-real-evidence\unreal\ai-showcase-automation-ai-showcase-real-20260629.json`
- `automationReady=true`
- `logErrorCount=0`
- `requestedTestCount=4`
- `passedTestCount=4`
- `testFilter=DivineBeastsArena.Showcase.AIShowcase`
- `platform=Win64`
- `configuration=Development`

随后执行：

```powershell
.\scripts\collect-production-evidence.ps1 -EvidenceRoot .tmp\ai-showcase-real-evidence -ReleaseId ai-showcase-real-20260629
```

结果：`unreal.ai_showcase_automation` 在 `production-evidence-manifest.json` 中为 `present`，并引用 `unreal/ai-showcase-automation-ai-showcase-real-20260629.json`。

### 2026-06-30 Widget Tree 证据刷新

本地已执行：

```powershell
.\scripts\run-ai-showcase-automation.ps1 -EvidenceDir Artifacts\ProductionEvidence -RunId ai-showcase-widget-tree-20260630b
```

产物：

- `Artifacts\ProductionEvidence\unreal\ai-showcase-automation-ai-showcase-widget-tree-20260630b.json`
- `automationReady=true`
- `logErrorCount=0`
- `logWarningCount=0`
- `requestedTestCount=5`
- `passedTestCount=5`
- `testFilter=DivineBeastsArena.Showcase.AIShowcase`

说明：日志中仍有 UE 启动阶段 Core low-level smoke error 行，但这些行出现在 `Found 5 automation tests based on 'DivineBeastsArena.Showcase.AIShowcase'` 之前，当前 runner 的项目自动化窗口统计会正确忽略它们。

2026-06-30 已刷新 `Artifacts\ProductionEvidence\production-evidence-manifest.json`、`release-readiness-report.json` 与 `release-blocker-actions.json`。当前 manifest 将 `unreal.ai_showcase_automation` 归为 `present`，并引用 5/5 WidgetTree 自动化证据；剩余 blocker 均为真实 CDN、公开包路径、签名证书、timestamp 等外部发布输入。

同日新增 `scripts/validate-release-blockers-external-only.ps1`，用于验证当前剩余 blocker 是否全部由外部发布输入阻塞；如果 `unreal.ai_showcase_automation` 等本地自动化缺口重新出现在 blocker action 中，该校验会失败，避免把可本地修复的问题误归类为“外部输入暂缺”。

该校验已接入 `scripts/production-preflight.ps1 -CollectEvidence` 流程，并且 `scripts/collect-production-evidence.ps1` 会排除 `release-blockers-external-only-validation.json` 这类派生报告，避免它被反向索引为生产证据。

## 相关输出位置

- AI_Showcase 自动化日志：
  - `Artifacts\Logs\ai-showcase-automation-<RunId>.log`

## 后续建议

1. 继续把 AI_Showcase 的交互输入链路做成可自动验证项，而不只依赖 overlap 与 smoke。
2. 在 Motion Graphics / UMGBridge 工具可用时，再补 `SEQ_ShowcaseIntro` 或 UMG 动画导出链路。
3. 若后续继续演化 AI_Showcase 资产，优先同步更新 `DBAAIShowcaseTests.cpp`，确保回归测试始终覆盖关键路径。
