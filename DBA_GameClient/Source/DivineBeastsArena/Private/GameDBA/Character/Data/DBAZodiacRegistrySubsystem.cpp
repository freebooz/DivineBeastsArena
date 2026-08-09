// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Character/Data/DBAZodiacRegistrySubsystem.h"

#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"

namespace
{
	bool TryParseZodiacTag(const FString& Value, EDBAZodiac& OutZodiac)
	{
		OutZodiac = EDBAZodiac::None;
		FString EnumName = Value;
		EnumName.RemoveFromStart(TEXT("EDBAZodiac::"));
		const UEnum* ZodiacEnum = StaticEnum<EDBAZodiac>();
		if (!ZodiacEnum)
		{
			return false;
		}

		const int64 NumericValue = ZodiacEnum->GetValueByNameString(EnumName);
		if (NumericValue == INDEX_NONE || NumericValue == static_cast<int64>(EDBAZodiac::None))
		{
			return false;
		}

		OutZodiac = static_cast<EDBAZodiac>(NumericValue);
		return true;
	}

	void AddIssue(TArray<FString>* Issues, const FString& Message)
	{
		if (Issues)
		{
			Issues->Add(Message);
		}
	}
}

void UDBAZodiacRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RefreshRegistry();
}

void UDBAZodiacRegistrySubsystem::Deinitialize()
{
	TArray<EDBAZodiac> LoadedZodiacs;
	LoadedAssets.GetKeys(LoadedZodiacs);
	for (const EDBAZodiac Zodiac : LoadedZodiacs)
	{
		Release(Zodiac);
	}

	PrimaryAssetIds.Reset();
	PrimaryAssetIdByZodiac.Reset();
	PendingCallbacks.Reset();
	ActiveLoadHandles.Reset();
	Super::Deinitialize();
}

bool UDBAZodiacRegistrySubsystem::RefreshRegistry()
{
	return RefreshRegistryInternal(nullptr, nullptr);
}

const TArray<FPrimaryAssetId>& UDBAZodiacRegistrySubsystem::GetAll()
{
	RefreshRegistry();
	return PrimaryAssetIds;
}

void UDBAZodiacRegistrySubsystem::GetAllZodiacTypes(TArray<EDBAZodiac>& OutZodiacTypes)
{
	RefreshRegistry();
	OutZodiacTypes.Reset();

	const UEnum* ZodiacEnum = StaticEnum<EDBAZodiac>();
	if (!ZodiacEnum)
	{
		return;
	}

	for (int32 Index = 0; Index < ZodiacEnum->NumEnums() - 1; ++Index)
	{
		const EDBAZodiac Zodiac = static_cast<EDBAZodiac>(ZodiacEnum->GetValueByIndex(Index));
		if (Zodiac != EDBAZodiac::None && PrimaryAssetIdByZodiac.Contains(Zodiac))
		{
			OutZodiacTypes.Add(Zodiac);
		}
	}
}

bool UDBAZodiacRegistrySubsystem::Find(EDBAZodiac Zodiac, FPrimaryAssetId& OutPrimaryAssetId) const
{
	OutPrimaryAssetId = FPrimaryAssetId();
	if (const FPrimaryAssetId* Found = PrimaryAssetIdByZodiac.Find(Zodiac))
	{
		OutPrimaryAssetId = *Found;
		return true;
	}

	return false;
}

bool UDBAZodiacRegistrySubsystem::LoadAsync(EDBAZodiac Zodiac, FDBAOnZodiacHeroAssetLoaded Completion)
{
	RefreshRegistry();
	if (const TWeakObjectPtr<UDBAZodiacHeroDataAsset>* LoadedAsset = LoadedAssets.Find(Zodiac))
	{
		if (LoadedAsset->IsValid())
		{
			Completion.ExecuteIfBound(Zodiac, LoadedAsset->Get());
			return true;
		}
	}

	FPrimaryAssetId PrimaryAssetId;
	if (!Find(Zodiac, PrimaryAssetId))
	{
		UE_LOG(LogDBAData, Warning, TEXT("[生肖资产注册表] 未找到生肖 %d 的 Primary Asset 配置。"), static_cast<int32>(Zodiac));
		Completion.ExecuteIfBound(Zodiac, nullptr);
		return false;
	}

	PendingCallbacks.FindOrAdd(Zodiac).Add(MoveTemp(Completion));
	if (ActiveLoadHandles.Contains(Zodiac))
	{
		return true;
	}

	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager)
	{
		UE_LOG(LogDBAData, Error, TEXT("[生肖资产注册表] AssetManager 尚未初始化，无法异步加载 %s。"), *PrimaryAssetId.ToString());
		CompleteLoad(Zodiac, PrimaryAssetId);
		return false;
	}

	TWeakObjectPtr<UDBAZodiacRegistrySubsystem> WeakThis(this);
	const TSharedPtr<FStreamableHandle> Handle = AssetManager->LoadPrimaryAsset(
		PrimaryAssetId,
		TArray<FName>(),
		FStreamableDelegate::CreateLambda([WeakThis, Zodiac, PrimaryAssetId]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->CompleteLoad(Zodiac, PrimaryAssetId);
			}
		}));

	if (!Handle.IsValid())
	{
		UE_LOG(LogDBAData, Error, TEXT("[生肖资产注册表] 无法创建 %s 的异步加载句柄。"), *PrimaryAssetId.ToString());
		CompleteLoad(Zodiac, PrimaryAssetId);
		return false;
	}

	ActiveLoadHandles.Add(Zodiac, Handle);
	return true;
}

