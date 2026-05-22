# 网络同步需求规格说明书

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.2.0
> 日期: 2026-05-05

---

## 1. 概述

### 1.1 目的

本文档定义神兽竞技场游戏网络同步系统的功能需求和非功能需求，作为网络系统设计和实现的依据。

### 1.2 范围

本文档涵盖以下网络同步范围：
- 角色移动同步
- 技能激活同步
- 属性状态同步
- 断线重连处理

---

## 2. 系统架构

### 2.1 网络模型

采用 **客户端预测 + 服务端权威** 模型：

```
客户端                              服务端
   │                                   │
   ├─ 输入 ──────────────────────────>│ 处理输入
   │                                   │
   │<──────── 状态复制 ────────────────│ 广播权威状态
   │                                   │
   ├─ 预测结果 (本地预览)              │
   │                                   │
   │<──────── 校正结果 ────────────────│ 校正预测误差
```

### 2.2 网络分层

| 层级 | 组件 | 职责 |
|------|------|------|
| RPC层 | DBARpcServer, DBARpcClient | 客户端/服务端方法调用 |
| 预测层 | DBAClientPredictionComponent | 输入预测和校正 |
| 复制层 | UE Actor Replication | 状态数据复制 |

---

## 3. 功能需求

### 3.1 移动同步

#### 3.1.1 客户端预测

**描述**: 客户端在发送输入后立即执行本地预测移动，无需等待服务端响应

**实现要求**:
- `FDBAAbilityRpcParams` 包含移动位置和方向
- 客户端立即更新角色位置
- 记录预测轨迹用于后续校正

**输入参数**:
```cpp
FVector_NetQuantize10 TargetLocation  // 目标位置
FRotator MoveDirection                // 移动方向
```

#### 3.1.2 服务端校正

**描述**: 服务端接收输入后执行权威移动，并将结果同步给所有客户端

**实现要求**:
- 服务端是移动的权威数据源
- 每100ms广播一次权威位置
- 客户端接收校正后平滑插值到正确位置

**输出参数**:
```cpp
FVector_NetQuantize10 ServerLocation  // 服务端位置
float ServerTimeStamp                 // 服务端时间戳
```

### 3.2 技能同步

#### 3.2.1 技能激活

**描述**: 客户端请求激活技能，服务端验证并执行

**RPC方法**:
```cpp
ServerTryActivateAbility(FDBAAbilityRpcParams Params)
```

**参数结构**:
```cpp
FGameplayAbilitySpecHandle AbilityHandle  // 技能句柄
TWeakObjectPtr<AActor> TargetActor       // 目标Actor
FVector_NetQuantize10 TargetLocation     // 目标位置
```

**流程**:
1. 客户端调用 `ServerTryActivateAbility`
2. 服务端验证技能条件（能量、冷却、范围）
3. 服务端激活技能并广播结果
4. 客户端接收结果并更新UI

#### 3.2.2 技能命中检测

**描述**: 客户端本地预判命中，服务端最终验证

**实现要求**:
- 客户端检测命中时立即播放特效
- 服务端收到命中报告后验证
- 验证失败时回滚客户端效果

**RPC方法**:
```cpp
ClientReceiveHit(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation)
```

### 3.3 属性同步

#### 3.3.1 战斗属性复制

**使用 UE DOREPLIFETIME 复制以下属性**:

| 属性 | 复制模式 | 频率 |
|------|----------|------|
| CurrentHealth | DOREPLIFETIME | 按需 |
| MaxHealth | DOREPLIFETIME | 仅改变时 |
| CurrentEnergy | DOREPLIFETIME | 按需 |
| MaxEnergy | DOREPLIFETIME | 仅改变时 |
| CurrentShield | DOREPLIFETIME | 按需 |
| UltimateEnergy | DOREPLIFETIME | 每 tick |
| ChainLevel | DOREPLIFETIME | 按需 |
| ResonanceLevel | DOREPLIFETIME | 按需 |

#### 3.3.2 Buff/Debuff同步

**描述**: 效果状态在服务端执行后同步给所有客户端

**实现要求**:
- GameplayEffect 在服务端执行
- 执行结果通过 `GameplayCue` 同步到客户端
- 客户端仅负责VFX/SFX播放

### 3.4 断线重连

#### 3.4.1 完整状态同步

**描述**: 客户端重新连接时，服务端发送完整的游戏状态

**同步数据**:
1. 角色完整属性 (Health, Energy, Shield)
2. 当前技能冷却状态
3. 当前Buff/Debuff列表
4. 位置和移动状态
5. 连锁等级和终极能量

**实现要求**:
- 客户端连接时发送 `PlayerId`
- 服务端查询并发送完整状态
- 客户端收到后重建游戏视图

**RPC方法**:
```cpp
ClientFullStateSync(FDBAGameStateSync State)
```

---

## 4. 非功能需求

### 4.1 性能指标

| 指标 | 要求 |
|------|------|
| 客户端输入响应延迟 | < 50ms (本地预测) |
| 服务端处理延迟 | < 20ms |
| 位置同步频率 | 10-30 Hz (可配置) |
| 带宽占用 | < 256 KB/s (每玩家) |

### 4.2 可靠性

| 需求 | 说明 |
|------|------|
| RPC可靠性 | 所有技能激活使用 Reliable UDP |
| 状态持久化 | 服务端崩溃后能恢复游戏状态 |
| 断线处理 | 玩家断线后保留角色30秒 |

### 4.3 安全性

| 需求 | 说明 |
|------|------|
| 服务端权威 | 所有伤害计算在服务端执行 |
| 作弊检测 | 检测客户端预测异常 |
| 验证频率 | 服务端对每个技能激活进行验证 |

---

## 5. 用户故事

### 5.1 移动同步
```
作为玩家，我希望能立即看到我的操作响应
以便获得流畅的游戏体验

验收标准:
- 客户端发送移动输入后立即响应
- 服务端校正时平滑过渡，不跳跃
```

### 5.2 技能同步
```
作为玩家，我希望技能效果立即反馈
以便获得爽快的操作手感

验收标准:
- 客户端激活技能后立即播放特效
- 服务端验证后同步最终命中结果
```

### 5.3 断线重连
```
作为玩家，我希望断线后能快速恢复游戏
以便不因网络波动而损失进度

验收标准:
- 重连后5秒内恢复完整游戏状态
- 不丢失已获得的buff/冷却进度
```

---

## 6. Open Questions

| 问题 | 选项 | 决策 |
|------|------|------|
| 观战模式 | 支持 / 不支持 | 待定 |
| 网络质量显示 | 简单 / 详细 | 简单 |
| 语音聊天 | 支持 / 不支持 | 不支持 |

---

## 7. 参考文档

- DBARpcInterface.h - RPC接口定义
- DBAMobaGameplayAbilityBase.h - 技能系统基类
- DBAZodiacCharacterBase.h - 角色基类

---

*文档生成时间: 2026-05-05*