// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBAPlayableSkillComponent.h"

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Combat/DBABloomHealingSpell.h"
#include "GameDBA/Combat/DBAChainLightningSpell.h"
#include "GameDBA/Combat/DBAHolyShieldSpell.h"
#include "GameDBA/Combat/DBAPlayableSkillDeveloperSettings.h"
#include "GameDBA/Combat/DBASkillProjectileBase.h"
#include "GameDBA/Services/DBASkillGroupGeneratorSubsystem.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
#include "GameCore/Core/DBALogChannels.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

namespace
{
	EDBAZodiac ToPlayableSkillCommonZodiac(EDBAZodiacType ZodiacType)
	{
		switch (ZodiacType)
		{
		case EDBAZodiacType::Rat: return EDBAZodiac::Rat;
		case EDBAZodiacType::Ox: return EDBAZodiac::Ox;
		case EDBAZodiacType::Tiger: return EDBAZodiac::Tiger;
		case EDBAZodiacType::Rabbit: return EDBAZodiac::Rabbit;
		case EDBAZodiacType::Dragon: return EDBAZodiac::Dragon;
		case EDBAZodiacType::Snake: return EDBAZodiac::Snake;
		case EDBAZodiacType::Horse: return EDBAZodiac::Horse;
		case EDBAZodiacType::Goat: return EDBAZodiac::Goat;
		case EDBAZodiacType::Monkey: return EDBAZodiac::Monkey;
		case EDBAZodiacType::Rooster: return EDBAZodiac::Rooster;
		case EDBAZodiacType::Dog: return EDBAZodiac::Dog;
		case EDBAZodiacType::Pig: return EDBAZodiac::Pig;
		default: return EDBAZodiac::Rat;
		}
	}

	EDBAElement ToPlayableSkillCommonElement(EDBAElementType ElementType)
	{
		switch (ElementType)
		{
		case EDBAElementType::Fire: return EDBAElement::Fire;
		case EDBAElementType::Water: return EDBAElement::Water;
		case EDBAElementType::Wood: return EDBAElement::Wood;
		case EDBAElementType::Metal: return EDBAElement::Gold;
		case EDBAElementType::Earth: return EDBAElement::Earth;
		default: return EDBAElement::Fire;
		}
	}

}

UDBAPlayableSkillComponent::UDBAPlayableSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ResetToDefaultSkillSpecs();
}

void UDBAPlayableSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	RequestDefaultSkillCatalogAsync();

	TArray<FString> ValidationErrors;
	if (!ValidateEffectiveSkillSpecs(ValidationErrors))
	{
		for (const FString& ValidationError : ValidationErrors)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAPlayableSkillComponent] 可用技能目录校验失败：拥有者=%s 错误=%s"), *GetNameSafe(GetOwner()), *ValidationError);
		}
	}

	PreloadSkillPresentationAssets();
	QueueNiagaraWarmupAssets();
}

bool UDBAPlayableSkillComponent::GetSkillSpec(int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec) const
{
	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);

	for (const FDBAPlayableSkillRuntimeSpec& Spec : EffectiveSpecs)
	{
		if (Spec.SkillSlot == SkillSlot)
		{
			OutSpec = Spec;
			if (bResolveSkillIdsFromEquippedSkillGroup)
			{
				OutSpec.SkillId = ResolveEquippedSkillId(SkillSlot, Spec.SkillId);
			}
			return true;
		}
	}

	return false;
}

TArray<FDBAPlayableSkillRuntimeSpec> UDBAPlayableSkillComponent::GetAllSkillSpecs() const
{
	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);

	TArray<FDBAPlayableSkillRuntimeSpec> ResolvedSpecs;
	ResolvedSpecs.Reserve(EffectiveSpecs.Num());
	for (const FDBAPlayableSkillRuntimeSpec& Spec : EffectiveSpecs)
	{
		FDBAPlayableSkillRuntimeSpec ResolvedSpec = Spec;
		if (bResolveSkillIdsFromEquippedSkillGroup)
		{
			ResolvedSpec.SkillId = ResolveEquippedSkillId(Spec.SkillSlot, Spec.SkillId);
		}
		ResolvedSpecs.Add(ResolvedSpec);
	}
	return ResolvedSpecs;
}