void UDBAZodiacRegistrySubsystem::Release(EDBAZodiac Zodiac)
{
	PendingCallbacks.Remove(Zodiac);
	if (TSharedPtr<FStreamableHandle>* Handle = ActiveLoadHandles.Find(Zodiac))
	{
		(*Handle)->CancelHandle();
		ActiveLoadHandles.Remove(Zodiac);
	}

	LoadedAssets.Remove(Zodiac);
	FPrimaryAssetId PrimaryAssetId;
	if (Find(Zodiac, PrimaryAssetId))
	{
		if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			AssetManager->UnloadPrimaryAsset(PrimaryAssetId);
		}
	}
}

bool UDBAZodiacRegistrySubsystem::ValidateConfiguration(TArray<FString>& OutErrors, TArray<FString>& OutWarnings)
{
	OutErrors.Reset();
	OutWarnings.Reset();
	const bool bIndexed = RefreshRegistryInternal(&OutErrors, &OutWarnings);
	if (PrimaryAssetIdByZodiac.Num() != DBAConstants::ZodiacCount)
	{
		OutErrors.Add(FString::Printf(TEXT("ZodiacHero Primary Asset 数量为 %d，期望 %d。"), PrimaryAssetIdByZodiac.Num(), DBAConstants::ZodiacCount));
	}

	const UEnum* ZodiacEnum = StaticEnum<EDBAZodiac>();
	if (ZodiacEnum)
	{
		for (int32 Index = 0; Index < ZodiacEnum->NumEnums() - 1; ++Index)
		{
			const EDBAZodiac Zodiac = static_cast<EDBAZodiac>(ZodiacEnum->GetValueByIndex(Index));
			if (Zodiac != EDBAZodiac::None && !PrimaryAssetIdByZodiac.Contains(Zodiac))
			{
				OutErrors.Add(FString::Printf(TEXT("缺少生肖 %s 的 ZodiacHero DataAsset。"), *ZodiacEnum->GetNameStringByValue(static_cast<int64>(Zodiac))));
			}
		}
	}

	for (const FString& Warning : OutWarnings)
	{
		UE_LOG(LogDBAData, Warning, TEXT("[生肖资产注册表] %s"), *Warning);
	}
	for (const FString& Error : OutErrors)
	{
		UE_LOG(LogDBAData, Error, TEXT("[生肖资产注册表] %s"), *Error);
	}

	return bIndexed && OutErrors.IsEmpty();
}

void UDBAZodiacRegistrySubsystem::CompleteLoad(EDBAZodiac Zodiac, const FPrimaryAssetId& PrimaryAssetId)
{
	TArray<FDBAOnZodiacHeroAssetLoaded> Callbacks;
	if (TArray<FDBAOnZodiacHeroAssetLoaded>* Pending = PendingCallbacks.Find(Zodiac))
	{
		Callbacks = MoveTemp(*Pending);
		PendingCallbacks.Remove(Zodiac);
	}
	ActiveLoadHandles.Remove(Zodiac);

	// Release 后的旧回调不得重新激活已离开的页面或预览。
	if (Callbacks.IsEmpty())
	{
		return;
	}

	UDBAZodiacHeroDataAsset* LoadedAsset = nullptr;
	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		LoadedAsset = AssetManager->GetPrimaryAssetObject<UDBAZodiacHeroDataAsset>(PrimaryAssetId);
	}

	if (LoadedAsset)
	{
		LoadedAssets.Add(Zodiac, LoadedAsset);
		UE_LOG(LogDBAData, Verbose, TEXT("[生肖资产注册表] 已异步加载生肖 %d 的静态配置：%s。"), static_cast<int32>(Zodiac), *PrimaryAssetId.ToString());
	}
	else
	{
		UE_LOG(LogDBAData, Error, TEXT("[生肖资产注册表] 生肖 %d 的静态配置加载失败：%s。"), static_cast<int32>(Zodiac), *PrimaryAssetId.ToString());
	}

	for (const FDBAOnZodiacHeroAssetLoaded& Callback : Callbacks)
	{
		Callback.ExecuteIfBound(Zodiac, LoadedAsset);
	}
}

