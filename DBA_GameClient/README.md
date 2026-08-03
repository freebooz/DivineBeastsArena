# 神兽竞技场 / Divine Beasts Arena

UE5.8 C++ MOBA 游戏工程。本目录同时承载 Game、Editor 和 Dedicated Server Target；目录名中的 Client 是历史名称，不表示这里只有客户端代码。

## 快速开始

### 环境要求

- Unreal Engine 5.8
- Visual Studio 2022 (17.8+) / Rider 2024.3+ / VS Code 1.85+
- Git 2.40+ with Git LFS 3.4+
- Android Studio Hedgehog (Android 开发)
- 16GB+ RAM
- 100GB+ 可用磁盘空间

### 首次构建

1. 克隆仓库
```bash
git clone 
cd DivineBeastsArena
git lfs install
git lfs pull
```

2. 生成项目文件
```bash
# Windows
"%UE5_ROOT%\Engine\Build\BatchFiles\GenerateProjectFiles.bat" DivineBeastsArena.uproject

# Linux
$UE5_ROOT/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh DivineBeastsArena.uproject
```

3. 编译项目

Windows 使用本机 UE 5.8 的 Build.bat：

```powershell
& "$env:UE5_ROOT\Engine\Build\BatchFiles\Build.bat" DivineBeastsArenaEditor Win64 Development "$PWD\DivineBeastsArena.uproject" -WaitMutex -NoHotReloadFromIDE
```

4. 启动 Editor
```bash
# Windows
"%UE5_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" DivineBeastsArena.uproject

# Linux
$UE5_ROOT/Engine/Binaries/Linux/UnrealEditor DivineBeastsArena.uproject
```

### VS Code 开发

1. 安装推荐扩展
```bash
code --install-extension ms-vscode.cpptools
code --install-extension llvm-vs-code-extensions.vscode-clangd
```

2. 生成 compile_commands.json
```bash
# 运行 VS Code Task: "DivineBeastsArena - Generate Compile Commands"
```

3. 使用 VS Code 调试
- F5 启动 Editor
- 选择 "Launch Dedicated Server" 启动服务器
- 选择 "Client Connect to Local Server" 连接测试

## 项目结构

- `Source/` - C++ 源代码
- `Content/` - 资源内容
- `Config/` - 配置文件
- `Plugins/` - 插件
- 不保留 `Scripts/` 或 `Tools/` 自动化目录与脚本入口
- `SourceArt/` - 可编辑美术源文件
- `SourceAssets/` - Unreal 导入前中间资源
- `Exports/` - 可追溯导出物，不是运行时权威数据源
- `Docs/` - 文档

## 开发规范

- C++ 类前缀：DBA
- 遵循 UE5.8 编码规范
- 所有代码必须有中文注释
- Dedicated Server 权威架构
- 新项目资产统一进入 `Content/DBA`
- 目录和命名以 `docs/Architecture/命名与目录登记表.md` 为准
