// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Character/Appearance/DBACharacterAppearanceComponent.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StreamableManager.h"
#include "GameDBA/Character/Appearance/DBAAppearanceCatalogDataAsset.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UDBACharacterAppearanceComponent::UDBACharacterAppearanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDBACharacterAppearanceComponent::BeginPlay()
{
	Super::BeginPlay();
	ResolveBaseMeshComponent();
}

void UDBACharacterAppearanceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	++AppearanceRequestVersion;
	if (ActiveLoadHandle.IsValid())
	{
		ActiveLoadHandle->CancelHandle();
		ActiveLoadHandle.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

void UDBACharacterAppearanceComponent::SetBaseMeshComponent(USkeletalMeshComponent* InBaseMeshComponent)
{
	if (BaseMeshComponent == InBaseMeshComponent && !OriginalBaseMaterials.IsEmpty())
	{
		return;
	}

	ClearModularParts();
	OriginalBaseMaterials.Reset();
	BaseMeshComponent = InBaseMeshComponent;
	if (BaseMeshComponent)
	{
		for (int32 MaterialIndex = 0; MaterialIndex < BaseMeshComponent->GetNumMaterials(); ++MaterialIndex)
		{
			OriginalBaseMaterials.Add(MaterialIndex, BaseMeshComponent->GetMaterial(MaterialIndex));
		}
	}
}

bool UDBACharacterAppearanceComponent::ApplyAppearance(const EDBAZodiac Zodiac, const FDBACharacterAppearance& Appearance)
{
	CurrentZodiac = Zodiac;
	CurrentAppearance = Appearance;
	++AppearanceRequestVersion;
	if (ActiveLoadHandle.IsValid())
	{
		ActiveLoadHandle->CancelHandle();
		ActiveLoadHandle.Reset();
	}

	if (Zodiac == EDBAZodiac::None)
	{
		UE_LOG(LogDBACharacter, Warning, TEXT("[角色外观] 未提供有效生肖，拒绝恢复外观：Actor=%s。"), *GetNameSafe(GetOwner()));
		OnAppearanceApplied.Broadcast(CurrentAppearance, false);
		return false;
	}
	if (IsDedicatedServer())
	{
		// Dedicated Server 仅持有稳定 ID，禁止加载 Mesh、Anim、材质或前台预览资源。
		return true;
	}

	BeginAsyncApply(AppearanceRequestVersion);
	return true;
}

void UDBACharacterAppearanceComponent::Reset()
{
	++AppearanceRequestVersion;
	if (ActiveLoadHandle.IsValid())
	{
		ActiveLoadHandle->CancelHandle();
		ActiveLoadHandle.Reset();
	}
	CurrentAppearance = FDBACharacterAppearance();
	CurrentZodiac = EDBAZodiac::None;
	ClearModularParts();
	RestoreBaseMaterials();
}

bool UDBACharacterAppearanceComponent::AsyncLoad()
{
	if (CurrentZodiac == EDBAZodiac::None || IsDedicatedServer())
	{
		return false;
	}
	++AppearanceRequestVersion;
	BeginAsyncApply(AppearanceRequestVersion);
	return true;
}

USkeletalMeshComponent* UDBACharacterAppearanceComponent::ResolveBaseMeshComponent()
{
	if (!BaseMeshComponent && GetOwner())
	{
		BaseMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	}
	return BaseMeshComponent;
}

void UDBACharacterAppearanceComponent::CollectRequestedOptions(TArray<FRequestedOption>& OutOptions) const
{
	OutOptions.Reset();
	auto Add = [&OutOptions](const EDBAAppearanceSlot Slot, const FName OptionId)
	{
		if (!OptionId.IsNone())
		{
			OutOptions.Add({ Slot, OptionId });
		}
	};

	Add(EDBAAppearanceSlot::Gender, CurrentAppearance.GenderId);
	Add(EDBAAppearanceSlot::BodyType, CurrentAppearance.BodyTypeId);
	Add(EDBAAppearanceSlot::Face, CurrentAppearance.FaceId);
	Add(EDBAAppearanceSlot::Hair, CurrentAppearance.HairId);
	Add(EDBAAppearanceSlot::HairColor, CurrentAppearance.HairColorId);
	Add(EDBAAppearanceSlot::SkinColor, CurrentAppearance.SkinColorId);
	Add(EDBAAppearanceSlot::EyeColor, CurrentAppearance.EyeColorId);
	Add(EDBAAppearanceSlot::Marking, CurrentAppearance.MarkingId);
	Add(EDBAAppearanceSlot::Horn, CurrentAppearance.HornId);
	Add(EDBAAppearanceSlot::Ear, CurrentAppearance.EarId);
	Add(EDBAAppearanceSlot::Tail, CurrentAppearance.TailId);
	for (const FName EquipmentVisualId : CurrentAppearance.EquipmentVisualIds)
	{
		Add(EDBAAppearanceSlot::Equipment, EquipmentVisualId);
	}
	Add(EDBAAppearanceSlot::Weapon, CurrentAppearance.WeaponVisualId);
	Add(EDBAAppearanceSlot::Skin, CurrentAppearance.SkinId);
}

void UDBACharacterAppearanceComponent::BeginAsyncApply(const uint32 RequestVersion)
{
	if (!ResolveBaseMeshComponent())
	{
		UE_LOG(LogDBACharacter, Warning, TEXT("[角色外观] 未找到主体骨骼网格，无法恢复外观：Actor=%s。"), *GetNameSafe(GetOwner()));
		OnAppearanceApplied.Broadcast(CurrentAppearance, false);
		return;
	}
	if (AppearanceCatalog.IsNull())
	{
		UE_LOG(LogDBACharacter, Warning, TEXT("[角色外观] 未配置 Appearance Catalog，保留主体模型：Actor=%s。"), *GetNameSafe(GetOwner()));
		OnAppearanceApplied.Broadcast(CurrentAppearance, false);
		return;
	}

	UDBAAppearanceCatalogDataAsset* Catalog = AppearanceCatalog.Get();
	TWeakObjectPtr<UDBACharacterAppearanceComponent> WeakThis(this);
	if (!Catalog)
	{
		ActiveLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			AppearanceCatalog.ToSoftObjectPath(),
			FStreamableDelegate::CreateLambda([WeakThis, RequestVersion]()
			{
				if (WeakThis.IsValid() && WeakThis->AppearanceRequestVersion == RequestVersion)
				{
					WeakThis->BeginAsyncApply(RequestVersion);
				}
			}));
		return;
	}

	TArray<FRequestedOption> RequestedOptions;
	CollectRequestedOptions(RequestedOptions);
	TArray<FSoftObjectPath> ResourcesToLoad;
	for (const FRequestedOption& RequestedOption : RequestedOptions)
	{
		bool bUsedFallback = false;
		if (const FDBAAppearanceOptionDefinition* Definition = ResolveDefinition(RequestedOption, bUsedFallback))
		{
			if (!Definition->SkeletalMesh.IsNull()) ResourcesToLoad.AddUnique(Definition->SkeletalMesh.ToSoftObjectPath());
			if (!Definition->CopyPoseAnimationClass.IsNull()) ResourcesToLoad.AddUnique(Definition->CopyPoseAnimationClass.ToSoftObjectPath());
			for (const TSoftObjectPtr<UMaterialInterface>& Material : Definition->MaterialOverrides)
			{
				if (!Material.IsNull()) ResourcesToLoad.AddUnique(Material.ToSoftObjectPath());
			}
		}
	}

	if (ResourcesToLoad.IsEmpty())
	{
		ApplyResolvedOptions(RequestVersion);
		return;
	}
	ActiveLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		ResourcesToLoad,
		FStreamableDelegate::CreateLambda([WeakThis, RequestVersion]()
		{
			if (WeakThis.IsValid() && WeakThis->AppearanceRequestVersion == RequestVersion)
			{
				WeakThis->ApplyResolvedOptions(RequestVersion);
			}
		}));
}

