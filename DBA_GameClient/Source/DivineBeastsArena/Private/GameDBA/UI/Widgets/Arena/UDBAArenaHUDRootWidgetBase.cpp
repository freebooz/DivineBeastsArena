// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Arena/UDBAArenaHUDRootWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/UI/Controllers/Arena/UDBAArenaHUDWidgetController.h"
#include "GameDBA/UI/Widgets/Arena/UDBAAbilityBarWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBAArenaObjectiveTrackerWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBABuffBarWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBACCBarWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBAChainUltimatePanelWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBACombatAnnouncementWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBACriticalStateHintWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBADebuffBarWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBAArenaEventFeedWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBAMomentumPanelWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBAPassiveAndResonancePanelWidgetBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBAPlayerUnitFrameWidgetBase.h"
#include "GameDBA/UI/Controllers/Arena/UDBAPlayerUnitFrameWidgetController.h"
#include "GameDBA/UI/Widgets/Arena/UDBAUltimateReadyPromptWidgetBase.h"
#include "GameDBA/Characters/DBAZodiacCharacterBase.h"

/**
 * 构造函数
 * 初始化 HUD 根 Widget
 * @param ObjectInitializer 对象初始化器
 */
UDBAArenaHUDRootWidgetBase::UDBAArenaHUDRootWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsEditMode(false)
{
}

/**
 * 原生构建回调
 * 当 Widget 构建到屏幕时调用，进行事件绑定等初始化操作
 */
void UDBAArenaHUDRootWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

/**
 * 原生销毁回调
 * 当 Widget 从屏幕移除时调用，进行事件解绑等清理操作
 */
void UDBAArenaHUDRootWidgetBase::NativeDestruct()
{
	if (WidgetController)
	{
		WidgetController->OnUltimateEnergyChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateEnergyUpdated);
		WidgetController->OnChainLevelChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerChainLevelUpdated);
		WidgetController->OnResonanceLevelChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerResonanceLevelUpdated);
		WidgetController->OnMomentumChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerMomentumUpdated);
		WidgetController->OnStatusBuffAdded.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffAdded);
		WidgetController->OnStatusBuffRemoved.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffRemoved);
		WidgetController->OnStatusBuffsCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffsCleared);
		WidgetController->OnStatusDebuffAdded.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffAdded);
		WidgetController->OnStatusDebuffRemoved.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffRemoved);
		WidgetController->OnStatusDebuffsCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffsCleared);
		WidgetController->OnStatusCCEffectAdded.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectAdded);
		WidgetController->OnStatusCCEffectRemoved.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectRemoved);
		WidgetController->OnStatusCCEffectsCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectsCleared);
		WidgetController->OnCombatAnnouncementShown.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementShown);
		WidgetController->OnCombatAnnouncementCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementCleared);
		WidgetController->OnCriticalStateHintsChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCriticalStateHintsChanged);
		WidgetController->OnArenaObjectiveUpdated.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerArenaObjectiveUpdated);
		WidgetController->OnArenaObjectiveCompleted.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerArenaObjectiveCompleted);
		WidgetController->OnEventFeedEntryAdded.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedEntryAdded);
		WidgetController->OnEventFeedCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedCleared);
		WidgetController->OnUltimateReadyPromptShown.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptShown);
		WidgetController->OnUltimateReadyPromptHidden.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptHidden);
	}

	Super::NativeDestruct();
}

/**
 * Widget 被激活时的回调
 * 当 HUD 显示时调用，可重写以执行显示逻辑
 */
void UDBAArenaHUDRootWidgetBase::NativeOnActivated()
{
}

/**
 * Widget 被停用时的回调
 * 当 HUD 隐藏时调用，可重写以执行隐藏逻辑
 */
void UDBAArenaHUDRootWidgetBase::NativeOnDeactivated()
{
}

/**
 * 设置 Widget 控制器
 * 将控制器与 Widget 关联，使 Widget 可以接收控制器更新的数据
 * @param InController HUD Widget 控制器指针
 */
