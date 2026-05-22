# DivineBeastsArena Project Structure

本文档记录当前工程结构约定，后续新增代码、资源和工具时按这里归位。

## 代码模块

| 模块 | 职责 | 依赖方向 |
| --- | --- | --- |
| `Source/GameCore` | 通用账号、会话、基础类型和跨玩法服务 | 不依赖 DBA 玩法模块 |
| `Source/GameMoba` | MOBA 通用 UI、GAS、RPC、类型基础设施 | 公开依赖 `GameCore` |
| `Source/DivineBeastsArena` | DBA 具体玩法、角色、技能、UI、地图逻辑 | 公开依赖 `GameCore`、`GameMoba`、`Niagara` |
| `Plugins/DBA_GameBackendClient` | 后端客户端、遥测、崩溃上传封装 | 作为插件被游戏模块私有使用 |

公开头文件里暴露的类型，其模块必须放在 `PublicDependencyModuleNames`。只在 `.cpp` 内使用的模块才放在 `PrivateDependencyModuleNames`。

## 内容资源

- 项目自有正式资源优先放在 `Content/DBA/`。
- 第三方或批量导入后仍需保持原始命名的资源，可以保留独立顶层目录，例如 `Content/ProjectileHitVFX/`。
- `Content/UI`、`Content/Data`、`Content/Blueprints` 等旧目录只作为兼容区使用，新资源不要继续扩散到这些目录。
- `.uasset` 和 `.umap` 的物理迁移必须通过 Unreal Editor 或命令let 完成，并在迁移后执行 Fix Up Redirectors。

## 工具和脚本

- `Tools/`：正式工具入口，例如 `LaunchLobby.ps1`、导入脚本和编辑器工具。
- `Scripts/`：编辑器辅助、验证、数据生成和一次性迁移脚本。
- 根目录的 `Start*.bat`、`run_*.bat` 仅作为兼容快捷方式，实际启动逻辑以 `Tools/LaunchLobby.ps1` 为准。

## 当前整理结果

- 已把主游戏模块公开头文件需要的 `GameCore`、`GameMoba`、`Niagara` 显式声明为公开依赖。
- 已把 `GameMoba` 对 `GameCore` 的公开类型引用声明为公开依赖。
- 已清理大厅 HUD 对不存在旧技能 DataTable 的运行时加载。
- 已让 UI 管理器在加载蓝图类前确认包存在，减少旧路径带来的无效加载和日志噪声。
- 已从仓库移除 Python 缓存文件，并在 `.gitignore` 中忽略后续缓存。
