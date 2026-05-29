# Niagara 魔法粒子执行任务计划

> 来源文档: `DBA_GameClient/Docs/NiagaraMagicVFX_Design.md`  
> 日期: 2026-05-29  
> 目标: 先完成代码管线，让后续 Niagara 资产制作能直接读取统一 `User.*` 参数。

## Phase 1: 运行时参数管线

| 状态 | 任务 | 交付物 |
| --- | --- | --- |
| Done | 梳理现有技能 VFX 播放路径 | 确认入口为 `FDBAPlayableSkillRuntimeSpec`、`ADBASkillProjectileBase`、`ADBAChainLightningSpell`、`ADBABloomHealingSpell`、`ADBAHolyShieldSpell`、`UDBAEffectPlayer` |
| Done | 定义统一 Niagara 参数结构 | 新增可编辑结构，承载 `EffectRadius`、`Duration`、`TickInterval`、`Intensity`、元素色等 |
| Done | 投射物飞行/命中设置 `User.*` | 火球、寒冰、暗影等 projectile 系统自动收到参数 |
| Done | Beam / Area / Buff 技能设置 `User.*` | 闪电链、绽放治疗、神圣护盾生成 Niagara 时自动传参 |
| Done | 通用 EffectPlayer 设置基础参数 | DataTable 播放的释放/命中特效也能收到基础方向和位置参数 |

## Phase 2: P0 技能默认值

| 状态 | 技能 | 参数目标 |
| --- | --- | --- |
| Done | `Lobby.Skill01.MageFireball` | 火焰主色、飞行 3s、燃烧 Tick 2s、拖尾长度 |
| Done | `Lobby.Skill02.FrostShard` | 冰霜主色、减速持续 3s、冷雾 Tick |
| Done | `Lobby.Skill03.BloomHealing` | 自然主色、治疗半径、治疗 Tick |
| Done | `Lobby.Skill04.ChainLightning` | 金色闪电主色、跳跃半径、短持续高强度 |
| Done | `Lobby.Skill05.PriestShield` | 神圣金色、护盾持续、保护半径 |
| Done | `Lobby.Skill06.ShadowBolt` | 暗影紫色、飞行 3s、碎片/冲击强度 |

## Phase 3: 资产制作

| 状态 | 任务 | 说明 |
| --- | --- | --- |
| Tooling Done | 创建规范目录 | `Scripts/create_magic_vfx_skill_assets.py` 会在 Unreal Editor 内创建 `/Game/DBA/VFX/Skills/<Class>/<Skill>/...` |
| Tooling Done | 制作 P0 Niagara 模板 | 脚本会从现有项目 Niagara 资产复制 Fireball、FrostBolt、ShadowBolt、ChainLightning、BloomHealing、Sanctuary 占位模板 |
| Pending | 将默认技能资产路径迁移到新目录 | 先保留旧路径兼容，再逐步替换 DataAsset / C++ 默认值 |
| Pending | 配置 LOD 和固定 Bounds | 按设计文档性能预算执行 |

## Phase 4: 验证

| 状态 | 任务 | 验证点 |
| --- | --- | --- |
| Blocked Locally | 编译客户端模块 | 当前环境未找到 UnrealBuildTool / Unreal Editor；需要安装 UE 后执行 |
| Pending | PIE 单机验证 | 释放、飞行、命中、Buff 移除均有 VFX |
| Pending | 客户端/服务端验证 | 命中由服务端结算，Multicast VFX 参数一致 |
| Pending | 画质验证 | Low/Medium/High 下粒子数量和光源符合预算 |

## Unreal Editor 执行命令

在安装 Unreal Editor 的机器上执行：

```powershell
UnrealEditor.exe D:\DivineBeastsArena\DBA_GameClient\DivineBeastsArena.uproject -ExecutePythonScript=D:\DivineBeastsArena\DBA_GameClient\Scripts\create_magic_vfx_skill_assets.py
```

生成后运行校验：

```powershell
UnrealEditor.exe D:\DivineBeastsArena\DBA_GameClient\DivineBeastsArena.uproject -ExecutePythonScript=D:\DivineBeastsArena\DBA_GameClient\Scripts\validate_magic_vfx_skill_assets.py
```

校验通过后，再执行“将默认技能资产路径迁移到新目录”。

## 已执行范围

已经执行 Phase 1 与 Phase 2 的代码部分，并补齐 Phase 3 的编辑器脚本：

1. 增加技能级 Niagara 参数结构。
2. 内置六个 P0 技能配置默认参数。
3. 投射物、闪电链、治疗、护盾和通用 EffectPlayer 在生成 Niagara 后设置标准 `User.*` 参数。
4. 新增 P0 魔法 VFX 资产脚手架生成脚本。
5. 新增 P0 魔法 VFX 资产校验脚本。

当前环境未找到 Unreal Editor，无法直接生成 `.uasset`；脚本需要在 UE 编辑器环境中运行。
