// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/GAS/DBAAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/World.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Core/DBAInterfacesCore.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "GameDBA/GAS/Abilities/DBAElementSkillAbility_Generic.h"
#include "GameDBA/GAS/Abilities/DBAZodiacPassiveAbility_Generic.h"
#include "GameDBA/GAS/Abilities/DBAZodiacUltimateAbility_Generic.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/GAS/DBAAbilitySetLibrary.h"
#include "GameMoba/GAS/DBAMobaGameplayAbilityBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

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
		World->GetTimerManager().SetTimer(CooldownSyncTimerHandle, this, &UDBAAbilitySystemComponent::SyncCooldownsToCharacter, CooldownSyncInterval, true);
	}
}

void UDBAAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UltimateEnergyRegenTimerHandle);
		World->GetTimerManager().ClearTimer(ChainResetTimerHandle);
		World->GetTimerManager().ClearTimer(CooldownSyncTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UDBAAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
}

void UDBAAbilitySystemComponent::GrantAbilitiesFromFixedSkillGroup(const FName& FixedSkillGroupId)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	RemoveAllGrantedAbilities();

	UDBAFixedSkillGroupDataAsset* AbilitySet = UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(FixedSkillGroupId);
	auto GrantAbility = [this](TSubclassOf<UGameplayAbility> AbilityClass, int32 InputID)
	{
		if (!AbilityClass)
		{
			return;
		}

		FGameplayAbilitySpec Spec(AbilityClass, 1, InputID, this);
		const FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);
		GrantedAbilityHandles.Add(Handle);
		AbilityClassToHandleMap.Add(AbilityClass.Get(), Handle);
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
		GrantAbility(ActiveSkills[Index], SkillInputIDs[Index]);
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
	if (SameElementCount >= 5)
	{
		return 4;
	}
	if (SameElementCount >= 4)
	{
		return 3;
	}
	if (SameElementCount >= 3)
	{
		return 2;
	}
	if (SameElementCount >= 2)
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
}

void UDBAAbilitySystemComponent::AddUltimateEnergy(float Amount)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		UltimateEnergy = FMath::Clamp(UltimateEnergy + Amount, 0.0f, DBAConstants::MaxUltimateEnergy);
	}
}

bool UDBAAbilitySystemComponent::ConsumeUltimateEnergy(float Amount)
{
	if (GetOwnerRole() != ROLE_Authority || UltimateEnergy < Amount)
	{
		return false;
	}

	UltimateEnergy -= Amount;
	return true;
}

bool UDBAAbilitySystemComponent::HasEnoughUltimateEnergy(float Amount) const
{
	return UltimateEnergy >= Amount;
}

void UDBAAbilitySystemComponent::AddChainLevel(int32 Amount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	ChainLevel = FMath::Clamp(ChainLevel + Amount, 0, DBAConstants::MaxChainLevel);
	LastHitTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

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

	ChainLevel = 0;
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
	if (GetOwnerRole() == ROLE_Authority)
	{
		ResonanceLevel = FMath::Clamp(Level, 0, DBAConstants::MaxResonanceLevel);
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

	AActor* SourceActor = GetOwner();
	if (!SourceActor || !SourceActor->Implements<UDBATeamAgentInterface>() || !Target->Implements<UDBATeamAgentInterface>())
	{
		return false;
	}

	const int32 SourceTeamId = IDBATeamAgentInterface::Execute_GetTeamId(SourceActor);
	const int32 TargetTeamId = IDBATeamAgentInterface::Execute_GetTeamId(Target);
	return TargetTeamId != -1 && SourceTeamId != TargetTeamId;
}

void UDBAAbilitySystemComponent::TriggerGameplayCue(const FGameplayTag& CueTag, AActor* Target)
{
	if (!CueTag.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAAbilitySystemComponent] GameplayCue 标签无效。"));
		return;
	}

	FGameplayCueParameters CueParams;
	CueParams.Instigator = GetOwner();
	CueParams.EffectCauser = Target;
	ExecuteGameplayCue(CueTag, CueParams);
	OnSkillCueExecuted.Broadcast(CueTag.GetTagName(), Target);
}

void UDBAAbilitySystemComponent::PassiveRegenUltimateEnergy()
{
	AddUltimateEnergy(1.0f);
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
	constexpr int32 ExpectedSlots = 5;
	OutCooldowns.Init(0.0f, ExpectedSlots);

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
			SlotIndex = 4;
		}

		if (OutCooldowns.IsValidIndex(SlotIndex))
		{
			OutCooldowns[SlotIndex] = Ability->GetCooldownTimeRemaining(AbilityActorInfo.Get());
		}
	}
}

void UDBAAbilitySystemComponent::NormalizeSkillCooldowns(const TArray<float>& InCooldowns, TArray<float>& OutNormalized)
{
	constexpr int32 ExpectedSlots = 5;
	OutNormalized.Init(0.0f, ExpectedSlots);

	for (int32 Index = 0; Index < ExpectedSlots && Index < InCooldowns.Num(); ++Index)
	{
		OutNormalized[Index] = InCooldowns[Index];
	}
}

void UDBAAbilitySystemComponent::SyncCooldownsToCharacter()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
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
