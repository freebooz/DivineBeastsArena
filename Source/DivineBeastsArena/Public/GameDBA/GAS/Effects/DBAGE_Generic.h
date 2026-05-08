// Copyright Freebooz Games, Inc. All Rights Reserved.
// 泛化GameplayEffect类 - 通过SkillID + DataTable配置

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Engine/DataTable.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/Data/DBASkillDataRow.h"
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
