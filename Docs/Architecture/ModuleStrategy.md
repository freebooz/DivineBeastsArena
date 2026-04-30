# 神兽竞技场 - 模块化策略

## 一、为什么首期使用单主模块？

### 1. 降低初期复杂度
- 避免过早的模块依赖管理
- 减少 Build.cs 配置复杂度
- 简化编译链路与调试流程
- 加快迭代速度

### 2. 首次编译通过优先于过早模块化
- 单模块更容易保证编译通过
- 减少跨模块接口设计的前期负担
- 避免循环依赖问题
- 便于快速验证架构设计

### 3. 目录先分层、模块后增量
```
阶段 1（当前）：单模块 + 目录分层
    DivineBeastsArena/
        Public/Core/
        Public/MobaBase/
        Public/DBA/
        ...

阶段 2（第 30-35 部分评估）：按需拆分模块
    DBACore/
    DBAMobaBase/
    DBAProject/
    DBALobby/
    ...
```

### 4. 何时拆分模块？
- 当某个子系统需要独立编译单元时
- 当需要插件化某个功能时
- 当需要严格的模块边界隔离时
- 当团队规模扩大需要并行开发时
- 当需要按模块进行版本管理时

## 二、单模块内的逻辑隔离

### Public/Private/Internal 隔离
```cpp
// Public: 对外 API
Source/DivineBeastsArena/Public/Core/DBATypes.h

// Private: 实现细节
Source/DivineBeastsArena/Private/Core/DBATypes.cpp

// Internal: 模块内部共享
Source/DivineBeastsArena/Internal/Helpers/DBAHelper.h
```

### 命名空间隔离
```cpp
namespace DBA
{
    namespace Core { ... }
    namespace MobaBase { ... }
    namespace Project { ... }
}
```

### 编译条件隔离
```cpp
// Client 专用代码
#if !UE_SERVER
    // 客户端逻辑
#endif

// Server 专用代码
#if UE_SERVER
    // 服务器逻辑
#endif

// Editor 专用代码
#if WITH_EDITOR
    // 编辑器逻辑
#endif
```

## 三、模块增量原则

### 首期最小依赖集
- Core
- CoreUObject
- Engine
- InputCore
- EnhancedInput
- GameplayAbilities
- GameplayTags
- GameplayTasks
- UMG
- Slate
- SlateCore

### 后续增量添加
- 第 18 部分：Enhanced Input 详细配置
- 第 19-20 部分：GAS 完整依赖
- 第 33-38 部分：UI 框架依赖（CommonUI 可选）
- 第 30-31 部分：AI 依赖（AIModule、NavigationSystem）
- 第 39 部分：网络优化依赖（ReplicationGraph 可选）

### 可选依赖策略
- CommonUI: 可选，首期使用 UMG
- MVVM: 可选，首期使用传统 WidgetController
- ReplicationGraph: 可选，首期使用默认复制
- Iris: 可选，UE5.7 新特性，后续评估
- StateTree: 可选，首期使用传统状态机
- HTTP/Json: 仅用于可选 Monitoring/GameOps 客户端

## 四、依赖边界原则

### PublicDependency
- 在 Public 头文件中使用的模块
- 需要暴露给其他模块的依赖
- 示例：
  - GameplayAbilities（GAS 接口暴露）
  - UMG（UI 组件暴露）

### PrivateDependency
- 仅在 Private 实现中使用的模块
- 不暴露给其他模块的依赖
- 示例：
  - Slate（内部 UI 实现）
  - AIModule（内部 AI 实现）
  - HTTP（内部网络请求）

### 原则
- 优先使用 PrivateDependency
- 仅在必须暴露时使用 PublicDependency
- 减少依赖传递，降低耦合