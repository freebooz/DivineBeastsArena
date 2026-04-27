// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/FixedSkillGroup/DBAFixedSkillGroupSubsystem.h"
#include "Data/DBAFixedSkillGroupData.h"
#include "Common/DBALogChannels.h"

UDBAFixedSkillGroupSubsystem::UDBAFixedSkillGroupSubsystem()
{
}

void UDBAFixedSkillGroupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFixedSkillGroupSubsystem] 初始化完成"));
}

void UDBAFixedSkillGroupSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

FDBAFixedSkillGroup UDBAFixedSkillGroupSubsystem::GetFixedSkillGroup(FName HeroId, EDBAElement Element) const
{
    FDBAFixedSkillGroup SkillGroup;
    SkillGroup.GroupId = FName(*FString::Printf(TEXT("%s_%s"), *HeroId.ToString(), *UEnum::GetValueAsName(Element).ToString()));
    SkillGroup.HeroId = HeroId;
    SkillGroup.Element = Element;

    switch (Element)
    {
    case EDBAElement::Gold:
        SkillGroup.PassiveSkillId = FName(TEXT("Passive_Metal_001"));
        SkillGroup.Skill01Id = FName(TEXT("Metal_Skill01"));
        SkillGroup.Skill02Id = FName(TEXT("Metal_Skill02"));
        SkillGroup.Skill03Id = FName(TEXT("Metal_Skill03"));
        SkillGroup.Skill04Id = FName(TEXT("Metal_Skill04"));
        SkillGroup.UltimateSkillId = FName(TEXT("Ultimate_Zodiac_001"));
        SkillGroup.ResonanceLevel = 4;
        SkillGroup.ResonanceDescription = FText::FromString(TEXT("控制时间 +1.0秒，护盾值 +20%"));
        break;
    case EDBAElement::Wood:
        SkillGroup.PassiveSkillId = FName(TEXT("Passive_Wood_001"));
        SkillGroup.Skill01Id = FName(TEXT("Wood_Skill01"));
        SkillGroup.Skill02Id = FName(TEXT("Wood_Skill02"));
        SkillGroup.Skill03Id = FName(TEXT("Wood_Skill03"));
        SkillGroup.Skill04Id = FName(TEXT("Wood_Skill04"));
        SkillGroup.UltimateSkillId = FName(TEXT("Ultimate_Zodiac_001"));
        SkillGroup.ResonanceLevel = 4;
        SkillGroup.ResonanceDescription = FText::FromString(TEXT("生命值 +15%，持续恢复 +10%"));
        break;
    case EDBAElement::Water:
        SkillGroup.PassiveSkillId = FName(TEXT("Passive_Water_001"));
        SkillGroup.Skill01Id = FName(TEXT("Water_Skill01"));
        SkillGroup.Skill02Id = FName(TEXT("Water_Skill02"));
        SkillGroup.Skill03Id = FName(TEXT("Water_Skill03"));
        SkillGroup.Skill04Id = FName(TEXT("Water_Skill04"));
        SkillGroup.UltimateSkillId = FName(TEXT("Ultimate_Zodiac_001"));
        SkillGroup.ResonanceLevel = 4;
        SkillGroup.ResonanceDescription = FText::FromString(TEXT("法力值 +20%，技能冷却 -10%"));
        break;
    case EDBAElement::Fire:
        SkillGroup.PassiveSkillId = FName(TEXT("Passive_Fire_001"));
        SkillGroup.Skill01Id = FName(TEXT("Fire_Skill01"));
        SkillGroup.Skill02Id = FName(TEXT("Fire_Skill02"));
        SkillGroup.Skill03Id = FName(TEXT("Fire_Skill03"));
        SkillGroup.Skill04Id = FName(TEXT("Fire_Skill04"));
        SkillGroup.UltimateSkillId = FName(TEXT("Ultimate_Zodiac_001"));
        SkillGroup.ResonanceLevel = 4;
        SkillGroup.ResonanceDescription = FText::FromString(TEXT("攻击速度 +15%，暴击率 +10%"));
        break;
    case EDBAElement::Earth:
        SkillGroup.PassiveSkillId = FName(TEXT("Passive_Earth_001"));
        SkillGroup.Skill01Id = FName(TEXT("Earth_Skill01"));
        SkillGroup.Skill02Id = FName(TEXT("Earth_Skill02"));
        SkillGroup.Skill03Id = FName(TEXT("Earth_Skill03"));
        SkillGroup.Skill04Id = FName(TEXT("Earth_Skill04"));
        SkillGroup.UltimateSkillId = FName(TEXT("Ultimate_Zodiac_001"));
        SkillGroup.ResonanceLevel = 4;
        SkillGroup.ResonanceDescription = FText::FromString(TEXT("护甲 +20%，韧性 +15%"));
        break;
    default:
        break;
    }

    return SkillGroup;
}

