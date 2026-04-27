# 神兽竞技场 - Content 目录结构规划

## 一、资源分层原则

### 1. Core 层
- 全局共享的核心资源
- 不依赖任何业务逻辑
- 包含：
  - 核心数据表
  - 主材质
  - 通用纹理
  - 工具蓝图

### 2. MobaBase 层
- 通用 MOBA 竞技资源
- 不绑定项目专属内容
- 可复用于其他 MOBA 项目
- 包含：
  - 框架蓝图基类
  - GAS 资产基类
  - 通用战斗效果
  - 通用 AI 行为

### 3. DBA 项目层
- 神兽竞技场专属资源
- 包含：
  - 十二生肖资源
  - 自然元素之力资源
  - 五大阵营表现包
  - 英雄实现
  - 项目数据表

### 4. Frontend / Arena / Practice 功能层
- 按功能模块隔离资源
- Frontend: 前台系统资源
- Arena: 对局资源
- Practice: 练习模式资源

### 5. Developer 层
- 开发者专用资源
- 不进入 Shipping 构建
- 包含：
  - 调试工具
  - 测试地图
  - 开发辅助资产

## 二、资源命名规范

### 通用命名规则
```
[Prefix]_[Descriptor]_[Variant]_[Suffix]

示例：
- T_Metal_Skill01_D.uasset          # 纹理：金系技能01 漫反射
- M_Fire_Effect_Inst.uasset         # 材质实例：火系特效
- BP_Dragon_Fire_Hero.uasset        # 蓝图：龙-火英雄
- DT_Zodiacs.uasset                 # 数据表：十二生肖
- DA_GameConfig.uasset              # 数据资产：游戏配置
- WBP_PlayerFrame.uasset            # Widget 蓝图：玩家框架
- NS_Water_Splash.uasset            # Niagara 系统：水花飞溅
- AM_Combat_Attack01.uasset         # 动画蒙太奇：战斗攻击01
- BT_Minion_Melee.uasset            # 行为树：近战小兵
- BB_Minion.uasset                  # 黑板：小兵
```

### 前缀规范
| 前缀 | 资产类型 |
|------|----------|
| T_ | Texture（纹理） |
| M_ | Material（材质） |
| MI_ | Material Instance（材质实例） |
| MF_ | Material Function（材质函数） |
| BP_ | Blueprint（蓝图） |
| WBP_ | Widget Blueprint（UI 蓝图） |
| DT_ | Data Table（数据表） |
| DA_ | Data Asset（数据资产） |
| ST_ | String Table（字符串表） |
| SM_ | Static Mesh（静态网格） |
| SK_ | Skeletal Mesh（骨骼网格） |
| AS_ | Animation Sequence（动画序列） |
| AM_ | Animation Montage（动画蒙太奇） |
| ABP_ | Animation Blueprint（动画蓝图） |
| NS_ | Niagara System（Niagara 系统） |
| NE_ | Niagara Emitter（Niagara 发射器） |
| S_ | Sound（音效） |
| SC_ | Sound Cue（音效 Cue） |
| MS_ | MetaSound（MetaSound） |
| BT_ | Behavior Tree（行为树） |
| BB_ | Blackboard（黑板） |
| EQS_ | Environment Query（环境查询） |

### 后缀规范
| 后缀 | 含义 |
|------|------|
| _D | Diffuse（漫反射） |
| _N | Normal（法线） |
| _M | Mask（遮罩） |
| _E | Emissive（自发光） |
| _R | Roughness（粗糙度） |
| _AO | Ambient Occlusion（环境光遮蔽） |
| _Inst | Instance（实例） |
| _Base | Base Class（基类） |
| _Child | Child Class（子类） |

## 三、DataTable / DataAsset 规划

### DataTable 使用场景
- 静态配置数据
- 策划可编辑的表格数据
- 支持 CSV 导入导出
- 示例：
  - DT_Zodiacs: 十二生肖配置
  - DT_Elements: 自然元素之力配置
  - DT_Skills: 技能配置
  - DT_Heroes: 英雄配置

### DataAsset 使用场景
- 复杂配置对象
- 需要引用其他资产的配置
- 运行时动态加载的配置
- 示例：
  - DA_GameConfig: 游戏全局配置
  - DA_BalanceConfig: 平衡配置
  - DA_AbilitySet: 技能组配置

### CSV 数据流程
```
策划编辑 CSV → 导入为 DataTable → C++ 读取
```

### StringTable 本地化
```
Content/Core/Data/StringTables/
├── ST_UI_Common.uasset              # 通用 UI 文本
├── ST_UI_Frontend.uasset            # 前台 UI 文本
├── ST_UI_Arena.uasset               # 对局 UI 文本
├── ST_Skills.uasset                 # 技能文本
└── ST_Heroes.uasset                 # 英雄文本
```

## 四、Blueprint 子类规划

### 蓝图继承层次
```
C++ Base Class
    ↓
Blueprint Base Class (MobaBase/)
    ↓
Blueprint Project Class (DBA/)
```

### 示例：英雄蓝图
```
ADBAMobaCharacterBase (C++)
    ↓
BP_MobaHeroBase (MobaBase/Framework/Characters/)
    ↓
BP_Dragon_Fire_Hero (DBA/Heroes/Dragon_Fire/)
```

