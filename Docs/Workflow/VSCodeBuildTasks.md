# VS Code 构建任务说明

## 一、UBT 调用流程

### 构建流程
```
VS Code Task (tasks.json)
    ↓ 调用
UE5 UBT (UnrealBuildTool.exe)
    ↓ 读取
Target.cs (DivineBeastsArena.Target.cs / DivineBeastsArenaServer.Target.cs)
    ↓ 读取
Build.cs (DivineBeastsArena.Build.cs)
    ↓ 生成
编译命令
    ↓ 执行
编译器 (MSVC / Clang)
```

## 二、Target 选择

### Client Target
```bash
Engine/Build/BatchFiles/Build.bat DivineBeastsArena Win64 Development
```
- 使用 `DivineBeastsArena.Target.cs`
- 定义 `DBA_CLIENT=1`
- 包含客户端渲染、UI、音频等

### Editor Target
```bash
Engine/Build/BatchFiles/Build.bat DivineBeastsArenaEditor Win64 Development
```
- 使用 `DivineBeastsArenaEditor.Target.cs`
- 定义 `DBA_EDITOR=1`
- 包含编辑器工具、资产编辑器等

### Dedicated Server Target
```bash
Engine/Build/BatchFiles/Build.bat DivineBeastsArenaServer Win64 Development
```
- 使用 `DivineBeastsArenaServer.Target.cs`
- 定义 `DBA_SERVER=1`, `DBA_DEDICATED_SERVER=1`
- 排除客户端渲染、UI、音频等
- 优化网络性能和内存占用

## 三、VS Code 任务配置

### tasks.json 中的任务
```json
{
    "label": "DivineBeastsArena - Build Server (Development)",
    "type": "shell",
    "command": "Engine/Build/BatchFiles/Build.bat",
    "args": [
        "DivineBeastsArenaServer",      // Target 名称
        "Win64",                         // 平台
        "Development",                   // 配置
        "-Project=${workspaceFolder}/DivineBeastsArena.uproject",
        "-WaitMutex"
    ],
    "options": {
        "cwd": "${env:UE5_ROOT}"        // UE5 引擎根目录
    }
}
```

### 环境变量要求
- `UE5_ROOT`: UE5 引擎安装路径
- 示例：`C:\Program Files\Epic Games\UE_5.7`

## 四、为什么现在就要有 Server Target？

### 1. Dedicated Server 优先架构
- 从第一天开始就按 Dedicated Server 架构设计
- 避免后期重构客户端/服务器代码分离
- 确保所有权威逻辑在服务器端

### 2. 编译验证
- 确保代码在 Server Target 下可编译
- 及早发现客户端依赖问题
- 避免使用 Server 不支持的 API

### 3. 测试流程
- 本地启动 Dedicated Server 测试
- Client 连接本地 Server 验证网络逻辑
- 模拟真实部署环境

### 4. 依赖管理
- Build.cs 中区分 Client/Server 依赖
- 避免 Server 引入不必要的客户端模块
- 减少 Server 包体大小和内存占用

## 五、配置差异

### Client vs Server 依赖
```csharp
// Client 需要，Server 不需要
if (Target.Type != TargetType.Server)
{
    PrivateDependencyModuleNames.AddRange(new string[]
    {
        "RenderCore",       // 渲染
        "RHI",              // 渲染硬件接口
        "Niagara",          // 特效
        "AudioMixer",       // 音频
    });
}
```

### 全局定义差异
```csharp
// Client Target
GlobalDefinitions.Add("DBA_CLIENT=1");
GlobalDefinitions.Add("DBA_SERVER=0");

// Server Target
GlobalDefinitions.Add("DBA_CLIENT=0");
GlobalDefinitions.Add("DBA_SERVER=1");
GlobalDefinitions.Add("DBA_DEDICATED_SERVER=1");
```

### 代码中使用
```cpp
// 仅在 Client 编译
#if !UE_SERVER
    CreateUI();
    PlaySound();
#endif

// 仅在 Server 编译
#if UE_SERVER
    ValidateAuthority();
    ReplicateToClients();
#endif
```