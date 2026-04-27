// Copyright FreeboozStudio. All Rights Reserved.

#include "Common/Account/DBAAccountSaveGame.h"
#include "Common/Account/DBASaveGameVersions.h"
#include "Common/DBALogChannels.h"
#include "Misc/DateTime.h"

UDBAAccountSaveGame::UDBAAccountSaveGame()
	: SchemaVersion(DBASaveGameVersions::SCHEMA_VERSION)
	, DataVersion(DBASaveGameVersions::DATA_VERSION)
	, LastSaveTime(0)
{
}

bool UDBAAccountSaveGame::IsValid() const
{
	return AccountInfo.IsValid();
}

bool UDBAAccountSaveGame::IsVersionCompatible() const
{
	return DBASaveGameVersions::IsVersionCompatible(SchemaVersion, DataVersion);
}

bool UDBAAccountSaveGame::NeedsMigration() const
{
	return DBASaveGameVersions::NeedsMigration(SchemaVersion, DataVersion);
}

bool UDBAAccountSaveGame::MigrateToCurrentVersion()
{
	if (!IsVersionCompatible())
	{
		UE_LOG(LogDBACore, Error, TEXT("UDBAAccountSaveGame::MigrateToCurrentVersion - 版本不兼容，无法迁移：SchemaVersion=%d, DataVersion=%d"),
			SchemaVersion, DataVersion);
		return false;
	}

	if (!NeedsMigration())
	{
		return true;
	}

	UE_LOG(LogDBACore, Warning, TEXT("UDBAAccountSaveGame::MigrateToCurrentVersion - 开始迁移：从 SchemaVersion=%d, DataVersion=%d 到 SchemaVersion=%d, DataVersion=%d"),
		SchemaVersion, DataVersion,
		DBASaveGameVersions::SCHEMA_VERSION, DBASaveGameVersions::DATA_VERSION);

	// 当前版本无需迁移逻辑
	// 未来版本在此添加迁移代码

	// 更新版本号
	SchemaVersion = DBASaveGameVersions::SCHEMA_VERSION;
	DataVersion = DBASaveGameVersions::DATA_VERSION;

	UE_LOG(LogDBACore, Log, TEXT("UDBAAccountSaveGame::MigrateToCurrentVersion - 迁移成功"));
	return true;
}

void UDBAAccountSaveGame::ResetToDefault()
{
	SchemaVersion = DBASaveGameVersions::SCHEMA_VERSION;
	DataVersion = DBASaveGameVersions::DATA_VERSION;
	AccountInfo = FDBAAccountInfo();
	Characters.Empty();
	CurrentCharacterId = FDBACharacterId();
	LastSaveTime = 0;

	UE_LOG(LogDBACore, Log, TEXT("UDBAAccountSaveGame::ResetToDefault - 重置为默认数据"));
}

bool UDBAAccountSaveGame::AddCharacter(const FDBACharacterSummary& Character)
{
	if (!Character.IsValid())
	{
		UE_LOG(LogDBACore, Error, TEXT("UDBAAccountSaveGame::AddCharacter - 角色无效"));
		return false;
	}

	// 检查是否已存在
	if (FindCharacter(Character.CharacterId))
	{
		UE_LOG(LogDBACore, Warning, TEXT("UDBAAccountSaveGame::AddCharacter - 角色已存在：%s"), *Character.CharacterId.ToString());
		return false;
	}

	Characters.Add(Character);
	UE_LOG(LogDBACore, Log, TEXT("UDBAAccountSaveGame::AddCharacter - 添加角色成功：%s"), *Character.CharacterName);
	return true;
}

