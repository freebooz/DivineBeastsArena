// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "Engine/DataTable.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Data/Tables/DBASkillNameRow.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UDBAZodiacHeroDataAsset::UDBAZodiacHeroDataAsset()
{
}

const FPrimaryAssetType& UDBAZodiacHeroDataAsset::GetZodiacHeroPrimaryAssetType()
{
	static const FPrimaryAssetType PrimaryAssetType(TEXT("ZodiacHero"));
	return PrimaryAssetType;
}

FPrimaryAssetId UDBAZodiacHeroDataAsset::GetPrimaryAssetId() const
{
	// 旧聚合资产继续保持原有直接软引用用途，不进入单生肖 Registry。
	return bLegacyTableCatalog
		? Super::GetPrimaryAssetId()
		: FPrimaryAssetId(GetZodiacHeroPrimaryAssetType(), GetFName());
}

bool UDBAZodiacHeroDataAsset::GetZodiacHeroDisplayData(EDBAZodiac Zodiac, FDBAZodiacHeroDisplayRow& OutRow) const
{
	if (Zodiac == EDBAZodiac::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ZodiacHeroDisplayTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildZodiacRowName(Zodiac);
	return FindDataRow(DataTable, RowName, OutRow);
}

bool UDBAZodiacHeroDataAsset::GetZodiacHeroConfigData(EDBAZodiac Zodiac, FDBAZodiacHeroConfigRow& OutRow) const
{
	if (Zodiac == EDBAZodiac::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(ZodiacHeroConfigTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildZodiacRowName(Zodiac);
	return FindDataRow(DataTable, RowName, OutRow);
}

bool UDBAZodiacHeroDataAsset::GetFixedSkillGroupData(EDBAZodiac Zodiac, EDBAElement Element, FDBAZodiacElementFixedSkillGroupRow& OutRow) const
{
	if (Zodiac == EDBAZodiac::None || Element == EDBAElement::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(FixedSkillGroupTable);
	if (!DataTable)
	{
		return false;
	}

	TArray<FDBAZodiacElementFixedSkillGroupRow*> SkillGroups;
	DataTable->GetAllRows(TEXT("固定技能组查询"), SkillGroups);
	for (const FDBAZodiacElementFixedSkillGroupRow* SkillGroup : SkillGroups)
	{
		if (SkillGroup && SkillGroup->ZodiacType == Zodiac && SkillGroup->ElementType == Element)
		{
			OutRow = *SkillGroup;
			return true;
		}
	}

	return false;
}

bool UDBAZodiacHeroDataAsset::GetAbilitySetSummaryData(EDBAZodiac Zodiac, FDBAZodiacHeroAbilitySetSummaryRow& OutRow) const
{
	if (Zodiac == EDBAZodiac::None)
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(AbilitySetSummaryTable);
	if (!DataTable)
	{
		return false;
	}

	const FName RowName = BuildZodiacRowName(Zodiac);
	return FindDataRow(DataTable, RowName, OutRow);
}

bool UDBAZodiacHeroDataAsset::GetCharacterSelectionSummaryText(EDBAZodiac Zodiac, EDBAElement Element, FText& OutText) const
{
	OutText = FText::GetEmpty();
	if (Zodiac == EDBAZodiac::None || Element == EDBAElement::None)
	{
		return false;
	}

	UDataTable* BalanceTable = LoadDataTable(HeroBalanceTable);
	UDataTable* NamesTable = LoadDataTable(SkillNameTable);
	if (!BalanceTable || !NamesTable)
	{
		return false;
	}

	const EDBAZodiacType ZodiacEnumType = static_cast<EDBAZodiacType>(static_cast<int32>(Zodiac));
	FString HeroBalanceRowName = UEnum::GetValueAsString(ZodiacEnumType);
	HeroBalanceRowName.RemoveFromStart(TEXT("EDBAZodiacType::"));
	const FDBAHeroBalanceRow* BalanceRow = BalanceTable->FindRow<FDBAHeroBalanceRow>(FName(*HeroBalanceRowName), TEXT("角色选择展示"));
	if (!BalanceRow)
	{
		UE_LOG(LogDBAData, Warning, TEXT("[生肖角色数据] 未找到生肖 %d 的英雄属性说明。"), static_cast<int32>(Zodiac));
		return false;
	}

	TArray<FDBASkillNameRow*> SkillRows;
	NamesTable->GetAllRows(TEXT("角色选择展示"), SkillRows);
	SkillRows.RemoveAll([ZodiacEnumType](const FDBASkillNameRow* Row)
	{
		return !Row || Row->Zodiac != ZodiacEnumType;
	});
	SkillRows.Sort([](const FDBASkillNameRow& Left, const FDBASkillNameRow& Right)
	{
		return Left.SkillSlotIndex < Right.SkillSlotIndex;
	});

	TArray<FString> SkillNames;
	for (const FDBASkillNameRow* SkillRow : SkillRows)
	{
		SkillNames.Add(SkillRow->DisplayName.ToString());
	}

	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	const bool bHasSkillGroup = GetFixedSkillGroupData(Zodiac, Element, SkillGroup);
	const FString SkillGroupDescription = bHasSkillGroup ? SkillGroup.Description.ToString() : TEXT("元素技能组正在异步加载。");
	OutText = FText::FromString(FString::Printf(
		TEXT("%s\n定位：%s｜%s\n属性：生存%d 伤害%d 控制%d 机动%d 辅助%d 难度%d 团战%d\n优势：%s\n短板：%s\n技能：%s\n元素技能组：%s"),
		*BalanceRow->CharacterName,
		*BalanceRow->CoreRole,
		*BalanceRow->TeamRole,
		BalanceRow->Survivability,
		BalanceRow->Damage,
		BalanceRow->Control,
		BalanceRow->Mobility,
		BalanceRow->Support,
		BalanceRow->Difficulty,
		BalanceRow->TeamFightImpact,
		*BalanceRow->Advantages,
		*BalanceRow->Weaknesses,
		*FString::Join(SkillNames, TEXT("、")),
		*SkillGroupDescription));
	return true;
}

void UDBAZodiacHeroDataAsset::GetAllAvailableZodiacs(TArray<EDBAZodiac>& OutZodiacs) const
{
	OutZodiacs.Empty();

	UDataTable* DataTable = LoadDataTable(ZodiacHeroDisplayTable);
	if (!DataTable)
	{
		return;
	}

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FDBAZodiacHeroDisplayRow* Row = DataTable->FindRow<FDBAZodiacHeroDisplayRow>(RowName, TEXT(""));
		if (Row && Row->bIsAvailable && !Row->bIsInDevelopment)
		{
			OutZodiacs.Add(Row->Zodiac);
		}
	}
}

bool UDBAZodiacHeroDataAsset::IsZodiacAvailable(EDBAZodiac Zodiac) const
{
	FDBAZodiacHeroDisplayRow DisplayRow;
	if (!GetZodiacHeroDisplayData(Zodiac, DisplayRow))
	{
		return false;
	}

	return DisplayRow.bIsAvailable && !DisplayRow.bIsInDevelopment;
}

bool UDBAZodiacHeroDataAsset::IsSkillGroupAvailable(EDBAZodiac Zodiac, EDBAElement Element) const
{
	FDBAZodiacElementFixedSkillGroupRow SkillGroupRow;
	if (!GetFixedSkillGroupData(Zodiac, Element, SkillGroupRow))
	{
		return false;
	}

	return SkillGroupRow.bEnabled && !SkillGroupRow.bIsInDevelopment;
}

bool UDBAZodiacHeroDataAsset::ValidateDataIntegrity(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();
	bool bIsValid = true;

	if (!bLegacyTableCatalog)
	{
		if (ZodiacType == EDBAZodiac::None)
		{
			OutErrors.Add(TEXT("单生肖数据资产未配置 ZodiacType。"));
			bIsValid = false;
		}

		if (!DeprecatedLegacyClassificationId.IsNone())
		{
			OutErrors.Add(TEXT("单生肖数据资产包含已禁止的旧 Faction/分类引用。"));
			bIsValid = false;
		}

		return bIsValid;
	}

	// 验证数据表引用
	if (ZodiacHeroDisplayTable.IsNull())
	{
		OutErrors.Add(TEXT("ZodiacHeroDisplayTable 未配置"));
		bIsValid = false;
	}

	if (ZodiacHeroConfigTable.IsNull())
	{
		OutErrors.Add(TEXT("ZodiacHeroConfigTable 未配置"));
		bIsValid = false;
	}

	if (FixedSkillGroupTable.IsNull())
	{
		OutErrors.Add(TEXT("FixedSkillGroupTable 未配置"));
		bIsValid = false;
	}

	if (AbilitySetSummaryTable.IsNull())
	{
		OutErrors.Add(TEXT("AbilitySetSummaryTable 未配置"));
		bIsValid = false;
	}

	bIsValid &= ValidateLoadedTableRowCount(ZodiacHeroDisplayTable, TEXT("ZodiacHeroDisplayTable"), DBAConstants::ZodiacCount, OutErrors);
	bIsValid &= ValidateLoadedTableRowCount(FixedSkillGroupTable, TEXT("FixedSkillGroupTable"), DBAConstants::FixedSkillGroupRowCount, OutErrors);

	return bIsValid;
}

bool UDBAZodiacHeroDataAsset::ValidateData_Implementation(TArray<FString>& OutErrors) const
{
	// 委托给已有的数据完整性校验逻辑，统一通过 IDBAValidatableInterface 暴露运行时校验入口。
	const bool bIsValid = ValidateDataIntegrity(OutErrors);
	if (!bIsValid)
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAZodiacHeroDataAsset] 运行时数据校验失败，共 %d 项错误"), OutErrors.Num());
	}
	return bIsValid;
}

#if WITH_EDITOR
EDataValidationResult UDBAZodiacHeroDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (bLegacyTableCatalog)
	{
		Context.AddWarning(FText::FromString(TEXT("该资产仍为旧 DataTable 聚合模式；请仅将其作为兼容来源，并迁移为十二个单生肖资产。")));
		return Result;
	}

	if (ZodiacType == EDBAZodiac::None)
	{
		Context.AddError(FText::FromString(TEXT("单生肖数据资产必须配置 ZodiacType。")));
		Result = EDataValidationResult::Invalid;
	}

	if (!DeprecatedLegacyClassificationId.IsNone())
	{
		Context.AddError(FText::FromString(TEXT("单生肖数据资产不得包含旧 Faction 或旧分类引用。")));
		Result = EDataValidationResult::Invalid;
	}

	auto AddMissingResourceWarning = [&Context](bool bMissing, const TCHAR* Label)
	{
		if (bMissing)
		{
			Context.AddWarning(FText::FromString(FString::Printf(TEXT("单生肖数据资产缺少可选表现资源：%s。"), Label)));
		}
	};

	AddMissingResourceWarning(Portrait.IsNull(), TEXT("Portrait"));
	AddMissingResourceWarning(PreviewActorClass.IsNull(), TEXT("PreviewActorClass"));
	AddMissingResourceWarning(GameplayCharacterClass.IsNull(), TEXT("GameplayCharacterClass"));
	AddMissingResourceWarning(BodyMesh.IsNull(), TEXT("BodyMesh"));
	AddMissingResourceWarning(AnimationBlueprintClass.IsNull(), TEXT("AnimationBlueprintClass"));
	AddMissingResourceWarning(IdleAnimation.IsNull(), TEXT("IdleAnimation"));

	return Result;
}
#endif

