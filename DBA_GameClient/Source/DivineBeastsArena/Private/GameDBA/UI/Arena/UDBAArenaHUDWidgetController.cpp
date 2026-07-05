// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetController.h"
#include "GameFramework/PlayerController.h"

namespace
{
bool NormalizeStatusEffectId(const FString& EffectId, FString& OutEffectId)
{
	OutEffectId = EffectId.TrimStartAndEnd();
	return !OutEffectId.IsEmpty();
}

bool NormalizeHUDFeedbackText(const FText& Text, FText& OutText)
{
	const FString NormalizedText = Text.ToString().TrimStartAndEnd();
	if (NormalizedText.IsEmpty())
	{
		return false;
	}

	OutText = FText::FromString(NormalizedText);
	return true;
}

bool UpsertStatusEffect(TArray<FDBAArenaStatusEffectEntry>& Effects, const FString& EffectId, float Duration, FString& OutEffectId)
{
	if (!NormalizeStatusEffectId(EffectId, OutEffectId))
	{
		return false;
	}

	const int32 ExistingIndex = Effects.IndexOfByPredicate(
		[&OutEffectId](const FDBAArenaStatusEffectEntry& Entry)
		{
			return Entry.EffectId == OutEffectId;
		});

	FDBAArenaStatusEffectEntry Entry;
	Entry.EffectId = OutEffectId;
	Entry.Duration = FMath::Max(0.0f, Duration);

	if (ExistingIndex != INDEX_NONE)
	{
		Effects[ExistingIndex] = Entry;
	}
	else
	{
		Effects.Add(MoveTemp(Entry));
	}

	return true;
}

bool RemoveStatusEffect(TArray<FDBAArenaStatusEffectEntry>& Effects, const FString& EffectId, FString& OutEffectId)
{
	if (!NormalizeStatusEffectId(EffectId, OutEffectId))
	{
		return false;
	}

	Effects.RemoveAll(
		[&OutEffectId](const FDBAArenaStatusEffectEntry& Entry)
		{
			return Entry.EffectId == OutEffectId;
		});
	return true;
}
}

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栫帺瀹跺睘鎬ч粯璁ゅ€? */
UDBAArenaHUDWidgetController::UDBAArenaHUDWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentHP(1000.0f)
	, MaxHP(1000.0f)
	, CurrentEnergy(100.0f)
	, MaxEnergy(100.0f)
	, CurrentLevel(1)
	, CurrentUltimateEnergy(0.0f)
	, MaxUltimateEnergy(DBAConstants::MaxUltimateEnergy)
	, CurrentChainLevel(0)
	, CurrentResonanceLevel(0)
	, CurrentMomentumLevel(0)
	, CurrentMomentumProgress(0.0f)
{
}

void UDBAArenaHUDWidgetController::InitializeController(APlayerController* InPlayerController)
{
	Super::InitializeController(InPlayerController);

	if (PlayerUnitFrameWidgetController)
	{
		PlayerUnitFrameWidgetController->SetOwningPlayerController(GetPlayerController());
		PlayerUnitFrameWidgetController->InitializeController();
		PlayerUnitFrameWidgetController->SetVitals(CurrentHP, MaxHP, CurrentEnergy, MaxEnergy);
		PlayerUnitFrameWidgetController->SetCurrentLevel(CurrentLevel);
	}
}

/**
 * 鏇存柊鐜╁鐢熷懡鍊? * @param InCurrentHP 褰撳墠鐢熷懡鍊? * @param InMaxHP 鏈€澶х敓鍛藉€? * 鏇存柊鍚庡箍鎾?OnPlayerHPChanged 浜嬩欢閫氱煡 UI 鏇存柊
 */
void UDBAArenaHUDWidgetController::UpdatePlayerHP(float InCurrentHP, float InMaxHP)
{
	CurrentHP = FMath::Max(0.0f, InCurrentHP);
	MaxHP = FMath::Max(0.0f, InMaxHP);
	OnPlayerHPChanged.Broadcast(CurrentHP, MaxHP);

	if (PlayerUnitFrameWidgetController)
	{
		PlayerUnitFrameWidgetController->SetVitals(CurrentHP, MaxHP, CurrentEnergy, MaxEnergy);
	}
}

/**
 * 鏇存柊鐜╁鑳介噺鍊? * @param InCurrentEnergy 褰撳墠鑳介噺鍊? * @param InMaxEnergy 鏈€澶ц兘閲忓€? * 鏇存柊鍚庡箍鎾?OnPlayerEnergyChanged 浜嬩欢閫氱煡 UI 鏇存柊
 */
void UDBAArenaHUDWidgetController::UpdatePlayerEnergy(float InCurrentEnergy, float InMaxEnergy)
{
	CurrentEnergy = FMath::Max(0.0f, InCurrentEnergy);
	MaxEnergy = FMath::Max(0.0f, InMaxEnergy);
	OnPlayerEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);

	if (PlayerUnitFrameWidgetController)
	{
		PlayerUnitFrameWidgetController->SetVitals(CurrentHP, MaxHP, CurrentEnergy, MaxEnergy);
	}
}

