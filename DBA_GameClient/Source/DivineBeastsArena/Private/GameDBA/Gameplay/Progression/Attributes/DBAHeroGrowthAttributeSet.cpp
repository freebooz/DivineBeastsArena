// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/Progression/Attributes/DBAHeroGrowthAttributeSet.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBAHeroGrowthDefaultsDataAsset.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Net/UnrealNetwork.h"

UDBAHeroGrowthAttributeSet::UDBAHeroGrowthAttributeSet()
{
	// P1-3 改造：默认值已迁移到 UDBAHeroGrowthDefaultsDataAsset 数据资产驱动
	// 构造函数不再硬编码，由 ADBAPlayerState::RequestDefaultHeroGrowthDefaultsAsync 异步加载后调用 ApplyDefaultAttributes
}

void UDBAHeroGrowthAttributeSet::ApplyDefaultAttributes(const UDBAHeroGrowthDefaultsDataAsset* DefaultsData)
{
	if (!DefaultsData)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[英雄成长属性集] 应用默认属性失败：传入的数据资产为空。"));
		return;
	}

	// 执行数据资产校验，校验失败则输出中文错误日志并拒绝应用
	TArray<FString> ValidationErrors;
	if (!DefaultsData->Execute_ValidateData(DefaultsData, ValidationErrors))
	{
		for (const FString& ValidationError : ValidationErrors)
		{
			UE_LOG(LogDBACombat, Error, TEXT("[英雄成长属性集] 默认属性数据资产校验失败：%s"), *ValidationError);
		}
		return;
	}

	// 从数据资产应用默认值到 AttributeSet
	const FDBAHeroGrowthDefaults& Defaults = DefaultsData->GetDefaults();
	InitHeroLevel(FMath::Max(Defaults.HeroLevel, 1.0f));
	InitExperience(FMath::Max(Defaults.Experience, 0.0f));
	InitExperienceToNextLevel(FMath::Max(Defaults.ExperienceToNextLevel, 1.0f));
	InitRespawnTime(FMath::Max(Defaults.RespawnTime, 0.0f));
	InitGoldBounty(FMath::Max(Defaults.GoldBounty, 0.0f));
}

void UDBAHeroGrowthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 注册复制属性
	DOREPLIFETIME_CONDITION_NOTIFY(UDBAHeroGrowthAttributeSet, HeroLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBAHeroGrowthAttributeSet, Experience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBAHeroGrowthAttributeSet, ExperienceToNextLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBAHeroGrowthAttributeSet, RespawnTime, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBAHeroGrowthAttributeSet, GoldBounty, COND_None, REPNOTIFY_Always);
}

// ========== OnRep 函数实现 ==========

void UDBAHeroGrowthAttributeSet::OnRep_HeroLevel(const FGameplayAttributeData& OldHeroLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBAHeroGrowthAttributeSet, HeroLevel, OldHeroLevel);
}

void UDBAHeroGrowthAttributeSet::OnRep_Experience(const FGameplayAttributeData& OldExperience)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBAHeroGrowthAttributeSet, Experience, OldExperience);
}

void UDBAHeroGrowthAttributeSet::OnRep_ExperienceToNextLevel(const FGameplayAttributeData& OldExperienceToNextLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBAHeroGrowthAttributeSet, ExperienceToNextLevel, OldExperienceToNextLevel);
}

void UDBAHeroGrowthAttributeSet::OnRep_RespawnTime(const FGameplayAttributeData& OldRespawnTime)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBAHeroGrowthAttributeSet, RespawnTime, OldRespawnTime);
}

void UDBAHeroGrowthAttributeSet::OnRep_GoldBounty(const FGameplayAttributeData& OldGoldBounty)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBAHeroGrowthAttributeSet, GoldBounty, OldGoldBounty);
}
