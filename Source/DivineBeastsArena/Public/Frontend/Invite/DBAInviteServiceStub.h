// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DBAInviteServiceStub.generated.h"

/**
 * 邀请类型枚举
 * 定义不同类型的邀请
 */
UENUM(BlueprintType)
enum class EDBAInviteType : uint8
{
    Party UMETA(DisplayName = "Party 邀请"),

    Friend UMETA(DisplayName = "好友邀请"),

    Guild UMETA(DisplayName = "公会邀请")
};

/**
 * 邀请信息结构体
 * 存储邀请的详细信息
 */
USTRUCT(BlueprintType)
struct FDBAInviteInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Invite")
    FString InviteId;

    UPROPERTY(BlueprintReadOnly, Category = "Invite")
    EDBAInviteType InviteType = EDBAInviteType::Party;

    UPROPERTY(BlueprintReadOnly, Category = "Invite")
    FString InviterId;

    UPROPERTY(BlueprintReadOnly, Category = "Invite")
    FString InviterName;

    UPROPERTY(BlueprintReadOnly, Category = "Invite")
    FString TargetId;

    UPROPERTY(BlueprintReadOnly, Category = "Invite")
    FDateTime InviteTime;

    UPROPERTY(BlueprintReadOnly, Category = "Invite")
    bool bExpired = false;
};

UCLASS()
class DIVINEBEASTSARENA_API UDBAInviteServiceStub : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "DBA|Invite")
    void SendPartyInvite(const FString& TargetPlayerId, const FString& PartyId);

    UFUNCTION(BlueprintCallable, Category = "DBA|Invite")
    void SendFriendInvite(const FString& TargetPlayerId);

    UFUNCTION(BlueprintPure, Category = "DBA|Invite")
    TArray<FDBAInviteInfo> GetPendingInvites() const { return PendingInvites; }

    UFUNCTION(BlueprintCallable, Category = "DBA|Invite")
    void ClearExpiredInvites();

private:
    UPROPERTY()
    TArray<FDBAInviteInfo> PendingInvites;

    static constexpr float InviteTimeoutSeconds = 60.0f;
};