/**
 * 鏇存柊缁堟瀬鑳介噺鍊? * @param InCurrentEnergy 褰撳墠缁堟瀬鑳介噺鍊? * @param InMaxEnergy 鏈€澶х粓鏋佽兘閲忓€硷紙鍥哄畾100锛? * 鏇存柊鍚庡箍鎾?OnUltimateEnergyChanged 浜嬩欢閫氱煡 UI 鏇存柊
 * 鐢ㄤ簬澶ф嫑鍏呰兘鏄剧ず鍜屽氨缁彁绀? */
void UDBAArenaHUDWidgetController::UpdateUltimateEnergy(float InCurrentEnergy, float InMaxEnergy)
{
	MaxUltimateEnergy = FMath::Max(1.0f, InMaxEnergy);
	CurrentUltimateEnergy = FMath::Clamp(InCurrentEnergy, 0.0f, MaxUltimateEnergy);
	OnUltimateEnergyChanged.Broadcast(CurrentUltimateEnergy, MaxUltimateEnergy);
}

UDBAPlayerUnitFrameWidgetController* UDBAArenaHUDWidgetController::GetPlayerUnitFrameWidgetController()
{
	if (!PlayerUnitFrameWidgetController)
	{
		SetPlayerUnitFrameWidgetController(NewObject<UDBAPlayerUnitFrameWidgetController>(this));
	}

	return PlayerUnitFrameWidgetController;
}

void UDBAArenaHUDWidgetController::SetPlayerUnitFrameWidgetController(UDBAPlayerUnitFrameWidgetController* InController)
{
	PlayerUnitFrameWidgetController = InController;

	if (!PlayerUnitFrameWidgetController)
	{
		return;
	}

	PlayerUnitFrameWidgetController->SetOwningPlayerController(GetPlayerController());
	PlayerUnitFrameWidgetController->InitializeController();
	PlayerUnitFrameWidgetController->SetVitals(CurrentHP, MaxHP, CurrentEnergy, MaxEnergy);
	PlayerUnitFrameWidgetController->SetCurrentLevel(CurrentLevel);
}

void UDBAArenaHUDWidgetController::UpdatePlayerLevel(int32 InLevel)
{
	CurrentLevel = FMath::Max(1, InLevel);

	if (PlayerUnitFrameWidgetController)
	{
		PlayerUnitFrameWidgetController->SetCurrentLevel(CurrentLevel);
	}
}

void UDBAArenaHUDWidgetController::UpdateChainLevel(int32 InChainLevel)
{
	CurrentChainLevel = FMath::Clamp(InChainLevel, 0, DBAConstants::MaxChainLevel);
	OnChainLevelChanged.Broadcast(CurrentChainLevel);
}

void UDBAArenaHUDWidgetController::UpdateResonanceLevel(int32 InResonanceLevel)
{
	CurrentResonanceLevel = FMath::Clamp(InResonanceLevel, 0, DBAConstants::MaxResonanceLevel);
	OnResonanceLevelChanged.Broadcast(CurrentResonanceLevel);
}

void UDBAArenaHUDWidgetController::UpdateMomentum(int32 InMomentumLevel, float InMomentumProgress)
{
	CurrentMomentumLevel = FMath::Max(0, InMomentumLevel);
	CurrentMomentumProgress = FMath::Clamp(InMomentumProgress, 0.0f, 1.0f);
	OnMomentumChanged.Broadcast(CurrentMomentumLevel, CurrentMomentumProgress);
}

void UDBAArenaHUDWidgetController::AddStatusBuff(const FString& BuffId, float Duration)
{
	FString NormalizedBuffId;
	if (!UpsertStatusEffect(ActiveStatusBuffs, BuffId, Duration, NormalizedBuffId))
	{
		return;
	}
	OnStatusBuffAdded.Broadcast(NormalizedBuffId, FMath::Max(0.0f, Duration));
}

void UDBAArenaHUDWidgetController::RemoveStatusBuff(const FString& BuffId)
{
	FString NormalizedBuffId;
	if (!RemoveStatusEffect(ActiveStatusBuffs, BuffId, NormalizedBuffId))
	{
		return;
	}
	OnStatusBuffRemoved.Broadcast(NormalizedBuffId);
}

void UDBAArenaHUDWidgetController::ClearStatusBuffs()
{
	ActiveStatusBuffs.Reset();
	OnStatusBuffsCleared.Broadcast();
}

void UDBAArenaHUDWidgetController::AddStatusDebuff(const FString& DebuffId, float Duration)
{
	FString NormalizedDebuffId;
	if (!UpsertStatusEffect(ActiveStatusDebuffs, DebuffId, Duration, NormalizedDebuffId))
	{
		return;
	}
	OnStatusDebuffAdded.Broadcast(NormalizedDebuffId, FMath::Max(0.0f, Duration));
}

