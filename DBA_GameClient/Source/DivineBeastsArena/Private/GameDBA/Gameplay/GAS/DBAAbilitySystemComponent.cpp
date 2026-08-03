// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/GAS/DBAAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "GameDBA/Characters/DBAZodiacCharacterBase.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Gameplay/Abilities/DBAElementAbilityBase.h"
#include "GameDBA/Gameplay/Abilities/DBAElementSkillAbility_Generic.h"
#include "GameDBA/Gameplay/Abilities/DBAZodiacPassiveAbility_Generic.h"
#include "GameDBA/Gameplay/Abilities/DBAZodiacUltimateAbility_Generic.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameMoba/GAS/DBAMobaGameplayAbilityBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace
{
	int32 MapAbilityInputIDToCooldownSkillSlot(int32 InputID)
	{
		switch (static_cast<EDBAAbilityInputID>(InputID))
		{
		case EDBAAbilityInputID::Skill01:
			return 1;
		case EDBAAbilityInputID::Skill02:
			return 2;
		case EDBAAbilityInputID::Skill03:
			return 3;
		case EDBAAbilityInputID::Skill04:
			return 4;
		case EDBAAbilityInputID::Ultimate:
			return DBAConstants::ArenaCombatSkillSlotCount;
		default:
			return INDEX_NONE;
		}
	}

	bool ResolveActorTeamIdForAbilityTargeting(const AActor* Actor, int32& OutTeamId)
	{
		OutTeamId = 0;

		const ADBAZodiacCharacterBase* ZodiacCharacter = Cast<ADBAZodiacCharacterBase>(Actor);
		if (!ZodiacCharacter)
		{
			return false;
		}

		const int32 ResolvedTeamId = ZodiacCharacter->GetTeamID();
		if (ResolvedTeamId > 0)
		{
			OutTeamId = ResolvedTeamId;
			return true;
		}

		return false;
	}
}

UDBAAbilitySystemComponent::UDBAAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, UltimateEnergy(0.0f)
	, ChainLevel(0)
	, ResonanceLevel(0)
	, LastHitTime(0.0f)
{
	SetIsReplicatedByDefault(true);
}

void UDBAAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(UltimateEnergyRegenTimerHandle, this, &UDBAAbilitySystemComponent::PassiveRegenUltimateEnergy, 1.0f, true);
	}

	OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UDBAAbilitySystemComponent::HandleCooldownGameplayEffectAddedToSelf);
	OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UDBAAbilitySystemComponent::HandleCooldownGameplayEffectRemoved);
}

void UDBAAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
	OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UltimateEnergyRegenTimerHandle);
		World->GetTimerManager().ClearTimer(ChainResetTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UDBAAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
}

ADBAZodiacCharacterBase* UDBAAbilitySystemComponent::GetDBAAvatarCharacter() const
{
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (ActorInfo)
	{
		if (ADBAZodiacCharacterBase* AvatarCharacter = Cast<ADBAZodiacCharacterBase>(ActorInfo->AvatarActor.Get()))
		{
			return AvatarCharacter;
		}
	}

	return Cast<ADBAZodiacCharacterBase>(GetOwner());
}

