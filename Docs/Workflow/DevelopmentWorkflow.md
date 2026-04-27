# 神兽竞技场 - 开发流程

## 一、日常开发流程

### 1. 拉取最新代码
```bash
git pull origin main
git lfs pull
```

### 2. 生成项目文件（如有 .uproject 或 .cs 变更）
```bash
# Windows
"%UE5_ROOT%\Engine\Build\BatchFiles\GenerateProjectFiles.bat" DivineBeastsArena.uproject

# Linux
$UE5_ROOT/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh DivineBeastsArena.uproject
```

### 3. 编译项目
```bash
# 使用 Scripts
Scripts\Build\BuildEditor.bat

# 或使用 VS Code
Ctrl+Shift+B → 选择构建任务

# 或使用 Visual Studio
打开 DivineBeastsArena.sln → 构建解决方案

# 或使用 Rider
打开项目 → Build → Build Solution
```

### 4. 启动 Editor
```bash
# 使用 VS Code
F5 → 选择 "Launch Editor"

# 或直接运行
"%UE5_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" DivineBeastsArena.uproject
```

### 5. 测试 Dedicated Server
```bash
# 启动 Dedicated Server
Scripts\Build\BuildServer.bat
Binaries\Win64\DivineBeastsArenaServer.exe /Game/Maps/Arena_Test -server -log

# 启动 Client 连接
Binaries\Win64\DivineBeastsArena.exe 127.0.0.1:7777 -game -log
```

## 二、添加新功能流程

### 1. 确定功能所属层级
- Core: 基础设施
- MobaBase: 通用 MOBA 逻辑
- DBA: 项目专属逻辑
- Frontend: 前台系统
- UI: 界面系统

### 2. 创建文件
```
Public/[Layer]/[Feature]/[ClassName].h
Private/[Layer]/[Feature]/[ClassName].cpp
```

### 3. 遵循命名规范
- 类名前缀：DBA
- Base 层类名：DBAMoba*Base
- 项目层类名：DBA*

### 4. 添加详细中文注释
```cpp
/**
 * DBAHero
 * 神兽竞技场英雄基类
 * 继承自 MobaBase 层，添加十二生肖、自然元素之力、五大阵营
 */
UCLASS()
class DIVINEBEASTSARENA_API ADBAHero : public ADBAMobaCharacterBase
{
    GENERATED_BODY()

    // ...
};
```

### 5. 编译验证
```bash
Scripts\Build\BuildEditor.bat
```

### 6. 提交代码
```bash
git add .
git commit -m "feat: 添加英雄基类"
git push origin feature/hero-base
```

## 三、目录选择指南

### 何时放在 Core/?
- 全局使用的基础类型
- 日志分类
- 常量定义
- 不依赖任何业务逻辑

### 何时放在 MobaBase/?
- 可复用于其他 MOBA 项目的逻辑
- 不绑定项目专属内容
- 通用竞技机制

### 何时放在 DBA/?
- 项目专属逻辑
- 十二生肖、自然元素之力、五大阵营相关
- 继承 MobaBase 并添加项目特性

### 何时放在 Shared/?
- Client/Server 共用的定义
- 网络复制相关
- DTO 结构体

### 何时放在 Client/?
- 仅客户端需要的逻辑
- 输入处理
- UI 创建
- 本地预测

### 何时放在 Server/?
- 仅服务器需要的逻辑
- 权威验证
- 反作弊
- 服务器优化