void UDBAArenaHUDWidgetController::RemoveStatusDebuff(const FString& DebuffId)
{
	FString NormalizedDebuffId;
	if (!RemoveStatusEffect(ActiveStatusDebuffs, DebuffId, NormalizedDebuffId))
	{
		return;
	}
	OnStatusDebuffRemoved.Broadcast(NormalizedDebuffId);
}

void UDBAArenaHUDWidgetController::ClearStatusDebuffs()
{
	ActiveStatusDebuffs.Reset();
	OnStatusDebuffsCleared.Broadcast();
}

void UDBAArenaHUDWidgetController::AddStatusCCEffect(const FString& CCId, float Duration)
{
	FString NormalizedCCId;
	if (!UpsertStatusEffect(ActiveStatusCCEffects, CCId, Duration, NormalizedCCId))
	{
		return;
	}
	OnStatusCCEffectAdded.Broadcast(NormalizedCCId, FMath::Max(0.0f, Duration));
}

void UDBAArenaHUDWidgetController::RemoveStatusCCEffect(const FString& CCId)
{
	FString NormalizedCCId;
	if (!RemoveStatusEffect(ActiveStatusCCEffects, CCId, NormalizedCCId))
	{
		return;
	}
	OnStatusCCEffectRemoved.Broadcast(NormalizedCCId);
}

void UDBAArenaHUDWidgetController::ClearStatusCCEffects()
{
	ActiveStatusCCEffects.Reset();
	OnStatusCCEffectsCleared.Broadcast();
}

void UDBAArenaHUDWidgetController::ShowCombatAnnouncement(const FText& Text, float Duration)
{
	FText NormalizedText;
	if (!NormalizeHUDFeedbackText(Text, NormalizedText))
	{
		return;
	}

	LastCombatAnnouncement.bIsValid = true;
	LastCombatAnnouncement.Text = NormalizedText;
	LastCombatAnnouncement.Duration = FMath::Max(0.0f, Duration);
	OnCombatAnnouncementShown.Broadcast(NormalizedText, LastCombatAnnouncement.Duration);
}

void UDBAArenaHUDWidgetController::ClearCombatAnnouncement()
{
	LastCombatAnnouncement = FDBAArenaCombatAnnouncementEntry();
	OnCombatAnnouncementCleared.Broadcast();
}

void UDBAArenaHUDWidgetController::UpdateCriticalStateHints(bool bLowHP, bool bLowEnergy)
{
	LastCriticalStateHints.bIsValid = true;
	LastCriticalStateHints.bLowHP = bLowHP;
	LastCriticalStateHints.bLowEnergy = bLowEnergy;
	OnCriticalStateHintsChanged.Broadcast(LastCriticalStateHints.bLowHP, LastCriticalStateHints.bLowEnergy);
}

void UDBAArenaHUDWidgetController::UpdateArenaObjective(const FText& ObjectiveText, float Progress)
{
	FText NormalizedObjectiveText;
	if (!NormalizeHUDFeedbackText(ObjectiveText, NormalizedObjectiveText))
	{
		return;
	}

	LastArenaObjectiveState.bIsValid = true;
	LastArenaObjectiveState.bIsCompleted = false;
	LastArenaObjectiveState.ObjectiveText = NormalizedObjectiveText;
	LastArenaObjectiveState.Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
	OnArenaObjectiveUpdated.Broadcast(NormalizedObjectiveText, LastArenaObjectiveState.Progress);
}

void UDBAArenaHUDWidgetController::CompleteArenaObjective()
{
	LastArenaObjectiveState.bIsValid = true;
	LastArenaObjectiveState.bIsCompleted = true;
	LastArenaObjectiveState.Progress = 1.0f;
	OnArenaObjectiveCompleted.Broadcast();
}

void UDBAArenaHUDWidgetController::AddEventFeedEntry(const FText& Text, float Duration)
{
	FText NormalizedText;
	if (!NormalizeHUDFeedbackText(Text, NormalizedText))
	{
		return;
	}

	LastEventFeedEntry.bIsValid = true;
	LastEventFeedEntry.Text = NormalizedText;
	LastEventFeedEntry.Duration = FMath::Max(0.0f, Duration);
	OnEventFeedEntryAdded.Broadcast(NormalizedText, LastEventFeedEntry.Duration);
}

void UDBAArenaHUDWidgetController::ClearEventFeed()
{
	LastEventFeedEntry = FDBAArenaEventFeedEntry();
	OnEventFeedCleared.Broadcast();
}

void UDBAArenaHUDWidgetController::ShowUltimateReadyPrompt()
{
	LastUltimateReadyPromptState.bIsValid = true;
	LastUltimateReadyPromptState.bIsShown = true;
	OnUltimateReadyPromptShown.Broadcast();
}

void UDBAArenaHUDWidgetController::HideUltimateReadyPrompt()
{
	LastUltimateReadyPromptState.bIsValid = true;
	LastUltimateReadyPromptState.bIsShown = false;
	OnUltimateReadyPromptHidden.Broadcast();
}
