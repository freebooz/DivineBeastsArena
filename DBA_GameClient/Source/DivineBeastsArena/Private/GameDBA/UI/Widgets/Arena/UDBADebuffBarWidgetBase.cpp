// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Arena/UDBADebuffBarWidgetBase.h"
#include "Math/UnrealMathUtility.h"

namespace
{
bool NormalizeDebuffWidgetId(const FString& EffectId, FString& OutEffectId)
{
	OutEffectId = EffectId.TrimStartAndEnd();
	return !OutEffectId.IsEmpty();
}
}

UDBADebuffBarWidgetBase::UDBADebuffBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBADebuffBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	BP_OnDebuffsCleared();
	for (const TPair<FString, float>& CachedDebuff : CachedActiveDebuffs)
	{
		BP_OnDebuffAdded(CachedDebuff.Key, CachedDebuff.Value);
	}
}

void UDBADebuffBarWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBADebuffBarWidgetBase::AddDebuff(const FString& DebuffId, float Duration)
{
	FString NormalizedDebuffId;
	if (!NormalizeDebuffWidgetId(DebuffId, NormalizedDebuffId))
	{
		return;
	}

	CachedActiveDebuffs.Add(NormalizedDebuffId, FMath::Max(0.0f, Duration));
	BP_OnDebuffAdded(NormalizedDebuffId, CachedActiveDebuffs[NormalizedDebuffId]);
}

void UDBADebuffBarWidgetBase::RemoveDebuff(const FString& DebuffId)
{
	FString NormalizedDebuffId;
	if (!NormalizeDebuffWidgetId(DebuffId, NormalizedDebuffId))
	{
		return;
	}

	CachedActiveDebuffs.Remove(NormalizedDebuffId);
	BP_OnDebuffRemoved(NormalizedDebuffId);
}

void UDBADebuffBarWidgetBase::ClearAllDebuffs()
{
	CachedActiveDebuffs.Reset();
	BP_OnDebuffsCleared();
}