void UDBAAbilitySystemComponent::GrantAbilitiesFromFixedSkillGroup(const FName& FixedSkillGroupId)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	RemoveAllGrantedAbilities();

	UDBAFixedSkillGroupDataAsset* AbilitySet = UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(FixedSkillGroupId);
	auto GrantAbility = [this](TSubclassOf<UGameplayAbility> AbilityClass, int32 InputID, const FDBAAbilityRuntimeConfig* RuntimeConfig = nullptr)
	{
		if (!AbilityClass)
		{
			return;
		}

		FGameplayAbilitySpec Spec(AbilityClass, 1, InputID, this);
		const FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);
		GrantedAbilityHandles.Add(Handle);
		AbilityClassToHandleMap.Add(AbilityClass.Get(), Handle);
		if (RuntimeConfig && InputID != INDEX_NONE)
		{
			AbilityRuntimeConfigsByInputID.Add(InputID, *RuntimeConfig);
		}
	};

	if (!AbilitySet)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAAbilitySystemComponent] \u672a\u627e\u5230\u56fa\u5b9a\u6280\u80fd\u7ec4\uff1a%s\uff0c\u5df2\u4f7f\u7528\u901a\u7528\u6280\u80fd\u515c\u5e95\u3002"), *FixedSkillGroupId.ToString());

		GrantAbility(UDBAZodiacPassiveAbility_Generic::StaticClass(), INDEX_NONE);
		GrantAbility(UDBAElementSkillAbility_Generic::StaticClass(), static_cast<int32>(EDBAAbilityInputID::Skill01));
		GrantAbility(UDBAElementSkillAbility_Generic::StaticClass(), static_cast<int32>(EDBAAbilityInputID::Skill02));
		GrantAbility(UDBAElementSkillAbility_Generic::StaticClass(), static_cast<int32>(EDBAAbilityInputID::Skill03));
		GrantAbility(UDBAElementSkillAbility_Generic::StaticClass(), static_cast<int32>(EDBAAbilityInputID::Skill04));
		GrantAbility(UDBAZodiacUltimateAbility_Generic::StaticClass(), static_cast<int32>(EDBAAbilityInputID::Ultimate));
		SetResonanceLevel(0);
		return;
	}

	GrantAbility(AbilitySet->PassiveAbilityClass, INDEX_NONE);

	TArray<TSubclassOf<UDBAElementAbilityBase>> ActiveSkills;
	ActiveSkills.Add(AbilitySet->Skill01Class);
	ActiveSkills.Add(AbilitySet->Skill02Class);
	ActiveSkills.Add(AbilitySet->Skill03Class);
	ActiveSkills.Add(AbilitySet->Skill04Class);

	const int32 SkillInputIDs[] =
	{
		static_cast<int32>(EDBAAbilityInputID::Skill01),
		static_cast<int32>(EDBAAbilityInputID::Skill02),
		static_cast<int32>(EDBAAbilityInputID::Skill03),
		static_cast<int32>(EDBAAbilityInputID::Skill04)
	};

	for (int32 Index = 0; Index < ActiveSkills.Num(); ++Index)
	{
		const FDBAAbilityRuntimeConfig* RuntimeConfig = nullptr;
		switch (static_cast<EDBAAbilityInputID>(SkillInputIDs[Index]))
		{
		case EDBAAbilityInputID::Skill01:
			RuntimeConfig = &AbilitySet->Skill01RuntimeConfig;
			break;
		case EDBAAbilityInputID::Skill02:
			RuntimeConfig = &AbilitySet->Skill02RuntimeConfig;
			break;
		case EDBAAbilityInputID::Skill03:
			RuntimeConfig = &AbilitySet->Skill03RuntimeConfig;
			break;
		case EDBAAbilityInputID::Skill04:
			RuntimeConfig = &AbilitySet->Skill04RuntimeConfig;
			break;
		default:
			break;
		}
		GrantAbility(ActiveSkills[Index], SkillInputIDs[Index], RuntimeConfig);
	}

	GrantAbility(AbilitySet->ZodiacUltimateClass, static_cast<int32>(EDBAAbilityInputID::Ultimate));
	GrantAbility(AbilitySet->ResonanceAbilityClass, INDEX_NONE);

	EDBAElement PrimaryElement = EDBAElement::None;
	int32 SameElementCount = 0;
	for (const TSubclassOf<UDBAElementAbilityBase>& SkillClass : ActiveSkills)
	{
		const UDBAElementAbilityBase* SkillCDO = SkillClass ? SkillClass->GetDefaultObject<UDBAElementAbilityBase>() : nullptr;
		if (!SkillCDO)
		{
			continue;
		}

		const EDBAElement SkillElement = SkillCDO->GetElementType();
		if (PrimaryElement == EDBAElement::None && SkillElement != EDBAElement::None)
		{
			PrimaryElement = SkillElement;
		}

		if (SkillElement == PrimaryElement && SkillElement != EDBAElement::None)
		{
			++SameElementCount;
		}
	}

	SetResonanceLevel(CalculateResonanceLevel(SameElementCount));
}

