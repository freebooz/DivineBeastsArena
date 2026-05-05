# DivineBeastsArena Scripts

## 目录结构

```
Scripts/
├── Tools/           # 工具脚本
├── Build/           # 构建脚本
├── Deploy/          # 部署脚本
└── Test/            # 测试脚本
```

## 工具脚本

| 脚本 | 用途 |
|------|------|
| generate_zodiac_abilities.py | 生成十二生肖技能代码 |
| generate_skill_vfx_sfx.py | 生成技能VFX/SFX代码 |
| generate_skill_projectile_scripts.py | 生成投射物脚本 |
| generate_gas_cues_and_effects.py | 生成GAS Cue和Effect |
| generate_zodiac_vfx_sfx.py | 生成生肖VFX/SFX |
| generate_animation_blueprint_scripts.py | 生成动画蓝图脚本 |

## 使用方法

```bash
# 生成所有脚本
python Scripts/generate_zodiac_abilities.py

# 生成技能VFX/SFX
python Scripts/generate_skill_vfx_sfx.py
```