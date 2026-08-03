// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 技能类配置说明
// 技能现在使用泛化类 + DataTable 配置，不再需要每个生肖每个技能单独创建类

#pragma once

#ifndef DIVINEBEASTSARENA_GAMEPLAY_ABILITIES_GENERATED_H
#define DIVINEBEASTSARENA_GAMEPLAY_ABILITIES_GENERATED_H

/**
 * 技能类替代方案:
 *
 * 原有 60 个自动生成的技能类 (12生肖 × 5技能) 现已被泛化类替代:
 *
 * - UDBAElementSkillAbility_Generic    (替代 Q/W/E 技能, 共 36 个)
 * - UDBAZodiacPassiveAbility_Generic    (替代 Passive 技能, 共 12 个)
 * - UDBAZodiacUltimateAbility_Generic  (替代 R 技能, 共 12 个)
 *
 * 使用方法:
 * 1. 在蓝图中创建 UDBAElementSkillAbility_Generic 子类
 * 2. 设置 SkillID 属性 (例如 "Tiger_Q", "Dragon_W")
 * 3. 配置 DataTable 获取技能数据
 *
 * 技能配置示例:
 *   SkillID = "Tiger_Q"
 *   SkillID = "Dragon_Passive"
 *   SkillID = "Horse_R"
 *
 * 这样可以通过 DataTable 配置实现技能差异化，无需创建新类。
 */

#endif // DIVINEBEASTSARENA_GAMEPLAY_ABILITIES_GENERATED_H