int32 UDBAAbilitySystemComponent::CalculateResonanceLevel(int32 SameElementCount)
{
	if (SameElementCount >= DBAConstants::ResonanceLevel4_SkillCount)
	{
		return DBAConstants::MaxResonanceLevel;
	}
	if (SameElementCount >= DBAConstants::ResonanceLevel3_SkillCount)
	{
		return 3;
	}
	if (SameElementCount >= DBAConstants::ResonanceLevel2_SkillCount)
	{
		return 2;
	}
	if (SameElementCount >= DBAConstants::ResonanceLevel1_SkillCount)
	{
		return 1;
	}
	return 0;
}

void UDBAAbilitySystemComponent::RemoveAllGrantedAbilities()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		ClearAbility(Handle);
	}

	GrantedAbilityHandles.Empty();
	AbilityClassToHandleMap.Empty();
	AbilityRuntimeConfigsByInputID.Empty();
}

void UDBAAbilitySystemComponent::AddUltimateEnergy(float Amount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	const float PreviousUltimateEnergy = UltimateEnergy;
	UltimateEnergy = FMath::Clamp(UltimateEnergy + Amount, 0.0f, DBAConstants::MaxUltimateEnergy);
	if (!FMath::IsNearlyEqual(PreviousUltimateEnergy, UltimateEnergy, KINDA_SMALL_NUMBER))
	{
		BroadcastUltimateEnergyChanged();
	}
}

bool UDBAAbilitySystemComponent::ConsumeUltimateEnergy(float Amount)
{
	if (GetOwnerRole() != ROLE_Authority || UltimateEnergy < Amount)
	{
		return false;
	}

	const float PreviousUltimateEnergy = UltimateEnergy;
	UltimateEnergy = FMath::Clamp(UltimateEnergy - Amount, 0.0f, DBAConstants::MaxUltimateEnergy);
	if (!FMath::IsNearlyEqual(PreviousUltimateEnergy, UltimateEnergy, KINDA_SMALL_NUMBER))
	{
		BroadcastUltimateEnergyChanged();
	}
	return true;
}

bool UDBAAbilitySystemComponent::HasEnoughUltimateEnergy(float Amount) const
{
	return UltimateEnergy >= Amount;
}

void UDBAAbilitySystemComponent::OnRep_UltimateEnergy()
{
	BroadcastUltimateEnergyChanged();
}

void UDBAAbilitySystemComponent::BroadcastUltimateEnergyChanged()
{
	OnUltimateEnergyChanged.Broadcast(UltimateEnergy, DBAConstants::MaxUltimateEnergy);
}

void UDBAAbilitySystemComponent::OnRep_ChainLevel()
{
	BroadcastChainLevelChanged();
}

void UDBAAbilitySystemComponent::BroadcastChainLevelChanged()
{
	OnChainLevelChanged.Broadcast(ChainLevel);
}

void UDBAAbilitySystemComponent::OnRep_ResonanceLevel()
{
	BroadcastResonanceLevelChanged();
}

void UDBAAbilitySystemComponent::BroadcastResonanceLevelChanged()
{
	OnResonanceLevelChanged.Broadcast(ResonanceLevel);
}

