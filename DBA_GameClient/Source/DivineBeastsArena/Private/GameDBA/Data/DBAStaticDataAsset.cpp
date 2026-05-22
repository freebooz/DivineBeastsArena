// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Data/DBAStaticDataAsset.h"
#include "Engine/DataTable.h"
#include "GameDBA/Core/DBALogChannels.h"

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
