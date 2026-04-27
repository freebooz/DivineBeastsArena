// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAFixedSkillGroupSubsystem.generated.h"

class UDBAFixedSkillGroupDataAsset;

USTRUCT(BlueprintType)
struct FDBAFixedSkillGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FName GroupId;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FName HeroId;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    EDBAElement Element;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FName PassiveSkillId;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FName Skill01Id;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FName Skill02Id;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FName Skill03Id;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FName Skill04Id;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FName UltimateSkillId;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    int32 ResonanceLevel;

    UPROPERTY(BlueprintReadOnly, Category = "FixedSkillGroup")
    FText ResonanceDescription;

    FDBAFixedSkillGroup()
        : Element(EDBAElement::None)
        , ResonanceLevel(0)
    {
    }
};

USTRUCT(BlueprintType)
struct FDBABuildSummary
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FName HeroId;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EDBAZodiac Zodiac;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EDBAElement Element;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    EDBAFiveCamp FiveCamp;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FDBAFixedSkillGroup SkillGroup;

    FDBABuildSummary()
        : Zodiac(EDBAZodiac::None)
        , Element(EDBAElement::None)
        , FiveCamp(EDBAFiveCamp::None)
    {
    }
};

UCLASS()
class DIVINEBEASTSARENA_API UDBAFixedSkillGroupSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAFixedSkillGroupSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
    FDBAFixedSkillGroup GetFixedSkillGroup(FName HeroId, EDBAElement Element) const;

    UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
    bool HasFixedSkillGroup(FName HeroId, EDBAElement Element) const;

    UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
    int32 CalculateResonanceLevel(EDBAElement Element, const TArray<FName>& SkillIds) const;

    UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
    FText GetResonanceDescription(EDBAElement Element, int32 ResonanceLevel) const;

    UFUNCTION(BlueprintPure, Category = "DBA|FixedSkillGroup")
    FDBABuildSummary GetCurrentBuildSummary() const { return CurrentBuildSummary; }

    UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
    void SetCurrentBuildSummary(const FDBABuildSummary& Summary);

    UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
    void ClearCurrentBuildSummary();

private:
    UPROPERTY()
    FDBABuildSummary CurrentBuildSummary;

    UPROPERTY()
    TObjectPtr<UDBAFixedSkillGroupDataAsset> FixedSkillGroupDataAsset;
};