void UDBAPlayableSkillComponent::SetSkillSpec(int32 SkillSlot, const FDBAPlayableSkillRuntimeSpec& InSpec)
{
	FDBAPlayableSkillRuntimeSpec NormalizedSpec = InSpec;
	NormalizedSpec.SkillSlot = SkillSlot;

	for (FDBAPlayableSkillRuntimeSpec& Spec : SkillSpecs)
	{
		if (Spec.SkillSlot == SkillSlot)
		{
			Spec = NormalizedSpec;
			if (HasBegunPlay())
			{
				PreloadSkillPresentationAssets();
			}
			return;
		}
	}

	SkillSpecs.Add(NormalizedSpec);
	SkillSpecs.Sort([](const FDBAPlayableSkillRuntimeSpec& Left, const FDBAPlayableSkillRuntimeSpec& Right)
	{
		return Left.SkillSlot < Right.SkillSlot;
	});
	if (HasBegunPlay())
	{
		PreloadSkillPresentationAssets();
	}
}

void UDBAPlayableSkillComponent::ResetToDefaultSkillSpecs()
{
	SkillSpecs.Reset();

	if (HasBegunPlay())
	{
		RequestDefaultSkillCatalogAsync();
		PreloadSkillPresentationAssets();
	}
}

void UDBAPlayableSkillComponent::RequestDefaultSkillCatalogAsync()
{
	if (SkillCatalog || LoadedDefaultSkillCatalog)
	{
		return;
	}

	const TSoftObjectPtr<UDBAPlayableSkillCatalogDataAsset> CatalogAsset = ResolveDefaultSkillCatalogAsset();
	if (CatalogAsset.IsNull())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAPlayableSkillComponent] 默认可玩技能目录未配置：拥有者=%s。请在组件 DefaultSkillCatalog 或 DBA 可玩技能设置中配置数据资产。"), *GetNameSafe(GetOwner()));
		return;
	}

	if (UDBAPlayableSkillCatalogDataAsset* ResolvedCatalog = CatalogAsset.Get())
	{
		HandleDefaultSkillCatalogLoaded(ResolvedCatalog);
		return;
	}

	TWeakObjectPtr<UDBAPlayableSkillComponent> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<UDBAPlayableSkillCatalogDataAsset>(this, CatalogAsset, [WeakThis](UDBAPlayableSkillCatalogDataAsset* LoadedCatalog)
	{
		if (UDBAPlayableSkillComponent* StrongThis = WeakThis.Get())
		{
			StrongThis->HandleDefaultSkillCatalogLoaded(LoadedCatalog);
		}
	});
}

void UDBAPlayableSkillComponent::SetSkillCatalog(UDBAPlayableSkillCatalogDataAsset* InSkillCatalog)
{
	SkillCatalog = InSkillCatalog;
	if (HasBegunPlay())
	{
		PreloadSkillPresentationAssets();
	}
}

void UDBAPlayableSkillComponent::SetAppendDefaultSkillsWhenCatalogMissingSlots(bool bInAppendDefaults)
{
	bAppendDefaultSkillsWhenCatalogMissingSlots = bInAppendDefaults;
}

bool UDBAPlayableSkillComponent::ValidateEffectiveSkillSpecs(TArray<FString>& OutErrors) const
{
	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);
	return UDBAPlayableSkillCatalogDataAsset::ValidateSkillSpecs(EffectiveSpecs, OutErrors);
}