void UDBAZodiacHeroDataAsset::PreloadAllDataTablesAsync() const
{
	RequestDataTableAsync(ZodiacHeroDisplayTable);
	RequestDataTableAsync(ZodiacHeroConfigTable);
	RequestDataTableAsync(FixedSkillGroupTable);
	RequestDataTableAsync(AbilitySetSummaryTable);
	RequestDataTableAsync(HeroBalanceTable);
	RequestDataTableAsync(SkillNameTable);
}

UDataTable* UDBAZodiacHeroDataAsset::LoadDataTable(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
{
	if (DataTablePtr.IsNull())
	{
		return nullptr;
	}

	if (UDataTable* LoadedTable = DataTablePtr.Get())
	{
		return LoadedTable;
	}

	RequestDataTableAsync(DataTablePtr);
	return nullptr;
}

void UDBAZodiacHeroDataAsset::RequestDataTableAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr) const
{
	if (DataTablePtr.IsNull() || DataTablePtr.Get())
	{
		return;
	}

	const FSoftObjectPath TablePath = DataTablePtr.ToSoftObjectPath();
	if (RequestedDataTableLoads.Contains(TablePath))
	{
		return;
	}

	RequestedDataTableLoads.Add(TablePath);
	TWeakObjectPtr<UDBAZodiacHeroDataAsset> WeakThis(const_cast<UDBAZodiacHeroDataAsset*>(this));
	DBAAsyncAssetLoader::RequestAsyncAsset<UDataTable>(const_cast<UDBAZodiacHeroDataAsset*>(this), DataTablePtr, [WeakThis, TablePath](UDataTable* LoadedTable)
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		if (!LoadedTable)
		{
			UE_LOG(LogDBAData, Warning, TEXT("[UDBAZodiacHeroDataAsset] 生肖英雄数据表异步加载失败：%s"), *TablePath.ToString());
			return;
		}

		UE_LOG(LogDBAData, Log, TEXT("[UDBAZodiacHeroDataAsset] 生肖英雄数据表异步加载完成：%s"), *TablePath.ToString());

		// 广播 DataTable 加载完成事件，供 UI 等订阅方驱动更新。
		WeakThis->OnDataTableLoaded.Broadcast(LoadedTable, TablePath);
	});

	UE_LOG(LogDBAData, Verbose, TEXT("[UDBAZodiacHeroDataAsset] 已发起生肖英雄数据表异步加载：%s"), *TablePath.ToString());
}

