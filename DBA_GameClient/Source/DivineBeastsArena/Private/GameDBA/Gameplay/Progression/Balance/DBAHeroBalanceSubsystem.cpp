// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：英雄数值平衡数据 Subsystem 实现，异步加载 DataTable 并提供查询。
- 阅读重点：InitializeWithSettings 发起异步加载，FindRowByZodiac 按生肖查找行，未就绪时返回默认值。
- 修改提示：保持非阻塞语义，未就绪时输出中文警告日志并返回默认值。
*/

#include "GameDBA/Gameplay/Progression/Balance/DBAHeroBalanceSubsystem.h"

#include "GameDBA/Gameplay/Progression/Balance/DBAHeroBalanceDeveloperSettings.h"
#include "GameDBA/Gameplay/Progression/Balance/DBAHeroBalanceRow.h"
#include "Engine/DataTable.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "HAL/PlatformFileManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDBAHeroBalance, Log, All);

namespace
{
	FName GetHeroBalanceRowName(EDBAZodiacType ZodiacType)
	{
		if (ZodiacType == EDBAZodiacType::None)
		{
			return NAME_None;
		}

		const UEnum* ZodiacEnum = StaticEnum<EDBAZodiacType>();
		const FString ZodiacName = ZodiacEnum
			? ZodiacEnum->GetNameStringByValue(static_cast<int64>(ZodiacType))
			: FString();
		return ZodiacName.IsEmpty() || ZodiacName.Equals(TEXT("MAX"), ESearchCase::IgnoreCase)
			? NAME_None
			: FName(*ZodiacName);
	}
}

void UDBAHeroBalanceSubsystem::OnSubsystemInitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Initialize，此处仅执行派生类初始化
	InitializeWithSettings();
}

void UDBAHeroBalanceSubsystem::OnSubsystemDeinitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Deinitialize，派生类无额外清理
}

void UDBAHeroBalanceSubsystem::InitializeWithSettings()
{
	const UDBAHeroBalanceDeveloperSettings* Settings = GetDefault<UDBAHeroBalanceDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogDBAHeroBalance, Warning, TEXT("[HeroBalanceSubsystem] 未找到 UDBAHeroBalanceDeveloperSettings，英雄平衡数据将不可用。"));
		return;
	}

	HeroBalanceTablePtr = Settings->DefaultHeroBalanceTable;

	if (!HeroBalanceTablePtr.ToSoftObjectPath().IsValid())
	{
		UE_LOG(LogDBAHeroBalance, Warning, TEXT("[HeroBalanceSubsystem] DefaultHeroBalanceTable 软引用路径无效，请在 DefaultGame.ini 中配置 [/Script/DivineBeastsArena.DBAHeroBalanceDeveloperSettings] DefaultHeroBalanceTable。"));
		return;
	}

	// 若软引用已加载，直接缓存；否则发起异步加载。
	if (HeroBalanceTablePtr.IsValid())
	{
		HandleDataTableLoaded();
	}
	else if (!bHasRequestedAsyncLoad)
	{
		bHasRequestedAsyncLoad = true;
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		Streamable.RequestAsyncLoad(
			HeroBalanceTablePtr.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &UDBAHeroBalanceSubsystem::HandleDataTableLoaded));

		UE_LOG(LogDBAHeroBalance, Log, TEXT("[HeroBalanceSubsystem] 已发起英雄平衡 DataTable 异步加载请求：%s"),
			*HeroBalanceTablePtr.ToSoftObjectPath().ToString());
	}
}

void UDBAHeroBalanceSubsystem::HandleDataTableLoaded()
{
	UDataTable* LoadedTable = HeroBalanceTablePtr.Get();
	if (!LoadedTable)
	{
		UE_LOG(LogDBAHeroBalance, Warning, TEXT("[HeroBalanceSubsystem] 英雄平衡 DataTable 异步加载完成但解析为空，请检查软引用路径：%s"),
			*HeroBalanceTablePtr.ToSoftObjectPath().ToString());
		return;
	}

	CachedDataTable = LoadedTable;
	bHasRequestedAsyncLoad = false;

	// 统计行数用于验证。
	int32 RowCount = LoadedTable->GetRowMap().Num();
	UE_LOG(LogDBAHeroBalance, Log, TEXT("[HeroBalanceSubsystem] 英雄平衡 DataTable 加载完成并已缓存，行数：%d（预期 12）"), RowCount);

	if (RowCount != 12)
	{
		UE_LOG(LogDBAHeroBalance, Warning, TEXT("[HeroBalanceSubsystem] 英雄平衡 DataTable 行数非 12，可能存在配置缺失或冗余。"));
	}
}

FDBAHeroBalanceData UDBAHeroBalanceSubsystem::GetHeroBalanceData(EDBAZodiacType ZodiacType) const
{
	const FDBAHeroBalanceRow* Row = FindRowByZodiac(ZodiacType);
	if (Row)
	{
		return Row->ToHeroBalanceData();
	}

	// 未就绪或未找到，输出中文警告日志（仅首次）。
	if (!bHasLoggedNotReady)
	{
		if (!CachedDataTable.IsValid())
		{
			UE_LOG(LogDBAHeroBalance, Warning, TEXT("[HeroBalanceSubsystem] 查询英雄平衡数据时 DataTable 尚未加载完成，返回默认值。生肖类型：%d"),
				static_cast<int32>(ZodiacType));
		}
		else
		{
			UE_LOG(LogDBAHeroBalance, Warning, TEXT("[HeroBalanceSubsystem] DataTable 已加载但未找到生肖类型 %d 对应的行，返回默认值。"),
				static_cast<int32>(ZodiacType));
		}
		bHasLoggedNotReady = true;
	}

	return FDBAHeroBalanceData{};
}

bool UDBAHeroBalanceSubsystem::GetAllHeroBalanceData(TArray<FDBAHeroBalanceData>& OutData) const
{
	OutData.Reset();

	UDataTable* Table = CachedDataTable.Get();
	if (!Table)
	{
		if (!bHasLoggedNotReady)
		{
			UE_LOG(LogDBAHeroBalance, Warning, TEXT("[HeroBalanceSubsystem] 查询全部英雄平衡数据时 DataTable 尚未加载完成，返回空数组。"));
			bHasLoggedNotReady = true;
		}
		return false;
	}

	// 重置警告标记，便于下次未就绪时再次提示。
	bHasLoggedNotReady = false;

	// 遍历所有行，转换为业务结构体。
	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FDBAHeroBalanceRow* Row = reinterpret_cast<const FDBAHeroBalanceRow*>(Pair.Value);
		if (Row)
		{
			OutData.Add(Row->ToHeroBalanceData());
		}
	}

	return OutData.Num() > 0;
}

bool UDBAHeroBalanceSubsystem::IsDataTableReady() const
{
	return CachedDataTable.IsValid();
}

const FDBAHeroBalanceRow* UDBAHeroBalanceSubsystem::FindRowByZodiac(EDBAZodiacType ZodiacType) const
{
	UDataTable* Table = CachedDataTable.Get();
	if (!Table)
	{
		return nullptr;
	}

	const FName RowName = GetHeroBalanceRowName(ZodiacType);
	if (RowName.IsNone())
	{
		return nullptr;
	}
	return Table->FindRow<FDBAHeroBalanceRow>(RowName, TEXT("HeroBalanceSubsystem"));
}
