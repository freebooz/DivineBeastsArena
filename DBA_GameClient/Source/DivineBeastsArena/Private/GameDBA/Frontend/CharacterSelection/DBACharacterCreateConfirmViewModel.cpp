// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateConfirmViewModel.h"

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"

namespace
{
	/** 将稳定外观 ID 压缩为可读摘要，不泄漏客户端资产路径，也不尝试把本地摘要当作服务端校验结果。 */
	int32 CountConfiguredAppearanceOptions(const FDBACharacterAppearance& Appearance)
	{
		int32 Count = Appearance.EquipmentVisualIds.Num();
		const TArray<FName> SingleOptions = {
			Appearance.GenderId, Appearance.BodyTypeId, Appearance.FaceId, Appearance.HairId, Appearance.HairColorId,
			Appearance.SkinColorId, Appearance.EyeColorId, Appearance.MarkingId, Appearance.HornId, Appearance.EarId,
			Appearance.TailId, Appearance.WeaponVisualId, Appearance.SkinId
		};
		for (const FName OptionId : SingleOptions)
		{
			Count += !OptionId.IsNone() ? 1 : 0;
		}
		return Count;
	}

	template <typename TEnum>
	FText ToEnumText(const TEnum Value)
	{
		return StaticEnum<TEnum>()->GetDisplayNameTextByValue(static_cast<int64>(Value));
	}
}

void UDBACharacterCreateConfirmViewModel::ApplyDraft(const FDBACharacterCreateDraft& Draft)
{
	CharacterName = Draft.CharacterName;
	ZodiacSummary = ToEnumText(Draft.ZodiacType);
	ElementSummary = ToEnumText(Draft.ElementType);
	FiveCampSummary = ToEnumText(Draft.FiveCampType);
	AppearanceSummary = FText::Format(
		NSLOCTEXT("DBACharacterCreateConfirm", "AppearanceSummary", "已配置 {0} 项外观选项"),
		FText::AsNumber(CountConfiguredAppearanceOptions(Draft.Appearance)));
	FixedBuildSummary = Draft.FixedSkillBuildRowId.IsNone()
		? NSLOCTEXT("DBACharacterCreateConfirm", "BuildPending", "固定构筑摘要尚未生成。")
		: FText::Format(NSLOCTEXT("DBACharacterCreateConfirm", "BuildSummary", "固定构筑：{0}"), FText::FromName(Draft.FixedSkillBuildRowId));
	AttributeSummary = Draft.PreviewSummary.IsEmpty()
		? NSLOCTEXT("DBACharacterCreateConfirm", "AttributePending", "属性预览将在服务端创建校验后最终确认。")
		: Draft.PreviewSummary;
	OnChanged.Broadcast();
}

void UDBACharacterCreateConfirmViewModel::SetSubmitting(const bool bInSubmitting)
{
	bIsSubmitting = bInSubmitting;
	OnChanged.Broadcast();
}

void UDBACharacterCreateConfirmViewModel::SetError(const FDBAApiError& Error)
{
	LastError = Error;
	OnChanged.Broadcast();
}

void UDBACharacterCreateConfirmViewModel::ClearError()
{
	LastError = FDBAApiError();
	OnChanged.Broadcast();
}