bool UDBAZodiacRegistrySubsystem::RefreshRegistryInternal(TArray<FString>* OutErrors, TArray<FString>* OutWarnings)
{
	PrimaryAssetIds.Reset();
	PrimaryAssetIdByZodiac.Reset();

	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager)
	{
		AddIssue(OutErrors, TEXT("AssetManager 尚未初始化。"));
		return false;
	}

	TArray<FAssetData> AssetDataList;
	if (!AssetManager->GetPrimaryAssetDataList(UDBAZodiacHeroDataAsset::GetZodiacHeroPrimaryAssetType(), AssetDataList))
	{
		AddIssue(OutWarnings, TEXT("尚未扫描到 ZodiacHero Primary Asset；请保存并重新扫描十二生肖 DataAsset。"));
		return true;
	}

	TSet<FPrimaryAssetId> SeenPrimaryAssetIds;
	const FName ZodiacTypeTag = GET_MEMBER_NAME_CHECKED(UDBAZodiacHeroDataAsset, ZodiacType);
	const FName LegacyCatalogTag = GET_MEMBER_NAME_CHECKED(UDBAZodiacHeroDataAsset, bLegacyTableCatalog);
	const FName LegacyClassificationTag = GET_MEMBER_NAME_CHECKED(UDBAZodiacHeroDataAsset, DeprecatedLegacyClassificationId);
	for (const FAssetData& AssetData : AssetDataList)
	{
		bool bLegacyCatalog = false;
		AssetData.GetTagValue(LegacyCatalogTag, bLegacyCatalog);
		if (bLegacyCatalog)
		{
			continue;
		}

		FString ZodiacValue;
		if (!AssetData.GetTagValue(ZodiacTypeTag, ZodiacValue))
		{
			AddIssue(OutErrors, FString::Printf(TEXT("资产 %s 缺少 ZodiacType 索引。"), *AssetData.GetObjectPathString()));
			continue;
		}

		EDBAZodiac Zodiac;
		if (!TryParseZodiacTag(ZodiacValue, Zodiac))
		{
			AddIssue(OutErrors, FString::Printf(TEXT("资产 %s 的 ZodiacType 非法：%s。"), *AssetData.GetObjectPathString(), *ZodiacValue));
			continue;
		}

		FName DeprecatedLegacyClassification;
		AssetData.GetTagValue(LegacyClassificationTag, DeprecatedLegacyClassification);
		if (!DeprecatedLegacyClassification.IsNone())
		{
			AddIssue(OutErrors, FString::Printf(TEXT("资产 %s 仍包含已禁止的旧 Faction/分类引用：%s。"), *AssetData.GetObjectPathString(), *DeprecatedLegacyClassification.ToString()));
			continue;
		}

		FPrimaryAssetId PrimaryAssetId = AssetData.GetPrimaryAssetId();
		if (!PrimaryAssetId.IsValid())
		{
			PrimaryAssetId = FPrimaryAssetId(UDBAZodiacHeroDataAsset::GetZodiacHeroPrimaryAssetType(), AssetData.AssetName);
		}
		if (SeenPrimaryAssetIds.Contains(PrimaryAssetId))
		{
			AddIssue(OutErrors, FString::Printf(TEXT("检测到重复 PrimaryAssetId：%s。"), *PrimaryAssetId.ToString()));
			continue;
		}
		SeenPrimaryAssetIds.Add(PrimaryAssetId);
		if (const FPrimaryAssetId* Existing = PrimaryAssetIdByZodiac.Find(Zodiac))
		{
			AddIssue(OutErrors, FString::Printf(TEXT("生肖 %d 同时映射到 %s 与 %s。"), static_cast<int32>(Zodiac), *Existing->ToString(), *PrimaryAssetId.ToString()));
			continue;
		}

		PrimaryAssetIdByZodiac.Add(Zodiac, PrimaryAssetId);
	}

	const UEnum* ZodiacEnum = StaticEnum<EDBAZodiac>();
	if (ZodiacEnum)
	{
		for (int32 Index = 0; Index < ZodiacEnum->NumEnums() - 1; ++Index)
		{
			const EDBAZodiac Zodiac = static_cast<EDBAZodiac>(ZodiacEnum->GetValueByIndex(Index));
			if (Zodiac != EDBAZodiac::None)
			{
				if (const FPrimaryAssetId* Found = PrimaryAssetIdByZodiac.Find(Zodiac))
				{
					PrimaryAssetIds.Add(*Found);
				}
			}
		}
	}

	return true;
}
