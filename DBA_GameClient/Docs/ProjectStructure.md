# DivineBeastsArena 工程结构

## 代码模块

| 模块 | 职责 | 依赖方向 |
| --- | --- | --- |
| `Source/GameCore` | 通用账号、会话、中性传输契约和跨玩法服务 | 不依赖 Arena 业务模块。 |
| `Source/GameMoba` | MOBA 通用 GAS、RPC、输入、UI 基类 | 公开依赖 `GameCore`。 |
| `Source/DivineBeastsArena` | DBA 具体玩法、角色、技能、UI、地图与资源绑定 | 公开依赖 `GameCore`、`GameMoba`、`Niagara`。 |
| `Plugins/GameBackendClient` | 后端客户端、遥测与崩溃上传适配 | 仅由游戏模块私有使用。 |

公开头文件中暴露的类型，其模块必须放在 `PublicDependencyModuleNames`；只在 `.cpp` 中使用的模块放在 `PrivateDependencyModuleNames`。

## 内容资源

- 正式项目资源统一进入 `Content/DBA/`。
- 旧的顶层 `Content/UI`、`Content/Data`、`Content/Blueprints` 仅作为兼容区，不得新增资源。
- 第三方资源可保留独立顶层目录，但必须登记来源和归属。
- `.uasset` 与 `.umap` 只能通过 Unreal Editor/MCP 事务迁移，并由人工审核引用结果。

## 工具边界

- 项目不保留 `Tools/`、`Scripts/`、`Start*.bat` 或 `run_*.bat` 自动化入口。
- Unreal Editor 操作由审核人员执行；不得通过脚本批量导入、移动、验证或启动资产与客户端。
- 结构、命名与资产归属以 `docs/Architecture/命名与目录登记表.md` 为准。
