// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "DBAServerDirectorySubsystem.generated.h"

UENUM(BlueprintType)
enum class EDBAServerDirectoryStatus : uint8
{
	Online,
	Busy,
	Full,
	Maintenance,
	Offline
};

USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAServerDirectoryEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	FString ServerId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	FText Name;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	FString Region;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	EDBAServerDirectoryStatus Status = EDBAServerDirectoryStatus::Offline;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	int32 Population = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	bool bRecommended = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	FText MaintenanceMessage;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	FString MinClientVersion;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerDirectory")
	bool bCanSelect = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnServerDirectoryChanged, bool, bSuccess, const FDBAApiError&, Error);

/** 前台唯一的区服目录读取入口；只消费 /api/v1/servers 的领域 DTO，不暴露原始 JSON 给 Widget。 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAServerDirectorySubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedInCurrentEnvironment() const override;

	UFUNCTION(BlueprintCallable, Category = "DBA|ServerDirectory")
	bool RefreshDirectory(const FString& Region, const FString& ClientVersion, const FString& Platform);

	UFUNCTION(BlueprintPure, Category = "DBA|ServerDirectory")
	bool IsRefreshInFlight() const { return bRefreshInFlight; }

	UFUNCTION(BlueprintPure, Category = "DBA|ServerDirectory")
	const TArray<FDBAServerDirectoryEntry>& GetCachedServers() const { return CachedServers; }

	const FDBAServerDirectoryEntry* FindSelectableServer(const FString& ServerId) const;

	/** 仅保存稳定 ServerId 的本地偏好；绝不写入令牌、密码或连接地址。 */
	FString GetLastSelectedServerId(const FString& AccountId) const;
	void RecordLastSelectedServer(const FString& AccountId, const FString& ServerId);

	UPROPERTY(BlueprintAssignable, Category = "DBA|ServerDirectory")
	FDBAOnServerDirectoryChanged OnDirectoryChanged;

	static bool ParseDirectoryJson(const FString& Json, TArray<FDBAServerDirectoryEntry>& OutServers, FString& OutError);

private:
	static EDBAServerDirectoryStatus ParseStatus(const FString& Value);
	static FString BuildPath(const FString& Region, const FString& ClientVersion, const FString& Platform);

	UPROPERTY(Transient)
	TArray<FDBAServerDirectoryEntry> CachedServers;
	bool bRefreshInFlight = false;
	uint64 DirectoryRequestGeneration = 0;
};
