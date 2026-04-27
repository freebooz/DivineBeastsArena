// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/GAS/DBAMobaAbilitySystemComponentBase.h"
#include "MobaBase/Data/DBAMobaAbilitySetData.h"
#include "MobaBase/GAS/DBAMobaGameplayAbilityBase.h"
#include "Common/DBALogChannels.h"
#include "GameplayEffect.h"
#include "AttributeSet.h"

UDBAMobaAbilitySystemComponentBase::UDBAMobaAbilitySystemComponentBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDBAMobaAbilitySystemComponentBase::InitializeComponent()
{
	Super::InitializeComponent();
}

void UDBAMobaAbilitySystemComponentBase::BeginPlay()
{
	Super::BeginPlay();
}

void UDBAMobaAbilitySystemComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理所有映射
	InputIDToAbilitySpecMap.Reset();
	InputTagToAbilitySpecMap.Reset();

	Super::EndPlay(EndPlayReason);
}

// ========== Ability 赋予 / 移除 ==========

FDBAMobaAbilityGrantHandle UDBAMobaAbilitySystemComponentBase::GrantAbility(const FDBAMobaAbilityGrantConfig& Config)
{
	FDBAMobaAbilityGrantHandle Handle;

	if (!Config.AbilityClass.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantAbility 失败：AbilityClass 无效"));
		return Handle;
	}

	// 加载 Ability 类
	TSubclassOf<UDBAMobaGameplayAbilityBase> AbilityClass = Config.AbilityClass.LoadSynchronous();
	if (!AbilityClass)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantAbility 失败：无法加载 AbilityClass"));
		return Handle;
	}

	// 创建 Ability Spec
	FGameplayAbilitySpec AbilitySpec(AbilityClass, Config.AbilityLevel, Config.InputID, GetOwner());

	// 赋予 Ability
	Handle.AbilitySpecHandle = GiveAbility(AbilitySpec);

	// 记录输入映射
	if (Config.InputID >= 0)
	{
		InputIDToAbilitySpecMap.Add(Config.InputID, Handle.AbilitySpecHandle);
	}

	// 记录输入 Tag 映射
	if (UDBAMobaGameplayAbilityBase* AbilityCDO = AbilityClass->GetDefaultObject<UDBAMobaGameplayAbilityBase>())
	{
		if (AbilityCDO->InputTag.IsValid())
		{
			InputTagToAbilitySpecMap.Add(AbilityCDO->InputTag, Handle.AbilitySpecHandle);
		}
	}

	return Handle;
}

void UDBAMobaAbilitySystemComponentBase::RemoveAbility(const FDBAMobaAbilityGrantHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	// 移除输入映射
	for (auto It = InputIDToAbilitySpecMap.CreateIterator(); It; ++It)
	{
		if (It.Value() == Handle.AbilitySpecHandle)
		{
			It.RemoveCurrent();
			break;
		}
	}

	for (auto It = InputTagToAbilitySpecMap.CreateIterator(); It; ++It)
	{
		if (It.Value() == Handle.AbilitySpecHandle)
		{
			It.RemoveCurrent();
			break;
		}
	}

	// 移除 Ability（检查 Spec 是否仍然有效）
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle.AbilitySpecHandle);
	if (Spec)
	{
		ClearAbility(Handle.AbilitySpecHandle);
	}
}

TArray<FDBAMobaAbilityGrantHandle> UDBAMobaAbilitySystemComponentBase::GrantAbilities(const TArray<FDBAMobaAbilityGrantConfig>& Configs)
{
	TArray<FDBAMobaAbilityGrantHandle> Handles;
	for (const FDBAMobaAbilityGrantConfig& Config : Configs)
	{
		Handles.Add(GrantAbility(Config));
	}
	return Handles;
}

void UDBAMobaAbilitySystemComponentBase::RemoveAbilities(const TArray<FDBAMobaAbilityGrantHandle>& Handles)
{
	for (const FDBAMobaAbilityGrantHandle& Handle : Handles)
	{
		RemoveAbility(Handle);
	}
}

// ========== GameplayEffect 赋予 / 移除 ==========

FDBAMobaEffectGrantHandle UDBAMobaAbilitySystemComponentBase::GrantEffect(const FDBAMobaEffectGrantConfig& Config)
{
	FDBAMobaEffectGrantHandle Handle;

	if (!Config.EffectClass.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantEffect 失败：EffectClass 无效"));
		return Handle;
	}

	// 加载 Effect 类
	TSubclassOf<UGameplayEffect> EffectClass = Config.EffectClass.LoadSynchronous();
	if (!EffectClass)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantEffect 失败：无法加载 EffectClass"));
		return Handle;
	}

	// 创建 Effect Context
	FGameplayEffectContextHandle EffectContext = MakeEffectContext();
	EffectContext.AddSourceObject(GetOwner());

	// 创建 Effect Spec
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, Config.EffectLevel, EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantEffect 失败：无法创建 EffectSpec"));
		return Handle;
	}

	// 应用 Effect
	Handle.EffectHandle = ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	return Handle;
}

void UDBAMobaAbilitySystemComponentBase::RemoveEffect(const FDBAMobaEffectGrantHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	RemoveActiveGameplayEffect(Handle.EffectHandle);
}

TArray<FDBAMobaEffectGrantHandle> UDBAMobaAbilitySystemComponentBase::GrantEffects(const TArray<FDBAMobaEffectGrantConfig>& Configs)
{
	TArray<FDBAMobaEffectGrantHandle> Handles;
	for (const FDBAMobaEffectGrantConfig& Config : Configs)
	{
		Handles.Add(GrantEffect(Config));
	}
	return Handles;
}

