<#
Exercises validate-unreal-moba-foundation.ps1 against focused GameMoba
foundation fixtures.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$testRoot = Join-Path $repoRoot (".tmp\unreal-moba-foundation-tests-{0}" -f [guid]::NewGuid().ToString("N"))
if (Test-Path -LiteralPath $testRoot) {
    Remove-Item -LiteralPath $testRoot -Recurse -Force
}

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Write-FixtureFile {
    param(
        [Parameter(Mandatory = $true)][string]$Root,
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string]$Content
    )

    $path = Join-Path $Root $RelativePath
    $directory = Split-Path -Parent $path
    New-Item -ItemType Directory -Force -Path $directory | Out-Null
    $Content | Set-Content -LiteralPath $path -Encoding UTF8
}

function Write-MobaFixture {
    param(
        [Parameter(Mandatory = $true)][string]$Root,
        [switch]$InvalidHudController
    )

    Write-FixtureFile $Root "DBA_GameClient\Source\GameMoba\GameMoba.Build.cs" @'
using UnrealBuildTool;

public class GameMoba : ModuleRules
{
    public GameMoba(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "UMG",
            "GameCore",
        });
    }
}
'@

    Write-FixtureFile $Root "DBA_GameClient\Source\GameMoba\Public\GameMoba\GAS\DBAMobaAbilitySystemComponentBase.h" @'
#pragma once
#include "AbilitySystemComponent.h"
#include "DBAMobaAbilitySystemComponentBase.generated.h"

UCLASS(ClassGroup=(GameMoba), meta=(BlueprintSpawnableComponent))
class GAMEMOBA_API UDBAMobaAbilitySystemComponentBase : public UAbilitySystemComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
    virtual void InitializeAbilities(AActor* InOwnerActor, AActor* InAvatarActor);
    UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
    void RemoveAllGrantedAbilities();
protected:
    UPROPERTY(BlueprintReadOnly, Category = "DBA|Ability|Base")
    bool bAbilitiesInitialized = false;
    UPROPERTY()
    TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
};
'@

    Write-FixtureFile $Root "DBA_GameClient\Source\GameMoba\Private\GameMoba\GAS\DBAMobaAbilitySystemComponentBase.cpp" @'
#include "GameMoba/GAS/DBAMobaAbilitySystemComponentBase.h"

void UDBAMobaAbilitySystemComponentBase::InitializeAbilities(AActor* InOwnerActor, AActor* InAvatarActor)
{
    InitAbilityActorInfo(InOwnerActor, InAvatarActor);
    bAbilitiesInitialized = true;
}

void UDBAMobaAbilitySystemComponentBase::RemoveAllGrantedAbilities()
{
    for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
    {
        ClearAbility(Handle);
    }
    GrantedAbilityHandles.Empty();
}
'@

    Write-FixtureFile $Root "DBA_GameClient\Source\GameMoba\Public\GameMoba\GAS\DBAMobaGameplayAbilityBase.h" @'
#pragma once
#include "Abilities/GameplayAbility.h"
#include "DBAMobaGameplayAbilityBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API UDBAMobaGameplayAbilityBase : public UGameplayAbility
{
    GENERATED_BODY()
protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
    float CooldownDuration = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
    float EnergyCost = 0.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
    bool bIsUltimate = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
    FName ElementType = NAME_None;
};
'@

    $controllerPointer = if ($InvalidHudController) {
        "UPROPERTY()`n    TObjectPtr<class APlayerController> PlayerController;"
    }
    else {
        "UPROPERTY()`n    TWeakObjectPtr<class APlayerController> PlayerController;"
    }

    Write-FixtureFile $Root "DBA_GameClient\Source\GameMoba\Public\GameMoba\UI\DBAMobaHUDWidgetControllerBase.h" @"
#pragma once
#include "DBAMobaHUDWidgetControllerBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API UDBAMobaHUDWidgetControllerBase : public UObject
{
    GENERATED_BODY()
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerHPChanged, float, CurrentHP, float, MaxHP);
    UFUNCTION(BlueprintCallable, Category = "DBA|HUD|WidgetController")
    virtual void InitializeController(class APlayerController* InPlayerController);
    UFUNCTION(BlueprintCallable, Category = "DBA|HUD|WidgetController")
    virtual void UpdatePlayerHP(float CurrentHP, float MaxHP);
    UPROPERTY(BlueprintAssignable, Category = "DBA|HUD|WidgetController")
    FOnPlayerHPChanged OnPlayerHPChanged;
protected:
    $controllerPointer
    UPROPERTY(BlueprintReadOnly, Category = "DBA|HUD|WidgetController")
    bool bIsInitialized = false;
};
"@

    Write-FixtureFile $Root "DBA_GameClient\Source\GameMoba\Public\GameMoba\UI\UDBAMobaUserWidgetBase.h" @'
#pragma once
#include "Blueprint/UserWidget.h"
#include "UDBAMobaUserWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API UDBAMobaUserWidgetBase : public UUserWidget
{
    GENERATED_BODY()
protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|Base", meta = (DisplayName = "On Show"))
    void BP_OnShow();
    UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|Base", meta = (DisplayName = "On Hide"))
    void BP_OnHide();
    UPROPERTY()
    TWeakObjectPtr<class APlayerController> OwnerPlayerController;
};
'@
}

$validRoot = Join-Path $testRoot "valid"
$invalidRoot = Join-Path $testRoot "invalid"
Write-MobaFixture -Root $validRoot
Write-MobaFixture -Root $invalidRoot -InvalidHudController

& (Join-Path $repoRoot "scripts\validate-unreal-moba-foundation.ps1") -RepoRoot $validRoot

$failedAsExpected = $false
try {
    & (Join-Path $repoRoot "scripts\validate-unreal-moba-foundation.ps1") -RepoRoot $invalidRoot
}
catch {
    $failedAsExpected = $true
}

Assert-True $failedAsExpected "Expected invalid HUD controller fixture to fail."

Write-Host "PASS: Unreal Moba foundation fixtures" -ForegroundColor Green
