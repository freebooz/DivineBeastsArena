// Copyright FreeboozStudio. All Rights Reserved.

#include "Data/DBAStaticDataAsset.h"
#include "Engine/DataTable.h"
#include "Common/DBALogChannels.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UDBAStaticDataAsset::UDBAStaticDataAsset()
{
	// 默认构造函数
}

void UDBAStaticDataAsset::PreloadAllTables()
{
	// 同步加载所有静态数据表
	// 在 GameInstance 初始化时调用
	// Dedicated Server 必须加载这些表以获取游戏规则

	if (!ZodiacStaticTable.IsNull())
	{
		ZodiacStaticTable.LoadSynchronous();
	}

	if (!ElementDefinitionTable.IsNull())
	{
		ElementDefinitionTable.LoadSynchronous();
	}

	if (!FiveCampDisplayTable.IsNull())
	{
		FiveCampDisplayTable.LoadSynchronous();
	}

	if (!MapDefinitionTable.IsNull())
	{
		MapDefinitionTable.LoadSynchronous();
	}

	if (!ModeDefinitionTable.IsNull())
	{
		ModeDefinitionTable.LoadSynchronous();
	}
}

bool UDBAStaticDataAsset::ValidateAllTables() const
{
	// 验证所有静态数据表是否有效
	bool bAllValid = true;

	if (ZodiacStaticTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 生肖数据表引用为空"));
		bAllValid = false;
	}

	if (ElementDefinitionTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 元素数据表引用为空"));
		bAllValid = false;
	}

	if (FiveCampDisplayTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 阵营数据表引用为空"));
		bAllValid = false;
	}

	if (MapDefinitionTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 地图数据表引用为空"));
		bAllValid = false;
	}

	if (ModeDefinitionTable.IsNull())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAStaticDataAsset] 模式数据表引用为空"));
		bAllValid = false;
	}

	return bAllValid;
}

UDataTable* UDBAStaticDataAsset::GetZodiacStaticTable() const
{
	return ZodiacStaticTable.LoadSynchronous();
}

UDataTable* UDBAStaticDataAsset::GetElementDefinitionTable() const
{
	return ElementDefinitionTable.LoadSynchronous();
}

UDataTable* UDBAStaticDataAsset::GetFiveCampDisplayTable() const
{
	return FiveCampDisplayTable.LoadSynchronous();
}

UDataTable* UDBAStaticDataAsset::GetMapDefinitionTable() const
{
	return MapDefinitionTable.LoadSynchronous();
}

UDataTable* UDBAStaticDataAsset::GetModeDefinitionTable() const
{
	return ModeDefinitionTable.LoadSynchronous();
}

#if WITH_EDITOR
EDataValidationResult UDBAStaticDataAsset::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	// 验证生肖数据表
	if (ZodiacStaticTable.IsNull())
	{
		ValidationErrors.Add(FText::FromString(TEXT("生肖数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		UDataTable* Table = ZodiacStaticTable.LoadSynchronous();
		if (!Table)
		{
			ValidationErrors.Add(FText::FromString(TEXT("生肖数据表加载失败")));
			Result = EDataValidationResult::Invalid;
		}
		else if (Table->GetRowNames().Num() == 0)
		{
			ValidationErrors.Add(FText::FromString(TEXT("生肖数据表为空")));
			Result = EDataValidationResult::Invalid;
		}
	}

	// 验证自然元素之力数据表
	if (ElementDefinitionTable.IsNull())
	{
		ValidationErrors.Add(FText::FromString(TEXT("自然元素之力数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		UDataTable* Table = ElementDefinitionTable.LoadSynchronous();
		if (!Table)
		{
			ValidationErrors.Add(FText::FromString(TEXT("自然元素之力数据表加载失败")));
			Result = EDataValidationResult::Invalid;
		}
		else if (Table->GetRowNames().Num() != 5)
		{
			ValidationErrors.Add(FText::FromString(TEXT("自然元素之力数据表必须包含 5 个元素（Metal/Wood/Water/Fire/Earth）")));
			Result = EDataValidationResult::Invalid;
		}
	}

	// 验证五大阵营数据表
	if (FiveCampDisplayTable.IsNull())
	{
		ValidationErrors.Add(FText::FromString(TEXT("五大阵营数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		UDataTable* Table = FiveCampDisplayTable.LoadSynchronous();
		if (!Table)
		{
			ValidationErrors.Add(FText::FromString(TEXT("五大阵营数据表加载失败")));
			Result = EDataValidationResult::Invalid;
		}
		else if (Table->GetRowNames().Num() != 5)
		{
			ValidationErrors.Add(FText::FromString(TEXT("五大阵营数据表必须包含 5 个阵营（Byakko/Qinglong/Xuanwu/Zhuque/Kirin）")));
			Result = EDataValidationResult::Invalid;
		}
	}

	// 验证地图数据表
	if (MapDefinitionTable.IsNull())
	{
		ValidationErrors.Add(FText::FromString(TEXT("地图数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		UDataTable* Table = MapDefinitionTable.LoadSynchronous();
		if (!Table)
		{
			ValidationErrors.Add(FText::FromString(TEXT("地图数据表加载失败")));
			Result = EDataValidationResult::Invalid;
		}
		else if (Table->GetRowNames().Num() == 0)
		{
			ValidationErrors.Add(FText::FromString(TEXT("地图数据表为空")));
			Result = EDataValidationResult::Invalid;
		}
	}

	// 验证游戏模式数据表
	if (ModeDefinitionTable.IsNull())
	{
		ValidationErrors.Add(FText::FromString(TEXT("游戏模式数据表引用为空")));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		UDataTable* Table = ModeDefinitionTable.LoadSynchronous();
		if (!Table)
		{
			ValidationErrors.Add(FText::FromString(TEXT("游戏模式数据表加载失败")));
			Result = EDataValidationResult::Invalid;
		}
		else if (Table->GetRowNames().Num() == 0)
		{
			ValidationErrors.Add(FText::FromString(TEXT("游戏模式数据表为空")));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif
