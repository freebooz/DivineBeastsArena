// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 月华灵兔

#pragma once

#include "CoreMinimal.h"
#include "Character/DBAZodiacCharacterBase.h"
#include "DBAZodiacCharacter_Rabbit.generated.h"

/**
 * ADBAZodiacCharacter_Rabbit
 * 月华灵兔角色类
 * 继承自生肖角色基类，配置月华灵兔特有的资源和行为
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacter_Rabbit : public ADBAZodiacCharacterBase
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacter_Rabbit();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 角色配置 ====================

	/** 获取角色名称 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FText GetCharacterName() const { return FText::FromString(TEXT("月华灵兔")); }

	/** 获取角色元素 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FName GetElement() const { return FName(TEXT("Gold")); }

protected:
	// ==================== 资源路径 ====================

	/** 骨骼网格体路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString SkeletalMeshPath = TEXT("/Game/Models/Zodiac/Rabbit/SK_Rabbit_Mesh.SK_Rabbit_Mesh");

	/** 动画蓝图路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString AnimBlueprintPath = TEXT("/Game/Animation/Zodiac/Rabbit/ABP_Rabbit.ABP_Rabbit");

	/** 角色图标路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString IconPath = TEXT("/Game/UI/Icons/Zodiac/Rabbit_Icon.Rabbit_Icon");
};
