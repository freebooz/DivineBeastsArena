// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：技能名称 Subsystem 实现，承载异步加载与按生肖/槽位查询逻辑。
- 阅读重点：InitializeWithSettings 异步加载入口、FindRowByZodiac 查询核心、BuildSkillNameRowName 行名构造。
- 修改提示：新增查询接口时确保未加载时返回默认值并输出中文警告，不阻塞 GameThread。
*/

#include "GameDBA/Data/Tables/Runtime/DBASkillNameSubsystem.h"
#include "GameDBA/Data/Tables/DBASkillNameRow.h"
#include "GameDBA/Data/Tables/Settings/DBASkillNameDeveloperSettings.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"

#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UDBASkillNameSubsystem::OnSubsystemInitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Initialize，此处仅执行派生类初始化
	InitializeWithSettings();
}

void UDBASkillNameSubsystem::OnSubsystemDeinitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Deinitialize，此处仅清理派生类状态
	if (DataTableStreamableHandle.IsValid())
	{
		DataTableStreamableHandle->CancelHandle();
		DataTableStreamableHandle.Reset();
	}
}

void UDBASkillNameSubsystem::InitializeWithSettings()
{
	const UDBASkillNameDeveloperSettings* Settings = GetDefault<UDBASkillNameDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBASkillNameSubsystem] 未找到 UDBASkillNameDeveloperSettings，技能名称 DataTable 不会加载"));
		return;
	}

	if (Settings->DefaultSkillNameTable.IsNull())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBASkillNameSubsystem] DefaultSkillNameTable 未配置，请在 Project Settings > DBA 技能名称配置 中指定"));
		return;
	}

	// 已加载则直接缓存
	if (UDataTable* LoadedTable = Settings->DefaultSkillNameTable.Get())
	{
		CachedDataTable = LoadedTable;
		UE_LOG(LogDBAData, Log, TEXT("[UDBASkillNameSubsystem] 技能名称 DataTable 已加载：%s"), *Settings->DefaultSkillNameTable.GetAssetName());
		return;
	}

	// 发起异步加载
	const FSoftObjectPath TablePath = Settings->DefaultSkillNameTable.ToSoftObjectPath();
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

	DataTableStreamableHandle = StreamableManager.RequestAsyncLoad(TablePath, FStreamableDelegate::CreateWeakLambda(this, [this, TablePath]()
	{
		HandleDataTableLoaded();
	}));

	if (DataTableStreamableHandle.IsValid())
	{
		UE_LOG(LogDBAData, Log, TEXT("[UDBASkillNameSubsystem] 已发起技能名称 DataTable 异步加载：%s"), *TablePath.ToString());
	}
	else
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBASkillNameSubsystem] 发起技能名称 DataTable 异步加载失败（句柄无效）：%s"), *TablePath.ToString());
	}
}

void UDBASkillNameSubsystem::HandleDataTableLoaded()
{
	if (!DataTableStreamableHandle.IsValid())
	{
		return;
	}

	UObject* LoadedObject = DataTableStreamableHandle->GetLoadedAsset();
	DataTableStreamableHandle.Reset();

	UDataTable* LoadedTable = Cast<UDataTable>(LoadedObject);
	if (!LoadedTable)
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBASkillNameSubsystem] 技能名称 DataTable 加载完成但类型不是 UDataTable"));
		return;
	}

	CachedDataTable = LoadedTable;

	const int32 RowCount = LoadedTable->GetRowNames().Num();
	const int32 ExpectedRowCount = DBAConstants::ZodiacCount * DBAConstants::CoreCombatInputCount;
	if (RowCount != ExpectedRowCount)
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBASkillNameSubsystem] 技能名称 DataTable 行数异常：当前 %d 行，期望 %d 行（12 生肖 × 6 槽位）"), RowCount, ExpectedRowCount);
	}
	else
	{
		UE_LOG(LogDBAData, Log, TEXT("[UDBASkillNameSubsystem] 技能名称 DataTable 加载完成，共 %d 行"), RowCount);
	}
}

bool UDBASkillNameSubsystem::IsDataTableReady() const
{
	return CachedDataTable.IsValid();
}

