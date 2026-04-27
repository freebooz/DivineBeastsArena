# 可选依赖策略

## 一、HTTP / Json / JsonUtilities

### 用途
- 仅用于可选的 Monitoring / GameOps 客户端
- 不是游戏运行的必需依赖
- 可通过配置完全禁用

### 依赖条件
```csharp
// Build.cs 中仅在非 Shipping 构建添加
if (Target.Configuration != UnrealTargetConfiguration.Shipping)
{
    PrivateDependencyModuleNames.AddRange(new string[]
    {
        "HTTP",
        "Json",
        "JsonUtilities",
    });
}
```

### 代码中使用
```cpp
// 编译条件保护
#if ENABLE_EXTERNAL_SERVICES
    // 可选的监控上报代码
    SendMetricsToMonitoring();
#endif
```

### 配置开关
```ini
[/Script/DivineBeastsArena.DBAExternalServicesSettings]
; 是否启用外部服务客户端
bEnableMonitoring=False
bEnableGameOps=False

; 外部服务 URL（可选）
MonitoringURL=""
GameOpsURL=""
```

## 二、CommonUI / MVVM

### 首期策略
- 不使用 CommonUI
- 使用传统 UMG + WidgetController 模式
- 原因：
  - 降低学习曲线
  - 减少首期复杂度
  - 保持与传统 UE 开发方式一致

### 后续评估
- 第 33-38 部分 UI 框架搭建时评估
- 如团队有 CommonUI 经验可考虑切换
- MVVM 可作为可选模式引入

## 三、ReplicationGraph

### 首期策略
- 不使用 ReplicationGraph
- 使用 UE 默认复制系统
- 原因：
  - 首期玩家数量有限（10人）
  - 默认复制已足够
  - 降低配置复杂度

### 后续优化
- 第 39 部分网络优化时评估
- 当对局玩家数量增加时
- 当网络带宽成为瓶颈时

## 四、Iris

### 首期策略
- 不使用 Iris
- 使用 UE5 传统复制系统
- 原因：
  - UE5.7 新特性，有待验证
  - 首期不需要高性能复制
  - 保持稳定性优先

### 后续评估
- 当项目需要高性能复制时
- 当网络同步成为瓶颈时
- 当 UE 官方推荐用于游戏时

## 五、StateTree

### 首期策略
- 不使用 StateTree
- 使用传统状态机（AActor::GetCurrentState）
- 使用 BehaviorTree
- 原因：
  - 团队熟悉传统状态机
  - BehaviorTree 足够 AI 需求
  - 降低首期复杂度

### 后续评估
- 当需要复杂分层状态机时
- 当状态机逻辑复杂难以维护时
- 当团队有 StateTree 经验时