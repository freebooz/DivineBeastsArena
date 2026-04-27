// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "Common/Account/DBAAccountTypes.h"
#include "Common/Account/DBAProfileTypes.h"
#include "DBAAccountServiceBase.generated.h"

class UDBAAccountSaveGame;
class UDBAProfileSaveGame;

/**
 * 账户服务基类
 *
 * 功能：
 * - 账户登录 / 注册 / 登出
 * - 角色创建 / 删除 / 选择
 * - 角色数据加载 / 保存
 * - Profile 加载 / 保存
 * - 存档管理
 *
 * 实现策略：
 * - 基类提供存档管理和通用逻辑
 * - 派生类实现具体的登录 / 注册逻辑
 * - MockAccountService：本地 Guest 模式，用于开发和离线测试
 * - OnlineAccountService：在线模式，连接外部 Auth 服务（未来实现）
 *
 * 外部服务边界：
 * - 外部 Auth 服务不可用时，自动降级到 MockAccountService
 * - 所有外部请求必须异步、可超时、可丢弃、可熔断、可禁用
 * - 外部服务回调必须检查 UObject 生命周期
 *
 * Dedicated Server：
 * - Dedicated Server 不调用登录接口
 * - Dedicated Server 不加载 Profile
 * - Dedicated Server 只验证客户端提供的 SessionToken
 *
 * 线程安全：
 * - 所有接口必须在 GameThread 调用
 * - 异步回调必须切回 GameThread
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAAccountServiceBase : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UDBAAccountServiceBase();

	// UDBAGameInstanceSubsystemBase interface
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;
	virtual bool IsSupportedInCurrentEnvironment() const override;
	// End of UDBAGameInstanceSubsystemBase interface

	/**
	 * 登录
	 *
	 * @param Request 登录请求
	 * @param OnComplete 完成回调
	 */
	virtual void Login(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete);

	/**
	 * 注册
	 *
	 * @param Request 登录请求（包含注册信息）
	 * @param OnComplete 完成回调
	 */
	virtual void Register(const FDBALoginRequest& Request, FDBAOnLoginComplete OnComplete);

	/**
	 * 游客登录
	 *
	 * @param OnComplete 完成回调
	 */
	virtual void GuestLogin(FDBAOnLoginComplete OnComplete);

	/**
	 * 自动登录
	 * 尝试使用本地存档自动登录
	 *
	 * @param OnComplete 完成回调
	 */
	virtual void AutoLogin(FDBAOnLoginComplete OnComplete);

	/**
	 * 登出
	 *
	 * @param OnComplete 完成回调
	 */
	virtual void Logout(FDBAOnLogoutComplete OnComplete);

	/**
	 * 获取角色列表
	 *
	 * @param OnComplete 完成回调
	 */
	virtual void GetCharacterList(FDBAOnCharacterListLoaded OnComplete);

	/**
	 * 获取角色详细信息
	 *
	 * @param CharacterId 角色 ID
	 * @param OnComplete 完成回调
	 */
	virtual void GetCharacterProfile(const FDBACharacterId& CharacterId, FDBAOnCharacterProfileLoaded OnComplete);

	/**
	 * 创建角色
	 *
	 * @param Request 创建请求
	 * @param OnComplete 完成回调
	 */
	virtual void CreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete);

	/**
	 * 删除角色
	 *
	 * @param CharacterId 角色 ID
	 * @param OnComplete 完成回调
	 */
	virtual void DeleteCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterDeleted OnComplete);

	/**
	 * 选择角色
	 *
	 * @param CharacterId 角色 ID
	 * @param OnComplete 完成回调
	 */
	virtual void SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete);

	/**
	 * 获取当前账户信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Account")
	const FDBAAccountInfo& GetCurrentAccountInfo() const { return CurrentAccountInfo; }

	/**
	 * 获取当前角色 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Account")
	const FDBACharacterId& GetCurrentCharacterId() const { return CurrentCharacterId; }

	/**
	 * 获取当前 Profile
	 */
	UFUNCTION(BlueprintCallable, Category = "Account")
	const FDBAPlayerProfile& GetCurrentProfile() const { return CurrentProfile; }

	/**
	 * 检查是否已登录
	 */
	UFUNCTION(BlueprintCallable, Category = "Account")
	bool IsLoggedIn() const { return CurrentAccountInfo.IsValid(); }

	/**
	 * 保存 Profile
	 *
	 * @return 是否保存成功
	 */
	UFUNCTION(BlueprintCallable, Category = "Account")
	bool SaveProfile();

protected:
	/**
	 * 当前账户信息
	 */
	UPROPERTY()
	FDBAAccountInfo CurrentAccountInfo;

	/**
	 * 当前角色 ID
	 */
	UPROPERTY()
	FDBACharacterId CurrentCharacterId;

	/**
	 * 当前 Profile
	 */
	UPROPERTY()
	FDBAPlayerProfile CurrentProfile;

	/**
	 * 会话 Token
	 */
	UPROPERTY()
	FString SessionToken;

	/**
	 * 加载账户存档
	 *
	 * @return 账户存档，加载失败返回 nullptr
	 */
	UDBAAccountSaveGame* LoadAccountSaveGame();

	/**
	 * 保存账户存档
	 *
	 * @param SaveGame 账户存档
	 * @return 是否保存成功
	 */
	bool SaveAccountSaveGame(UDBAAccountSaveGame* SaveGame);

	/**
	 * 加载 Profile 存档
	 *
	 * @return Profile 存档，加载失败返回 nullptr
	 */
	UDBAProfileSaveGame* LoadProfileSaveGame();

	/**
	 * 保存 Profile 存档
	 *
	 * @param SaveGame Profile 存档
	 * @return 是否保存成功
	 */
	bool SaveProfileSaveGame(UDBAProfileSaveGame* SaveGame);

	/**
	 * 创建默认账户存档
	 *
	 * @return 默认账户存档
	 */
	UDBAAccountSaveGame* CreateDefaultAccountSaveGame();

	/**
	 * 创建默认 Profile 存档
	 *
	 * @return 默认 Profile 存档
	 */
	UDBAProfileSaveGame* CreateDefaultProfileSaveGame();

	/**
	 * 处理存档损坏
	 *
	 * @param SlotName 存档槽名称
	 * @return 是否修复成功
	 */
	bool HandleCorruptedSaveGame(const FString& SlotName);

	/**
	 * 生成 Guest 账户 ID
	 *
	 * @return Guest 账户 ID
	 */
	FDBAAccountId GenerateGuestAccountId();

	/**
	 * 生成角色 ID
	 *
	 * @return 角色 ID
	 */
	FDBACharacterId GenerateCharacterId();

	/**
	 * 验证角色名称
	 *
	 * @param CharacterName 角色名称
	 * @param OutErrorMessage 错误消息
	 * @return 是否有效
	 */
	bool ValidateCharacterName(const FString& CharacterName, FString& OutErrorMessage);
};
