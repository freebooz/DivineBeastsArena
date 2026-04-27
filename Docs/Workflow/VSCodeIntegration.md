# VS Code 与 Scripts/Tools 集成说明

## 一、关系定义

### .vscode/ - VS Code 工作区配置
- 存储 VS Code 特定的编辑器配置
- 包含 tasks.json（构建任务）
- 包含 launch.json（调试配置）
- 包含 settings.json（编辑器设置）
- 不包含实际构建逻辑

### Scripts/ - 跨平台构建脚本
- 包含实际构建逻辑
- 可被 VS Code、命令行、CI 调用
- 平台无关的自动化脚本
- 不依赖特定 IDE

### Tools/ - 辅助工具
- 独立的工具程序
- 可被 Scripts 调用
- 可被 VS Code 调用
- 可独立运行

## 二、调用关系

```
VS Code Tasks (.vscode/tasks.json)
    ↓ 调用
Scripts/ (BuildEditor.bat / BuildClient.bat)
    ↓ 调用
UE5 UBT (Engine/Build/BatchFiles/Build.bat)
    ↓ 可选调用
Tools/ (CheckCodingStyle.py / ValidateAssets.py)
```

## 三、设计原则

1. **VS Code 不是唯一开发工具**
   - Visual Studio 用户直接使用 .sln
   - Rider 用户使用 Rider 项目
   - 命令行用户直接调用 Scripts/

2. **Scripts 是构建逻辑的唯一真实来源**
   - VS Code tasks.json 只是调用 Scripts/
   - CI 直接调用 Scripts/
   - 避免逻辑重复

3. **Tools 保持独立性**
   - 不依赖 VS Code
   - 不依赖特定 IDE
   - 可被任何环境调用

## 四、示例：编译 Editor

### VS Code 方式
1. 按 Ctrl+Shift+B
2. 选择 "DivineBeastsArena - Build Editor (Development)"
3. VS Code 调用 Scripts/Build/BuildEditor.bat

### 命令行方式
```bash
cd Scripts/Build
BuildEditor.bat
```

### CI 方式
```bash
docker run -v $(pwd):/workspace ue5-builder \
  /workspace/Scripts/Build/BuildEditor.sh
```

所有方式最终都调用同一个构建脚本，保证一致性。