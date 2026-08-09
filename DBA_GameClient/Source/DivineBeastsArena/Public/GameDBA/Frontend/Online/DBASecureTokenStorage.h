// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DBASecureTokenStorage.generated.h"

/**
 * RefreshToken 的平台安全存储抽象。AccessToken 始终只保存在传输子系统内存中。
 * 当前开发实现不跨进程持久化，发布平台必须以 Keychain / Keystore / 凭据管理器实现替换。
 */
UCLASS(Abstract, Transient)
class DIVINEBEASTSARENA_API UDBASecureTokenStorage : public UObject
{
	GENERATED_BODY()

public:
	virtual bool StoreRefreshToken(const FString& InRefreshToken) PURE_VIRTUAL(UDBASecureTokenStorage::StoreRefreshToken, return false;);
	virtual bool LoadRefreshToken(FString& OutRefreshToken) const PURE_VIRTUAL(UDBASecureTokenStorage::LoadRefreshToken, return false;);
	virtual void ClearRefreshToken() PURE_VIRTUAL(UDBASecureTokenStorage::ClearRefreshToken, );
};

/** 仅供开发环境使用的进程内实现：不会将 RefreshToken 写入 SaveGame、Config 或日志。 */
UCLASS(Transient)
class DIVINEBEASTSARENA_API UDBADevelopmentTokenStorage final : public UDBASecureTokenStorage
{
	GENERATED_BODY()

public:
	virtual bool StoreRefreshToken(const FString& InRefreshToken) override;
	virtual bool LoadRefreshToken(FString& OutRefreshToken) const override;
	virtual void ClearRefreshToken() override;

private:
	FString RefreshToken;
};
