// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Arena/UDBAArenaHUDRootWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"
#include "GameDBA/UI/Arena/UDBAAbilityBarWidgetBase.h"
#include "GameDBA/UI/Arena/UDBAArenaObjectiveTrackerWidgetBase.h"
#include "GameDBA/UI/Arena/UDBABuffBarWidgetBase.h"
#include "GameDBA/UI/Arena/UDBACCBarWidgetBase.h"
#include "GameDBA/UI/Arena/UDBAChainUltimatePanelWidgetBase.h"
#include "GameDBA/UI/Arena/UDBACombatAnnouncementWidgetBase.h"
#include "GameDBA/UI/Arena/UDBACriticalStateHintWidgetBase.h"
#include "GameDBA/UI/Arena/UDBADebuffBarWidgetBase.h"
#include "GameDBA/UI/Arena/UDBAArenaEventFeedWidgetBase.h"
#include "GameDBA/UI/Arena/UDBAMomentumPanelWidgetBase.h"
#include "GameDBA/UI/Arena/UDBAPassiveAndResonancePanelWidgetBase.h"
#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetBase.h"
#include "GameDBA/UI/Arena/UDBAPlayerUnitFrameWidgetController.h"
#include "GameDBA/UI/Arena/UDBAUltimateReadyPromptWidgetBase.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖?HUD 鏍?Widget
 * @param ObjectInitializer 瀵硅薄鍒濆鍖栧櫒
 */
UDBAArenaHUDRootWidgetBase::UDBAArenaHUDRootWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsEditMode(false)
{
}

/**
 * 鍘熺敓鏋勫缓鍥炶皟
 * 褰?Widget 鏋勫缓鍒板睆骞曟椂璋冪敤锛岃繘琛屼簨浠剁粦瀹氱瓑鍒濆鍖栨搷浣? */
void UDBAArenaHUDRootWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

/**
 * 鍘熺敓閿€姣佸洖璋? * 褰?Widget 浠庡睆骞曠Щ闄ゆ椂璋冪敤锛岃繘琛屼簨浠惰В缁戠瓑娓呯悊鎿嶄綔
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
 * 鍘熺敓 Tick 鍥炶皟
 * 姣忓抚鏇存柊 HUD 鐘舵€? * @param MyGeometry 褰撳墠 Widget 鍑犱綍淇℃伅
 * @param InDeltaTime 甯ч棿闅旀椂闂? */
void UDBAArenaHUDRootWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

/**
 * Widget 琚縺娲绘椂鐨勫洖璋? * 褰?HUD 鏄剧ず鏃惰皟鐢紝鍙噸鍐欎互鎵ц鏄剧ず閫昏緫
 */
void UDBAArenaHUDRootWidgetBase::NativeOnActivated()
{
}

/**
 * Widget 琚仠鐢ㄦ椂鐨勫洖璋? * 褰?HUD 闅愯棌鏃惰皟鐢紝鍙噸鍐欎互鎵ц闅愯棌閫昏緫
 */
void UDBAArenaHUDRootWidgetBase::NativeOnDeactivated()
{
}

/**
 * 璁剧疆 Widget 鎺у埗鍣? * 灏嗘帶鍒跺櫒涓?Widget 鍏宠仈锛屼娇 Widget 鍙互鎺ユ敹鎺у埗鍣ㄦ洿鏂扮殑鏁版嵁
 * @param InController HUD Widget 鎺у埗鍣ㄦ寚閽? */
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
 * 璁剧疆 HUD 鍙鎬? * @param bVisible true 鏄剧ず HUD锛宖alse 闅愯棌 HUD
 */
void UDBAArenaHUDRootWidgetBase::SetHUDVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

/**
 * 璁剧疆 HUD 缂栬緫妯″紡
 * @param bEditMode true 缂栬緫妯″紡锛宖alse 鏅€氭ā寮? * 缂栬緫妯″紡涓嬪彲鑳芥樉绀洪澶栬皟璇曚俊鎭? */
void UDBAArenaHUDRootWidgetBase::SetHUDEditMode(bool bEditMode)
{
	bIsEditMode = bEditMode;
}

/**
 * 搴旂敤浜斿ぇ闃佃惀涓婚
 * 鏍规嵁閫夋嫨鐨勯樀钀ユ敼鍙?HUD 鐨勯厤鑹插拰鏍峰紡
 * @param FiveCamp 闃佃惀绫诲瀷锛?-4 瀵瑰簲浜斿ぇ闃佃惀锛? */
void UDBAArenaHUDRootWidgetBase::ApplyFiveCampTheme(uint8 FiveCamp)
{
	const uint8 NormalizedFiveCamp = FMath::Clamp(
		FiveCamp,
		static_cast<uint8>(EDBAFiveCamp::None),
		static_cast<uint8>(EDBAFiveCamp::Center));

	BP_OnApplyFiveCampTheme(NormalizedFiveCamp);
}