FDBAPlayableSkillCatalogSummary UDBAPlayableSkillComponent::GetSkillCatalogSummary() const
{
	FDBAPlayableSkillCatalogSummary Summary;
	const UDBAPlayableSkillCatalogDataAsset* EffectiveCatalog = GetEffectiveSkillCatalog();
	Summary.Source = EffectiveCatalog
		? ((bAppendDefaultSkillsWhenCatalogMissingSlots && !SkillSpecs.IsEmpty()) ? EDBAPlayableSkillCatalogSource::DataAssetWithDefaults : EDBAPlayableSkillCatalogSource::DataAssetOnly)
		: EDBAPlayableSkillCatalogSource::BuiltInDefaults;
	Summary.CatalogId = EffectiveCatalog ? EffectiveCatalog->CatalogId : NAME_None;
	Summary.ConfiguredCatalogSkillCount = EffectiveCatalog ? EffectiveCatalog->GetAllSkillSpecs().Num() : 0;
	Summary.bAppendsBuiltInDefaults = EffectiveCatalog && bAppendDefaultSkillsWhenCatalogMissingSlots && !SkillSpecs.IsEmpty();

	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);
	Summary.SkillCount = EffectiveSpecs.Num();
	Summary.bIsValid = UDBAPlayableSkillCatalogDataAsset::ValidateSkillSpecs(EffectiveSpecs, Summary.ValidationErrors);
	return Summary;
}

void UDBAPlayableSkillComponent::BuildEffectiveSkillSpecs(TArray<FDBAPlayableSkillRuntimeSpec>& OutSpecs) const
{
	OutSpecs.Reset();

	const UDBAPlayableSkillCatalogDataAsset* EffectiveCatalog = GetEffectiveSkillCatalog();
	if (!EffectiveCatalog || bAppendDefaultSkillsWhenCatalogMissingSlots)
	{
		OutSpecs = SkillSpecs;
	}

	if (EffectiveCatalog)
	{
		const TArray<FDBAPlayableSkillRuntimeSpec> CatalogSpecs = EffectiveCatalog->GetAllSkillSpecs();
		for (const FDBAPlayableSkillRuntimeSpec& CatalogSpec : CatalogSpecs)
		{
			if (FDBAPlayableSkillRuntimeSpec* ExistingSpec = OutSpecs.FindByPredicate([&CatalogSpec](const FDBAPlayableSkillRuntimeSpec& Candidate)
				{
					return Candidate.SkillSlot == CatalogSpec.SkillSlot;
				}))
			{
				*ExistingSpec = CatalogSpec;
			}
			else
			{
				OutSpecs.Add(CatalogSpec);
			}
		}
	}

	OutSpecs.Sort([](const FDBAPlayableSkillRuntimeSpec& Left, const FDBAPlayableSkillRuntimeSpec& Right)
	{
		return Left.SkillSlot < Right.SkillSlot;
	});
}

const UDBAPlayableSkillCatalogDataAsset* UDBAPlayableSkillComponent::GetEffectiveSkillCatalog() const
{
	return SkillCatalog ? SkillCatalog.Get() : LoadedDefaultSkillCatalog.Get();
}

TSoftObjectPtr<UDBAPlayableSkillCatalogDataAsset> UDBAPlayableSkillComponent::ResolveDefaultSkillCatalogAsset() const
{
	if (!DefaultSkillCatalog.IsNull())
	{
		return DefaultSkillCatalog;
	}

	const UDBAPlayableSkillDeveloperSettings* Settings = GetDefault<UDBAPlayableSkillDeveloperSettings>();
	return Settings ? Settings->DefaultSkillCatalog : TSoftObjectPtr<UDBAPlayableSkillCatalogDataAsset>();
}

