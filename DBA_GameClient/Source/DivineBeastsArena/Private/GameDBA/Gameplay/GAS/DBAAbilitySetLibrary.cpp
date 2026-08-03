// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/GAS/DBAAbilitySetLibrary.h"

#include "Engine/AssetManager.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Gameplay/Abilities/DBAElementSkillAbility_Generic.h"
#include "GameDBA/Gameplay/Abilities/DBAZodiacPassiveAbility_Generic.h"
#include "GameDBA/Gameplay/Abilities/DBAZodiacUltimateAbility_Generic.h"
#include "Misc/PackageName.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	FGameplayTag ResolveCooldownTagBySlotName(const TCHAR* CooldownTagName)
	{
		return FGameplayTag::RequestGameplayTag(FName(CooldownTagName), false);
	}

	void ConfigureFallbackRuntimeConfig(FDBAAbilityRuntimeConfig& RuntimeConfig, const FText& DisplayName, const TCHAR* CooldownTagName)
	{
		RuntimeConfig.DisplayName = DisplayName;
		RuntimeConfig.EnergyCost = 0.0f;
		RuntimeConfig.CooldownDuration = 1.0f;
		RuntimeConfig.CooldownTag = ResolveCooldownTagBySlotName(CooldownTagName);
	}

	void ReportRuntimeConfigValidationErrors(const FName& FixedSkillGroupId, const UDBAFixedSkillGroupDataAsset* AbilitySet)
	{
		if (!AbilitySet)
		{
			return;
		}

		TArray<FString> ValidationErrors;
		if (AbilitySet->ValidateRuntimeAbilityConfigs(ValidationErrors))
		{
			return;
		}

		for (const FString& ValidationError : ValidationErrors)
		{
			UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] 固定技能组运行配置未通过校验：技能组=%s，原因=%s"),
				*FixedSkillGroupId.ToString(),
				*ValidationError);
		}
	}

	FSoftObjectPath MakeObjectPath(const FString& PackagePath, const FString& AssetName)
	{
		return FSoftObjectPath(FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName));
	}

	bool DoesObjectPackageExist(const FSoftObjectPath& AssetPath)
	{
		const FString PackageName = AssetPath.GetLongPackageName();
		return !PackageName.IsEmpty() && FPackageName::DoesPackageExist(PackageName);
	}

	int32 GetStandardSkillGroupIndex(const FString& FixedSkillGroupId)
	{
		const FString LowerId = FixedSkillGroupId.ToLower();
		if (LowerId.Contains(TEXT("_fire")))
		{
			return 1;
		}
		if (LowerId.Contains(TEXT("_water")))
		{
			return 2;
		}
		if (LowerId.Contains(TEXT("_wood")))
		{
			return 3;
		}
		if (LowerId.Contains(TEXT("_gold")) || LowerId.Contains(TEXT("_metal")))
		{
			return 4;
		}
		if (LowerId.Contains(TEXT("_earth")))
		{
			return 5;
		}
		return 1;
	}

	TArray<FSoftObjectPath> MakeFixedSkillGroupCandidatePaths(const FName& FixedSkillGroupId)
	{
		const FString AssetName = FixedSkillGroupId.ToString();
		TArray<FSoftObjectPath> CandidatePaths;
		CandidatePaths.Reserve(4);

		CandidatePaths.Add(MakeObjectPath(FString::Printf(TEXT("%s/%s"), DBAPaths::DT_FixedSkillGroups, *AssetName), AssetName));
		CandidatePaths.Add(MakeObjectPath(FString::Printf(TEXT("/Game/DBA/Data/SkillGroups/%s"), *AssetName), AssetName));
		CandidatePaths.Add(MakeObjectPath(FString::Printf(TEXT("/Game/DBA/Data/SkillGroups/DA_FSG_%s"), *AssetName), FString::Printf(TEXT("DA_FSG_%s"), *AssetName)));

		const FString StandardAssetName = FString::Printf(TEXT("DA_FSG_Standard_5v5_%d"), GetStandardSkillGroupIndex(AssetName));
		CandidatePaths.Add(MakeObjectPath(FString::Printf(TEXT("/Game/DBA/Data/SkillGroups/%s"), *StandardAssetName), StandardAssetName));

		return CandidatePaths;
	}

	UDBAFixedSkillGroupDataAsset* ResolveLoadedFixedSkillGroupAsset(const FSoftObjectPath& AssetPath)
	{
		if (!DoesObjectPackageExist(AssetPath))
		{
			return nullptr;
		}

		return Cast<UDBAFixedSkillGroupDataAsset>(AssetPath.ResolveObject());
	}

	bool FindExistingFixedSkillGroupPath(const FName& FixedSkillGroupId, FSoftObjectPath& OutAssetPath)
	{
		for (const FSoftObjectPath& CandidatePath : MakeFixedSkillGroupCandidatePaths(FixedSkillGroupId))
		{
			if (DoesObjectPackageExist(CandidatePath))
			{
				OutAssetPath = CandidatePath;
				return true;
			}
		}
		return false;
	}

	TMap<FName, TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>>& GetFallbackSkillGroupCache()
	{
		static TMap<FName, TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>> Cache;
		return Cache;
	}

	UDBAFixedSkillGroupDataAsset* GetOrCreateFallbackFixedSkillGroup(const FName& FixedSkillGroupId)
	{
		TMap<FName, TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>>& Cache = GetFallbackSkillGroupCache();
		if (TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>* Existing = Cache.Find(FixedSkillGroupId))
		{
			return Existing->Get();
		}

		UDBAFixedSkillGroupDataAsset* FallbackAsset = NewObject<UDBAFixedSkillGroupDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
		FallbackAsset->FixedSkillGroupId = FixedSkillGroupId;
		FallbackAsset->PassiveAbilityClass = UDBAZodiacPassiveAbility_Generic::StaticClass();
		FallbackAsset->Skill01Class = UDBAElementSkillAbility_Generic::StaticClass();
		FallbackAsset->Skill02Class = UDBAElementSkillAbility_Generic::StaticClass();
		FallbackAsset->Skill03Class = UDBAElementSkillAbility_Generic::StaticClass();
		FallbackAsset->Skill04Class = UDBAElementSkillAbility_Generic::StaticClass();
		FallbackAsset->ZodiacUltimateClass = UDBAZodiacUltimateAbility_Generic::StaticClass();
		ConfigureFallbackRuntimeConfig(FallbackAsset->Skill01RuntimeConfig, NSLOCTEXT("DBAFixedSkillGroup", "FallbackSkill01", "兜底技能一"), TEXT("Cooldown.Skill01"));
		ConfigureFallbackRuntimeConfig(FallbackAsset->Skill02RuntimeConfig, NSLOCTEXT("DBAFixedSkillGroup", "FallbackSkill02", "兜底技能二"), TEXT("Cooldown.Skill02"));
		ConfigureFallbackRuntimeConfig(FallbackAsset->Skill03RuntimeConfig, NSLOCTEXT("DBAFixedSkillGroup", "FallbackSkill03", "兜底技能三"), TEXT("Cooldown.Skill03"));
		ConfigureFallbackRuntimeConfig(FallbackAsset->Skill04RuntimeConfig, NSLOCTEXT("DBAFixedSkillGroup", "FallbackSkill04", "兜底技能四"), TEXT("Cooldown.Skill04"));

		Cache.Add(FixedSkillGroupId, TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>(FallbackAsset));
		UE_LOG(LogDBAData, Verbose, TEXT("[UDBAFixedSkillGroupLibrary] \u4f7f\u7528\u8fd0\u884c\u65f6\u515c\u5e95\u6280\u80fd\u7ec4\uff1a%s"), *FixedSkillGroupId.ToString());
		return FallbackAsset;
	}
}

