// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Lobby/DBALobbySubsystem.h"
#include "Frontend/Lobby/DBACharacterRosterSubsystem.h"
#include "Frontend/Newbie/DBANewbieGuideGateSubsystem.h"
#include "Common/DBALogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UDBALobbySubsystem::UDBALobbySubsystem()
    : CurrentLobbyType(EDBALobbyType::None)
    , CurrentLobbyState(EDBALobbyState::Unloaded)
{
}

void UDBALobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CharacterRosterSubsystem = GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>();
    NewbieGuideGateSubsystem = GetGameInstance()->GetSubsystem<UDBANewbieGuideGateSubsystem>();

    UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 初始化完成"));
}

void UDBALobbySubsystem::Deinitialize()
{
    // 清理 Timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LoadTimerHandle);
    }

    Super::Deinitialize();
}

void UDBALobbySubsystem::EnterLobby()
{
    EDBALobbyType TargetLobby = EDBALobbyType::MainLobby;

    if (NewbieGuideGateSubsystem && !NewbieGuideGateSubsystem->HasCompletedNewbieGuide())
    {
        TargetLobby = EDBALobbyType::NewbieVillage;
        UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 新手引导未完成，进入新手村"));
    }
    else
    {
        UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 新手引导已完成，进入主大厅"));
    }

    EnterSpecificLobby(TargetLobby);
}

void UDBALobbySubsystem::EnterSpecificLobby(EDBALobbyType LobbyType)
{
    if (LobbyType == EDBALobbyType::None)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBALobbySubsystem] 无效的大厅类型"));
        return;
    }

    if (CurrentLobbyState == EDBALobbyState::Loading)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBALobbySubsystem] 正在加载大厅，忽略重复请求"));
        return;
    }

    CurrentLobbyType = LobbyType;
    InitializeLobbyConfig(LobbyType);
    LoadLobbyLevel(LobbyType);
}

void UDBALobbySubsystem::LeaveLobby()
{
    if (CurrentLobbyState != EDBALobbyState::Ready)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBALobbySubsystem] 当前不在大厅中"));
        return;
    }

    // 清理加载 Timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LoadTimerHandle);
    }

    SetLobbyState(EDBALobbyState::Unloaded);
    CurrentLobbyType = EDBALobbyType::None;

    UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 离开大厅"));
}

void UDBALobbySubsystem::ApplyCampTheme(EDBAFiveCampType CampType)
{
    if (!IsInLobby())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBALobbySubsystem] 不在大厅中，无法应用阵营主题"));
        return;
    }

    CurrentLobbyConfig.ThemeCamp = CampType;

    UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 应用五大阵营主题: %d"), static_cast<int32>(CampType));
}

FString UDBALobbySubsystem::GetLobbyLevelPath(EDBALobbyType LobbyType)
{
    switch (LobbyType)
    {
    case EDBALobbyType::NewbieVillage:
        return TEXT("/Game/Maps/Frontend/NewbieVillage");
    case EDBALobbyType::MainLobby:
        return TEXT("/Game/Maps/Frontend/MainLobby");
    default:
        return TEXT("");
    }
}

void UDBALobbySubsystem::SetLobbyState(EDBALobbyState NewState)
{
    if (CurrentLobbyState != NewState)
    {
        CurrentLobbyState = NewState;
        OnLobbyStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 大厅状态变更: %d"), static_cast<int32>(NewState));
    }
}

void UDBALobbySubsystem::LoadLobbyLevel(EDBALobbyType LobbyType)
{
    SetLobbyState(EDBALobbyState::Loading);

    FString LevelPath = GetLobbyLevelPath(LobbyType);
    if (LevelPath.IsEmpty())
    {
        UE_LOG(LogDBAUI, Error, TEXT("[DBALobbySubsystem] 无效的大厅关卡路径"));
        HandleLevelLoadCompleted(false);
        return;
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 开始加载大厅关卡: %s"), *LevelPath);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            LoadTimerHandle,
            [this]()
            {
                HandleLevelLoadCompleted(true);
            },
            2.0f,
            false
        );
    }
}

void UDBALobbySubsystem::HandleLevelLoadCompleted(bool bSuccess)
{
    if (bSuccess)
    {
        SetLobbyState(EDBALobbyState::Ready);
        ApplyLobbyConfig();
        UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 大厅加载成功: %d"), static_cast<int32>(CurrentLobbyType));
    }
    else
    {
        SetLobbyState(EDBALobbyState::LoadFailed);
        UE_LOG(LogDBAUI, Error, TEXT("[DBALobbySubsystem] 大厅加载失败"));
    }

    OnLobbyLoaded.Broadcast(CurrentLobbyType, bSuccess);
}

void UDBALobbySubsystem::InitializeLobbyConfig(EDBALobbyType LobbyType)
{
    CurrentLobbyConfig.LobbyType = LobbyType;
    CurrentLobbyConfig.LevelPath = GetLobbyLevelPath(LobbyType);

    switch (LobbyType)
    {
    case EDBALobbyType::NewbieVillage:
        CurrentLobbyConfig.DisplayName = FText::FromString(TEXT("新手村"));
        CurrentLobbyConfig.bEnableParty = false;
        CurrentLobbyConfig.bEnablePractice = true;
        CurrentLobbyConfig.bEnableMatchmaking = false;
        CurrentLobbyConfig.bShowNewbieGuide = true;
        CurrentLobbyConfig.ThemeCamp = EDBAFiveCampType::None;
        break;

    case EDBALobbyType::MainLobby:
        CurrentLobbyConfig.DisplayName = FText::FromString(TEXT("主大厅"));
        CurrentLobbyConfig.bEnableParty = true;
        CurrentLobbyConfig.bEnablePractice = true;
        CurrentLobbyConfig.bEnableMatchmaking = true;
        CurrentLobbyConfig.bShowNewbieGuide = false;

        if (CharacterRosterSubsystem)
        {
            TArray<FName> OwnedCharacters = CharacterRosterSubsystem->GetOwnedCharacters();
            if (OwnedCharacters.Num() > 0)
            {
                // Use the first owned character to determine the camp theme
                CurrentLobbyConfig.ThemeCamp = EDBAFiveCampType::None;
            }
        }
        break;

    default:
        break;
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 初始化大厅配置: %s"), *CurrentLobbyConfig.DisplayName.ToString());
}

void UDBALobbySubsystem::ApplyLobbyConfig()
{
    UE_LOG(LogDBAUI, Log, TEXT("[DBALobbySubsystem] 应用大厅配置"));
    UE_LOG(LogDBAUI, Log, TEXT("  - Party: %s"), CurrentLobbyConfig.bEnableParty ? TEXT("启用") : TEXT("禁用"));
    UE_LOG(LogDBAUI, Log, TEXT("  - Practice: %s"), CurrentLobbyConfig.bEnablePractice ? TEXT("启用") : TEXT("禁用"));
    UE_LOG(LogDBAUI, Log, TEXT("  - Matchmaking: %s"), CurrentLobbyConfig.bEnableMatchmaking ? TEXT("启用") : TEXT("禁用"));
    UE_LOG(LogDBAUI, Log, TEXT("  - NewbieGuide: %s"), CurrentLobbyConfig.bShowNewbieGuide ? TEXT("显示") : TEXT("隐藏"));
}