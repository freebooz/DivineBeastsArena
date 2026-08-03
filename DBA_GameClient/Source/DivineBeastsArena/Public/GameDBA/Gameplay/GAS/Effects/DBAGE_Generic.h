// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化GameplayEffect类 - 通过SkillID + DataTable配置

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Engine/DataTable.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/Data/Tables/DBASkillDataRow.h"
#include "DBAGE_Generic.generated.h"

/**
 * UDBAGE_Generic
 * 泛化GameplayEffect类
 * 通过 SkillID 从 DataTable 读取配置
 * 替代原有的 DBAGE_<Zodiac>_<Skill> 60个类
 *
 * 使用方式:
 * 1. 在蓝图中设置 SkillID 和 SkillTable
 * 2. 运行时通过构造函数加载配置并应用效果
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API UDBAGE_Generic : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDBAGE_Generic();

public:
	/** DataTable 中定义的 SkillID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	FName SkillID;

	/** 技能 DataTable 引用 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	TObjectPtr<UDataTable> SkillTable;

protected:
	/** 从 DataTable 加载配置并应用效果 */
	void LoadAndApplyModifiers();
};
