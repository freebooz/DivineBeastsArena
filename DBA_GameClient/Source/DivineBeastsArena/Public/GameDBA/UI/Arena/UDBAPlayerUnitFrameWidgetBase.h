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
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "Components/ProgressBar.h"
#include "UDBAPlayerUnitFrameWidgetBase.generated.h"

class UDBAPlayerUnitFrameWidgetController;

/**
 * UDBAPlayerUnitFrameWidgetBase
 * 玩家单元框Widget
 * 显示角色HP、能量、经验、终极能量等信息
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAPlayerUnitFrameWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAPlayerUnitFrameWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void SetWidgetController(UDBAPlayerUnitFrameWidgetController* InController);

	/** 更新生命值显示 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateHP(float CurrentHP, float MaxHP);

	/** 更新能量显示 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateEnergy(float CurrentEnergy, float MaxEnergy);

	/** 更新经验显示 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateXP(float CurrentXP, float MaxXP);

	/** 更新终极能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateUltimateEnergy(float Energy);

	/** 使用动态上限更新终极能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateUltimateEnergyWithMax(float Energy, float MaxEnergy);

	/** 更新等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateLevel(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void ApplyFiveCampTheme(uint8 FiveCamp);

protected:
	UFUNCTION()
	void HandleControllerHPUpdated(float CurrentHP, float MaxHP);

	UFUNCTION()
	void HandleControllerEnergyUpdated(float CurrentEnergy, float MaxEnergy);

	UFUNCTION()
	void HandleControllerLevelUpdated(int32 Level);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Update HP"))
	void BP_OnUpdateHP(float CurrentHP, float MaxHP, float Percentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Update Energy"))
	void BP_OnUpdateEnergy(float CurrentEnergy, float MaxEnergy, float Percentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Update XP"))
	void BP_OnUpdateXP(float CurrentXP, float MaxXP, float Percentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Update Ultimate Energy"))
	void BP_OnUpdateUltimateEnergy(float Energy, float Percentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Update Level"))
	void BP_OnUpdateLevel(int32 Level);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Apply FiveCamp Theme"))
	void BP_OnApplyFiveCampTheme(uint8 FiveCamp);

public:
	/** 生命条 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	/** 能量条 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> EnergyBar;

	/** 经验条 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> XPBar;

	/** 终极能量条 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> UltimateEnergyBar;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	TObjectPtr<UDBAPlayerUnitFrameWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedCurrentHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedMaxHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedCurrentEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedMaxEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedCurrentXP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedMaxXP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedUltimateEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedMaxUltimateEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	int32 CurrentLevel;
};
