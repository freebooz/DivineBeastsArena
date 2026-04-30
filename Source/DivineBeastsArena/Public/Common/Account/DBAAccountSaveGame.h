// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Common/Account/DBAAccountTypes.h"
#include "Common/Account/DBAProfileTypes.h"
#include "DBAAccountSaveGame.generated.h"

/**
 * 账户存档
 *
 * 存储内容：
 * - 账户信息
 * - 角色列表
 * - 当前选中角色
 * - 会话 Token（支持记住登录）
 *
 * 持久化策略：
 * - 本地持久化：Guest 模式存储在本地
 * - 可选云同步：在线模式可以同步到云端（未来功能）
 * - 跨设备同步：同一账户可在不同设备登录（在线模式）
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAAccountSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UDBAAccountSaveGame();

	/**
	 * 存档结构版本
	 */
	UPROPERTY()
	int32 SchemaVersion;

	/**
	 * 数据版本
	 */
	UPROPERTY()
	int32 DataVersion;

	/**
	 * 账户信息
	 */
	UPROPERTY()
	FDBAAccountInfo AccountInfo;

	/**
	 * 角色列表
	 */
	UPROPERTY()
	TArray<FDBACharacterSummary> Characters;

	/**
	 * 当前选中角色 ID
	 */
	UPROPERTY()
	FDBACharacterId CurrentCharacterId;

	/**
	 * 会话 Token（支持记住登录）
	 */
	UPROPERTY()
	FString SessionToken;

	/**
	 * 最后保存时间（Unix 时间戳）
	 */
	UPROPERTY()
	int64 LastSaveTime;

	/**
	 * 数据校验和（用于检测存档篡改）
	 */
	UPROPERTY()
	int32 DataChecksum;

	/**
	 * 检查存档是否有效
	 */
	bool IsValid() const;

	/**
	 * 检查存档数据完整性（防篡改）
	 */
	bool ValidateDataIntegrity() const;

	/**
	 * 检查版本是否兼容
	 */
	bool IsVersionCompatible() const;

	/**
	 * 检查是否需要迁移
	 */
	bool NeedsMigration() const;

	/**
	 * 迁移到当前版本
	 *
	 * @return 是否迁移成功
	 */
	bool MigrateToCurrentVersion();

	/**
	 * 重置为默认数据
	 */
	void ResetToDefault();

	/**
	 * 添加角色
	 *
	 * @param Character 角色摘要
	 * @return 是否添加成功
	 */
	bool AddCharacter(const FDBACharacterSummary& Character);

	/**
	 * 移除角色
	 *
	 * @param CharacterId 角色 ID
	 * @return 是否移除成功
	 */
	bool RemoveCharacter(const FDBACharacterId& CharacterId);

	/**
	 * 查找角色
	 *
	 * @param CharacterId 角色 ID
	 * @return 角色摘要指针，未找到返回 nullptr
	 */
	FDBACharacterSummary* FindCharacter(const FDBACharacterId& CharacterId);

	/**
	 * 查找角色（常量版本）
	 */
	const FDBACharacterSummary* FindCharacter(const FDBACharacterId& CharacterId) const;

	/**
	 * 更新最后保存时间
	 */
	void UpdateLastSaveTime();

	/**
	 * 计算并更新校验和
	 */
	void UpdateChecksum();

protected:
	/**
	 * 计算数据的校验和
	 */
	int32 CalculateChecksum() const;
};

/**
 * Profile 存档
 *
 * 存储内容：
 * - 玩家设置
 * - 按键绑定
 * - UI 布局
 *
 * 持久化策略：
 * - 本地持久化：所有模式都存储在本地
 * - 可选云同步：在线模式可以同步到云端（未来功能）
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAProfileSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UDBAProfileSaveGame();

	/**
	 * 存档结构版本
	 */
	UPROPERTY()
	int32 SchemaVersion;

	/**
	 * 数据版本
	 */
	UPROPERTY()
	int32 DataVersion;

	/**
	 * 玩家 Profile
	 */
	UPROPERTY()
	FDBAPlayerProfile PlayerProfile;

	/**
	 * 最后保存时间（Unix 时间戳）
	 */
	UPROPERTY()
	int64 LastSaveTime;

	/**
	 * 数据校验和（用于检测存档篡改）
	 */
	UPROPERTY()
	int32 DataChecksum;

	/**
	 * 检查存档是否有效
	 */
	bool IsValid() const;

	/**
	 * 检查存档数据完整性（防篡改）
	 */
	bool ValidateDataIntegrity() const;

	/**
	 * 检查版本是否兼容
	 */
	bool IsVersionCompatible() const;

	/**
	 * 检查是否需要迁移
	 */
	bool NeedsMigration() const;

	/**
	 * 迁移到当前版本
	 *
	 * @return 是否迁移成功
	 */
	bool MigrateToCurrentVersion();

	/**
	 * 重置为默认数据
	 */
	void ResetToDefault();

	/**
	 * 更新最后保存时间
	 */
	void UpdateLastSaveTime();

	/**
	 * 计算并更新校验和
	 */
	void UpdateChecksum();

protected:
	/**
	 * 计算数据的校验和
	 */
	int32 CalculateChecksum() const;
};
