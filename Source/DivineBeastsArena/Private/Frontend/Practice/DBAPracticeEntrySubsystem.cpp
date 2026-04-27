// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Practice/DBAPracticeEntrySubsystem.h"
#include "Frontend/Lobby/DBACharacterRosterSubsystem.h"
#include "Frontend/HeroSelect/DBAHeroSelectSubsystem.h"
#include "Common/DBALogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UDBAPracticeEntrySubsystem::UDBAPracticeEntrySubsystem()
    : bIsInPractice(false)
{
}

void UDBAPracticeEntrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPracticeEntrySubsystem] 初始化完成"));
}

void UDBAPracticeEntrySubsystem::Deinitialize()
{
    // 清理 Timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LoadTimerHandle);
    }

    Super::Deinitialize();
}

void UDBAPracticeEntrySubsystem::EnterPractice(const FDBAPracticeConfig& Config)
{
    if (bIsInPractice)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPracticeEntrySubsystem] 已在 Practice 模式中"));
        return;
    }

    if (!Config.IsValid())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPracticeEntrySubsystem] Practice 配置无效"));
        OnPracticeEntered.Broadcast(false);
        return;
    }

    CurrentPracticeConfig = Config;

    // 如果使用当前角色，从 HeroSelectSubsystem 获取当前选中的英雄信息
    if (Config.bUseCurrentCharacter)
    {
        UE_LOG(LogDBAUI, Log, TEXT("[DBAPracticeEntrySubsystem] 使用当前角色进入练习模式"));

        // 从 HeroSelectSubsystem 获取当前选中的英雄 ID
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UDBAHeroSelectSubsystem* HeroSelectSubsystem = GameInstance->GetSubsystem<UDBAHeroSelectSubsystem>())
            {
                FName SelectedHeroId = HeroSelectSubsystem->GetSelectedHeroId();
                if (!SelectedHeroId.IsNone())
                {
                    UE_LOG(LogDBAUI, Log, TEXT("[DBAPracticeEntrySubsystem] 当前选中英雄: %s"), *SelectedHeroId.ToString());
                    // 设置到配置中（如果需要使用）
                }
            }
        }
    }

    if (CurrentPracticeConfig.MapPath.IsEmpty())
    {
        CurrentPracticeConfig.MapPath = GetDefaultPracticeMapPath(Config.ModeType);
    }

    UWorld* World = GetWorld();
    if (World)
    {
        ReturnLobbyPath = World->GetMapName();
    }

    LoadPracticeMap(CurrentPracticeConfig);
}

void UDBAPracticeEntrySubsystem::EnterPracticeWithCurrentCharacter()
{
    FDBAPracticeConfig Config;
    Config.ModeType = EDBAPracticeModeType::FreePractice;
    Config.bUseCurrentCharacter = true;
    Config.bInfiniteHealth = true;
    Config.bInfiniteEnergy = true;
    Config.bShowDamageNumbers = true;

    EnterPractice(Config);
}

void UDBAPracticeEntrySubsystem::ExitPractice()
{
    if (!bIsInPractice)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPracticeEntrySubsystem] 当前不在 Practice 模式中"));
        return;
    }

    // 清理加载 Timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LoadTimerHandle);
    }

    bIsInPractice = false;

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPracticeEntrySubsystem] 退出 Practice，返回大厅: %s"), *ReturnLobbyPath);

    OnPracticeExited.Broadcast();
}

FString UDBAPracticeEntrySubsystem::GetDefaultPracticeMapPath(EDBAPracticeModeType ModeType)
{
    switch (ModeType)
    {
    case EDBAPracticeModeType::FreePractice:
        return TEXT("/Game/Maps/Practice/FreePractice");
    case EDBAPracticeModeType::SkillPractice:
        return TEXT("/Game/Maps/Practice/SkillPractice");
    case EDBAPracticeModeType::CombatPractice:
        return TEXT("/Game/Maps/Practice/CombatPractice");
    case EDBAPracticeModeType::NewbieTutorial:
        return TEXT("/Game/Maps/Practice/NewbieTutorial");
    default:
        return TEXT("/Game/Maps/Practice/FreePractice");
    }
}

void UDBAPracticeEntrySubsystem::LoadPracticeMap(const FDBAPracticeConfig& Config)
{
    UE_LOG(LogDBAUI, Log, TEXT("[DBAPracticeEntrySubsystem] 开始加载 Practice 地图: %s"), *Config.MapPath);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            LoadTimerHandle,
            [this]()
            {
                HandlePracticeMapLoaded(true);
            },
            1.5f,
            false
        );
    }
}

void UDBAPracticeEntrySubsystem::HandlePracticeMapLoaded(bool bSuccess)
{
    if (bSuccess)
    {
        bIsInPractice = true;
        ApplyPracticeConfig();
        UE_LOG(LogDBAUI, Log, TEXT("[DBAPracticeEntrySubsystem] Practice 地图加载成功"));
    }
    else
    {
        UE_LOG(LogDBAUI, Error, TEXT("[DBAPracticeEntrySubsystem] Practice 地图加载失败"));
    }

    OnPracticeEntered.Broadcast(bSuccess);
}

void UDBAPracticeEntrySubsystem::ApplyPracticeConfig()
{
    UE_LOG(LogDBAUI, Log, TEXT("[DBAPracticeEntrySubsystem] 应用 Practice 配置"));
    UE_LOG(LogDBAUI, Log, TEXT("  - 模式: %d"), static_cast<int32>(CurrentPracticeConfig.ModeType));
    UE_LOG(LogDBAUI, Log, TEXT("  - 生肖: %d"), static_cast<int32>(CurrentPracticeConfig.Zodiac));
    UE_LOG(LogDBAUI, Log, TEXT("  - 自然元素之力: %d"), static_cast<int32>(CurrentPracticeConfig.Element));
    UE_LOG(LogDBAUI, Log, TEXT("  - 无限生命: %s"), CurrentPracticeConfig.bInfiniteHealth ? TEXT("是") : TEXT("否"));
    UE_LOG(LogDBAUI, Log, TEXT("  - 无限能量: %s"), CurrentPracticeConfig.bInfiniteEnergy ? TEXT("是") : TEXT("否"));
}