bool UDBAZodiacHeroDataAsset::ValidateLoadedTableRowCount(
	const TSoftObjectPtr<UDataTable>& DataTablePtr,
	const TCHAR* TableLabel,
	int32 ExpectedRowCount,
	TArray<FString>& OutErrors) const
{
	if (DataTablePtr.IsNull())
	{
		return false;
	}

	UDataTable* DataTable = LoadDataTable(DataTablePtr);
	if (!DataTable)
	{
		OutErrors.Add(FString::Printf(TEXT("%s 尚未加载完成，已发起异步加载；请等待加载完成后重试校验。"), TableLabel));
		return false;
	}

	const int32 RowCount = DataTable->GetRowNames().Num();
	if (RowCount != ExpectedRowCount)
	{
		OutErrors.Add(FString::Printf(TEXT("%s 应包含 %d 行数据，实际 %d 行"), TableLabel, ExpectedRowCount, RowCount));
		return false;
	}

	return true;
}

FName UDBAZodiacHeroDataAsset::BuildZodiacRowName(EDBAZodiac Zodiac) const
{
	const FString ZodiacString = UEnum::GetValueAsString(Zodiac);
	// 移除 "EDBAZodiac::" 前缀
	FString CleanZodiacString = ZodiacString;
	CleanZodiacString.RemoveFromStart(TEXT("EDBAZodiac::"));

	return FName(*FString::Printf(TEXT("Zodiac_%s"), *CleanZodiacString));
}
