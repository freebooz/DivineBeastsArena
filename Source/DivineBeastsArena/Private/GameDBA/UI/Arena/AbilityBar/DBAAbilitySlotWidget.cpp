// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能槽Widget实现

#include "GameDBA/UI/Arena/AbilityBar/DBAAbilitySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

UDBAAbilitySlotWidget::UDBAAbilitySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAAbilitySlotWidget::SetAbilityInfo(const FDBAAbilityInfo& Info)
{
	AbilityInfo = Info;
	UpdateIconDisplay();

	if (HotkeyText)
	{
		if (Info.Hotkey.IsValid())
		{
			HotkeyText->SetText(FText::FromString(Info.Hotkey.ToString()));
		}
		else
		{
			HotkeyText->SetText(FText::GetEmpty());
		}
	}

	SetAvailable(Info.bEnabled);
}

void UDBAAbilitySlotWidget::SetCooldown(float RemainingTime, float TotalTime)
{
	AbilityInfo.CurrentCooldown = RemainingTime;
	UpdateCooldownDisplay();
}

void UDBAAbilitySlotWidget::SetAvailable(bool bAvailable)
{
	AbilityInfo.bEnabled = bAvailable;

	if (AvailableHighlight)
	{
		AvailableHighlight->SetVisibility(bAvailable ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// 设置图标透明度
	if (SkillIcon)
	{
		FLinearColor Color = SkillIcon->GetColorAndOpacity();
		Color.A = bAvailable ? 1.0f : 0.5f;
		SkillIcon->SetColorAndOpacity(Color);
	}
}

void UDBAAbilitySlotWidget::UpdateCooldownDisplay()
{
	if (!CooldownOverlay || !CooldownText)
	{
		return;
	}

	if (AbilityInfo.CurrentCooldown > 0.0f)
	{
		CooldownOverlay->SetVisibility(ESlateVisibility::Visible);
		CooldownText->SetVisibility(ESlateVisibility::Visible);

		// 计算冷却百分比并更新遮罩
		float CooldownPercent = 0.0f;
		if (AbilityInfo.Cooldown > 0.0f)
		{
			CooldownPercent = AbilityInfo.CurrentCooldown / AbilityInfo.Cooldown;
		}

		// 更新冷却文字
		FText CooldownTextValue = FText::AsNumber(FMath::CeilToInt(AbilityInfo.CurrentCooldown));
		CooldownText->SetText(CooldownTextValue);
	}
	else
	{
		CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		CooldownText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UDBAAbilitySlotWidget::UpdateIconDisplay()
{
	if (SkillIcon && AbilityInfo.Icon)
	{
		SkillIcon->SetBrushFromTexture(AbilityInfo.Icon);
	}
}