void UDBAAbilitySystemComponent::AddChainLevel(int32 Amount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	const int32 PreviousChainLevel = ChainLevel;
	ChainLevel = FMath::Clamp(ChainLevel + Amount, 0, DBAConstants::MaxChainLevel);
	LastHitTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (PreviousChainLevel != ChainLevel)
	{
		BroadcastChainLevelChanged();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChainResetTimerHandle);
		World->GetTimerManager().SetTimer(ChainResetTimerHandle, this, &UDBAAbilitySystemComponent::CheckChainReset, DBAConstants::ChainTimeout, false);
	}
}

void UDBAAbilitySystemComponent::ResetChainLevel()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	const int32 PreviousChainLevel = ChainLevel;
	ChainLevel = 0;
	if (PreviousChainLevel != ChainLevel)
	{
		BroadcastChainLevelChanged();
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChainResetTimerHandle);
	}
}

bool UDBAAbilitySystemComponent::ShouldTriggerChainFinisher() const
{
	return ChainLevel >= DBAConstants::MaxChainLevel;
}

void UDBAAbilitySystemComponent::SetResonanceLevel(int32 Level)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	const int32 PreviousResonanceLevel = ResonanceLevel;
	ResonanceLevel = FMath::Clamp(Level, 0, DBAConstants::MaxResonanceLevel);
	if (PreviousResonanceLevel != ResonanceLevel)
	{
		BroadcastResonanceLevelChanged();
	}
}

bool UDBAAbilitySystemComponent::CanActivateAbility(TSubclassOf<UDBAMobaGameplayAbilityBase> AbilityClass, AActor* Target) const
{
	if (GetOwnerRole() != ROLE_Authority || !AbilityClass)
	{
		return false;
	}

	const FGameplayAbilitySpecHandle* Handle = AbilityClassToHandleMap.Find(AbilityClass.Get());
	if (!Handle)
	{
		return false;
	}

	const FGameplayAbilitySpec* Spec = const_cast<UDBAAbilitySystemComponent*>(this)->FindAbilitySpecFromHandle(*Handle);
	if (!Spec)
	{
		return false;
	}

	const UDBABattleAttributeSet* AttrSet = GetSet<UDBABattleAttributeSet>();
	const UDBAMobaGameplayAbilityBase* AbilityCDO = AbilityClass->GetDefaultObject<UDBAMobaGameplayAbilityBase>();
	if (AttrSet && AbilityCDO && AbilityCDO->GetEnergyCost() > AttrSet->GetCurrentEnergy())
	{
		return false;
	}

	return !Target || IsValidTarget(Target, true);
}

bool UDBAAbilitySystemComponent::TryActivateAbilityByInputID(int32 InputID, AActor* Target)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	if (Target && !IsValid(Target))
	{
		return false;
	}

	if (IsInputAbilityOnCooldown(InputID))
	{
		return false;
	}

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		if (!Spec)
		{
			continue;
		}

		if (Spec->InputID == InputID)
		{
			const bool bActivated = TryActivateAbility(Spec->Handle, false);
			if (bActivated)
			{
				const FName SkillCueName = ResolveSkillCueNameForInputID(InputID);
				OnSkillCueExecuted.Broadcast(SkillCueName, Target ? Target : GetDBAAvatarCharacter());
				SyncCooldownsToCharacter();
			}
			return bActivated;
		}
	}

	return false;
}

bool UDBAAbilitySystemComponent::IsInputAbilityOnCooldown(int32 InputID) const
{
	const ADBAZodiacCharacterBase* Character = GetDBAAvatarCharacter();
	const int32 CooldownSkillSlot = MapAbilityInputIDToCooldownSkillSlot(InputID);
	if (!Character || CooldownSkillSlot == INDEX_NONE)
	{
		return false;
	}

	for (const FDBAPlayableSkillRuntimeSpec& SkillSpec : Character->GetPlayableSkillSpecs())
	{
		if (SkillSpec.SkillSlot == CooldownSkillSlot)
		{
			return Character->IsAbilityOnCooldown(SkillSpec.SkillId);
		}
	}

	return false;
}

