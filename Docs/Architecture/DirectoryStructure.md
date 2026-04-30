# 神兽竞技场 - 目录结构规划

## 一、首期单模块策略

### 为什么首期使用单主模块？

1. **降低初期复杂度**
   - 避免过早的模块依赖管理
   - 减少 Build.cs 配置复杂度
   - 简化编译链路与调试流程

2. **目录分层优先**
   - 通过 Public/Private/Internal 实现逻辑隔离
   - 通过命名空间实现代码组织
   - 为后续模块拆分预留清晰边界

3. **避免循环依赖**
   - 单模块内更容易控制 include 顺序
   - 减少跨模块接口设计的前期负担
   - 便于快速迭代与重构

4. **何时拆分模块？**
   - 当某个子系统需要独立编译单元时
   - 当需要插件化某个功能时
   - 当需要严格的模块边界隔离时
   - 预计在第 30-35 部分评估模块拆分需求

## 二、目录层级职责

### Core/ - 核心基础设施
- 基础类型、枚举、常量定义
- 日志分类与宏定义
- GameplayTag 根命名
- 全局配置与 DeveloperSettings
- 不依赖任何业务逻辑

### MobaBase/ - MOBA Base 层
- 通用 MOBA 竞技逻辑
- 不绑定十二生肖、自然元素之力、五大阵营
- 可复用于其他 MOBA 项目
- 包含：
  - Framework: GameMode/GameState/PlayerState/Character Base
  - GAS: AttributeSet/GameplayAbility/GameplayEffect Base
  - Combat: 伤害计算、连锁系统、克制系统框架
  - Network: RPC 基类、复制策略
  - AI: BehaviorTree/EQS Base
  - Data: DataTable/DataAsset 基类

### DBA/ - DBA 项目层
- 神兽竞技场专属实现
- 继承 MobaBase 并添加项目特性
- 包含：
  - Zodiac: 十二生肖数据与逻辑
  - Element: 自然元素之力（Metal/Wood/Water/Fire/Earth）
  - FiveCamp: 五大阵营表现包
  - Skills: 技能实现（继承 MobaBase Ability）
  - Heroes: 英雄实现（继承 MobaBase Character）
  - Data: 项目专属数据表

### Lobby/ - 大厅系统 (原Frontend)
- 登录、大厅、组队、匹配、英雄选择
- 不参与对局内战斗逻辑
- 可在 Dedicated Server 上禁用部分功能
- 子目录: Common, ElementSelect, FiveCampSelect, HeroSelect, Loading

### Client/UI/ - UI 系统
- Framework: CommonUI/WidgetController 基础
- HUD: 对局内 HUD（UnitFrame/BuffBar/ActionBar）
- Widgets: 通用 UI 组件
- Android: 触控适配与安全区处理
- Dedicated Server 永不创建 UI

### Animation/ - 动画系统
- AnimNotify 实现
- AnimInstance 实现
- 动画蓝图 C++ 基类

### Audio/ - 音频系统
- 音频管理器
- 音效触发逻辑
- Dedicated Server 可选禁用

### Shared/ - 跨层共享
- Client/Server 共用的接口、结构体、枚举
- 网络复制相关定义
- DTO（Data Transfer Object）

### Client/ - 客户端专用
- 输入处理（Enhanced Input）
- 本地玩家逻辑
- 渲染相关（Niagara/Material）
- Dedicated Server 不编译此目录

### Server/ - 服务器专用
- Dedicated Server 权威逻辑
- 复制策略实现
- 反作弊检测
- Client 不需要此目录代码

### Internal/ - 内部实现
- 模块内部辅助工具
- 不对外暴露的实现细节
- 仅在 Private/ 中 include

## 三、Public/Private/Internal 组织原则

