<#
Validates GameMoba foundation contracts for GAS and UI data boundaries.
Run from the repository root:
  .\scripts\validate-unreal-moba-foundation.ps1
#>

[CmdletBinding()]
param(
    [string]$RepoRoot
)

$ErrorActionPreference = "Stop"
$repoRoot = if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
}
else {
    (Resolve-Path -LiteralPath $RepoRoot).ProviderPath
}
$sourceRoot = Join-Path $repoRoot "DBA_GameClient\Source"
$mobaRoot = Join-Path $sourceRoot "GameMoba"
$failures = New-Object System.Collections.Generic.List[string]

function Add-Failure {
    param([Parameter(Mandatory = $true)][string]$Message)
    $failures.Add($Message) | Out-Null
}

function Get-RequiredFileContent {
    param([Parameter(Mandatory = $true)][string]$RelativePath)

    $path = Join-Path $sourceRoot $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        Add-Failure "Missing GameMoba foundation file: $RelativePath"
        return ""
    }

    return Get-Content -Raw -Encoding UTF8 -LiteralPath $path
}

function Test-ContainsAll {
    param(
        [Parameter(Mandatory = $true)][string]$Content,
        [Parameter(Mandatory = $true)][string[]]$Tokens,
        [Parameter(Mandatory = $true)][string]$FailurePrefix
    )

    foreach ($token in $Tokens) {
        if (-not $Content.Contains($token)) {
            Add-Failure "$FailurePrefix`: missing $token"
        }
    }
}

function Test-BuildDependencies {
    $content = Get-RequiredFileContent "GameMoba\GameMoba.Build.cs"
    if ([string]::IsNullOrWhiteSpace($content)) {
        return
    }

    Test-ContainsAll `
        -Content $content `
        -FailurePrefix "GameMoba Build.cs foundation dependencies are incomplete" `
        -Tokens @(
            '"GameplayAbilities"',
            '"GameplayTags"',
            '"GameplayTasks"',
            '"UMG"',
            '"GameCore"'
        )
}

function Test-GasFoundation {
    $ascHeader = Get-RequiredFileContent "GameMoba\Public\GameMoba\GAS\DBAMobaAbilitySystemComponentBase.h"
    $ascCpp = Get-RequiredFileContent "GameMoba\Private\GameMoba\GAS\DBAMobaAbilitySystemComponentBase.cpp"
    $abilityHeader = Get-RequiredFileContent "GameMoba\Public\GameMoba\GAS\DBAMobaGameplayAbilityBase.h"

    if (-not [string]::IsNullOrWhiteSpace($ascHeader)) {
        if ($ascHeader -notmatch 'class\s+GAMEMOBA_API\s+UDBAMobaAbilitySystemComponentBase\s*:\s*public\s+UAbilitySystemComponent') {
            Add-Failure "UDBAMobaAbilitySystemComponentBase must remain the GameMoba-owned AbilitySystemComponent base."
        }

        Test-ContainsAll `
            -Content $ascHeader `
            -FailurePrefix "GameMoba AbilitySystemComponent base contract is incomplete" `
            -Tokens @(
                'InitializeAbilities',
                'RemoveAllGrantedAbilities',
                'bAbilitiesInitialized',
                'GrantedAbilityHandles',
                'BlueprintSpawnableComponent'
            )
    }

    if (-not [string]::IsNullOrWhiteSpace($ascCpp)) {
        Test-ContainsAll `
            -Content $ascCpp `
            -FailurePrefix "GameMoba AbilitySystemComponent implementation contract is incomplete" `
            -Tokens @(
                'InitAbilityActorInfo',
                'ClearAbility',
                'GrantedAbilityHandles.Empty()'
            )
    }

    if (-not [string]::IsNullOrWhiteSpace($abilityHeader)) {
        if ($abilityHeader -notmatch 'class\s+GAMEMOBA_API\s+UDBAMobaGameplayAbilityBase\s*:\s*public\s+UGameplayAbility') {
            Add-Failure "UDBAMobaGameplayAbilityBase must remain the GameMoba-owned GameplayAbility base."
        }

        Test-ContainsAll `
            -Content $abilityHeader `
            -FailurePrefix "GameMoba GameplayAbility base contract is incomplete" `
            -Tokens @(
                'CooldownDuration',
                'EnergyCost',
                'bIsUltimate',
                'ElementType'
            )
    }
}

function Test-UiFoundation {
    $controllerHeader = Get-RequiredFileContent "GameMoba\Public\GameMoba\UI\DBAMobaHUDWidgetControllerBase.h"
    $widgetHeader = Get-RequiredFileContent "GameMoba\Public\GameMoba\UI\UDBAMobaUserWidgetBase.h"

    if (-not [string]::IsNullOrWhiteSpace($controllerHeader)) {
        if ($controllerHeader -notmatch 'class\s+GAMEMOBA_API\s+UDBAMobaHUDWidgetControllerBase\s*:\s*public\s+UObject') {
            Add-Failure "UDBAMobaHUDWidgetControllerBase must remain a UObject data controller, not a Widget subclass."
        }

        Test-ContainsAll `
            -Content $controllerHeader `
            -FailurePrefix "GameMoba HUD WidgetController data boundary is incomplete" `
            -Tokens @(
                'TWeakObjectPtr<class APlayerController>',
                'OnPlayerHPChanged',
                'UpdatePlayerHP',
                'bIsInitialized',
                'BlueprintAssignable'
            )
    }

    if (-not [string]::IsNullOrWhiteSpace($widgetHeader)) {
        if ($widgetHeader -notmatch 'class\s+GAMEMOBA_API\s+UDBAMobaUserWidgetBase\s*:\s*public\s+UUserWidget') {
            Add-Failure "UDBAMobaUserWidgetBase must remain the GameMoba-owned UUserWidget base."
        }

        Test-ContainsAll `
            -Content $widgetHeader `
            -FailurePrefix "GameMoba UserWidget base contract is incomplete" `
            -Tokens @(
                'BP_OnShow',
                'BP_OnHide',
                'TWeakObjectPtr<class APlayerController>'
            )
    }
}

function Test-NoBackendCoupling {
    if (-not (Test-Path -LiteralPath $mobaRoot)) {
        Add-Failure "Missing GameMoba module root: $mobaRoot"
        return
    }

    $matches = & rg -n "GameBackendClient|GameBackend" $mobaRoot -g "*.h" -g "*.cpp" -g "*.Build.cs"
    if ($LASTEXITCODE -gt 1) {
        throw "rg GameMoba backend coupling scan failed with code $LASTEXITCODE"
    }

    if ($matches) {
        Add-Failure "GameMoba foundation must not depend on backend client services:`n$($matches -join "`n")"
    }
}

Push-Location $repoRoot
try {
    Test-BuildDependencies
    Test-GasFoundation
    Test-UiFoundation
    Test-NoBackendCoupling
}
finally {
    Pop-Location
}

if ($failures.Count -gt 0) {
    foreach ($failure in $failures) {
        Write-Host "FAIL: $failure" -ForegroundColor Red
    }
    throw "Unreal Moba foundation validation failed."
}

Write-Host "PASS: Unreal Moba foundation" -ForegroundColor Green