void UDBAPlayableSkillComponent::HandleDefaultSkillCatalogLoaded(UDBAPlayableSkillCatalogDataAsset* LoadedCatalog)
{
	if (!LoadedCatalog)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAPlayableSkillComponent] 默认可玩技能目录异步加载失败：拥有者=%s。"), *GetNameSafe(GetOwner()));
		return;
	}

	if (SkillCatalog)
	{
		return;
	}

	LoadedDefaultSkillCatalog = LoadedCatalog;

	TArray<FString> ValidationErrors;
	if (!ValidateEffectiveSkillSpecs(ValidationErrors))
	{
		for (const FString& ValidationError : ValidationErrors)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAPlayableSkillComponent] 默认可玩技能目录校验失败：目录=%s 错误=%s"), *LoadedCatalog->GetName(), *ValidationError);
		}
	}

	if (HasBegunPlay())
	{
		PreloadSkillPresentationAssets();
		QueueNiagaraWarmupAssets();
	}
}

FName UDBAPlayableSkillComponent::ResolveEquippedSkillId(int32 SkillSlot, FName FallbackSkillId) const
{
	const ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
	if (!Character || !Character->GetWorld())
	{
		return FallbackSkillId;
	}

	const UGameInstance* GameInstance = Character->GetWorld()->GetGameInstance();
	const UDBASkillGroupGeneratorSubsystem* SkillGroups = GameInstance
		? GameInstance->GetSubsystem<UDBASkillGroupGeneratorSubsystem>()
		: nullptr;
	if (!SkillGroups)
	{
		return FallbackSkillId;
	}

	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	if (!SkillGroups->GetSkillGroup(ToPlayableSkillCommonZodiac(Character->GetZodiacType()), ToPlayableSkillCommonElement(Character->GetElementType()), SkillGroup))
	{
		return FallbackSkillId;
	}

	switch (SkillSlot)
	{
	case 1: return SkillGroup.ElementSkill1Id.IsNone() ? FallbackSkillId : SkillGroup.ElementSkill1Id;
	case 2: return SkillGroup.ElementSkill2Id.IsNone() ? FallbackSkillId : SkillGroup.ElementSkill2Id;
	case 3: return SkillGroup.ElementSkill3Id.IsNone() ? FallbackSkillId : SkillGroup.ElementSkill3Id;
	case 4: return SkillGroup.ElementSkill4Id.IsNone() ? FallbackSkillId : SkillGroup.ElementSkill4Id;
	case 5: return SkillGroup.ZodiacUltimateSkillId.IsNone() ? FallbackSkillId : SkillGroup.ZodiacUltimateSkillId;
	default: return FallbackSkillId;
	}
}

void UDBAPlayableSkillComponent::PreloadSkillPresentationAssets() const
{
	if (GetOwner() && GetOwner()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);

	TArray<FSoftObjectPath> Paths;
	for (const FDBAPlayableSkillRuntimeSpec& Spec : EffectiveSpecs)
	{
		DBAAsyncAssetLoader::AddPreloadPath(Spec.CastNiagaraVFXAsset, Paths);
		DBAAsyncAssetLoader::AddPreloadPath(Spec.ProjectileNiagaraVFXAsset, Paths);
		DBAAsyncAssetLoader::AddPreloadPath(Spec.ImpactNiagaraVFXAsset, Paths);
		DBAAsyncAssetLoader::AddPreloadPath(Spec.CastSFXAsset, Paths);
		DBAAsyncAssetLoader::AddPreloadPath(Spec.FlySFXAsset, Paths);
		DBAAsyncAssetLoader::AddPreloadPath(Spec.ImpactSFXAsset, Paths);

		if (Spec.ProjectileClass)
		{
			if (ADBASkillProjectileBase* ProjectileCDO = Spec.ProjectileClass->GetDefaultObject<ADBASkillProjectileBase>())
			{
				ProjectileCDO->PreloadPresentationAssets();
			}
		}
		if (Spec.BloomHealingClass)
		{
			if (ADBABloomHealingSpell* BloomCDO = Spec.BloomHealingClass->GetDefaultObject<ADBABloomHealingSpell>())
			{
				BloomCDO->PreloadPresentationAssets();
			}
		}
		if (Spec.ChainLightningClass)
		{
			if (ADBAChainLightningSpell* ChainCDO = Spec.ChainLightningClass->GetDefaultObject<ADBAChainLightningSpell>())
			{
				ChainCDO->PreloadPresentationAssets();
			}
		}
		if (Spec.HolyShieldClass)
		{
			if (ADBAHolyShieldSpell* ShieldCDO = Spec.HolyShieldClass->GetDefaultObject<ADBAHolyShieldSpell>())
			{
				ShieldCDO->PreloadPresentationAssets();
			}
		}
	}
	DBAAsyncAssetLoader::RequestAsyncPreload(const_cast<UDBAPlayableSkillComponent*>(this), Paths);
}

