<#
Validates Arena HUD UltimateReadyPrompt can be driven through controller, root widget, and UI manager.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

$controllerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.h"
$controllerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDWidgetController.cpp"
$rootHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.h"
$rootCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAArenaHUDRootWidgetBase.cpp"
$promptHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAUltimateReadyPromptWidgetBase.h"
$promptCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAUltimateReadyPromptWidgetBase.cpp"
$managerHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\DBAGameUIManager.h"
$managerCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$ultimateReadyPromptTestPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAArenaHUDUltimateReadyPromptTests.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$controllerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerHeaderPath
$controllerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $controllerCppPath
$rootHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootHeaderPath
$rootCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $rootCppPath
$promptHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $promptHeaderPath
$promptCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $promptCppPath
$managerHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerHeaderPath
$managerCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $managerCppPath
$ultimateReadyPromptTest = Get-Content -Raw -Encoding UTF8 -LiteralPath $ultimateReadyPromptTestPath

Assert-True ($controllerHeader -match "ShowUltimateReadyPrompt\s*\(") "Expected controller show ultimate-ready prompt entrypoint."
Assert-True ($controllerHeader -match "HideUltimateReadyPrompt\s*\(") "Expected controller hide ultimate-ready prompt entrypoint."
Assert-True ($controllerHeader -match "FDBAArenaUltimateReadyPromptState") "Expected controller cached ultimate-ready prompt state type."
Assert-True ($controllerHeader -match "GetLastUltimateReadyPromptState") "Expected controller cached ultimate-ready prompt getter."
Assert-True ($controllerHeader -match "FOnUltimateReadyPromptShown") "Expected controller shown delegate."
Assert-True ($controllerHeader -match "FOnUltimateReadyPromptHidden") "Expected controller hidden delegate."
Assert-True ($controllerCpp -match "LastUltimateReadyPromptState\.bIsValid\s*=\s*true") "Expected controller to mark cached ultimate-ready prompt state valid."
Assert-True ($controllerCpp -match "LastUltimateReadyPromptState\.bIsShown\s*=\s*true") "Expected controller to cache ultimate-ready shown state."
Assert-True ($controllerCpp -match "LastUltimateReadyPromptState\.bIsShown\s*=\s*false") "Expected controller to cache ultimate-ready hidden state."
Assert-True ($controllerCpp -match "OnUltimateReadyPromptShown\.Broadcast\(\)") "Expected controller to broadcast ultimate-ready prompt shown."
Assert-True ($controllerCpp -match "OnUltimateReadyPromptHidden\.Broadcast\(\)") "Expected controller to broadcast ultimate-ready prompt hidden."

Assert-True ($rootHeader -match "HandleControllerUltimateReadyPromptShown") "Expected root shown handler."
Assert-True ($rootHeader -match "HandleControllerUltimateReadyPromptHidden") "Expected root hidden handler."
Assert-True ($rootCpp.Contains('#include "GameDBA/UI/Arena/UDBAUltimateReadyPromptWidgetBase.h"')) "Expected root implementation to include UltimateReadyPrompt widget."
Assert-True ($rootCpp -match "OnUltimateReadyPromptShown\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptShown\)") "Expected root to bind shown delegate."
Assert-True ($rootCpp -match "OnUltimateReadyPromptHidden\.AddDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptHidden\)") "Expected root to bind hidden delegate."
Assert-True ($rootCpp -match "OnUltimateReadyPromptShown\.RemoveDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptShown\)") "Expected root to unbind shown delegate."
Assert-True ($rootCpp -match "OnUltimateReadyPromptHidden\.RemoveDynamic\(this,\s*&UDBAArenaHUDRootWidgetBase::HandleControllerUltimateReadyPromptHidden\)") "Expected root to unbind hidden delegate."
Assert-True ($rootCpp -match "GetLastUltimateReadyPromptState\(\)") "Expected root HUD to inspect cached ultimate-ready prompt state after binding."
Assert-True ($rootCpp -match "LastUltimateReadyPromptState\.bIsValid") "Expected root HUD to gate cached ultimate-ready prompt replay."
Assert-True ($rootCpp -match "LastUltimateReadyPromptState\.bIsShown") "Expected root HUD to branch on cached ultimate-ready prompt visibility."
Assert-True ($rootCpp -match "UltimateReadyPrompt->ShowUltimateReady\(\)") "Expected root to show UltimateReadyPrompt."
Assert-True ($rootCpp -match "UltimateReadyPrompt->HideUltimateReady\(\)") "Expected root to hide UltimateReadyPrompt."

Assert-True ($promptHeader -match "bCachedUltimateReadyVisible") "Expected UltimateReadyPrompt widget to cache visible state."
Assert-True ($promptCpp -match "NativeConstruct[\s\S]{0,260}if\s*\(\s*bCachedUltimateReadyVisible\s*\)[\s\S]{0,160}BP_OnUltimateReady\(\)") "Expected UltimateReadyPrompt NativeConstruct to replay visible state after widget construction."
Assert-True ($promptCpp -match "NativeConstruct[\s\S]{0,420}else[\s\S]{0,160}BP_OnUltimateHidden\(\)") "Expected UltimateReadyPrompt NativeConstruct to replay hidden state after widget construction."
Assert-True ($promptCpp -match "ShowUltimateReady[\s\S]*bCachedUltimateReadyVisible\s*=\s*true[\s\S]*BP_OnUltimateReady\(\)") "Expected UltimateReadyPrompt show function to cache visible state and call Blueprint event."
Assert-True ($promptCpp -match "HideUltimateReady[\s\S]*bCachedUltimateReadyVisible\s*=\s*false[\s\S]*BP_OnUltimateHidden\(\)") "Expected UltimateReadyPrompt hide function to cache hidden state and call Blueprint event."

Assert-True ($managerHeader -match "ShowArenaHUDUltimateReadyPrompt\s*\(") "Expected UI manager show ultimate-ready prompt entrypoint."
Assert-True ($managerHeader -match "HideArenaHUDUltimateReadyPrompt\s*\(") "Expected UI manager hide ultimate-ready prompt entrypoint."
Assert-True ($managerCpp -match "Controller->ShowUltimateReadyPrompt\(\)") "Expected UI manager to route show to controller."
Assert-True ($managerCpp -match "Controller->HideUltimateReadyPrompt\(\)") "Expected UI manager to route hide to controller."
Assert-True ($ultimateReadyPromptTest -match "UltimateReadyPromptCachesLatestState") "Expected automation coverage for cached ultimate-ready prompt state."
Assert-True ($ultimateReadyPromptTest -match "GetLastUltimateReadyPromptState") "Expected automation test to exercise cached ultimate-ready prompt getter."
Assert-True ($ultimateReadyPromptTest -match "ShowUltimateReadyPrompt\(\)") "Expected automation test to verify cached ultimate-ready prompt show."
Assert-True ($ultimateReadyPromptTest -match "HideUltimateReadyPrompt\(\)") "Expected automation test to verify cached ultimate-ready prompt hide."

Write-Host "PASS: Arena HUD UltimateReadyPrompt sync contract" -ForegroundColor Green
