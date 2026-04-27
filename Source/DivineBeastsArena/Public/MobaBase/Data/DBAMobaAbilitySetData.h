// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MobaBase/GAS/DBAMobaAbilityGrantTypes.h"
#include "DBAMobaAbilitySetData.generated.h"

/**
 * Ability Set 数据资产
 * 用于批量配置 Ability / GameplayEffect / AttributeSet
 * 支持一次性赋予整套技能组
 *
 * 使用场景：
 * - 角色初始化时赋予固定技能组
 * - Zodiac + Element 组合生成技能组
 * - 装备 / Buff 临时赋予额外技能
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAMobaAbilitySetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Ability 赋予配置列表 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Set", meta = (DisplayName = "Ability 列表"))
	TArray<FDBAMobaAbilityGrantConfig> Abilities;

	/** GameplayEffect 赋予配置列表 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Set", meta = (DisplayName = "GameplayEffect 列表"))
	TArray<FDBAMobaEffectGrantConfig> Effects;

	/** AttributeSet 赋予配置列表 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Set", meta = (DisplayName = "AttributeSet 列表"))
	TArray<FDBAMobaAttributeSetGrantConfig> AttributeSets;

	/**
	 * 验证 Ability Set 配置是否有效
	 * @return 是否有效
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability Set")
	bool IsValid() const;

	/**
	 * 获取 Ability Set 描述信息
	 * @return 描述信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability Set")
	FString GetDescription() const;
};
