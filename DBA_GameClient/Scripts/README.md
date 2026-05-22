# DivineBeastsArena Scripts

本目录只放编辑器辅助、资产检查、数据生成和一次性迁移脚本。生产启动、联机调试和进程管理脚本放在 `Tools/`，避免把临时验证脚本和运行入口混在一起。

## 当前分类

| 类型 | 示例 | 说明 |
| --- | --- | --- |
| 数据生成 | `GenerateSkillDataTable.py`, `GenerateHeroBalance.py`, `GenerateElementResonanceTable.py` | 生成或刷新 CSV、DataTable 相关数据 |
| 资产导入/修复 | `create_lobby_fireball_assets.py`, `fix_rosales_skeleton_asset.py`, `apply_starter_skeleton_and_retarget.py` | 通过 Unreal Python 创建或修复资产引用 |
| UI 迁移/验证 | `move_login_ui_to_frontend.py`, `verify_character_create_zh.py`, `inspect_lobby_hud_blueprint.py` | 检查和修复登录、角色、HUD 蓝图结构 |
| 探针脚本 | `probe_*.py`, `inspect_*.py`, `list_*.py`, `find_*.py` | 调查引擎 API、资产路径和项目状态 |

## 维护约定

- 新的正式运行入口优先放到 `Tools/`。
- `probe_*.py` 这类调查脚本完成任务后应删除，或在确认仍有价值时改名为明确的验证脚本。
- 不提交 `__pycache__`、`.pyc` 等 Python 运行缓存。
- 涉及 `.uasset` 移动时，优先使用 Unreal Editor/命令let，让工程生成 redirector 并执行 Fix Up Redirectors。