void UDBAPlayableSkillComponent::QueueNiagaraWarmupAssets()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || OwnerActor->GetNetMode() == NM_DedicatedServer || !GetWorld())
	{
		return;
	}

	PendingNiagaraWarmupPaths.Reset();

	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);
	for (const FDBAPlayableSkillRuntimeSpec& Spec : EffectiveSpecs)
	{
		AddNiagaraWarmupPath(Spec.CastNiagaraVFXAsset);
		AddNiagaraWarmupPath(Spec.ProjectileNiagaraVFXAsset);
		AddNiagaraWarmupPath(Spec.ImpactNiagaraVFXAsset);
	}

	if (PendingNiagaraWarmupPaths.IsEmpty())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(NiagaraWarmupTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		NiagaraWarmupTimerHandle,
		this,
		&UDBAPlayableSkillComponent::PumpNiagaraWarmupQueue,
		0.12f,
		true,
		0.35f);
}

void UDBAPlayableSkillComponent::AddNiagaraWarmupPath(const TSoftObjectPtr<UNiagaraSystem>& Asset)
{
	if (!Asset.IsNull())
	{
		PendingNiagaraWarmupPaths.AddUnique(Asset.ToSoftObjectPath());
	}
}

void UDBAPlayableSkillComponent::PumpNiagaraWarmupQueue()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (PendingNiagaraWarmupPaths.IsEmpty())
	{
		World->GetTimerManager().ClearTimer(NiagaraWarmupTimerHandle);
		return;
	}

	const FSoftObjectPath WarmupPath = PendingNiagaraWarmupPaths[0];
	PendingNiagaraWarmupPaths.RemoveAt(0, 1, EAllowShrinking::No);

	TSoftObjectPtr<UNiagaraSystem> WarmupAsset(WarmupPath);
	TWeakObjectPtr<UDBAPlayableSkillComponent> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(this, WarmupAsset, [WeakThis](UNiagaraSystem* LoadedSystem)
	{
		if (UDBAPlayableSkillComponent* StrongThis = WeakThis.Get())
		{
			StrongThis->WarmUpNiagaraSystem(LoadedSystem);
		}
	});
}

void UDBAPlayableSkillComponent::WarmUpNiagaraSystem(UNiagaraSystem* NiagaraSystem) const
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!NiagaraSystem || !OwnerActor || !World || OwnerActor->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector WarmupLocation = OwnerActor->GetActorLocation() - FVector(0.0f, 0.0f, 30000.0f);
	UNiagaraComponent* WarmupComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		NiagaraSystem,
		WarmupLocation,
		FRotator::ZeroRotator,
		FVector(0.01f),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		false);

	if (!WarmupComponent)
	{
		return;
	}

	WarmupComponent->SetVisibility(false, true);
	WarmupComponent->SetHiddenInGame(true);

	TWeakObjectPtr<UNiagaraComponent> WeakWarmup(WarmupComponent);
	FTimerHandle CleanupHandle;
	World->GetTimerManager().SetTimer(
		CleanupHandle,
		FTimerDelegate::CreateLambda([WeakWarmup]()
		{
			if (UNiagaraComponent* Component = WeakWarmup.Get())
			{
				Component->Deactivate();
				Component->DestroyComponent();
			}
		}),
		0.2f,
		false);
}
