# Development Documents

## Leadership Guide

- [ZodiacArena UE5.8 纲领性总控文档](./ZodiacArena_UE5_8_Codex_纲领性总控文档.md)
  当前长期工程化推进的纲领版本，定义阶段目标、执行约束和交付节奏。
- [ZodiacArena UE5.8 总控提示词执行摘要](./ZodiacArena_UE5_8_Codex_总控提示词_执行摘要.md)
  日常执行时优先查看的简版检查清单。
- [ZodiacArena UE5.8 总控提示词](./ZodiacArena_UE5_8_Codex_总控提示词.md)
  项目级权威约束全文，覆盖三层架构、编码规范、服务端权威和阶段目标。
- [ZodiacArena 阶段交付看板](./ZodiacArena_阶段交付看板.md)
  当前 P0 到 P4 的完成状态、证据、阻塞项和下一步动作。

## Standards

- [代码规范](./代码规范.md)
- [中文编码规范](./中文编码规范.md)
- [目录结构规范](./目录结构规范.md)
- [三层架构设计](../Architecture/三层架构设计.md)
- [总体架构设计与审查报告](../Architecture/总体架构设计与审查报告.md)
- [三层 UE5 Module / Plugin 结构与 Build.cs 依赖设计](../Architecture/三层UE5模块与Build依赖设计.md)
- [Project Root AGENTS](../../AGENTS.md)

## Operation Scripts

- `scripts/start-local-backend-compose.ps1`: starts the local Docker Compose backend in Development mode for UE online validation, runs EF migrations by default, and keeps Production server executable checks strict.

- `scripts/start-local-ue-validation.ps1`: 本地联机验证流程，覆盖访客登录、房间、会话、服务器分配、Dedicated Server 启动与双客户端拉起。
- `scripts/diagnose-local-ue-online-readiness.ps1`: 只读检查 Docker、Compose、backend health、UE 路径和本地 server 包，定位联机前置条件阻塞。
- `scripts/smoke-unreal-dedicated-server.ps1`: Focused dedicated server smoke test for map boot and runtime health.
- `scripts/validate-unreal-baseline-entrypoints.ps1`: 检查共享日志通道、基础 DataAsset 基类、角色构建摘要、ArenaGame fixed skill group 行身份、固定技能组生成器兜底身份保护和原生 GameplayTag 注册入口是否仍在正确层级。
- `scripts/validate-unreal-moba-foundation.ps1`: 检查 GameMoba 层 GAS 基类、HUD WidgetController 数据边界、UMG 基类和 Build.cs 依赖，确保通用 MOBA 基础不耦合后端服务。
- `scripts/diagnose-fixed-skill-group-datatable.ps1`: 只读检查 `/Game/DBA/Data/Tables/DT_FixedSkillGroups` 真实 DataTable 资产，并在资产存在时运行 `DivineBeastsArena.GameDBA.Data.FixedSkillGroup.AssetRows` Editor automation 验证 60 行与行身份。
- `scripts/write-fixed-skill-group-source-csv.ps1`: 生成并验证 `DBA_GameClient/Content/DBA/Data/Tables/Source/DT_FixedSkillGroups.csv`，作为后续通过 Editor/MCP 创建 `DT_FixedSkillGroups` DataTable 的受控导入源。
- `scripts/import-fixed-skill-group-datatable.ps1`: 受控导入入口，默认可用 `-CommandOnly` 验证命令；实际执行会通过 `scripts/unreal/import_fixed_skill_group_datatable.py` 仅写入 `/Game/DBA/Data/Tables/DT_FixedSkillGroups`。
- `scripts/validate-unreal-module-boundaries.ps1`: 检查 `GameCore -> GameMoba -> DivineBeastsArena` 依赖方向和源码 include 边界。
- `scripts/validate-unreal-source-guardrails.ps1`: 检查 Unreal 源码基础护栏，包括禁止 `LogTemp`、禁止 include `Private` 目录、客户端表现依赖必须放在非 Server guard 内，以及 Dedicated Server `player-joined` 构筑摘要、Session travel URL 和 URL admission 契约不能漂移。
- `scripts/test-unreal-source-guardrails.ps1`: 使用临时 fixture 验证 source guardrail 能抓到未保护的客户端表现依赖、Runtime 构筑摘要缺字段、Session travel / URL admission fixture 缺口，并接受正确的非 Server guard 与稳定构筑摘要写法。
- `scripts/test-dedicated-server-url-build-summary-admission-contract.ps1`: 独立检查 Dedicated Server URL admission 的源码契约，确保 travel URL 中的 `DBAZodiac`、`DBAElement`、`DBAFiveCamp`、`DBAFixedSkillGroupId` 会经过 URL decode、稳定名归一化、FixedSkillGroup 重新计算和缺字段/篡改拒绝。
- `scripts/test-runtime-match-lifecycle-contract.ps1`: 独立检查 Runtime 对局生命周期和结算 handoff 契约，覆盖后端 `match-started` / `match-ended` / `matches/results`、SettlementService 幂等结算，以及 UE RuntimeService 的 `NotifyMatchResults` 提交路径。
- `scripts/collect-production-evidence.ps1`: 汇总生产证据并生成 `production-evidence-manifest.json`；UE online validation 和 AI_Showcase automation 不再只按文件存在判定，必须分别证明双客户端联机日志语义，以及 `automationReady=true` 且 `logErrorCount=0`。
- `scripts/production-preflight.ps1`: 仓库级预检入口，覆盖后端、前端、启动器、Docker、Unreal 构建与联机验证分支。
- `scripts/test-production-evidence-automation.ps1`: 生产证据自动化总自测，覆盖发布 fixture、UE/AI 证据语义 fixture、Unreal source guardrail fixture、baseline validator 语法解析、PowerShell 语法解析和 workflow YAML 解析。