void UDBAArenaHUDRootWidgetBase::SetWidgetController(UDBAArenaHUDWidgetController* InController)
{
	if (WidgetController)
	{
		WidgetController->OnUltimateEnergyChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateEnergyUpdated);
		WidgetController->OnChainLevelChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerChainLevelUpdated);
		WidgetController->OnResonanceLevelChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerResonanceLevelUpdated);
		WidgetController->OnMomentumChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerMomentumUpdated);
		WidgetController->OnStatusBuffAdded.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffAdded);
		WidgetController->OnStatusBuffRemoved.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffRemoved);
		WidgetController->OnStatusBuffsCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffsCleared);
		WidgetController->OnStatusDebuffAdded.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffAdded);
		WidgetController->OnStatusDebuffRemoved.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffRemoved);
		WidgetController->OnStatusDebuffsCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffsCleared);
		WidgetController->OnStatusCCEffectAdded.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectAdded);
		WidgetController->OnStatusCCEffectRemoved.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectRemoved);
		WidgetController->OnStatusCCEffectsCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectsCleared);
		WidgetController->OnCombatAnnouncementShown.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementShown);
		WidgetController->OnCombatAnnouncementCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementCleared);
		WidgetController->OnCriticalStateHintsChanged.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCriticalStateHintsChanged);
		WidgetController->OnArenaObjectiveUpdated.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerArenaObjectiveUpdated);
		WidgetController->OnArenaObjectiveCompleted.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerArenaObjectiveCompleted);
		WidgetController->OnEventFeedEntryAdded.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedEntryAdded);
		WidgetController->OnEventFeedCleared.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedCleared);
		WidgetController->OnUltimateReadyPromptShown.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptShown);
		WidgetController->OnUltimateReadyPromptHidden.RemoveDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptHidden);
	}

	WidgetController = InController;
	if (WidgetController)
	{
		WidgetController->OnUltimateEnergyChanged.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateEnergyUpdated);
		WidgetController->OnChainLevelChanged.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerChainLevelUpdated);
		WidgetController->OnResonanceLevelChanged.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerResonanceLevelUpdated);
		WidgetController->OnMomentumChanged.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerMomentumUpdated);
		WidgetController->OnStatusBuffAdded.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffAdded);
		WidgetController->OnStatusBuffRemoved.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffRemoved);
		WidgetController->OnStatusBuffsCleared.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffsCleared);
		WidgetController->OnStatusDebuffAdded.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffAdded);
		WidgetController->OnStatusDebuffRemoved.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffRemoved);
		WidgetController->OnStatusDebuffsCleared.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffsCleared);
		WidgetController->OnStatusCCEffectAdded.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectAdded);
		WidgetController->OnStatusCCEffectRemoved.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectRemoved);
		WidgetController->OnStatusCCEffectsCleared.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectsCleared);
		WidgetController->OnCombatAnnouncementShown.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementShown);
		WidgetController->OnCombatAnnouncementCleared.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementCleared);
		WidgetController->OnCriticalStateHintsChanged.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerCriticalStateHintsChanged);
		WidgetController->OnArenaObjectiveUpdated.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerArenaObjectiveUpdated);
		WidgetController->OnArenaObjectiveCompleted.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerArenaObjectiveCompleted);
		WidgetController->OnEventFeedEntryAdded.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedEntryAdded);
		WidgetController->OnEventFeedCleared.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedCleared);
		WidgetController->OnUltimateReadyPromptShown.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptShown);
		WidgetController->OnUltimateReadyPromptHidden.AddDynamic(this, &UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptHidden);
		HandleControllerUltimateEnergyUpdated(WidgetController->GetCurrentUltimateEnergy(), WidgetController->GetMaxUltimateEnergy());
		HandleControllerChainLevelUpdated(WidgetController->GetCurrentChainLevel());
		HandleControllerResonanceLevelUpdated(WidgetController->GetCurrentResonanceLevel());
		HandleControllerMomentumUpdated(WidgetController->GetCurrentMomentumLevel(), WidgetController->GetCurrentMomentumProgress());

		HandleControllerStatusBuffsCleared();
		for (const FDBAArenaStatusEffectEntry& ActiveBuff : WidgetController->GetActiveStatusBuffs())
		{
			HandleControllerStatusBuffAdded(ActiveBuff.EffectId, ActiveBuff.Duration);
		}

		HandleControllerStatusDebuffsCleared();
		for (const FDBAArenaStatusEffectEntry& ActiveDebuff : WidgetController->GetActiveStatusDebuffs())
		{
			HandleControllerStatusDebuffAdded(ActiveDebuff.EffectId, ActiveDebuff.Duration);
		}

		HandleControllerStatusCCEffectsCleared();
		for (const FDBAArenaStatusEffectEntry& ActiveCCEffect : WidgetController->GetActiveStatusCCEffects())
		{
			HandleControllerStatusCCEffectAdded(ActiveCCEffect.EffectId, ActiveCCEffect.Duration);
		}

		const FDBAArenaCriticalStateHintState& LastCriticalStateHints = WidgetController->GetLastCriticalStateHints();
		if (LastCriticalStateHints.bIsValid)
		{
			HandleControllerCriticalStateHintsChanged(LastCriticalStateHints.bLowHP, LastCriticalStateHints.bLowEnergy);
		}

		const FDBAArenaUltimateReadyPromptState& LastUltimateReadyPromptState = WidgetController->GetLastUltimateReadyPromptState();
		if (LastUltimateReadyPromptState.bIsValid)
		{
			if (LastUltimateReadyPromptState.bIsShown)
			{
				HandleControllerUltimateReadyPromptShown();
			}
			else
			{
				HandleControllerUltimateReadyPromptHidden();
			}
		}

		const FDBAArenaCombatAnnouncementEntry& LastCombatAnnouncement = WidgetController->GetLastCombatAnnouncement();
		if (LastCombatAnnouncement.bIsValid)
		{
			HandleControllerCombatAnnouncementShown(LastCombatAnnouncement.Text, LastCombatAnnouncement.Duration);
		}

		const FDBAArenaObjectiveState& LastArenaObjectiveState = WidgetController->GetLastArenaObjectiveState();
		if (LastArenaObjectiveState.bIsValid)
		{
			HandleControllerArenaObjectiveUpdated(LastArenaObjectiveState.ObjectiveText, LastArenaObjectiveState.Progress);
			if (LastArenaObjectiveState.bIsCompleted)
			{
				HandleControllerArenaObjectiveCompleted();
			}
		}

		const FDBAArenaEventFeedEntry& LastEventFeedEntry = WidgetController->GetLastEventFeedEntry();
		if (LastEventFeedEntry.bIsValid)
		{
			HandleControllerEventFeedEntryAdded(LastEventFeedEntry.Text, LastEventFeedEntry.Duration);
		}
	}

	SetPlayerUnitFrameWidgetController(WidgetController ? WidgetController->GetPlayerUnitFrameWidgetController() : nullptr);
}

