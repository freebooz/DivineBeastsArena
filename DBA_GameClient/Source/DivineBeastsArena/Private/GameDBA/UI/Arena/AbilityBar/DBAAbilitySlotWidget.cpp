// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 技能槽Widget实现

#include "GameDBA/UI/Arena/AbilityBar/DBAAbilitySlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameDBA/Combat/DBAPlayableSkillTypes.h"

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
	UpdateCooldownDisplay();
}

void UDBAAbilitySlotWidget::SetAbilityFromPlayableSkill(const FDBAPlayableSkillRuntimeSpec& SkillSpec, FKey InHotkey)
{
	FDBAAbilityInfo Info;
	Info.AbilityName = SkillSpec.DisplayName.IsEmpty() ? FText::FromName(SkillSpec.SkillId) : SkillSpec.DisplayName;
	Info.Hotkey = InHotkey;
	Info.Cooldown = SkillSpec.Cooldown;
	Info.CurrentCooldown = 0.0f;
	Info.bEnabled = true;
	SetAbilityInfo(Info);
}

void UDBAAbilitySlotWidget::SetCooldown(float RemainingTime, float TotalTime)
{
	AbilityInfo.CurrentCooldown = RemainingTime;
	AbilityInfo.Cooldown = TotalTime;
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
