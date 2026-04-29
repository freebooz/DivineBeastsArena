// Copyright FreeboozStudio. All Rights Reserved.

#include "DBA/GAS/Attributes/DBAHeroGrowthAttributeSet.h"
#include "Net/UnrealNetwork.h"

UDBAHeroGrowthAttributeSet::UDBAHeroGrowthAttributeSet()
{
	// 初始化默认值：英雄 1 级，0 经验
	InitHeroLevel(1.0f);
	InitExperience(0.0f);
	InitExperienceToNextLevel(100.0f);
	InitRespawnTime(10.0f);
	InitGoldBounty(300.0f);
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