### Public/
- 对外暴露的 API
- 其他模块可 include 的头文件
- 必须保持接口稳定性
- 包含：
  - UCLASS/USTRUCT/UENUM 声明
  - 公共接口（UINTERFACE）
  - 公共委托（DECLARE_DYNAMIC_MULTICAST_DELEGATE）

### Private/
- 实现文件（.cpp）
- 模块内部实现细节
- 不对外暴露的头文件
- 可以 include Public/ 和 Internal/

### Internal/
- 模块内部共享的头文件
- 仅在 Private/ 中使用
- 不对外暴露
- 用于模块内部代码复用

## 四、如何降低循环依赖

### 1. 依赖方向规则
```
Core → MobaBase → DBA
       ↓           ↓
    Lobby      Combat
       ↓           ↓
      UI        Animation
```

### 2. 前向声明优先
```cpp
// 头文件中优先使用前向声明
class ADBACharacter;
class UDBAAbilitySystemComponent;

// 实现文件中再 include 完整定义
#include "DBA/Heroes/DBACharacter.h"
```

### 3. 接口隔离
```cpp
// 使用 UINTERFACE 解耦
UINTERFACE()
class UDBADamageable : public UInterface { ... };

// 而不是直接依赖具体类
```

### 4. DTO 传递数据
```cpp
// 使用轻量级结构体传递数据
USTRUCT()
struct FDBADamageInfo { ... };

// 而不是传递完整的 Actor 指针
```

## 五、Base 层与 DBA 项目层分离

### MobaBase 层特征
- 文件命名：`DBAMoba*Base.h/cpp`
- 类名前缀：`UDBAMoba*Base` / `ADBAMoba*Base`
- 不包含：
  - 十二生肖相关逻辑
  - 自然元素之力相关逻辑
  - 五大阵营相关逻辑
  - 项目专属 UI 文案
  - 项目专属资源引用

### DBA 项目层特征
- 文件命名：`DBA*.h/cpp`（不含 MobaBase）
- 类名前缀：`UDBA*` / `ADBA*`
- 继承 MobaBase 类
- 添加项目专属逻辑
- 绑定项目资源

### 示例
```cpp
// MobaBase 层
class DIVINEBEASTSARENA_API ADBAMobaCharacterBase : public ACharacter
{
    // 通用 MOBA 角色逻辑
};

// DBA 项目层
class DIVINEBEASTSARENA_API ADBAHero : public ADBAMobaCharacterBase
{
    // 添加十二生肖、自然元素之力、五大阵营
    UPROPERTY()
    EDBAZodiac Zodiac;

    UPROPERTY()
    EDBAElement Element;

    UPROPERTY()
    EDBAFiveCamp FiveCamp;
};
```

## 六、Shared/Client/Server 目录预留

### Shared/ - 双端共享
- 网络复制的结构体
- RPC 参数定义
- 枚举与常量
- 接口定义

### Client/ - 客户端专用
- 编译条件：`#if !UE_SERVER`
- 输入处理
- UI 创建
- 本地预测
- 表现效果

### Server/ - 服务器专用
- 编译条件：`#if UE_SERVER`
- 权威逻辑
- 反作弊
- 服务器优化
- 复制策略

## 七、外部服务目录预留

### Docs/OpsClient/ - 运维客户端文档
- 外部 Monitoring API 对接文档
- 外部 GameOps API 对接文档
- 可选集成方案
- 降级策略

### Source/DivineBeastsArena/Private/External/ - 外部客户端实现（后续添加）
- MonitoringClient/: 可选监控上报客户端
- GameOpsClient/: 可选运维可见性客户端
- 特征：
  - 低耦合：外部服务不可用时游戏正常运行
  - 异步：所有请求异步执行
  - 可降级：超时/失败自动熔断
  - 可禁用：配置开关控制是否启用

### 重要约束
- 外部服务不得成为游戏启动必需依赖
- 外部服务不得成为战斗权威
- Client 不直接调用高危管理接口
- 所有外部请求必须可超时、可丢弃、可熔断