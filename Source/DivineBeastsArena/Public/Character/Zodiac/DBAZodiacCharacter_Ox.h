// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 镇岳神牛

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacCharacter_Ox.generated.h"
#include "Character/DBAZodiacCharacterBase.h"

/**
 * ADBAZodiacCharacter_Ox
 * 镇岳神牛角色类
 * 继承自生肖角色基类，配置镇岳神牛特有的资源和行为
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacter_Ox : public ADBAZodiacCharacterBase
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacter_Ox();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 角色配置 ====================

	/** 获取角色名称 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FText GetCharacterName() const { return FText::FromString(TEXT("镇岳神牛")); }

	/** 获取角色元素 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FName GetElement() const { return FName(TEXT("Water")); }

protected:
	// ==================== 资源路径 ====================

	/** 骨骼网格体路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString SkeletalMeshPath = TEXT("/Game/Models/Zodiac/Ox/SK_Ox_Mesh.SK_Ox_Mesh");

	/** 动画蓝图路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString AnimBlueprintPath = TEXT("/Game/Animation/Zodiac/Ox/ABP_Ox.ABP_Ox");

	/** 角色图标路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString IconPath = TEXT("/Game/UI/Icons/Zodiac/Ox_Icon.Ox_Icon");
};
