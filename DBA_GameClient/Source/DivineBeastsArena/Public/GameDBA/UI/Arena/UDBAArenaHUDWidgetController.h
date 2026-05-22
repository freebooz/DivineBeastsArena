// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "UDBAArenaHUDWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaHUDWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBAArenaHUDWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	void UpdatePlayerHP(float CurrentHP, float MaxHP) override;

	void UpdatePlayerEnergy(float CurrentEnergy, float MaxEnergy);

	void UpdateUltimateEnergy(float CurrentEnergy, float MaxEnergy);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnPlayerEnergyChanged OnPlayerEnergyChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUltimateEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnUltimateEnergyChanged OnUltimateEnergyChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float CurrentHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float MaxHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float CurrentEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float MaxEnergy;
};

