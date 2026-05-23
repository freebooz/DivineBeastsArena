/*
中文阅读说明：
- 所属应用：DBA_GameLauncher 游戏启动器。
- 文件职责：Tauri Rust 后端命令实现，负责本地文件、下载校验、启动游戏等高权限能力。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

fn main() {
    game_launcher_lib::run()
}
