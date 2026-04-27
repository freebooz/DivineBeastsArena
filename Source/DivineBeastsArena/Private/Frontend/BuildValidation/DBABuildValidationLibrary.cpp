// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/BuildValidation/DBABuildValidationLibrary.h"
#include "Frontend/FixedSkillGroup/DBAFixedSkillGroupSubsystem.h"

FDBABuildValidationInfo UDBABuildValidationLibrary::ValidateBuild(
    const FDBABuildSummary& BuildSummary,
    const TArray<FName>& AvailableHeroIds,
    const TArray<EDBAElement>& AvailableElements,
    const TArray<EDBAFiveCamp>& AvailableFiveCamps)
{
    FDBABuildValidationInfo ValidationInfo;

    if (!IsHeroAvailable(BuildSummary.HeroId, AvailableHeroIds))
    {
        ValidationInfo.Result = EDBABuildValidationResult::InvalidHero;
        ValidationInfo.bIsValid = false;
        ValidationInfo.Errors.Add(FText::FromString(TEXT("所选英雄不可用")));
        ValidationInfo.ValidationMessage = GetValidationResultMessage(EDBABuildValidationResult::InvalidHero);
        return ValidationInfo;
    }

    if (!IsElementAvailable(BuildSummary.Element, AvailableElements))
    {
        ValidationInfo.Result = EDBABuildValidationResult::InvalidElement;
        ValidationInfo.bIsValid = false;
        ValidationInfo.Errors.Add(FText::FromString(TEXT("所选元素不可用")));
        ValidationInfo.ValidationMessage = GetValidationResultMessage(EDBABuildValidationResult::InvalidElement);
        return ValidationInfo;
    }

    if (!IsFiveCampAvailable(BuildSummary.FiveCamp, AvailableFiveCamps))
    {
        ValidationInfo.Result = EDBABuildValidationResult::InvalidFiveCamp;
        ValidationInfo.bIsValid = false;
        ValidationInfo.Errors.Add(FText::FromString(TEXT("所选阵营不可用")));
        ValidationInfo.ValidationMessage = GetValidationResultMessage(EDBABuildValidationResult::InvalidFiveCamp);
        return ValidationInfo;
    }

    if (BuildSummary.Element != EDBAElement::None)
    {
        EDBAElement CounterElement = GetCounterElement(BuildSummary.Element);
        ValidationInfo.Warnings.Add(FText::FromString(FString::Printf(
            TEXT("注意：%s 克制 %s，选择被克制元素可能处于劣势"),
            *UEnum::GetValueAsName(BuildSummary.Element).ToString(),
            *UEnum::GetValueAsName(CounterElement).ToString())));
    }

    ValidationInfo.Result = EDBABuildValidationResult::Valid;
    ValidationInfo.bIsValid = true;
    ValidationInfo.ValidationMessage = FText::FromString(TEXT("Build 验证通过"));
    return ValidationInfo;
}

bool UDBABuildValidationLibrary::IsHeroAvailable(FName HeroId, const TArray<FName>& AvailableHeroIds)
{
    if (HeroId.IsNone())
    {
        return false;
    }

    if (AvailableHeroIds.Num() == 0)
    {
        return true;
    }

    return AvailableHeroIds.Contains(HeroId);
}

bool UDBABuildValidationLibrary::IsElementAvailable(EDBAElement Element, const TArray<EDBAElement>& AvailableElements)
{
    if (Element == EDBAElement::None)
    {
        return false;
    }

    if (AvailableElements.Num() == 0)
    {
        return true;
    }

    return AvailableElements.Contains(Element);
}

bool UDBABuildValidationLibrary::IsFiveCampAvailable(EDBAFiveCamp FiveCamp, const TArray<EDBAFiveCamp>& AvailableFiveCamps)
{
    if (FiveCamp == EDBAFiveCamp::None)
    {
        return false;
    }

    if (AvailableFiveCamps.Num() == 0)
    {
        return true;
    }

    return AvailableFiveCamps.Contains(FiveCamp);
}

FText UDBABuildValidationLibrary::GetValidationResultMessage(EDBABuildValidationResult Result)
{
    switch (Result)
    {
    case EDBABuildValidationResult::Valid:
        return FText::FromString(TEXT("Build 验证通过"));
    case EDBABuildValidationResult::InvalidHero:
        return FText::FromString(TEXT("所选英雄不可用"));
    case EDBABuildValidationResult::InvalidElement:
        return FText::FromString(TEXT("所选元素不可用"));
    case EDBABuildValidationResult::InvalidFiveCamp:
        return FText::FromString(TEXT("所选阵营不可用"));
    case EDBABuildValidationResult::MissingSkillGroup:
        return FText::FromString(TEXT("缺少技能组配置"));
    case EDBABuildValidationResult::ElementFiveCampConflict:
        return FText::FromString(TEXT("元素与阵营存在冲突"));
    default:
        return FText::FromString(TEXT("未知验证结果"));
    }
}

EDBABuildValidationResult UDBABuildValidationLibrary::GetCounterElementValidationResult(EDBAElement Element)
{
    return EDBABuildValidationResult::Valid;
}

EDBAElement UDBABuildValidationLibrary::GetCounterElement(EDBAElement Element)
{
    switch (Element)
    {
    case EDBAElement::Gold: return EDBAElement::Wood;
    case EDBAElement::Wood: return EDBAElement::Earth;
    case EDBAElement::Earth: return EDBAElement::Water;
    case EDBAElement::Water: return EDBAElement::Fire;
    case EDBAElement::Fire: return EDBAElement::Gold;
    default: return EDBAElement::None;
    }
}

EDBAElement UDBABuildValidationLibrary::GetCounteredByElement(EDBAElement Element)
{
    switch (Element)
    {
    case EDBAElement::Gold: return EDBAElement::Fire;
    case EDBAElement::Wood: return EDBAElement::Gold;
    case EDBAElement::Earth: return EDBAElement::Wood;
    case EDBAElement::Water: return EDBAElement::Earth;
    case EDBAElement::Fire: return EDBAElement::Water;
    default: return EDBAElement::None;
    }
}