### 蓝图使用原则
- 不在蓝图中实现服务端权威逻辑
- 蓝图用于：
  - 资源绑定
  - 表现层配置
  - 编辑器可视化调整
  - 快速原型验证
- 核心逻辑必须在 C++ 中实现

## 五、Android 低配资源规划

### 低配资源目录
```
Content/Android/LowSpec/
├── Materials/                       # 简化材质（减少指令数）
├── Textures/                        # 低分辨率纹理（512x512 或更低）
└── Effects/                         # 简化特效（减少粒子数）
```

### 资源切换策略
```cpp
// C++ 中根据设备性能选择资源
if (IsLowEndDevice())
{
    MaterialPath = TEXT("/Game/Android/LowSpec/Materials/M_Simple");
}
else
{
    MaterialPath = TEXT("/Game/DBA/Materials/M_HighQuality");
}
```

### 纹理格式
- Android: ASTC
- 低配设备: ASTC 4x4（较高质量）或 ASTC 8x8（较低质量）

## 六、Developer / Debug 资产隔离

### Developer 目录特征
- 仅在 Editor 和 Development 构建中可用
- Shipping 构建自动排除
- 包含：
  - 调试 UI
  - 测试地图
  - 开发工具

### 配置排除规则
**`Config/DefaultGame.ini`**
```ini
[/Script/UnrealEd.ProjectPackagingSettings]
+DirectoriesToNeverCook=(Path="/Game/Developer")
+DirectoriesToNeverCook=(Path="/Game/Test")
```

### 代码中检查
```cpp
#if !UE_BUILD_SHIPPING
    // 仅在非 Shipping 构建中执行
    LoadDebugAssets();
#endif
```

## 七、AssetManager 分层准备

### PrimaryAssetLabel 规划
```
Content/Core/Data/AssetLabels/
├── PAL_CoreAssets.uasset            # 核心资产标签
├── PAL_FrontendAssets.uasset        # 前台资产标签
├── PAL_ArenaAssets.uasset           # 对局资产标签
└── PAL_HeroAssets.uasset            # 英雄资产标签
```

### Chunk 分层策略
- Chunk 0: 核心资产（必须）
- Chunk 1: 前台资产（启动时加载）
- Chunk 2: 对局资产（进入对局时加载）
- Chunk 3+: 英雄资产（按需加载）

### 预加载策略
```cpp
// 启动时预加载
- Core 资产
- Frontend UI
- 通用音效

// 进入对局时加载
- 地图资产
- 对局 UI
- 战斗音效

// 按需加载
- 英雄资产（根据选择）
- 技能特效（根据技能组）
- 阵营表现包（根据选择）
```

## 八、五大阵营资源规划

### Byakko（白虎阵营）
- 主题：西方、金属、锋利、白色
- 特效风格：金属光泽、锐利线条
- 音效风格：金属碰撞、清脆
- UI 风格：银白色调、几何图案

### Qinglong（青龙阵营）
- 主题：东方、木、生长、青色
- 特效风格：藤蔓、叶片、自然
- 音效风格：风声、叶片摩擦
- UI 风格：青绿色调、流动曲线

### Xuanwu（玄武阵营）
- 主题：北方、水、防御、黑色
- 特效风格：水流、冰晶、护盾
- 音效风格：水流、冰裂
- UI 风格：深蓝黑色调、厚重边框

### Zhuque（朱雀阵营）
- 主题：南方、火、爆发、红色
- 特效风格：火焰、爆炸、光芒
- 音效风格：燃烧、爆裂
- UI 风格：红橙色调、火焰纹理

### Kirin（麒麟阵营）
- 主题：中央、土、稳固、黄色
- 特效风格：岩石、大地、沉稳
- 音效风格：岩石碰撞、低沉
- UI 风格：土黄色调、方正结构

## 九、自然元素之力资源规划

### Metal（金）
- 技能特效：金属光泽、锐利切割
- 音效：金属碰撞、剑鸣
- 克制：Wood（木）

### Wood（木）
- 技能特效：藤蔓缠绕、叶片飞舞
- 音效：风吹树叶、木质碰撞
- 克制：Earth（土）

### Water（水）
- 技能特效：水流、冰晶、波纹
- 音效：水流、冰裂、波浪
- 克制：Fire（火）

### Fire（火）
- 技能特效：火焰、爆炸、灼烧
- 音效：燃烧、爆裂、火焰呼啸
- 克制：Metal（金）

### Earth（土）
- 技能特效：岩石、大地裂缝、沙尘
- 音效：岩石碰撞、地震、沙尘
- 克制：Water（水）

## 十、UI 资源结构（魔兽世界式）

### UnitFrame（单位框架）
- 显示：生命值、能量、名称、等级、Buff/Debuff
- 支持：玩家、宠物、目标、焦点
- 可移动、可缩放

### TargetFrame（目标框架）
- 显示当前选中目标信息
- 支持敌方/友方不同样式
- 显示施法条

### PartyFrame（队伍框架）
- 显示队友信息（最多 5 人）
- 紧凑布局
- 快速查看队友状态

### RaidFrame（团队框架）
- 显示团队信息（最多 10 人）
- 网格布局
- 支持按职业/队伍分组

### ActionBar（动作条）
- 显示技能按钮
- 支持快捷键绑定
- 显示冷却