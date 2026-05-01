// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 幻云灵猿

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacCharacter_Monkey.generated.h"
#include "Character/DBAZodiacCharacterBase.h"

/**
 * ADBAZodiacCharacter_Monkey
 * 幻云灵猿角色类
 * 继承自生肖角色基类，配置幻云灵猿特有的资源和行为
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacter_Monkey : public ADBAZodiacCharacterBase
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacter_Monkey();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 角色配置 ====================

	/** 获取角色名称 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FText GetCharacterName() const { return FText::FromString(TEXT("幻云灵猿")); }

	/** 获取角色元素 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FName GetElement() const { return FName(TEXT("Fire")); }

protected:
	// ==================== 资源路径 ====================

	/** 骨骼网格体路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString SkeletalMeshPath = TEXT("/Game/Models/Zodiac/Monkey/SK_Monkey_Mesh.SK_Monkey_Mesh");

	/** 动画蓝图路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString AnimBlueprintPath = TEXT("/Game/Animation/Zodiac/Monkey/ABP_Monkey.ABP_Monkey");

	/** 角色图标路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString IconPath = TEXT("/Game/UI/Icons/Zodiac/Monkey_Icon.Monkey_Icon");
};
