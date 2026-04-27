// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Account/DBAAccountServiceBase.h"
#include "Common/Account/DBAAccountTypes.h"
#include "DBAMockAccountService.generated.h"

/**
 * Mock 账户服务
 * 用于开发和离线测试
 *
 * 功能：
 * - Guest 登录
 * - 自动登录（使用本地存档）
 * - 角色创建 / 删除 / 选择
 * - 角色数据本地管理
 *
 * 限制：
 * - 不支持 Email / ThirdParty 登录
 * - 所有数据存储在本地
 * - 不连接任何外部服务
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAMockAccountService : public UDBAAccountServiceBase
{
	GENERATED_BODY()

public:
	UDBAMockAccountService();

	// UDBAAccountServiceBase interface
	virtual void Login(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete) override;
	virtual void Register(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete) override;
	virtual void GuestLogin(FDBAOnLoginComplete OnComplete) override;
	virtual void AutoLogin(FDBAOnLoginComplete OnComplete) override;
	virtual void Logout(FDBAOnLogoutComplete OnComplete) override;
	virtual void GetCharacterList(FDBAOnCharacterListLoaded OnComplete) override;
	virtual void GetCharacterProfile(const FDBACharacterId& CharacterId, FDBAOnCharacterProfileLoaded OnComplete) override;
	virtual void CreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete) override;
	virtual void DeleteCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterDeleted OnComplete) override;
	virtual void SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete) override;
	// End of UDBAAccountServiceBase interface

protected:
	/**
	 * 角色详细信息缓存
	 * 用于快速访问角色数据
	 */
	UPROPERTY()
	TMap<FDBACharacterId, FDBACharacterProfile> CharacterProfileCache;

	/**
	 * 执行 Guest 登录逻辑
	 *
	 * @param bCreateNew 是否创建新账户
	 * @param OnComplete 完成回调
	 */
	void PerformGuestLogin(bool bCreateNew, FDBAOnLoginComplete OnComplete);

	/**
	 * 创建新 Guest 账户
	 *
	 * @return 账户信息
	 */
	FDBAAccountInfo CreateNewGuestAccount();

	/**
	 * 保存当前账户
	 *
	 * @return 是否保存成功
	 */
	bool SaveCurrentAccount();

	/**
	 * 加载角色详细信息
	 *
	 * @param CharacterId 角色 ID
	 * @return 角色详细信息，未找到返回空
	 */
	FDBACharacterProfile LoadCharacterProfileFromCache(const FDBACharacterId& CharacterId);

	/**
	 * 保存角色详细信息到缓存
	 *
	 * @param Profile 角色详细信息
	 */
	void SaveCharacterProfileToCache(const FDBACharacterProfile& Profile);
};