bool UDBAAccountSaveGame::RemoveCharacter(const FDBACharacterId& CharacterId)
{
	const int32 Index = Characters.IndexOfByPredicate([&CharacterId](const FDBACharacterSummary& Character)
	{
		return Character.CharacterId == CharacterId;
	});

	if (Index == INDEX_NONE)
	{
		UE_LOG(LogDBACore, Warning, TEXT("UDBAAccountSaveGame::RemoveCharacter - 角色不存在：%s"), *CharacterId.ToString());
		return false;
	}

	Characters.RemoveAt(Index);

	// 如果删除的是当前选中角色，清空选中
	if (CurrentCharacterId == CharacterId)
	{
		CurrentCharacterId = FDBACharacterId();
	}

	UE_LOG(LogDBACore, Log, TEXT("UDBAAccountSaveGame::RemoveCharacter - 移除角色成功：%s"), *CharacterId.ToString());
	return true;
}

FDBACharacterSummary* UDBAAccountSaveGame::FindCharacter(const FDBACharacterId& CharacterId)
{
	return Characters.FindByPredicate([&CharacterId](const FDBACharacterSummary& Character)
	{
		return Character.CharacterId == CharacterId;
	});
}

const FDBACharacterSummary* UDBAAccountSaveGame::FindCharacter(const FDBACharacterId& CharacterId) const
{
	return Characters.FindByPredicate([&CharacterId](const FDBACharacterSummary& Character)
	{
		return Character.CharacterId == CharacterId;
	});
}

void UDBAAccountSaveGame::UpdateLastSaveTime()
{
	LastSaveTime = FDateTime::UtcNow().ToUnixTimestamp();
}

// ============================================================================
// UDBAProfileSaveGame
// ============================================================================

UDBAProfileSaveGame::UDBAProfileSaveGame()
	: SchemaVersion(DBASaveGameVersions::SCHEMA_VERSION)
	, DataVersion(DBASaveGameVersions::DATA_VERSION)
	, LastSaveTime(0)
{
}

bool UDBAProfileSaveGame::IsValid() const
{
	return true;
}

bool UDBAProfileSaveGame::IsVersionCompatible() const
{
	return DBASaveGameVersions::IsVersionCompatible(SchemaVersion, DataVersion);
}

bool UDBAProfileSaveGame::NeedsMigration() const
{
	return DBASaveGameVersions::NeedsMigration(SchemaVersion, DataVersion);
}

bool UDBAProfileSaveGame::MigrateToCurrentVersion()
{
	if (!IsVersionCompatible())
	{
		UE_LOG(LogDBACore, Error, TEXT("UDBAProfileSaveGame::MigrateToCurrentVersion - 版本不兼容，无法迁移：SchemaVersion=%d, DataVersion=%d"),
			SchemaVersion, DataVersion);
		return false;
	}

	if (!NeedsMigration())
	{
		return true;
	}

	UE_LOG(LogDBACore, Warning, TEXT("UDBAProfileSaveGame::MigrateToCurrentVersion - 开始迁移：从 SchemaVersion=%d, DataVersion=%d 到 SchemaVersion=%d, DataVersion=%d"),
		SchemaVersion, DataVersion,
		DBASaveGameVersions::SCHEMA_VERSION, DBASaveGameVersions::DATA_VERSION);

	// 当前版本无需迁移逻辑
	// 未来版本在此添加迁移代码

	// 更新版本号
	SchemaVersion = DBASaveGameVersions::SCHEMA_VERSION;
	DataVersion = DBASaveGameVersions::DATA_VERSION;

	UE_LOG(LogDBACore, Log, TEXT("UDBAProfileSaveGame::MigrateToCurrentVersion - 迁移成功"));
	return true;
}

void UDBAProfileSaveGame::ResetToDefault()
{
	SchemaVersion = DBASaveGameVersions::SCHEMA_VERSION;
	DataVersion = DBASaveGameVersions::DATA_VERSION;
	PlayerProfile = FDBAPlayerProfile();
	LastSaveTime = 0;

	UE_LOG(LogDBACore, Log, TEXT("UDBAProfileSaveGame::ResetToDefault - 重置为默认数据"));
}

void UDBAProfileSaveGame::UpdateLastSaveTime()
{
	LastSaveTime = FDateTime::UtcNow().ToUnixTimestamp();
}
