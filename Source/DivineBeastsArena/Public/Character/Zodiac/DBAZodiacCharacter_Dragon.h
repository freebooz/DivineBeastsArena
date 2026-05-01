// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 云巡龙君

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacCharacter_Dragon.generated.h"
#include "Character/DBAZodiacCharacterBase.h"

/**
 * ADBAZodiacCharacter_Dragon
 * 云巡龙君角色类
 * 继承自生肖角色基类，配置云巡龙君特有的资源和行为
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacter_Dragon : public ADBAZodiacCharacterBase
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacter_Dragon();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 角色配置 ====================

	/** 获取角色名称 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FText GetCharacterName() const { return FText::FromString(TEXT("云巡龙君")); }

	/** 获取角色元素 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FName GetElement() const { return FName(TEXT("Fire")); }

protected:
	// ==================== 资源路径 ====================

	/** 骨骼网格体路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString SkeletalMeshPath = TEXT("/Game/Models/Zodiac/Dragon/SK_Dragon_Mesh.SK_Dragon_Mesh");

	/** 动画蓝图路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString AnimBlueprintPath = TEXT("/Game/Animation/Zodiac/Dragon/ABP_Dragon.ABP_Dragon");

	/** 角色图标路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString IconPath = TEXT("/Game/UI/Icons/Zodiac/Dragon_Icon.Dragon_Icon");
};