void UDBACharacterAppearanceComponent::ApplyResolvedOptions(const uint32 RequestVersion)
{
	if (AppearanceRequestVersion != RequestVersion || IsDedicatedServer())
	{
		return;
	}
	ActiveLoadHandle.Reset();
	UDBAAppearanceCatalogDataAsset* Catalog = AppearanceCatalog.Get();
	if (!Catalog || !ResolveBaseMeshComponent())
	{
		OnAppearanceApplied.Broadcast(CurrentAppearance, false);
		return;
	}

	ClearModularParts();
	RestoreBaseMaterials();
	bool bAllOptionsResolved = true;
	TArray<FRequestedOption> RequestedOptions;
	CollectRequestedOptions(RequestedOptions);
	for (const FRequestedOption& RequestedOption : RequestedOptions)
	{
		bool bUsedFallback = false;
		const FDBAAppearanceOptionDefinition* Definition = ResolveDefinition(RequestedOption, bUsedFallback);
		if (!Definition)
		{
			bAllOptionsResolved = false;
			continue;
		}
		bAllOptionsResolved &= !bUsedFallback;
		ApplyPart(*Definition);
		ApplyMaterialParameters(*Definition);
	}

	OnAppearanceApplied.Broadcast(CurrentAppearance, bAllOptionsResolved);
}

