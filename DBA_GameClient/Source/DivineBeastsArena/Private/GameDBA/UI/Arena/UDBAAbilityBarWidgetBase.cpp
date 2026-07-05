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
#include "GameDBA/Combat/DBAPlayableSkillComponent.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/UI/Arena/AbilityBar/DBAAbilitySlotWidget.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

UDBAAbilityBarWidgetBase::UDBAAbilityBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAAbilityBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	CacheSkillSlotWidgets();
	if (BoundCharacter.IsValid())
	{
		RefreshSkillCatalog();
		RefreshCooldowns();
	}
	if (bAutoBindOwningPawn)
	{
		BindToCharacter(Cast<ADBAZodiacCharacterBase>(GetOwningPlayerPawn()));
	}
}

void UDBAAbilityBarWidgetBase::NativeDestruct()
{
	UnbindFromCooldownEvents();
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
	if (SlotIndex < 1 || SlotIndex > DBAConstants::CoreCombatInputCount)
	{
		return;
	}

	const float ClampedCooldown = FMath::Max(0.0f, Cooldown);
	const float ClampedManaCost = FMath::Max(0.0f, ManaCost);
	bool bOnCooldown = ClampedCooldown > 0.0f;
	if (SkillSlotWidgets.IsValidIndex(SlotIndex - 1))
	{
		const float TotalCooldown = CachedSkillSpecs.IsValidIndex(SlotIndex - 1)
			? FMath::Max(0.0f, CachedSkillSpecs[SlotIndex - 1].Cooldown)
			: ClampedCooldown;
		if (UDBAAbilitySlotWidget* SlotWidget = SkillSlotWidgets[SlotIndex - 1])
		{
			SlotWidget->SetCooldown(ClampedCooldown, TotalCooldown);
		}
	}
	BP_OnAbilityUpdated(SlotIndex, ClampedCooldown, ClampedManaCost, bOnCooldown);
}

void UDBAAbilityBarWidgetBase::SetAbilityEnabled(int32 SlotIndex, bool bEnabled)
{
	if (SlotIndex < 1 || SlotIndex > DBAConstants::CoreCombatInputCount)
	{
		return;
	}

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
		RefreshSkillCatalog();
		RefreshCooldowns();
		return;
	}

	UnbindFromCooldownEvents();
	BoundCharacter = InCharacter;
	if (InCharacter)
	{
		InCharacter->OnSkillCooldownsChanged.AddDynamic(this, &UDBAAbilityBarWidgetBase::HandleSkillCooldownsChanged);
		CooldownEventCharacter = InCharacter;
	}
	RefreshSkillCatalog();
	RefreshCooldowns();
}

void UDBAAbilityBarWidgetBase::RefreshSkillCatalog()
{
	CachedSkillSpecs.Reset();
	CachedSkillCatalogSummary = FDBAPlayableSkillCatalogSummary();

	ADBAZodiacCharacterBase* Character = BoundCharacter.Get();
	if (!Character)
	{
		BP_OnSkillCatalogRefreshed(CachedSkillSpecs);
		BP_OnSkillCatalogSummaryRefreshed(CachedSkillCatalogSummary);
		return;
	}

	CachedSkillSpecs = Character->GetPlayableSkillSpecs();
	if (const UDBAPlayableSkillComponent* SkillComponent = Character->GetPlayableSkillComponent())
	{
		CachedSkillCatalogSummary = SkillComponent->GetSkillCatalogSummary();
	}

	CachedSkillSpecs.Sort([](const FDBAPlayableSkillRuntimeSpec& Left, const FDBAPlayableSkillRuntimeSpec& Right)
	{
		return Left.SkillSlot < Right.SkillSlot;
	});

	for (FDBAPlayableSkillRuntimeSpec& SkillSpec : CachedSkillSpecs)
	{
		ApplyRuntimeConfigToSkillSpec(SkillSpec);

		const int32 SlotArrayIndex = SkillSpec.SkillSlot - 1;
		if (!SkillSlotWidgets.IsValidIndex(SlotArrayIndex))
		{
			continue;
		}

		if (UDBAAbilitySlotWidget* SlotWidget = SkillSlotWidgets[SlotArrayIndex])
		{
			SlotWidget->SetAbilityFromPlayableSkill(SkillSpec, ResolveHotkeyForSlot(SkillSpec.SkillSlot));
			RequestSkillIconAsync(SlotWidget, SkillSpec);
		}
	}

	BP_OnSkillCatalogRefreshed(CachedSkillSpecs);
	BP_OnSkillCatalogSummaryRefreshed(CachedSkillCatalogSummary);
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
		const int32 CooldownArrayIndex = SkillSlot - 1;
		const float Remaining = Cooldowns.IsValidIndex(CooldownArrayIndex) ? Cooldowns[CooldownArrayIndex] : 0.0f;
		UpdateAbility(SkillSlot, Remaining, 0.0f);
	}
}