void UDBAArenaHUDRootWidgetBase::SetPlayerUnitFrameWidgetController(UDBAPlayerUnitFrameWidgetController* InController)
{
	PlayerUnitFrameWidgetController = InController;

	if (PlayerUnitFrame)
	{
		PlayerUnitFrame->SetWidgetController(PlayerUnitFrameWidgetController);
	}
}

void UDBAArenaHUDRootWidgetBase::BindArenaHUDToCharacter(ADBAZodiacCharacterBase* InCharacter)
{
	BoundArenaHUDCharacter = InCharacter;

	if (AbilityBar)
	{
		AbilityBar->BindToCharacter(InCharacter);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerUltimateEnergyUpdated(float CurrentEnergy, float MaxEnergy)
{
	if (PlayerUnitFrame)
	{
		PlayerUnitFrame->UpdateUltimateEnergyWithMax(CurrentEnergy, MaxEnergy);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerChainLevelUpdated(int32 ChainLevel)
{
	if (ChainUltimatePanel)
	{
		ChainUltimatePanel->UpdateChainCount(ChainLevel);
		if (ChainLevel >= DBAConstants::MaxChainLevel)
		{
			ChainUltimatePanel->ShowChainReady();
		}
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerResonanceLevelUpdated(int32 ResonanceLevel)
{
	if (PassiveAndResonancePanel)
	{
		PassiveAndResonancePanel->UpdateResonanceLevel(ResonanceLevel);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerMomentumUpdated(int32 MomentumLevel, float MomentumProgress)
{
	if (MomentumPanel)
	{
		MomentumPanel->UpdateMomentumLevel(MomentumLevel);
		MomentumPanel->UpdateMomentumProgress(MomentumProgress);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffAdded(const FString& BuffId, float Duration)
{
	if (BuffBar)
	{
		BuffBar->AddBuff(BuffId, Duration);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffRemoved(const FString& BuffId)
{
	if (BuffBar)
	{
		BuffBar->RemoveBuff(BuffId);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusBuffsCleared()
{
	if (BuffBar)
	{
		BuffBar->ClearAllBuffs();
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffAdded(const FString& DebuffId, float Duration)
{
	if (DebuffBar)
	{
		DebuffBar->AddDebuff(DebuffId, Duration);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffRemoved(const FString& DebuffId)
{
	if (DebuffBar)
	{
		DebuffBar->RemoveDebuff(DebuffId);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusDebuffsCleared()
{
	if (DebuffBar)
	{
		DebuffBar->ClearAllDebuffs();
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectAdded(const FString& CCId, float Duration)
{
	if (CCBar)
	{
		CCBar->AddCCEffect(CCId, Duration);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectRemoved(const FString& CCId)
{
	if (CCBar)
	{
		CCBar->RemoveCCEffect(CCId);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerStatusCCEffectsCleared()
{
	if (CCBar)
	{
		CCBar->ClearAllCCEffects();
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementShown(const FText& Text, float Duration)
{
	if (CombatAnnouncement)
	{
		CombatAnnouncement->ShowAnnouncement(Text, Duration);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerCombatAnnouncementCleared()
{
	if (CombatAnnouncement)
	{
		CombatAnnouncement->ClearAnnouncement();
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerCriticalStateHintsChanged(bool bLowHP, bool bLowEnergy)
{
	if (CriticalStateHint)
	{
		CriticalStateHint->ShowCriticalHP(bLowHP);
		CriticalStateHint->ShowCriticalEnergy(bLowEnergy);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerArenaObjectiveUpdated(const FText& ObjectiveText, float Progress)
{
	if (ObjectiveTracker)
	{
		ObjectiveTracker->UpdateObjective(ObjectiveText, Progress);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerArenaObjectiveCompleted()
{
	if (ObjectiveTracker)
	{
		ObjectiveTracker->CompleteObjective();
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedEntryAdded(const FText& Text, float Duration)
{
	if (EventFeed)
	{
		EventFeed->AddEventEntry(Text, Duration);
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerEventFeedCleared()
{
	if (EventFeed)
	{
		EventFeed->ClearEventFeed();
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptShown()
{
	if (UltimateReadyPrompt)
	{
		UltimateReadyPrompt->ShowUltimateReady();
	}
}

void UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptHidden()
{
	if (UltimateReadyPrompt)
	{
		UltimateReadyPrompt->HideUltimateReady();
	}
}

/**
 * 设置 HUD 可见性
 * @param bVisible true 显示 HUD，false 隐藏 HUD
 */
void UDBAArenaHUDRootWidgetBase::SetHUDVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

/**
 * 设置 HUD 编辑模式
 * @param bEditMode true 编辑模式，false 普通模式
 * 编辑模式下可能显示额外调试信息
 */
void UDBAArenaHUDRootWidgetBase::SetHUDEditMode(bool bEditMode)
{
	bIsEditMode = bEditMode;
}

/**
 * 应用五阵营主题
 * 根据选择的阵营改变 HUD 的配色和样式
 * @param FiveCamp 阵营类型，0-4 对应五大阵营
 */
void UDBAArenaHUDRootWidgetBase::ApplyFiveCampTheme(uint8 FiveCamp)
{
	const uint8 NormalizedFiveCamp = FMath::Clamp(
		FiveCamp,
		static_cast<uint8>(EDBAFiveCamp::None),
		static_cast<uint8>(EDBAFiveCamp::Center));

	BP_OnApplyFiveCampTheme(NormalizedFiveCamp);
}
