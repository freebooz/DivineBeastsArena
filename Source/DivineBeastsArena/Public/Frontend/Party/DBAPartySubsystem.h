// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "DBAPartySubsystem.generated.h"

class UDBAInviteServiceStub;

/**
 * Party 成员角色枚举
 * 定义 Party 中的权限等级
 */
UENUM(BlueprintType)
enum class EDBAPartySubsystemMemberRole : uint8
{
    Leader UMETA(DisplayName = "队长"),
    Member UMETA(DisplayName = "队员")
};

/**
 * Party 状态枚举
 * 定义 Party 的当前状态
 */
UENUM(BlueprintType)
enum class EDBAPartySubsystemState : uint8
{
    None UMETA(DisplayName = "未组队"),
    InLobby UMETA(DisplayName = "在大厅中"),
    InQueue UMETA(DisplayName = "在队列中"),
    InMatch UMETA(DisplayName = "在对局中")
};

/**
 * Party 成员信息结构体
 * 存储 Party 成员的基本信息
 */
USTRUCT(BlueprintType)
struct FDBAPartySubsystemMemberInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    FString PlayerId;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    FString DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    int32 Level = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    EDBAPartySubsystemMemberRole Role = EDBAPartySubsystemMemberRole::Member;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    bool bIsReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    bool bIsOnline = true;
};

/**
 * Party 信息结构体
 * 存储完整的 Party 数据
 */
USTRUCT(BlueprintType)
struct FDBAPartySubsystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    FString PartyId;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    TArray<FDBAPartySubsystemMemberInfo> Members;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    EDBAPartySubsystemState State = EDBAPartySubsystemState::None;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    int32 MaxMembers = 5;

    bool GetLeader(FDBAPartySubsystemMemberInfo& OutLeader) const
    {
        for (const FDBAPartySubsystemMemberInfo& Member : Members)
        {
            if (Member.Role == EDBAPartySubsystemMemberRole::Leader)
            {
                OutLeader = Member;
                return true;
            }
        }
        return false;
    }

    bool IsLeader(const FString& PlayerId) const
    {
        FDBAPartySubsystemMemberInfo Leader;
        return GetLeader(Leader) && Leader.PlayerId == PlayerId;
    }

    bool IsFull() const
    {
        return Members.Num() >= MaxMembers;
    }

    bool AreAllMembersReady() const
    {
        for (const FDBAPartySubsystemMemberInfo& Member : Members)
        {
            if (!Member.bIsReady)
            {
                return false;
            }
        }
        return Members.Num() > 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnPartySubsystemStateChanged, EDBAPartySubsystemState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnPartySubsystemMemberJoined, const FDBAPartySubsystemMemberInfo&, MemberInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnPartySubsystemMemberLeft, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnPartySubsystemInviteReceived, const FString&, InviterName, const FString&, PartyId);

UCLASS()
class DIVINEBEASTSARENA_API UDBAPartySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAPartySubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void CreateParty();

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void DisbandParty();

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void LeaveParty();

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void InvitePlayer(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void AcceptPartyInvite(const FString& PartyId);

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void DeclinePartyInvite(const FString& PartyId);

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void KickMember(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void TransferLeadership(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "DBA|Party")
    void SetReady(bool bReady);

    UFUNCTION(BlueprintPure, Category = "DBA|Party")
    bool IsInParty() const { return !CurrentPartyInfo.PartyId.IsEmpty(); }

    UFUNCTION(BlueprintPure, Category = "DBA|Party")
    bool IsPartyLeader() const;

    UFUNCTION(BlueprintPure, Category = "DBA|Party")
    FDBAPartySubsystemInfo GetCurrentPartyInfo() const { return CurrentPartyInfo; }

    UFUNCTION(BlueprintPure, Category = "DBA|Party")
    bool HasPermission(bool bRequireLeader) const;

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|Party")
    FDBAOnPartySubsystemStateChanged OnPartyStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Party")
    FDBAOnPartySubsystemMemberJoined OnPartyMemberJoined;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Party")
    FDBAOnPartySubsystemMemberLeft OnPartyMemberLeft;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Party")
    FDBAOnPartySubsystemInviteReceived OnPartyInviteReceived;

private:
    void SetPartyState(EDBAPartySubsystemState NewState);
    FString GetCurrentPlayerId() const;

private:
    UPROPERTY()
    FDBAPartySubsystemInfo CurrentPartyInfo;

    UPROPERTY()
    TObjectPtr<UDBAInviteServiceStub> InviteServiceStub;
};