void UDBAAbilityBarWidgetBase::HandleSkillCooldownsChanged(const TArray<float>& Cooldowns)
{
	for (const FDBAPlayableSkillRuntimeSpec& SkillSpec : CachedSkillSpecs)
	{
		const int32 SkillSlot = SkillSpec.SkillSlot;
		const int32 CooldownArrayIndex = SkillSlot - 1;
		const float Remaining = Cooldowns.IsValidIndex(CooldownArrayIndex) ? Cooldowns[CooldownArrayIndex] : 0.0f;
		UpdateAbility(SkillSlot, Remaining, 0.0f);
	}
}

void UDBAAbilityBarWidgetBase::UnbindFromCooldownEvents()
{
	if (ADBAZodiacCharacterBase* Character = CooldownEventCharacter.Get())
	{
		Character->OnSkillCooldownsChanged.RemoveDynamic(this, &UDBAAbilityBarWidgetBase::HandleSkillCooldownsChanged);
	}
	CooldownEventCharacter.Reset();
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

void UDBAAbilityBarWidgetBase::ApplyRuntimeConfigToSkillSpec(FDBAPlayableSkillRuntimeSpec& InOutSkillSpec) const
{
	const ADBAZodiacCharacterBase* Character = BoundCharacter.Get();
	if (!Character)
	{
		return;
	}

	const UDBAAbilitySystemComponent* DBAAbilitySystem = Cast<UDBAAbilitySystemComponent>(Character->GetAbilitySystemComponent());
	if (!DBAAbilitySystem)
	{
		return;
	}

	const FDBAAbilityRuntimeConfig* RuntimeConfig = DBAAbilitySystem->FindAbilityRuntimeConfigByInputID(
		MapSkillSlotToAbilityInputID(InOutSkillSpec.SkillSlot));
	if (!RuntimeConfig)
	{
		return;
	}

	if (!RuntimeConfig->DisplayName.IsEmpty())
	{
		InOutSkillSpec.DisplayName = RuntimeConfig->DisplayName;
	}

	if (!RuntimeConfig->Icon.IsNull())
	{
		InOutSkillSpec.Icon = RuntimeConfig->Icon;
	}

	if (RuntimeConfig->CooldownDuration > 0.0f)
	{
		InOutSkillSpec.Cooldown = RuntimeConfig->CooldownDuration;
	}
}

int32 UDBAAbilityBarWidgetBase::MapSkillSlotToAbilityInputID(int32 SkillSlot) const
{
	switch (SkillSlot)
	{
	case 1: return static_cast<int32>(EDBAAbilityInputID::Skill01);
	case 2: return static_cast<int32>(EDBAAbilityInputID::Skill02);
	case 3: return static_cast<int32>(EDBAAbilityInputID::Skill03);
	case 4: return static_cast<int32>(EDBAAbilityInputID::Skill04);
	case 5: return static_cast<int32>(EDBAAbilityInputID::Ultimate);
	default: return static_cast<int32>(EDBAAbilityInputID::None);
	}
}

void UDBAAbilityBarWidgetBase::RequestSkillIconAsync(UDBAAbilitySlotWidget* SlotWidget, const FDBAPlayableSkillRuntimeSpec& SkillSpec)
{
	if (!SlotWidget || SkillSpec.Icon.IsNull())
	{
		return;
	}

	if (UTexture2D* LoadedIcon = SkillSpec.Icon.Get())
	{
		FDBAAbilityInfo Info = SlotWidget->GetAbilityInfo();
		Info.Icon = LoadedIcon;
		SlotWidget->SetAbilityInfo(Info);
		return;
	}

	TWeakObjectPtr<UDBAAbilitySlotWidget> WeakSlotWidget = SlotWidget;
	DBAAsyncAssetLoader::RequestAsyncAsset<UTexture2D>(this, SkillSpec.Icon, [WeakSlotWidget](UTexture2D* LoadedIcon)
	{
		UDBAAbilitySlotWidget* LoadedSlotWidget = WeakSlotWidget.Get();
		if (!LoadedSlotWidget || !LoadedIcon)
		{
			return;
		}

		FDBAAbilityInfo Info = LoadedSlotWidget->GetAbilityInfo();
		Info.Icon = LoadedIcon;
		LoadedSlotWidget->SetAbilityInfo(Info);
	});
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
