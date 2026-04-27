# 神兽竞技场 - 插件清单

## 一、必需插件（首期启用）

### 1. GameplayAbilities
- **用途**: GAS 核心系统
- **状态**: 必需启用
- **说明**: 技能系统、属性系统、GameplayEffect 的基础

### 2. GameplayTags
- **用途**: GameplayTag 系统
- **状态**: 必需启用
- **说明**: 用于技能标签、状态标签、事件标签等

### 3. GameplayTasks
- **用途**: GameplayTask 系统
- **状态**: 必需启用
- **说明**: 异步任务系统，GAS 依赖

### 4. EnhancedInput
- **用途**: 增强输入系统
- **状态**: 必需启用
- **说明**: UE5 推荐的输入系统，支持 PC / Android

### 5. Niagara
- **用途**: 特效系统
- **状态**: 必需启用（Client）
- **说明**: 技能特效、环境特效、UI 特效

### 6. UMG
- **用途**: UI 系统
- **状态**: 必需启用（Client）
- **说明**: 首期使用 UMG，不使用 CommonUI

### 7. Slate / SlateCore
- **用途**: UI 底层框架
- **状态**: 必需启用
- **说明**: UMG 依赖

### 8. AIModule
- **用途**: AI 系统
- **状态**: 必需启用
- **说明**: 小兵、野怪、防御塔 AI

### 9. NavigationSystem
- **用途**: 导航系统
- **状态**: 必需启用
- **说明**: AI 寻路、NavMesh

## 二、可选插件（后续评估）

### 1. CommonUI
- **用途**: 通用 UI 框架
- **状态**: 可选（首期不使用）
- **说明**: 提供跨平台 UI 抽象，后续评估是否迁移

### 2. CommonInput
- **用途**: 通用输入抽象
- **状态**: 可选（首期不使用）
- **说明**: 配合 CommonUI 使用

### 3. ModelViewViewModel (MVVM)
- **用途**: MVVM 架构支持
- **状态**: 可选（首期不使用）
- **说明**: UE5 新特性，后续评估

### 4. ReplicationGraph
- **用途**: 网络复制优化
- **状态**: 可选（后续优化阶段）
- **说明**: 大规模多人游戏网络优化

### 5. StateTree
- **用途**: 状态树系统
- **状态**: 可选（首期不使用）
- **说明**: UE5 新 AI 系统，后续评估

## 三、外部服务插件（可选）

### 1. HTTP
- **用途**: HTTP 请求
- **状态**: 可选（非 Shipping 构建）
- **说明**: 仅用于可选的 Monitoring / GameOps 客户端

### 2. Json / JsonUtilities
- **用途**: JSON 解析
- **状态**: 可选（非 Shipping 构建）
- **说明**: 配合 HTTP 使用

## 四、在线子系统（预留）

### 1. OnlineSubsystem
- **用途**: 在线服务抽象层
- **状态**: 预留（后续集成）
- **说明**: Session 管理、好友系统等

### 2. OnlineSubsystemUtils
- **用途**: 在线子系统工具
- **状态**: 预留（后续集成）
- **说明**: 配合 OnlineSubsystem 使用

## 五、插件启用方式

### .uproject 文件配置
```json
{
    "Plugins": [
        {
            "Name": "GameplayAbilities",
            "Enabled": true
        },
        {
            "Name": "EnhancedInput",
            "Enabled": true
        }
    ]
}
```

### Build.cs 依赖声明
```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "GameplayAbilities",
    "GameplayTags",
    "EnhancedInput",
});
```