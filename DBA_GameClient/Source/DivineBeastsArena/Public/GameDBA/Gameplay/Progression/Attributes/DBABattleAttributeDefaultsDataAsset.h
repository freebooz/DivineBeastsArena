// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明 GAS 战斗属性默认值数据资产，承载可调数值而不承载运行逻辑。
- 修改提示：新增战斗属性默认值时，同步更新校验逻辑与 AttributeSet 应用入口。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Data/DBADataAssetBase.h"
#include "GameDBA/Core/Interfaces/DBAValidatableInterface.h"
#include "DBABattleAttributeDefaultsDataAsset.generated.h"

USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBABattleAttributeDefaults
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Health", meta = (ClampMin = "0.0"))
	float MaxHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Health", meta = (ClampMin = "0.0"))
	float CurrentHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Attack", meta = (ClampMin = "0.0"))
	float AttackPower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Attack", meta = (ClampMin = "0.0"))
	float Defense = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Movement", meta = (ClampMin = "0.0"))
	float MoveSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Energy", meta = (ClampMin = "0.0"))
	float MaxEnergy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Energy", meta = (ClampMin = "0.0"))
	float CurrentEnergy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Energy", meta = (ClampMin = "0.0"))
	float EnergyRegen = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Critical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CriticalRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Critical", meta = (ClampMin = "1.0"))
	float CriticalMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Shield", meta = (ClampMin = "0.0"))
	float MaxShield = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Battle|Shield", meta = (ClampMin = "0.0"))
	float CurrentShield = 0.0f;
};

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBABattleAttributeDefaultsDataAsset : public UDBADataAssetBase, public IDBAValidatableInterface
{
	GENERATED_BODY()

public:
	const FDBABattleAttributeDefaults& GetDefaults() const { return Defaults; }

	virtual bool ValidateData_Implementation(TArray<FString>& OutErrors) const override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Battle", meta = (AllowPrivateAccess = "true"))
	FDBABattleAttributeDefaults Defaults;
};
