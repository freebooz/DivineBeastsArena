# 应用结构分析报告

## 当前结论

解决方案已经按真实应用目标扁平化到 `DivineBeastsArenaPlatform` 根目录。每个顶层目录都直接对应一个可构建、可运行、可部署或可运营的单元，避免继续使用 `GamePlatform/backend/admin/website/launcher` 这类分类壳。

客户端目录统一命名为 `DBA_GameClient`，与 `DBA_GameBackend`、`DBA_GameAdmin`、`DBA_GameWebsite`、`DBA_GameLauncher` 保持同一应用命名体系；Unreal 项目文件仍保留 `DivineBeastsArena.uproject`。平台根目录使用 `DivineBeastsArenaPlatform`，表达“游戏客户端 + 后端平台 + 运营工具”的完整解决方案。

## 应用清单

| 应用 | 目录 | 目标 |
| --- | --- | --- |
| DBA_GameClient | `DBA_GameClient` | Unreal 游戏客户端和专用服务器目标。 |
| DBA_GameBackend | `DBA_GameBackend` | 账号、角色、服务器、运营、版本和平台 API。 |
| DBA_GameAdmin | `DBA_GameAdmin` | 运营管理后台。 |
| DBA_GameWebsite | `DBA_GameWebsite` | 官网、下载、新闻、FAQ 和反馈入口。 |
| DBA_GameLauncher | `DBA_GameLauncher` | 桌面启动器和客户端校验启动流程。 |
| ops | `ops` | 容器、反向代理、监控、日志、备份和迁移脚本。 |
| configs | `configs` | 游戏配置样例和后续配置发布来源。 |
| docs | `docs` | 架构、部署、运维、审查和发布检查文档。 |
| scripts | `scripts` | 构建、测试、安全扫描和维护脚本。 |

## 已完成调整

- 原 `GamePlatform` 包装目录已移除，应用提升到解决方案根目录。
- 客户端目录保持 `DBA_GameClient` 命名，与其他 `Game*` 应用一致。
- CI、Docker、脚本、官网、启动器默认路径和平台应用清单均改为扁平化目录。
- 运维资源、配置、文档和脚本保留 `GamePlatform*` 前缀，用于区分平台辅助资产和实际应用。

## 后续建议

- 发布流水线应以 `DivineBeastsArenaPlatform` 为根路径。
- Unreal 打包产物建议输出到独立发布目录，不回写到源码目录。
- 平台应用清单可以继续由 `Game.Api` 提供给管理后台和运维页面展示。