const FDBAAppearanceOptionDefinition* UDBACharacterAppearanceComponent::ResolveDefinition(const FRequestedOption& RequestedOption, bool& bOutUsedFallback) const
{
	bOutUsedFallback = false;
	const UDBAAppearanceCatalogDataAsset* Catalog = AppearanceCatalog.Get();
	if (!Catalog)
	{
		return nullptr;
	}
	if (const FDBAAppearanceOptionDefinition* Definition = Catalog->FindOption(RequestedOption.OptionId))
	{
		if (Definition->Slot == RequestedOption.Slot && Catalog->IsOptionAllowed(*Definition, CurrentZodiac))
		{
			return Definition;
		}
	}

	bOutUsedFallback = true;
	UE_LOG(LogDBACharacter, Warning, TEXT("[角色外观] 外观选项 %s 不存在、槽位不匹配或不允许当前生肖，尝试安全回退。"), *RequestedOption.OptionId.ToString());
	return Catalog->FindFallback(RequestedOption.Slot, CurrentZodiac);
}

bool UDBACharacterAppearanceComponent::ApplyPart(const FDBAAppearanceOptionDefinition& Definition)
{
	USkeletalMeshComponent* BaseMesh = ResolveBaseMeshComponent();
	USkeletalMesh* PartMesh = Definition.SkeletalMesh.Get();
	if (!BaseMesh || !PartMesh)
	{
		return true;
	}

	USkeletalMeshComponent* PartComponent = nullptr;
	if (TObjectPtr<USkeletalMeshComponent>* Existing = ModularPartComponents.Find(Definition.OptionId))
	{
		PartComponent = *Existing;
	}
	else
	{
		PartComponent = NewObject<USkeletalMeshComponent>(GetOwner(), *FString::Printf(TEXT("AppearancePart_%s"), *Definition.OptionId.ToString()));
		PartComponent->SetupAttachment(BaseMesh, Definition.AttachSocket);
		PartComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PartComponent->SetGenerateOverlapEvents(false);
		PartComponent->RegisterComponent();
		ModularPartComponents.Add(Definition.OptionId, PartComponent);
	}

	PartComponent->SetSkeletalMesh(PartMesh);
	const USkeletalMesh* BaseAsset = BaseMesh->GetSkeletalMeshAsset();
	if (Definition.bUseLeaderPose && BaseAsset && BaseAsset->GetSkeleton() == PartMesh->GetSkeleton())
	{
		PartComponent->SetLeaderPoseComponent(BaseMesh, true);
	}
	else if (UClass* CopyPoseClass = Definition.CopyPoseAnimationClass.Get())
	{
		PartComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		PartComponent->SetAnimInstanceClass(CopyPoseClass);
	}
	else if (Definition.bUseLeaderPose)
	{
		UE_LOG(LogDBACharacter, Warning, TEXT("[角色外观] 部件 %s 与主体骨架不匹配且未配置 CopyPose AnimBP，已跳过该部件。"), *Definition.OptionId.ToString());
		PartComponent->SetSkeletalMesh(nullptr);
		return false;
	}

	PartComponent->SetVisibility(true, true);
	PartComponent->SetHiddenInGame(false, true);
	return true;
}

void UDBACharacterAppearanceComponent::ApplyMaterialParameters(const FDBAAppearanceOptionDefinition& Definition)
{
	USkeletalMeshComponent* BaseMesh = ResolveBaseMeshComponent();
	if (!BaseMesh || (Definition.MaterialOverrides.IsEmpty() && Definition.MaterialVectorParameters.IsEmpty()))
	{
		return;
	}

	UMaterialInterface* Material = Definition.MaterialOverrides.IsEmpty()
		? BaseMesh->GetMaterial(Definition.MaterialSlotIndex)
		: Definition.MaterialOverrides[0].Get();
	if (!Material)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
	if (!DynamicMaterial)
	{
		return;
	}
	for (const TPair<FName, FLinearColor>& Parameter : Definition.MaterialVectorParameters)
	{
		DynamicMaterial->SetVectorParameterValue(Parameter.Key, Parameter.Value);
	}
	BaseMesh->SetMaterial(Definition.MaterialSlotIndex, DynamicMaterial);
}

void UDBACharacterAppearanceComponent::RestoreBaseMaterials()
{
	if (USkeletalMeshComponent* BaseMesh = ResolveBaseMeshComponent())
	{
		for (const TPair<int32, TObjectPtr<UMaterialInterface>>& Pair : OriginalBaseMaterials)
		{
			BaseMesh->SetMaterial(Pair.Key, Pair.Value);
		}
	}
}

void UDBACharacterAppearanceComponent::ClearModularParts()
{
	for (const TPair<FName, TObjectPtr<USkeletalMeshComponent>>& Pair : ModularPartComponents)
	{
		if (USkeletalMeshComponent* PartComponent = Pair.Value)
		{
			PartComponent->DestroyComponent();
		}
	}
	ModularPartComponents.Reset();
}

bool UDBACharacterAppearanceComponent::IsDedicatedServer() const
{
	return GetOwner() && GetOwner()->GetNetMode() == NM_DedicatedServer;
}
