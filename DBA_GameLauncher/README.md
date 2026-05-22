# DBA_GameLauncher

DBA_GameLauncher 是 Divine Beasts Arena 的桌面启动器和更新器，使用 Tauri、React、TypeScript 与 Rust 实现。

## 功能

- 读取本地版本信息。
- 拉取远端 `manifest.json`。
- 比较版本并显示更新状态。
- 校验游戏文件 SHA256。
- 修复缺失或损坏文件。
- 启动 `DBA_GameClient` 打包后的游戏客户端。
- 打开本地日志目录。

## 本地开发

```powershell
npm install
npm run build
cargo check --manifest-path src-tauri/Cargo.toml
```

## 配置

`launcher.config.json` 保存启动器默认配置。生产环境需要把 `updateUrl` 指向正式 CDN 或后端 Launcher Manifest 接口。

## 生产化待办

- 替换默认 Tauri 图标。
- 接入真实 CDN manifest 和补丁包。
- 完成断点续传、失败回滚和下载速度统计的端到端测试。