## Unreal Automation Notes

Frontend flow automation that should not depend on a running backend can use
`-DBAForceMockAccount` or `-DBAAccountMode=mock` with a unique
`-DBASaveSlotSuffix=...`. This keeps local/offline regression tests
deterministic while the normal online path remains available for backend and
Dedicated Server validation.

## Recommended Commands

```powershell
.\scripts\production-preflight.ps1 -SkipUnrealOnlineValidation
.\scripts\production-preflight.ps1 -SkipNode -SkipDocker -SkipCargo -SkipUnrealOnlineValidation
.\\scripts\\start-local-backend-compose.ps1
.\\scripts\\diagnose-local-ue-online-readiness.ps1
.\scripts\start-local-ue-validation.ps1 -BaseUrl "http://localhost:8080" -SkipClientLaunch
.\scripts\start-local-ue-validation.ps1 -BaseUrl "http://localhost:8080" -ClientValidationWaitSec 45
```

## Launcher Release Gate

`scripts/production-preflight.ps1` runs both launcher Rust checks when Cargo is
enabled:

```powershell
cargo test --manifest-path src-tauri/Cargo.toml
cargo check --manifest-path src-tauri/Cargo.toml
```

Use `-SkipCargo` only when Rust is unavailable and record that limitation in
release notes or PR verification.

## Packaged Dedicated Server Commands

These commands cover the production-like server package path. A compiled
`DBA_GameClient/Binaries/Win64/DivineBeastsArenaServer.exe` is not enough for
packaged validation; the server must be cooked and staged with its content.
The package script defaults to server-only `-noclient` cooking; use
`-IncludeClientCook` only when validating client cook output as well.
Broad third-party VFX folders should not be forced through
`DirectoriesToAlwaysCook`; prefer referenced assets or curated DBA-owned cook
paths so unused incompatible Niagara assets do not block production cooks.

```powershell
.\scripts\diagnose-unreal-packaged-server-readiness.ps1
.\scripts\package-unreal-dedicated-server.ps1 -WhatIf
.\scripts\package-unreal-dedicated-server.ps1
.\scripts\smoke-unreal-dedicated-server.ps1 -UsePackagedServer
.\scripts\start-local-ue-validation.ps1 -BaseUrl "http://localhost:8080" -UsePackagedServer -ClientValidationWaitSec 45
```
