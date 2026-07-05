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

class UDBAPlayerUnitFrameWidgetController;
class APlayerController;

USTRUCT(BlueprintType)
struct FDBAArenaEventFeedEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	FText Text;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	float Duration = 0.0f;
};

USTRUCT(BlueprintType)
struct FDBAArenaCombatAnnouncementEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	FText Text;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	float Duration = 0.0f;
};

USTRUCT(BlueprintType)
struct FDBAArenaObjectiveState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bIsCompleted = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	FText ObjectiveText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	float Progress = 0.0f;
};

USTRUCT(BlueprintType)
struct FDBAArenaCriticalStateHintState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bLowHP = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bLowEnergy = false;
};

USTRUCT(BlueprintType)
struct FDBAArenaUltimateReadyPromptState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	bool bIsShown = false;
};

USTRUCT(BlueprintType)
struct FDBAArenaStatusEffectEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Status")
	FString EffectId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Status")
	float Duration = 0.0f;
};

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaHUDWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBAArenaHUDWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	void InitializeController(APlayerController* InPlayerController) override;

	void UpdatePlayerHP(float CurrentHP, float MaxHP) override;

	void UpdatePlayerEnergy(float CurrentEnergy, float MaxEnergy);

	void UpdateUltimateEnergy(float CurrentEnergy, float MaxEnergy);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	UDBAPlayerUnitFrameWidgetController* GetPlayerUnitFrameWidgetController();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void SetPlayerUnitFrameWidgetController(UDBAPlayerUnitFrameWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void UpdatePlayerLevel(int32 InLevel);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void UpdateChainLevel(int32 InChainLevel);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void UpdateResonanceLevel(int32 InResonanceLevel);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void UpdateMomentum(int32 InMomentumLevel, float InMomentumProgress);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void AddStatusBuff(const FString& BuffId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void RemoveStatusBuff(const FString& BuffId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void ClearStatusBuffs();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void AddStatusDebuff(const FString& DebuffId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void RemoveStatusDebuff(const FString& DebuffId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void ClearStatusDebuffs();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void AddStatusCCEffect(const FString& CCId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void RemoveStatusCCEffect(const FString& CCId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Status")
	void ClearStatusCCEffects();

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD|Status")
	TArray<FDBAArenaStatusEffectEntry> GetActiveStatusBuffs() const { return ActiveStatusBuffs; }

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD|Status")
	TArray<FDBAArenaStatusEffectEntry> GetActiveStatusDebuffs() const { return ActiveStatusDebuffs; }

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD|Status")
	TArray<FDBAArenaStatusEffectEntry> GetActiveStatusCCEffects() const { return ActiveStatusCCEffects; }

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void ShowCombatAnnouncement(const FText& Text, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void ClearCombatAnnouncement();

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD|Feedback")
	const FDBAArenaCombatAnnouncementEntry& GetLastCombatAnnouncement() const { return LastCombatAnnouncement; }

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void UpdateCriticalStateHints(bool bLowHP, bool bLowEnergy);

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD|Feedback")
	const FDBAArenaCriticalStateHintState& GetLastCriticalStateHints() const { return LastCriticalStateHints; }

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void UpdateArenaObjective(const FText& ObjectiveText, float Progress);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void CompleteArenaObjective();

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD|Feedback")
	const FDBAArenaObjectiveState& GetLastArenaObjectiveState() const { return LastArenaObjectiveState; }

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void AddEventFeedEntry(const FText& Text, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void ClearEventFeed();

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD|Feedback")
	const FDBAArenaEventFeedEntry& GetLastEventFeedEntry() const { return LastEventFeedEntry; }

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void ShowUltimateReadyPrompt();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD|Feedback")
	void HideUltimateReadyPrompt();

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD|Feedback")
	const FDBAArenaUltimateReadyPromptState& GetLastUltimateReadyPromptState() const { return LastUltimateReadyPromptState; }

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD")
	float GetCurrentUltimateEnergy() const { return CurrentUltimateEnergy; }

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD")
	float GetMaxUltimateEnergy() const { return MaxUltimateEnergy; }

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD")
	int32 GetCurrentChainLevel() const { return CurrentChainLevel; }

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD")
	int32 GetCurrentResonanceLevel() const { return CurrentResonanceLevel; }

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD")
	int32 GetCurrentMomentumLevel() const { return CurrentMomentumLevel; }

	UFUNCTION(BlueprintPure, Category = "DBA|UI|ArenaHUD")
	float GetCurrentMomentumProgress() const { return CurrentMomentumProgress; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnPlayerEnergyChanged OnPlayerEnergyChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUltimateEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnUltimateEnergyChanged OnUltimateEnergyChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChainLevelChanged, int32, ChainLevel);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnChainLevelChanged OnChainLevelChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResonanceLevelChanged, int32, ResonanceLevel);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnResonanceLevelChanged OnResonanceLevelChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMomentumChanged, int32, MomentumLevel, float, MomentumProgress);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnMomentumChanged OnMomentumChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusEffectAdded, const FString&, EffectId, float, Duration);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectRemoved, const FString&, EffectId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatusEffectCleared);

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectAdded OnStatusBuffAdded;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectRemoved OnStatusBuffRemoved;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectCleared OnStatusBuffsCleared;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectAdded OnStatusDebuffAdded;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectRemoved OnStatusDebuffRemoved;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectCleared OnStatusDebuffsCleared;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectAdded OnStatusCCEffectAdded;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectRemoved OnStatusCCEffectRemoved;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Status")
	FOnStatusEffectCleared OnStatusCCEffectsCleared;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatAnnouncementShown, const FText&, Text, float, Duration);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatAnnouncementCleared);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCriticalStateHintsChanged, bool, bLowHP, bool, bLowEnergy);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaObjectiveUpdated, const FText&, ObjectiveText, float, Progress);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnArenaObjectiveCompleted);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEventFeedEntryAdded, const FText&, Text, float, Duration);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEventFeedCleared);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUltimateReadyPromptShown);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUltimateReadyPromptHidden);

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnCombatAnnouncementShown OnCombatAnnouncementShown;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnCombatAnnouncementCleared OnCombatAnnouncementCleared;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnCriticalStateHintsChanged OnCriticalStateHintsChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnArenaObjectiveUpdated OnArenaObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnArenaObjectiveCompleted OnArenaObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnEventFeedEntryAdded OnEventFeedEntryAdded;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnEventFeedCleared OnEventFeedCleared;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnUltimateReadyPromptShown OnUltimateReadyPromptShown;

	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD|Feedback")
	FOnUltimateReadyPromptHidden OnUltimateReadyPromptHidden;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float CurrentHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float MaxHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float CurrentEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float MaxEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	int32 CurrentLevel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float CurrentUltimateEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float MaxUltimateEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	int32 CurrentChainLevel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	int32 CurrentResonanceLevel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	int32 CurrentMomentumLevel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float CurrentMomentumProgress;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	FDBAArenaEventFeedEntry LastEventFeedEntry;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	FDBAArenaCombatAnnouncementEntry LastCombatAnnouncement;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	FDBAArenaObjectiveState LastArenaObjectiveState;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	FDBAArenaCriticalStateHintState LastCriticalStateHints;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Feedback")
	FDBAArenaUltimateReadyPromptState LastUltimateReadyPromptState;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Status")
	TArray<FDBAArenaStatusEffectEntry> ActiveStatusBuffs;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Status")
	TArray<FDBAArenaStatusEffectEntry> ActiveStatusDebuffs;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD|Status")
	TArray<FDBAArenaStatusEffectEntry> ActiveStatusCCEffects;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	TObjectPtr<UDBAPlayerUnitFrameWidgetController> PlayerUnitFrameWidgetController;
};
