// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/GAS/Attributes/DBAHeroGrowthAttributeSet.h"
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
