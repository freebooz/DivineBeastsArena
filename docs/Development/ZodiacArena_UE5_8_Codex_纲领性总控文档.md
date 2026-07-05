# ZodiacArena UE5.8 纲领性总控文档（v1）

> 作为本项目长期工程化推进的基线规范。未完事项按“可验证最小闭环”逐步交付，优先保障 UE5.8 可稳定构建、可联机验证、可持续交付。

## 1. 开发目标

围绕一体化竞技场 MOBA（3v3 / 5v5）打造可上线的客户端 + Dedicated Server + 后端服务链路，持续输出以下能力：

1. 稳定的 UE5.8 C++ 多端工程（PC 主客户端 + Android/iOS 可推进）。
2. 完整的 `ZodiacFoundation / ZodiacMoba / ZodiacArenaGame` 分层架构。
3. GAS 能力闭环（Ability / Effect / Cue / Tag / Asset）可运行并可验证。
4. Dedicated Server、客户端、后端联调路径可重复执行（含 PIE/Automation）。
5. 运营、支付、匹配、战绩、公告等业务链路逐步收口到可验证功能。

## 2. 里程碑（P0~P4）

### P0（基线）
- 明确现状边界：先确认仓库结构、插件依赖、目标文件、基础配置是否齐备。
- 完成工程级一致化：中文文档 UTF-8、Build 脚本可复用、核心规则明确。
- 保持可复现构建：Backend / Admin / Website / Launcher 的基础构建命令执行通过。

### P1（模块基础）
- Foundation 与 MobaCore 骨架与边界稳定：Subsystem、AssetManager、Service、Auth/ServerStatus、GameplayTag。
- Unreal 与后端联通链路脚本可自动发现并回传基础健康状态。
- 建立核心数据资产定义与日志/错误码标准。

### P2（游戏核心）
- ArenaGame 完成基础对战框架：角色、Matchmaking、HUD、状态、战斗系统。
- 实现两类角色样例（如 Rat / Ox）到可运行 Demo 级别。
- 网络同步（Replication + Dedicated Server）主链路验证通过。

### P3（客户端交付）
- 登录、登录后菜单、角色选择、对局体验与基础 UI 完整闭环。
- 战斗 UI（技能条、血条、计分板、技能表现）可驱动展示。
- VFX/SFX/动画、输入（PC/移动）按层级接入。

### P4（运营与交付）
- PC + Server 全链路联调通过，Mock 服务可切换为真实服务。
- CI/Smoke/回归任务形成固定清单，包含关键网络和功能用例。
- 文档（Architecture / Development / Deployment）与生产准备计划一致。

## 3. 开发执行约束（高优先级）

1. 遵循仓库级 `AGENTS.md` 与模块 `AGENTS` 约束。  
2. 所有 Blueprint / UMG / 资产改动通过工具流执行，避免直接手改二进制资产。  
3. 修改后必须完成：Compile / Save / PIE Smoke / Git Diff；网络与服务链路验证作为可选必检项。  
4. 文本/日志统一 UTF-8，优先 `TEXT()` 和明确日志分类（Category + Level）。  
5. 优先使用 `TEXT/Build.cs/.uproject/.uplugin/.ini/.md` 规范化文件，避免临时性散落脚本。  
6. 默认持续推进：常规阶段切换、常规验证、文档同步、源码小步修改和本地证据补齐均不是确认点；除非触及破坏性操作、生产写入、密钥证书、项目全局设置、二进制资产直改或运行平台强制确认，否则不再询问，直接执行并用证据汇报。
7. 任何阻塞性操作先提需求评审再执行，不默认替代用户明确要求。  

## 4. 交付验收清单（每周复盘）

- 是否完成本周 P 阶段目标。  
- 关键文件结构是否落在三层框架内（Foundation / MobaCore / ArenaGame）。  
- 是否有可复现的本地验证证据（日志、截图、Diff、测试输出）。  
- 是否补齐了文档：开发约束、架构说明、部署说明。  

## 5. 立即行动（本轮）

1. 将本文件作为总控基线，并在 `Docs/Development/README.md` 标记为“纲领版入口”。  
2. 将执行状态按周更新到 `docs/Development/ZodiacArena_阶段交付看板.md`，并以 `docs/solution-audit-and-production-plan.md` 和 `docs/app-structure-code-audit.md` 为来源同步。  
3. 任何新需求先按本纲领映射到对应 P 阶段再立项。  
