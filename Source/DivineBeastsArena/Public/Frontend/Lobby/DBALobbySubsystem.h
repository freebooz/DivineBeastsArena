// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/DBAEnumsCore.h"
#include "DBALobbySubsystem.generated.h"

class UDBACharacterRosterSubsystem;
class UDBANewbieGuideGateSubsystem;

/**
 * 大厅类型枚举
 * 定义不同的大厅环境
 */
UENUM(BlueprintType)
enum class EDBALobbyType : uint8
{
    None UMETA(DisplayName = "未知"),

    NewbieVillage UMETA(DisplayName = "新手村"),

    MainLobby UMETA(DisplayName = "主大厅")
};

/**
 * 大厅状态枚举
 * 追踪大厅加载和初始化状态
 */
UENUM(BlueprintType)
enum class EDBALobbyState : uint8
{
    Unloaded UMETA(DisplayName = "未加载"),

    Loading UMETA(DisplayName = "正在加载"),

    Ready UMETA(DisplayName = "准备就绪"),

    LoadFailed UMETA(DisplayName = "加载失败")
};

/**
 * 大厅配置结构体
 * 存储大厅的显示和功能配置
 */
USTRUCT(BlueprintType)
struct FDBALobbyConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    EDBALobbyType LobbyType = EDBALobbyType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    FString LevelPath;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    FText DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    bool bEnableParty = true;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    bool bEnablePractice = true;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    bool bEnableMatchmaking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    bool bShowNewbieGuide = false;

    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    EDBAFiveCampType ThemeCamp = EDBAFiveCampType::None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnLobbyStateChanged, EDBALobbyState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnLobbyLoaded, EDBALobbyType, LobbyType, bool, bSuccess);

UCLASS()
class DIVINEBEASTSARENA_API UDBALobbySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBALobbySubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|Lobby")
    void EnterLobby();

    UFUNCTION(BlueprintCallable, Category = "DBA|Lobby")
    void EnterSpecificLobby(EDBALobbyType LobbyType);

    UFUNCTION(BlueprintCallable, Category = "DBA|Lobby")
    void LeaveLobby();

    UFUNCTION(BlueprintPure, Category = "DBA|Lobby")
    EDBALobbyType GetCurrentLobbyType() const { return CurrentLobbyType; }

    UFUNCTION(BlueprintPure, Category = "DBA|Lobby")
    EDBALobbyState GetLobbyState() const { return CurrentLobbyState; }

    UFUNCTION(BlueprintPure, Category = "DBA|Lobby")
    FDBALobbyConfig GetCurrentLobbyConfig() const { return CurrentLobbyConfig; }

    UFUNCTION(BlueprintPure, Category = "DBA|Lobby")
    bool IsInLobby() const { return CurrentLobbyState == EDBALobbyState::Ready; }

    UFUNCTION(BlueprintCallable, Category = "DBA|Lobby")
    void ApplyCampTheme(EDBAFiveCampType CampType);

    UFUNCTION(BlueprintPure, Category = "DBA|Lobby")
    static FString GetLobbyLevelPath(EDBALobbyType LobbyType);

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|Lobby")
    FDBAOnLobbyStateChanged OnLobbyStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Lobby")
    FDBAOnLobbyLoaded OnLobbyLoaded;

private:
    void SetLobbyState(EDBALobbyState NewState);
    void LoadLobbyLevel(EDBALobbyType LobbyType);
    void HandleLevelLoadCompleted(bool bSuccess);
    void InitializeLobbyConfig(EDBALobbyType LobbyType);
    void ApplyLobbyConfig();

private:
    UPROPERTY()
    EDBALobbyType CurrentLobbyType;

    UPROPERTY()
    EDBALobbyState CurrentLobbyState;

    UPROPERTY()
    FDBALobbyConfig CurrentLobbyConfig;

    UPROPERTY()
    TObjectPtr<UDBACharacterRosterSubsystem> CharacterRosterSubsystem;

    UPROPERTY()
    TObjectPtr<UDBANewbieGuideGateSubsystem> NewbieGuideGateSubsystem;

    /** 大厅加载 Timer */
    FTimerHandle LoadTimerHandle;
};