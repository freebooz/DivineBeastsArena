// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DBABuildValidationLibrary.generated.h"

class UDBAFixedSkillGroupSubsystem;

UENUM(BlueprintType)
enum class EDBABuildValidationResult : uint8
{
    Valid UMETA(DisplayName = "合法"),
    InvalidHero UMETA(DisplayName = "无效英雄"),
    InvalidElement UMETA(DisplayName = "无效元素"),
    InvalidFiveCamp UMETA(DisplayName = "无效阵营"),
    MissingSkillGroup UMETA(DisplayName = "缺少技能组"),
    ElementFiveCampConflict UMETA(DisplayName = "元素与阵营冲突")
};

USTRUCT(BlueprintType)
struct FDBABuildValidationInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "BuildValidation")
    EDBABuildValidationResult Result = EDBABuildValidationResult::Valid;

    UPROPERTY(BlueprintReadOnly, Category = "BuildValidation")
    bool bIsValid = true;

    UPROPERTY(BlueprintReadOnly, Category = "BuildValidation")
    FText ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "BuildValidation")
    TArray<FText> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "BuildValidation")
    TArray<FText> Errors;

    FDBABuildValidationInfo()
        : bIsValid(true)
    {
    }
};

UCLASS()
class DIVINEBEASTSARENA_API UDBABuildValidationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "DBA|BuildValidation")
    static FDBABuildValidationInfo ValidateBuild(
        UPARAM(ref) const FDBABuildSummary& BuildSummary,
        UPARAM(ref) const TArray<FName>& AvailableHeroIds,
        UPARAM(ref) const TArray<EDBAElement>& AvailableElements,
        UPARAM(ref) const TArray<EDBAFiveCamp>& AvailableFiveCamps);

    UFUNCTION(BlueprintPure, Category = "DBA|BuildValidation")
    static bool IsHeroAvailable(FName HeroId, UPARAM(ref) const TArray<FName>& AvailableHeroIds);

    UFUNCTION(BlueprintPure, Category = "DBA|BuildValidation")
    static bool IsElementAvailable(EDBAElement Element, UPARAM(ref) const TArray<EDBAElement>& AvailableElements);

    UFUNCTION(BlueprintPure, Category = "DBA|BuildValidation")
    static bool IsFiveCampAvailable(EDBAFiveCamp FiveCamp, UPARAM(ref) const TArray<EDBAFiveCamp>& AvailableFiveCamps);

    UFUNCTION(BlueprintPure, Category = "DBA|BuildValidation")
    static FText GetValidationResultMessage(EDBABuildValidationResult Result);

    UFUNCTION(BlueprintPure, Category = "DBA|BuildValidation")
    static EDBABuildValidationResult GetCounterElementValidationResult(EDBAElement Element);

    UFUNCTION(BlueprintPure, Category = "DBA|BuildValidation")
    static EDBAElement GetCounterElement(EDBAElement Element);

    UFUNCTION(BlueprintPure, Category = "DBA|BuildValidation")
    static EDBAElement GetCounteredByElement(EDBAElement Element);
};