bool FDBAAbilityRuntimeConfig::Validate(FStringView SlotName, TArray<FString>& OutErrors) const
{
	bool bIsValid = true;
	const FString SlotLabel(SlotName);

	if (DisplayName.IsEmpty())
	{
		OutErrors.Add(FString::Printf(TEXT("%s 缺少技能显示名称配置。"), *SlotLabel));
		bIsValid = false;
	}

	if (EnergyCost < 0.0f)
	{
		OutErrors.Add(FString::Printf(TEXT("%s 普通能量消耗不能小于 0，当前值：%.2f。"), *SlotLabel, EnergyCost));
		bIsValid = false;
	}

	if (CooldownDuration <= 0.0f)
	{
		OutErrors.Add(FString::Printf(TEXT("%s 冷却时长必须大于 0，当前值：%.2f。"), *SlotLabel, CooldownDuration));
		bIsValid = false;
	}

	if (!CooldownTag.IsValid())
	{
		OutErrors.Add(FString::Printf(TEXT("%s 缺少冷却 GameplayTag 配置。"), *SlotLabel));
		bIsValid = false;
	}

	return bIsValid;
}

bool UDBAFixedSkillGroupDataAsset::ValidateRuntimeAbilityConfigs(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();
	bool bIsValid = true;

	bIsValid &= Skill01RuntimeConfig.Validate(TEXT("Skill01"), OutErrors);
	bIsValid &= Skill02RuntimeConfig.Validate(TEXT("Skill02"), OutErrors);
	bIsValid &= Skill03RuntimeConfig.Validate(TEXT("Skill03"), OutErrors);
	bIsValid &= Skill04RuntimeConfig.Validate(TEXT("Skill04"), OutErrors);

	return bIsValid;
}

