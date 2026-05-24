// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Arena/UDBAAbilityBarWidgetBase.h"

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/UI/Arena/AbilityBar/DBAAbilitySlotWidget.h"
#include "InputCoreTypes.h"

UDBAAbilityBarWidgetBase::UDBAAbilityBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAAbilityBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	CacheSkillSlotWidgets();
	if (bAutoBindOwningPawn)
	{
		BindToCharacter(Cast<ADBAZodiacCharacterBase>(GetOwningPlayerPawn()));
	}
}

void UDBAAbilityBarWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAAbilityBarWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bAutoBindOwningPawn && !BoundCharacter.IsValid())
	{
		BindToCharacter(Cast<ADBAZodiacCharacterBase>(GetOwningPlayerPawn()));
	}
	if (bRefreshCooldownsEveryTick)
	{
		RefreshCooldowns();
	}
}

void UDBAAbilityBarWidgetBase::UpdateAbility(int32 SlotIndex, float Cooldown, float ManaCost)
{
	bool bOnCooldown = Cooldown > 0.0f;
	if (SkillSlotWidgets.IsValidIndex(SlotIndex - 1))
	{
		const float TotalCooldown = CachedSkillSpecs.IsValidIndex(SlotIndex - 1)
			? CachedSkillSpecs[SlotIndex - 1].Cooldown
			: Cooldown;
		if (UDBAAbilitySlotWidget* SlotWidget = SkillSlotWidgets[SlotIndex - 1])
		{
			SlotWidget->SetCooldown(Cooldown, TotalCooldown);
		}
	}
	BP_OnAbilityUpdated(SlotIndex, Cooldown, ManaCost, bOnCooldown);
}

void UDBAAbilityBarWidgetBase::SetAbilityEnabled(int32 SlotIndex, bool bEnabled)
{
	if (SkillSlotWidgets.IsValidIndex(SlotIndex - 1))
	{
		if (UDBAAbilitySlotWidget* SlotWidget = SkillSlotWidgets[SlotIndex - 1])
		{
			SlotWidget->SetAvailable(bEnabled);
		}
	}
	BP_OnAbilityEnabledChanged(SlotIndex, bEnabled);
}

void UDBAAbilityBarWidgetBase::BindToCharacter(ADBAZodiacCharacterBase* InCharacter)
{
	if (BoundCharacter.Get() == InCharacter)
	{
		return;
	}

	BoundCharacter = InCharacter;
	RefreshSkillCatalog();
	RefreshCooldowns();
}

void UDBAAbilityBarWidgetBase::RefreshSkillCatalog()
{
	CachedSkillSpecs.Reset();

	ADBAZodiacCharacterBase* Character = BoundCharacter.Get();
	if (!Character)
	{
		BP_OnSkillCatalogRefreshed(CachedSkillSpecs);
		return;
	}

	CachedSkillSpecs = Character->GetPlayableSkillSpecs();
	CachedSkillSpecs.Sort([](const FDBAPlayableSkillRuntimeSpec& Left, const FDBAPlayableSkillRuntimeSpec& Right)
	{
		return Left.SkillSlot < Right.SkillSlot;
	});

	for (const FDBAPlayableSkillRuntimeSpec& SkillSpec : CachedSkillSpecs)
	{
		const int32 SlotArrayIndex = SkillSpec.SkillSlot - 1;
		if (!SkillSlotWidgets.IsValidIndex(SlotArrayIndex))
		{
			continue;
		}

		if (UDBAAbilitySlotWidget* SlotWidget = SkillSlotWidgets[SlotArrayIndex])
		{
			SlotWidget->SetAbilityFromPlayableSkill(SkillSpec, ResolveHotkeyForSlot(SkillSpec.SkillSlot));
		}
	}

	BP_OnSkillCatalogRefreshed(CachedSkillSpecs);
}

void UDBAAbilityBarWidgetBase::RefreshCooldowns()
{
	const ADBAZodiacCharacterBase* Character = BoundCharacter.Get();
	if (!Character)
	{
		return;
	}

	const TArray<float> Cooldowns = Character->GetSkillCooldowns();
	for (const FDBAPlayableSkillRuntimeSpec& SkillSpec : CachedSkillSpecs)
	{
		const int32 SkillSlot = SkillSpec.SkillSlot;
		const float Remaining = Cooldowns.IsValidIndex(SkillSlot) ? Cooldowns[SkillSlot] : 0.0f;
		UpdateAbility(SkillSlot, Remaining, 0.0f);
	}
}

void UDBAAbilityBarWidgetBase::CacheSkillSlotWidgets()
{
	SkillSlotWidgets.Reset();
	SkillSlotWidgets.Add(SkillSlot01);
	SkillSlotWidgets.Add(SkillSlot02);
	SkillSlotWidgets.Add(SkillSlot03);
	SkillSlotWidgets.Add(SkillSlot04);
	SkillSlotWidgets.Add(SkillSlot05);
	SkillSlotWidgets.Add(SkillSlot06);
}

FKey UDBAAbilityBarWidgetBase::ResolveHotkeyForSlot(int32 SkillSlot) const
{
	switch (SkillSlot)
	{
	case 1: return EKeys::One;
	case 2: return EKeys::Two;
	case 3: return EKeys::Three;
	case 4: return EKeys::Four;
	case 5: return EKeys::R;
	case 6: return EKeys::Six;
	default: return FKey();
	}
}
