// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 福岳灵猪

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacCharacter_Pig.generated.h"
#include "Character/DBAZodiacCharacterBase.h"

/**
 * ADBAZodiacCharacter_Pig
 * 福岳灵猪角色类
 * 继承自生肖角色基类，配置福岳灵猪特有的资源和行为
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacter_Pig : public ADBAZodiacCharacterBase
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacter_Pig();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 角色配置 ====================

	/** 获取角色名称 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FText GetCharacterName() const { return FText::FromString(TEXT("福岳灵猪")); }

	/** 获取角色元素 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FName GetElement() const { return FName(TEXT("Gold")); }

protected:
	// ==================== 资源路径 ====================

	/** 骨骼网格体路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString SkeletalMeshPath = TEXT("/Game/Models/Zodiac/Pig/SK_Pig_Mesh.SK_Pig_Mesh");

	/** 动画蓝图路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString AnimBlueprintPath = TEXT("/Game/Animation/Zodiac/Pig/ABP_Pig.ABP_Pig");

	/** 角色图标路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString IconPath = TEXT("/Game/UI/Icons/Zodiac/Pig_Icon.Pig_Icon");
};
