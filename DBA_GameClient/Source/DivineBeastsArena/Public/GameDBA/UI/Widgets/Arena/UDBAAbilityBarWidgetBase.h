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
#include "GameDBA/Gameplay/Loadout/DBAPlayableSkillTypes.h"
#include "UDBAAbilityBarWidgetBase.generated.h"

class ADBAZodiacCharacterBase;
class APawn;
class APlayerController;
class UDBAAbilitySlotWidget;
class UDBAPlayableSkillComponent;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAAbilityBarWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAAbilityBarWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AbilityBar")
	void UpdateAbility(int32 SlotIndex, float Cooldown, float ManaCost);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AbilityBar")
	void SetAbilityEnabled(int32 SlotIndex, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AbilityBar")
	void BindToCharacter(ADBAZodiacCharacterBase* InCharacter);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AbilityBar")
	void RefreshSkillCatalog();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AbilityBar")
	void RefreshCooldowns();

	UFUNCTION(BlueprintPure, Category = "DBA|UI|AbilityBar")
	const FDBAPlayableSkillCatalogSummary& GetCachedSkillCatalogSummary() const { return CachedSkillCatalogSummary; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|AbilityBar", meta = (DisplayName = "On Ability Updated"))
	void BP_OnAbilityUpdated(int32 SlotIndex, float Cooldown, float ManaCost, bool bOnCooldown);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|AbilityBar", meta = (DisplayName = "On Skill Catalog Refreshed"))
	void BP_OnSkillCatalogRefreshed(const TArray<FDBAPlayableSkillRuntimeSpec>& SkillSpecs);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|AbilityBar", meta = (DisplayName = "On Skill Catalog Summary Refreshed"))
	void BP_OnSkillCatalogSummaryRefreshed(const FDBAPlayableSkillCatalogSummary& SkillCatalogSummary);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|AbilityBar", meta = (DisplayName = "On Ability Enabled Changed"))
	void BP_OnAbilityEnabledChanged(int32 SlotIndex, bool bEnabled);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|AbilityBar", meta = (BindWidgetOptional))
	TObjectPtr<UDBAAbilitySlotWidget> SkillSlot01;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|AbilityBar", meta = (BindWidgetOptional))
	TObjectPtr<UDBAAbilitySlotWidget> SkillSlot02;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|AbilityBar", meta = (BindWidgetOptional))
	TObjectPtr<UDBAAbilitySlotWidget> SkillSlot03;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|AbilityBar", meta = (BindWidgetOptional))
	TObjectPtr<UDBAAbilitySlotWidget> SkillSlot04;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|AbilityBar", meta = (BindWidgetOptional))
	TObjectPtr<UDBAAbilitySlotWidget> SkillSlot05;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|AbilityBar", meta = (BindWidgetOptional))
	TObjectPtr<UDBAAbilitySlotWidget> SkillSlot06;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|AbilityBar")
	bool bAutoBindOwningPawn = true;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DBA|UI|AbilityBar")
	TWeakObjectPtr<ADBAZodiacCharacterBase> BoundCharacter;

	UPROPERTY(Transient)
	TWeakObjectPtr<ADBAZodiacCharacterBase> CooldownEventCharacter;

	UPROPERTY(Transient)
	TWeakObjectPtr<UDBAPlayableSkillComponent> SkillCatalogEventComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerController> PawnEventController;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDBAAbilitySlotWidget>> SkillSlotWidgets;

	UPROPERTY(Transient)
	TArray<FDBAPlayableSkillRuntimeSpec> CachedSkillSpecs;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DBA|UI|AbilityBar")
	FDBAPlayableSkillCatalogSummary CachedSkillCatalogSummary;

	UFUNCTION()
	void HandleSkillCooldownsChanged(const TArray<float>& Cooldowns);

	UFUNCTION()
	void HandleSkillCatalogChanged(const FDBAPlayableSkillCatalogSummary& Summary);

	UFUNCTION()
	void HandleOwningPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void CacheSkillSlotWidgets();
	void UnbindFromCooldownEvents();
	void BindToOwningPlayerPawnEvent();
	void UnbindFromOwningPlayerPawnEvent();
	void ApplyRuntimeConfigToSkillSpec(FDBAPlayableSkillRuntimeSpec& InOutSkillSpec) const;
	int32 MapSkillSlotToAbilityInputID(int32 SkillSlot) const;
	void RequestSkillIconAsync(UDBAAbilitySlotWidget* SlotWidget, const FDBAPlayableSkillRuntimeSpec& SkillSpec);
	FKey ResolveHotkeyForSlot(int32 SkillSlot) const;
};