bool UDBAFixedSkillGroupSubsystem::HasFixedSkillGroup(FName HeroId, EDBAElement Element) const
{
    return Element != EDBAElement::None && !HeroId.IsNone();
}

int32 UDBAFixedSkillGroupSubsystem::CalculateResonanceLevel(EDBAElement Element, const TArray<FName>& SkillIds) const
{
    if (Element == EDBAElement::None)
    {
        return 0;
    }

    int32 SameElementCount = 0;
    FString ElementStr = UEnum::GetValueAsString(Element);
    for (const FName& SkillId : SkillIds)
    {
        if (SkillId.ToString().Contains(ElementStr))
        {
            SameElementCount++;
        }
    }

    return FMath::Clamp(SameElementCount, 0, 4);
}

FText UDBAFixedSkillGroupSubsystem::GetResonanceDescription(EDBAElement Element, int32 ResonanceLevel) const
{
    if (ResonanceLevel <= 0)
    {
        return FText::FromString(TEXT("无共鸣效果"));
    }

    switch (Element)
    {
    case EDBAElement::Gold:
        return FText::FromString(FString::Printf(TEXT("共鸣等级 %d: 控制时间 +%.1f秒，护盾值 +%d%%"),
            ResonanceLevel, ResonanceLevel * 0.25f, ResonanceLevel * 5));
    case EDBAElement::Wood:
        return FText::FromString(FString::Printf(TEXT("共鸣等级 %d: 生命值 +%d%%，持续恢复 +%d%%"),
            ResonanceLevel, ResonanceLevel * 3, ResonanceLevel * 2));
    case EDBAElement::Water:
        return FText::FromString(FString::Printf(TEXT("共鸣等级 %d: 法力值 +%d%%，技能冷却 -%d%%"),
            ResonanceLevel, ResonanceLevel * 4, ResonanceLevel * 2));
    case EDBAElement::Fire:
        return FText::FromString(FString::Printf(TEXT("共鸣等级 %d: 攻击速度 +%d%%，暴击率 +%d%%"),
            ResonanceLevel, ResonanceLevel * 3, ResonanceLevel * 2));
    case EDBAElement::Earth:
        return FText::FromString(FString::Printf(TEXT("共鸣等级 %d: 护甲 +%d%%，韧性 +%d%%"),
            ResonanceLevel, ResonanceLevel * 4, ResonanceLevel * 3));
    default:
        return FText::FromString(TEXT("无共鸣效果"));
    }
}

void UDBAFixedSkillGroupSubsystem::SetCurrentBuildSummary(const FDBABuildSummary& Summary)
{
    CurrentBuildSummary = Summary;
    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFixedSkillGroupSubsystem] Build Summary 已设置: Hero=%s, Element=%d, FiveCamp=%d"),
        *Summary.HeroId.ToString(), static_cast<int32>(Summary.Element), static_cast<int32>(Summary.FiveCamp));
}

void UDBAFixedSkillGroupSubsystem::ClearCurrentBuildSummary()
{
    CurrentBuildSummary = FDBABuildSummary();
}