UDBAFixedSkillGroupDataAsset* UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(const FName& FixedSkillGroupId)
{
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] \u56fa\u5b9a\u6280\u80fd\u7ec4 ID \u4e3a\u7a7a\u3002"));
		return nullptr;
	}

	for (const FSoftObjectPath& AssetPath : MakeFixedSkillGroupCandidatePaths(FixedSkillGroupId))
	{
		if (UDBAFixedSkillGroupDataAsset* LoadedAsset = ResolveLoadedFixedSkillGroupAsset(AssetPath))
		{
			ReportRuntimeConfigValidationErrors(FixedSkillGroupId, LoadedAsset);
			return LoadedAsset;
		}
		if (DoesObjectPackageExist(AssetPath))
		{
			if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
			{
				AssetManager->GetStreamableManager().RequestAsyncLoad(
					TArray<FSoftObjectPath>({ AssetPath }),
					FStreamableDelegate(),
					FStreamableManager::AsyncLoadHighPriority,
					true);
			}
			break;
		}
	}

	UDBAFixedSkillGroupDataAsset* FallbackAsset = GetOrCreateFallbackFixedSkillGroup(FixedSkillGroupId);
	ReportRuntimeConfigValidationErrors(FixedSkillGroupId, FallbackAsset);
	return FallbackAsset;
}

void UDBAFixedSkillGroupLibrary::LoadFixedSkillGroupByIdAsync(const FName& FixedSkillGroupId, FDBAOnFixedSkillGroupLoaded OnLoadedDelegate)
{
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] \u56fa\u5b9a\u6280\u80fd\u7ec4 ID \u4e3a\u7a7a\u3002"));
		OnLoadedDelegate.ExecuteIfBound(nullptr);
		return;
	}

	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager)
	{
		UDBAFixedSkillGroupDataAsset* FallbackAsset = GetOrCreateFallbackFixedSkillGroup(FixedSkillGroupId);
		ReportRuntimeConfigValidationErrors(FixedSkillGroupId, FallbackAsset);
		OnLoadedDelegate.ExecuteIfBound(FallbackAsset);
		return;
	}

	FSoftObjectPath AssetPath;
	if (!FindExistingFixedSkillGroupPath(FixedSkillGroupId, AssetPath))
	{
		UDBAFixedSkillGroupDataAsset* FallbackAsset = GetOrCreateFallbackFixedSkillGroup(FixedSkillGroupId);
		ReportRuntimeConfigValidationErrors(FixedSkillGroupId, FallbackAsset);
		OnLoadedDelegate.ExecuteIfBound(FallbackAsset);
		return;
	}

	AssetManager->GetStreamableManager().RequestAsyncLoad(
		TArray<FSoftObjectPath>({ AssetPath }),
		[AssetPath, FixedSkillGroupId, OnLoadedDelegate]()
		{
			UObject* LoadedObject = AssetPath.ResolveObject();
			UDBAFixedSkillGroupDataAsset* LoadedAsset = Cast<UDBAFixedSkillGroupDataAsset>(LoadedObject);
			if (!LoadedAsset)
			{
				LoadedAsset = GetOrCreateFallbackFixedSkillGroup(FixedSkillGroupId);
			}
			ReportRuntimeConfigValidationErrors(FixedSkillGroupId, LoadedAsset);
			OnLoadedDelegate.ExecuteIfBound(LoadedAsset);
		},
		FStreamableManager::AsyncLoadHighPriority,
		true);
}