FGameplayAbilitySpecHandle UDBAAbilitySystemComponent::FindAbilitySpecHandleByInputID(int32 InputID) const
{
	if (InputID == static_cast<int32>(EDBAAbilityInputID::None))
	{
		return FGameplayAbilitySpecHandle();
	}

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.InputID == InputID && Spec.Handle.IsValid())
		{
			return Spec.Handle;
		}
	}

	return FGameplayAbilitySpecHandle();
}

const FDBAAbilityRuntimeConfig* UDBAAbilitySystemComponent::FindAbilityRuntimeConfigByInputID(int32 InputID) const
{
	return AbilityRuntimeConfigsByInputID.Find(InputID);
}

FName UDBAAbilitySystemComponent::ResolveSkillCueNameForInputID(int32 InputID) const
{
	switch (static_cast<EDBAAbilityInputID>(InputID))
	{
	case EDBAAbilityInputID::Skill01:
		return FName(TEXT("Skill01"));
	case EDBAAbilityInputID::Skill02:
		return FName(TEXT("Skill02"));
	case EDBAAbilityInputID::Skill03:
		return FName(TEXT("Skill03"));
	case EDBAAbilityInputID::Skill04:
		return FName(TEXT("Skill04"));
	case EDBAAbilityInputID::Ultimate:
		return FName(TEXT("Ultimate"));
	default:
		return NAME_None;
	}
}

bool UDBAAbilitySystemComponent::IsValidTarget(AActor* Target, bool bRequireEnemy) const
{
	if (GetOwnerRole() != ROLE_Authority || !IsValid(Target))
	{
		return false;
	}

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(Target))
	{
		if (const UDBABattleAttributeSet* TargetAttrSet = TargetASC->GetSet<UDBABattleAttributeSet>())
		{
			if (TargetAttrSet->GetCurrentHealth() <= 0.0f)
			{
				return false;
			}
		}
	}

	if (!bRequireEnemy)
	{
		return true;
	}

	const AActor* SourceActor = GetDBAAvatarCharacter();
	int32 SourceTeamId = 0;
	int32 TargetTeamId = 0;
	if (!ResolveActorTeamIdForAbilityTargeting(SourceActor, SourceTeamId) || !ResolveActorTeamIdForAbilityTargeting(Target, TargetTeamId))
	{
		return false;
	}

	return SourceTeamId != TargetTeamId;
}

void UDBAAbilitySystemComponent::TriggerGameplayCue(const FGameplayTag& CueTag, AActor* Target)
{
	if (!CueTag.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAAbilitySystemComponent] GameplayCue 标签无效。"));
		return;
	}

	FGameplayCueParameters CueParams;
	ADBAZodiacCharacterBase* AvatarCharacter = GetDBAAvatarCharacter();
	CueParams.Instigator = AvatarCharacter ? AvatarCharacter : GetOwner();
	CueParams.EffectCauser = Target;
	ExecuteGameplayCue(CueTag, CueParams);
	OnSkillCueExecuted.Broadcast(CueTag.GetTagName(), Target);
}

void UDBAAbilitySystemComponent::PassiveRegenUltimateEnergy()
{
	AddUltimateEnergy(DBAConstants::UltimateEnergy_PassiveRegen);
}

void UDBAAbilitySystemComponent::CheckChainReset()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World && World->GetTimeSeconds() - LastHitTime >= DBAConstants::ChainTimeout)
	{
		ResetChainLevel();
	}
}