void UDBAMobaAbilitySystemComponentBase::RemoveEffects(const TArray<FDBAMobaEffectGrantHandle>& Handles)
{
	for (const FDBAMobaEffectGrantHandle& Handle : Handles)
	{
		RemoveEffect(Handle);
	}
}

// ========== AttributeSet 赋予 / 移除 ==========

FDBAMobaAttributeSetGrantHandle UDBAMobaAbilitySystemComponentBase::GrantAttributeSet(const FDBAMobaAttributeSetGrantConfig& Config)
{
	FDBAMobaAttributeSetGrantHandle Handle;

	if (!Config.AttributeSetClass.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantAttributeSet 失败：AttributeSetClass 无效"));
		return Handle;
	}

	// 加载 AttributeSet 类
	TSubclassOf<UAttributeSet> AttributeSetClass = Config.AttributeSetClass.LoadSynchronous();
	if (!AttributeSetClass)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantAttributeSet 失败：无法加载 AttributeSetClass"));
		return Handle;
	}

	// 创建 AttributeSet 实例
	UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(GetOwner(), AttributeSetClass);
	if (!NewAttributeSet)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantAttributeSet 失败：无法创建 AttributeSet 实例"));
		return Handle;
	}

	// 添加到 ASC
	AddAttributeSetSubobject(NewAttributeSet);

	Handle.AttributeSet = NewAttributeSet;
	return Handle;
}

void UDBAMobaAbilitySystemComponentBase::RemoveAttributeSet(const FDBAMobaAttributeSetGrantHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	// 从 ASC 移除 AttributeSet
	RemoveSpawnedAttribute(Handle.AttributeSet);
}

TArray<FDBAMobaAttributeSetGrantHandle> UDBAMobaAbilitySystemComponentBase::GrantAttributeSets(const TArray<FDBAMobaAttributeSetGrantConfig>& Configs)
{
	TArray<FDBAMobaAttributeSetGrantHandle> Handles;
	for (const FDBAMobaAttributeSetGrantConfig& Config : Configs)
	{
		Handles.Add(GrantAttributeSet(Config));
	}
	return Handles;
}

void UDBAMobaAbilitySystemComponentBase::RemoveAttributeSets(const TArray<FDBAMobaAttributeSetGrantHandle>& Handles)
{
	for (const FDBAMobaAttributeSetGrantHandle& Handle : Handles)
	{
		RemoveAttributeSet(Handle);
	}
}

// ========== AbilitySet 赋予 / 移除 ==========

FDBAMobaAbilitySetGrantResult UDBAMobaAbilitySystemComponentBase::GrantAbilitySet(UDBAMobaAbilitySetData* AbilitySetData)
{
	FDBAMobaAbilitySetGrantResult Result;

	if (!AbilitySetData)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaASC] GrantAbilitySet 失败：AbilitySetData 为空"));
		return Result;
	}

	// 赋予 Abilities
	Result.GrantedAbilities = GrantAbilities(AbilitySetData->Abilities);

	// 赋予 Effects
	Result.GrantedEffects = GrantEffects(AbilitySetData->Effects);

	// 赋予 AttributeSets
	Result.GrantedAttributeSets = GrantAttributeSets(AbilitySetData->AttributeSets);

	return Result;
}

void UDBAMobaAbilitySystemComponentBase::RemoveAbilitySet(const FDBAMobaAbilitySetGrantResult& GrantResult)
{
	// 移除 Abilities
	RemoveAbilities(GrantResult.GrantedAbilities);

	// 移除 Effects
	RemoveEffects(GrantResult.GrantedEffects);

	// 移除 AttributeSets
	RemoveAttributeSets(GrantResult.GrantedAttributeSets);
}

// ========== 输入绑定 ==========

void UDBAMobaAbilitySystemComponentBase::BindAbilityToInput(int32 InputID, FGameplayTag InputTag)
{
	if (InputTag.IsValid())
	{
		if (FGameplayAbilitySpecHandle* FoundHandle = InputTagToAbilitySpecMap.Find(InputTag))
		{
			InputIDToAbilitySpecMap.Add(InputID, *FoundHandle);
		}
	}
}

void UDBAMobaAbilitySystemComponentBase::UnbindAbilityFromInput(int32 InputID)
{
	InputIDToAbilitySpecMap.Remove(InputID);
}

void UDBAMobaAbilitySystemComponentBase::TryActivateAbilityByInputTag(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	if (FGameplayAbilitySpecHandle* FoundHandle = InputTagToAbilitySpecMap.Find(InputTag))
	{
		TryActivateAbility(*FoundHandle);
	}
}

// ========== 辅助函数 ==========

TArray<UDBAMobaGameplayAbilityBase*> UDBAMobaAbilitySystemComponentBase::GetAllGrantedAbilities() const
{
	TArray<UDBAMobaGameplayAbilityBase*> Result;

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (UDBAMobaGameplayAbilityBase* Ability = Cast<UDBAMobaGameplayAbilityBase>(Spec.Ability))
		{
			Result.Add(Ability);
		}
	}

	return Result;
}

UDBAMobaGameplayAbilityBase* UDBAMobaAbilitySystemComponentBase::FindAbilityByTag(FGameplayTag AbilityTag) const
{
	if (!AbilityTag.IsValid())
	{
		return nullptr;
	}

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (UDBAMobaGameplayAbilityBase* Ability = Cast<UDBAMobaGameplayAbilityBase>(Spec.Ability))
		{
			if (Ability->AbilityTag.MatchesTagExact(AbilityTag))
			{
				return Ability;
			}
		}
	}

	return nullptr;
}
