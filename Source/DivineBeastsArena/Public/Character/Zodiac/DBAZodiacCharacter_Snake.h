// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 玄花灵蛇

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacCharacter_Snake.generated.h"
#include "Character/DBAZodiacCharacterBase.h"

/**
 * ADBAZodiacCharacter_Snake
 * 玄花灵蛇角色类
 * 继承自生肖角色基类，配置玄花灵蛇特有的资源和行为
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacter_Snake : public ADBAZodiacCharacterBase
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacter_Snake();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 角色配置 ====================

	/** 获取角色名称 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FText GetCharacterName() const { return FText::FromString(TEXT("玄花灵蛇")); }

	/** 获取角色元素 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FName GetElement() const { return FName(TEXT("Water")); }

protected:
	// ==================== 资源路径 ====================

	/** 骨骼网格体路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString SkeletalMeshPath = TEXT("/Game/Models/Zodiac/Snake/SK_Snake_Mesh.SK_Snake_Mesh");

	/** 动画蓝图路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString AnimBlueprintPath = TEXT("/Game/Animation/Zodiac/Snake/ABP_Snake.ABP_Snake");

	/** 角色图标路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString IconPath = TEXT("/Game/UI/Icons/Zodiac/Snake_Icon.Snake_Icon");
};
