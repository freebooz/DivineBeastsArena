# 开发文档

## 核心文档

- [ZodiacArena UE5.8 总控提示词](./ZodiacArena_UE5_8_Codex_总控提示词.md)
- [ZodiacArena 阶段交付看板](./ZodiacArena_阶段交付看板.md)
- [全局工程策略](../Architecture/全局工程策略文档.md)
- [三层架构设计](../Architecture/三层架构设计.md)
- [三层模块与业务语义边界 ADR](../Architecture/ADR/ADR-0001-三层模块与业务语义边界.md)
- [最新 GameClient 架构审查与优化计划](../Architecture/GameClient项目架构审查报告与优化计划_2026-07-14.md)
- [GameClient 当前目录结构](../Architecture/DBA_GameClient项目目录结构说明_2026-07-13.md)
- [GAS 系统设计与审查](../Architecture/GAS系统设计与审查报告.md)
- [十二生肖角色系统](../Architecture/Characters/ZodiacCharacterSystem.md)
- [十二生肖技能设计 V2](../Architecture/Characters/ZodiacSkillDesign_V2_万象灵庭.md)

## 基础规范

- [代码规范](./代码规范.md)
- [中文编码规范](./中文编码规范.md)
- [目录结构规范](./目录结构规范.md)
- [项目协作约束](../../AGENTS.md)

## 人工审核验证

项目不再保留或运行自动化脚本、CI 工作流、自动登录、自动选角、自动匹配、自动旅行或模拟账户兜底路径。

登录、选角、创建角色、进入大厅和联机流程必须由审核人员在真实客户端界面中手工操作；服务端不可用、鉴权失败或角色选择失败必须显示真实失败结果，不得伪造成功。

构建仅用于工程编译检查，不构成业务验收。发布、性能、安全和联机结果均须由人工审核记录。
