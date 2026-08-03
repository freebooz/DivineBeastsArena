# 神兽竞技场 / DivineBeastsArena

## 开发纲领

- 项目协作规则：[AGENTS.md](./AGENTS.md)
- 长期开发纲领：[ZodiacArena UE5.8 总控提示词](./docs/Development/ZodiacArena_UE5_8_Codex_总控提示词.md)
- 三层架构：[三层架构设计](./docs/Architecture/三层架构设计.md)
- 命名与目录登记：[命名与目录登记表](./docs/Architecture/命名与目录登记表.md)

## 应用边界

| 目录 | 职责 |
| --- | --- |
| `DBA_GameClient/` | Unreal Engine 游戏工程，包含 Game、Editor 与 Dedicated Server Target。 |
| `DBA_GameBackend/` | .NET API、Worker、基础设施与 Dedicated Server 管理。 |
| `DBA_GameAdmin/` | Angular 运营管理后台。 |
| `DBA_GameWebsite/` | Next.js 官网。 |
| `DBA_GameLauncher/` | Tauri 启动器。 |
| `docs/` | 架构、开发与人工审核记录。 |
| `Artifacts/` | 经登记的长期证据，不存放可执行自动化入口。 |

## 架构规则

UE 模块依赖只能是 `DivineBeastsArena -> GameMoba -> GameCore` 或 `DivineBeastsArena -> GameCore`。Gameplay 与业务逻辑使用 C++，Blueprint 仅用于参数、资源引用与表现配置。运行数据必须由 DataAsset、DataTable、配置或后端数据提供。

项目禁止自动化脚本、CI 工作流、自动登录、自动选角、自动匹配和自动旅行。构建仅作工程检查；登录、选角、创建角色、进入大厅和联机流程必须由审核人员在真实界面手工验证。
