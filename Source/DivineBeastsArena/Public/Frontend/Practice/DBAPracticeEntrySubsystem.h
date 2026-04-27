// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/DBAEnumsCore.h"
#include "Common/DBAEnumsCore.h"
#include "DBAPracticeEntrySubsystem.generated.h"

/**
 * Practice 模式类型枚举
 * 定义不同的练习模式
 */
UENUM(BlueprintType)
enum class EDBAPracticeModeType : uint8
{
    FreePractice UMETA(DisplayName = "自由练习"),

    SkillPractice UMETA(DisplayName = "技能练习"),

    CombatPractice UMETA(DisplayName = "战斗练习"),

    NewbieTutorial UMETA(DisplayName = "新手教学")
};

/**
 * Practice 配置结构体
 * 存储 Practice 模式的配置参数
 */
USTRUCT(BlueprintType)
struct FDBAPracticeConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    EDBAPracticeModeType ModeType = EDBAPracticeModeType::FreePractice;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    EDBAZodiacType Zodiac = EDBAZodiacType::None;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    EDBAElementType Element = EDBAElementType::None;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    bool bUseCurrentCharacter = true;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    FString MapPath;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    bool bEnableAIOpponent = false;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    int32 AIOpponentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    bool bInfiniteHealth = true;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    bool bInfiniteEnergy = true;

    UPROPERTY(BlueprintReadWrite, Category = "Practice")
    bool bShowDamageNumbers = true;

    bool IsValid() const
    {
        if (bUseCurrentCharacter)
        {
            return true;
        }
        return Zodiac != EDBAZodiacType::None && Element != EDBAElementType::None;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnPracticeEntered, bool, bSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnPracticeExited);

UCLASS()
class DIVINEBEASTSARENA_API UDBAPracticeEntrySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAPracticeEntrySubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|Practice")
    void EnterPractice(const FDBAPracticeConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "DBA|Practice")
    void EnterPracticeWithCurrentCharacter();

    UFUNCTION(BlueprintCallable, Category = "DBA|Practice")
    void ExitPractice();

    UFUNCTION(BlueprintPure, Category = "DBA|Practice")
    bool IsInPractice() const { return bIsInPractice; }

    UFUNCTION(BlueprintPure, Category = "DBA|Practice")
    FDBAPracticeConfig GetCurrentPracticeConfig() const { return CurrentPracticeConfig; }

    UFUNCTION(BlueprintPure, Category = "DBA|Practice")
    static FString GetDefaultPracticeMapPath(EDBAPracticeModeType ModeType);

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|Practice")
    FDBAOnPracticeEntered OnPracticeEntered;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Practice")
    FDBAOnPracticeExited OnPracticeExited;

private:
    void LoadPracticeMap(const FDBAPracticeConfig& Config);
    void HandlePracticeMapLoaded(bool bSuccess);
    void ApplyPracticeConfig();

private:
    UPROPERTY()
    bool bIsInPractice;

    UPROPERTY()
    FDBAPracticeConfig CurrentPracticeConfig;

    UPROPERTY()
    FString ReturnLobbyPath;

    /** Practice 地图加载 Timer */
    FTimerHandle LoadTimerHandle;
};