bool UDBASkillNameSubsystem::GetSkillName(EDBAZodiacType Zodiac, int32 SkillSlotIndex, FText& OutName) const
{
	const FDBASkillNameRow* Row = FindRowByZodiac(Zodiac, SkillSlotIndex);
	if (!Row)
	{
		return false;
	}

	OutName = Row->DisplayName;
	return true;
}

bool UDBASkillNameSubsystem::GetAllSkillNamesByZodiac(EDBAZodiacType Zodiac, TArray<FText>& OutNames) const
{
	OutNames.Empty(DBAConstants::CoreCombatInputCount);

	if (!IsDataTableReady())
	{
		if (!bHasLoggedTableNotReady)
		{
			UE_LOG(LogDBAData, Warning, TEXT("[UDBASkillNameSubsystem] 技能名称 DataTable 尚未加载完成，GetAllSkillNamesByZodiac 返回空数组"));
			bHasLoggedTableNotReady = true;
		}
		return false;
	}

	const UDataTable* DataTable = CachedDataTable.Get();
	if (!DataTable)
	{
		return false;
	}

	// 遍历槽位索引 0~5，按顺序填充
	for (int32 SlotIndex = 0; SlotIndex < DBAConstants::CoreCombatInputCount; ++SlotIndex)
	{
		const FName RowName = BuildSkillNameRowName(Zodiac, SlotIndex);
		const FDBASkillNameRow* Row = DataTable->FindRow<FDBASkillNameRow>(RowName, TEXT("GetAllSkillNamesByZodiac"));
		if (Row)
		{
			OutNames.Add(Row->DisplayName);
		}
		else
		{
			OutNames.Add(FText::GetEmpty());
			UE_LOG(LogDBAData, Warning, TEXT("[UDBASkillNameSubsystem] 技能名称行不存在：Zodiac=%d, Slot=%d"), static_cast<int32>(Zodiac), SlotIndex);
		}
	}

	return OutNames.Num() == DBAConstants::CoreCombatInputCount;
}

const FDBASkillNameRow* UDBASkillNameSubsystem::FindRowByZodiac(EDBAZodiacType Zodiac, int32 SkillSlotIndex) const
{
	if (Zodiac == EDBAZodiacType::None || SkillSlotIndex < 0 || SkillSlotIndex >= DBAConstants::CoreCombatInputCount)
	{
		return nullptr;
	}

	if (!IsDataTableReady())
	{
		if (!bHasLoggedTableNotReady)
		{
			UE_LOG(LogDBAData, Warning, TEXT("[UDBASkillNameSubsystem] 技能名称 DataTable 尚未加载完成，GetSkillName 返回空"));
			bHasLoggedTableNotReady = true;
		}
		return nullptr;
	}

	const UDataTable* DataTable = CachedDataTable.Get();
	if (!DataTable)
	{
		return nullptr;
	}

	const FName RowName = BuildSkillNameRowName(Zodiac, SkillSlotIndex);
	return DataTable->FindRow<FDBASkillNameRow>(RowName, TEXT("FindRowByZodiac"));
}

// === BuildSkillNameRowName / GetSkillSlotNameByIndex 工具函数实现 ===

const TCHAR* GetSkillSlotNameByIndex(int32 SkillSlotIndex)
{
	switch (SkillSlotIndex)
	{
	case 0:  return TEXT("Passive");
	case 1:  return TEXT("Skill01");
	case 2:  return TEXT("Skill02");
	case 3:  return TEXT("Skill03");
	case 4:  return TEXT("Skill04");
	case 5:  return TEXT("Ultimate");
	default: return TEXT("Unknown");
	}
}

FName BuildSkillNameRowName(EDBAZodiacType Zodiac, int32 SkillSlotIndex)
{
	if (Zodiac == EDBAZodiacType::None)
	{
		return NAME_None;
	}

	const FString ZodiacString = UEnum::GetValueAsString(Zodiac);
	FString CleanZodiacString = ZodiacString;
	CleanZodiacString.RemoveFromStart(TEXT("EDBAZodiacType::"));

	const TCHAR* SlotName = GetSkillSlotNameByIndex(SkillSlotIndex);

	return FName(*FString::Printf(TEXT("%s_%s"), *CleanZodiacString, SlotName));
}