void UDBAAbilitySystemComponent::GetSkillCooldowns(TArray<float>& OutCooldowns) const
{
	constexpr int32 CooldownSlotCount = DBAConstants::ArenaCombatSkillSlotCount;
	OutCooldowns.Init(0.0f, CooldownSlotCount);

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		const FGameplayAbilitySpec* Spec = const_cast<UDBAAbilitySystemComponent*>(this)->FindAbilitySpecFromHandle(Handle);
		const UDBAMobaGameplayAbilityBase* Ability = Spec ? Cast<UDBAMobaGameplayAbilityBase>(Spec->Ability) : nullptr;
		if (!Ability)
		{
			continue;
		}

		int32 SlotIndex = INDEX_NONE;
		if (Spec->InputID == static_cast<int32>(EDBAAbilityInputID::Skill01))
		{
			SlotIndex = 0;
		}
		else if (Spec->InputID == static_cast<int32>(EDBAAbilityInputID::Skill02))
		{
			SlotIndex = 1;
		}
		else if (Spec->InputID == static_cast<int32>(EDBAAbilityInputID::Skill03))
		{
			SlotIndex = 2;
		}
		else if (Spec->InputID == static_cast<int32>(EDBAAbilityInputID::Skill04))
		{
			SlotIndex = 3;
		}
		else if (Spec->InputID == static_cast<int32>(EDBAAbilityInputID::Ultimate))
		{
			SlotIndex = DBAConstants::ActiveSkillCount;
		}

		if (OutCooldowns.IsValidIndex(SlotIndex))
		{
			float RemainingTime = 0.0f;
			float TotalDuration = 0.0f;
			Ability->GetCooldownTimeRemainingAndDuration(Spec->Handle, AbilityActorInfo.Get(), RemainingTime, TotalDuration);
			OutCooldowns[SlotIndex] = RemainingTime;
		}
	}
}

void UDBAAbilitySystemComponent::NormalizeSkillCooldowns(const TArray<float>& InCooldowns, TArray<float>& OutNormalized)
{
	constexpr int32 CooldownSlotCount = DBAConstants::ArenaCombatSkillSlotCount;
	OutNormalized.Init(0.0f, CooldownSlotCount);

	for (int32 Index = 0; Index < CooldownSlotCount && Index < InCooldowns.Num(); ++Index)
	{
		OutNormalized[Index] = InCooldowns[Index];
	}
}

void UDBAAbilitySystemComponent::HandleCooldownGameplayEffectAddedToSelf(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	if (TargetASC != this || GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);
	if (GrantedTags.HasTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown")), false)))
	{
		SyncCooldownsToCharacter();
	}
}

void UDBAAbilitySystemComponent::HandleCooldownGameplayEffectRemoved(const FActiveGameplayEffect& ActiveEffect)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	FGameplayTagContainer GrantedTags;
	ActiveEffect.Spec.GetAllGrantedTags(GrantedTags);
	if (GrantedTags.HasTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown")), false)))
	{
		SyncCooldownsToCharacter();
	}
}

void UDBAAbilitySystemComponent::SyncCooldownsToCharacter()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	ADBAZodiacCharacterBase* Character = GetDBAAvatarCharacter();
	if (!Character)
	{
		return;
	}

	TArray<float> Cooldowns;
	GetSkillCooldowns(Cooldowns);

	bool bHasChanged = CachedSkillCooldowns.Num() != Cooldowns.Num();
	for (int32 Index = 0; !bHasChanged && Index < Cooldowns.Num(); ++Index)
	{
		bHasChanged = !FMath::IsNearlyEqual(CachedSkillCooldowns[Index], Cooldowns[Index], 0.01f);
	}

	if (!bHasChanged)
	{
		return;
	}

	CachedSkillCooldowns = Cooldowns;
	Character->UpdateSkillCooldowns(Cooldowns);

	for (int32 Index = 0; Index < Cooldowns.Num(); ++Index)
	{
		OnSkillCooldownUpdated.Broadcast(Index, Cooldowns[Index]);
	}
	OnAllSkillCooldownsUpdated.Broadcast(Cooldowns);
}

void UDBAAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDBAAbilitySystemComponent, UltimateEnergy);
	DOREPLIFETIME(UDBAAbilitySystemComponent, ChainLevel);
	DOREPLIFETIME(UDBAAbilitySystemComponent, ResonanceLevel);
}
