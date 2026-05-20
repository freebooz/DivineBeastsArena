// Copyright FreeboozStudio. All Rights Reserved.

#include "GameDBA/Services/DBASkillGroupGeneratorSubsystem.h"
#include "GameDBA/Core/DBAConstants.h"
#include "Engine/DataTable.h"

UDBASkillGroupGeneratorSubsystem::UDBASkillGroupGeneratorSubsystem()
{
	// 默认使用固定路径
}

void UDBASkillGroupGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSkillGroupDataTable();
	LoadSkillGroupSummaryDataTable();
}

void UDBASkillGroupGeneratorSubsystem::Deinitialize()
{
	LoadedSkillGroupDataTable = nullptr;
	LoadedSkillGroupSummaryDataTable = nullptr;

	Super::Deinitialize();
}

void UDBASkillGroupGeneratorSubsystem::LoadSkillGroupDataTable()
{
	// 使用软引用加载数据表
	if (SkillGroupDataTable.IsNull())
	{
		SkillGroupDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(SkillGroupDataTablePath));
	}

	if (!LoadedSkillGroupDataTable)
	{
		LoadedSkillGroupDataTable = SkillGroupDataTable.LoadSynchronous();
	}
}

void UDBASkillGroupGeneratorSubsystem::LoadSkillGroupSummaryDataTable()
{
	if (SkillGroupSummaryDataTable.IsNull())
	{
		SkillGroupSummaryDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(SkillGroupSummaryDataTablePath));
	}

	if (!LoadedSkillGroupSummaryDataTable)
	{
		LoadedSkillGroupSummaryDataTable = SkillGroupSummaryDataTable.LoadSynchronous();
	}
}

bool UDBASkillGroupGeneratorSubsystem::GetSkillGroup(EDBAZodiac Zodiac, EDBAElement Element, FDBAZodiacElementFixedSkillGroupRow& OutSkillGroup) const
{
	if (!LoadedSkillGroupDataTable)
	{
		return false;
	}

	// 构建行名：Zodiac_Element 格式
	FString RowNameStr = FString::Printf(TEXT("%s_%s"),
		*UEnum::GetValueAsString(Zodiac),
		*UEnum::GetValueAsString(Element));
	FName RowName(*RowNameStr);

	return FindSkillGroupByRowName(RowName, OutSkillGroup);
}

bool UDBASkillGroupGeneratorSubsystem::GetSkillGroupSummary(EDBAZodiac Zodiac, FDBAZodiacHeroAbilitySetSummaryRow& OutSummary) const
{
	if (!LoadedSkillGroupSummaryDataTable)
	{
		return false;
	}

	FName RowName(*UEnum::GetValueAsString(Zodiac));
	const FDBAZodiacHeroAbilitySetSummaryRow* FoundRow = LoadedSkillGroupSummaryDataTable->FindRow<FDBAZodiacHeroAbilitySetSummaryRow>(RowName, TEXT(""));
	if (FoundRow)
	{
		OutSummary = *FoundRow;
		return true;
	}

	return false;
}

TArray<EDBAZodiac> UDBASkillGroupGeneratorSubsystem::GetAllZodiacTypes() const
{
	TArray<EDBAZodiac> ZodiacTypes;

	// 遍历所有生肖类型 (跳过 None)
	static const auto EnumNames = StaticEnum<EDBAZodiac>();
	for (int32 i = 1; i < EnumNames->NumEnums() - 1; ++i) // 跳过 MAX 和 None
	{
		EDBAZodiac Zodiac = static_cast<EDBAZodiac>(EnumNames->GetValueByIndex(i));
		if (Zodiac != EDBAZodiac::None)
		{
			ZodiacTypes.Add(Zodiac);
		}
	}

	return ZodiacTypes;
}

TArray<EDBAElement> UDBASkillGroupGeneratorSubsystem::GetAllElementTypes() const
{
	TArray<EDBAElement> ElementTypes;

	// 遍历所有元素类型 (跳过 None 和 MAX)
	static const auto EnumNames = StaticEnum<EDBAElement>();
	const int32 MaxEnums = EnumNames->NumEnums();
	for (int32 i = 1; i < MaxEnums; ++i)
	{
		EDBAElement Element = static_cast<EDBAElement>(EnumNames->GetValueByIndex(i));
		if (Element != EDBAElement::None)
		{
			ElementTypes.Add(Element);
		}
	}

	return ElementTypes;
}

bool UDBASkillGroupGeneratorSubsystem::IsSkillGroupConfigured(EDBAZodiac Zodiac, EDBAElement Element) const
{
	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	return GetSkillGroup(Zodiac, Element, SkillGroup);
}

FName UDBASkillGroupGeneratorSubsystem::GetZodiacUltimateSkillId(EDBAZodiac Zodiac) const
{
	// 获取任意元素的技能组都可以获取生肖大招ID
	// 通常用第一个元素来查询
	TArray<EDBAElement> Elements = GetAllElementTypes();
	if (Elements.Num() > 0)
	{
		FDBAZodiacElementFixedSkillGroupRow SkillGroup;
		if (GetSkillGroup(Zodiac, Elements[0], SkillGroup))
		{
			return SkillGroup.ZodiacUltimateSkillId;
		}
	}
	return NAME_None;
}

int32 UDBASkillGroupGeneratorSubsystem::CalculateResonanceLevel(const TArray<FName>& SkillIds, EDBAElement Element) const
{
	// 计算同元素技能数量
	int32 SameElementSkillCount = 0;
	for (const FName& SkillId : SkillIds)
	{
		// 这里应该查询技能表获取技能元素类型
		// 简化实现：假设所有技能都是同一元素
		SameElementSkillCount++;
	}

	// 根据技能数量计算共鸣等级
	if (SameElementSkillCount >= DBAConstants::ResonanceLevel4_SkillCount)
	{
		return 4;
	}
	else if (SameElementSkillCount >= DBAConstants::ResonanceLevel3_SkillCount)
	{
		return 3;
	}
	else if (SameElementSkillCount >= DBAConstants::ResonanceLevel2_SkillCount)
	{
		return 2;
	}
	else if (SameElementSkillCount >= DBAConstants::ResonanceLevel1_SkillCount)
	{
		return 1;
	}

	return 0;
}

bool UDBASkillGroupGeneratorSubsystem::FindSkillGroupByRowName(const FName& RowName, FDBAZodiacElementFixedSkillGroupRow& OutSkillGroup) const
{
	if (!LoadedSkillGroupDataTable)
	{
		return false;
	}

	const FDBAZodiacElementFixedSkillGroupRow* FoundRow = LoadedSkillGroupDataTable->FindRow<FDBAZodiacElementFixedSkillGroupRow>(RowName, TEXT(""));
	if (FoundRow)
	{
		OutSkillGroup = *FoundRow;
		return true;
	}

	return false;
}