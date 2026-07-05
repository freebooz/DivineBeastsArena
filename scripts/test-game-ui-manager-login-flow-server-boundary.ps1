<#
Validates DBAGameUIManager login flow presentation paths no-op before widget or UI audio creation in server-like runtimes.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$cppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
$cpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $cppPath

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Get-FunctionBody {
    param(
        [Parameter(Mandatory = $true)][string]$StartPattern,
        [Parameter(Mandatory = $true)][string]$EndPattern,
        [Parameter(Mandatory = $true)][string]$FunctionName
    )

    $match = [regex]::Match($cpp, $StartPattern)
    Assert-True $match.Success "Expected $FunctionName implementation."

    $remaining = $cpp.Substring($match.Index + $match.Length)
    $nextMatch = [regex]::Match($remaining, $EndPattern)
    Assert-True $nextMatch.Success "Expected end boundary after $FunctionName implementation."

    return $cpp.Substring($match.Index, $match.Length + $nextMatch.Index)
}

function Assert-GuardBeforeOperation {
    param(
        [Parameter(Mandatory = $true)][string]$Body,
        [Parameter(Mandatory = $true)][string]$FunctionName,
        [Parameter(Mandatory = $true)][string]$Operation
    )

    $worldGuardIndex = $Body.IndexOf("IsWorldSafeForWidgetCreation(World)")
    $serverGuardIndex = $Body.IndexOf("IsServerLikeRuntime(World)")
    $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $serverGuardIndex))
    $operationIndex = $Body.IndexOf($Operation)

    Assert-True ($worldGuardIndex -ge 0) "Expected $FunctionName to verify the world is safe before $Operation."
    Assert-True ($serverGuardIndex -ge 0) "Expected $FunctionName to no-op in server-like runtime before $Operation."
    Assert-True ($returnIndex -gt $serverGuardIndex) "Expected $FunctionName runtime guard to return before $Operation."
    Assert-True ($operationIndex -gt $returnIndex) "Expected $FunctionName to run $Operation only after runtime guards."
}

$ensureFlowBody = Get-FunctionBody `
    "template<typename WidgetType>\s*WidgetType\*\s+UDBAGameUIManager::EnsureFlowWidgetCreated\s*\([^)]*\)\s*\{" `
    "`ntemplate<|`nvoid\s+UDBAGameUIManager::ScheduleFlowWidgetRefreshRetry" `
    "EnsureFlowWidgetCreated"
Assert-GuardBeforeOperation $ensureFlowBody "EnsureFlowWidgetCreated" "CreateWidget<WidgetType>"

$setFlowBody = Get-FunctionBody `
    "void\s+UDBAGameUIManager::SetFlowWidgetVisible\s*\(\s*UUserWidget\*\s+WidgetToShow\s*\)\s*\{" `
    "`nvoid\s+UDBAGameUIManager::ShowSplashVideo" `
    "SetFlowWidgetVisible"
Assert-GuardBeforeOperation $setFlowBody "SetFlowWidgetVisible" "AddToViewport"
Assert-True ($setFlowBody.IndexOf("ApplyFrontendInputMode(World, WidgetToShow)") -gt $setFlowBody.IndexOf("IsServerLikeRuntime(World)")) `
    "Expected SetFlowWidgetVisible to apply input mode only through the guarded World."

$splashBody = Get-FunctionBody `
    "void\s+UDBAGameUIManager::ShowSplashVideo\s*\(\s*\)\s*\{" `
    "`nvoid\s+UDBAGameUIManager::HideSplashVideo" `
    "ShowSplashVideo"
Assert-GuardBeforeOperation $splashBody "ShowSplashVideo" "CreateWidget<UDBASplashVideoWidget>"
Assert-True ($splashBody.IndexOf("ApplySplashInputMode(World, SplashVideoWidget)") -gt $splashBody.IndexOf("IsServerLikeRuntime(World)")) `
    "Expected ShowSplashVideo to apply splash input mode only through the guarded World."

$bgmBody = Get-FunctionBody `
    "void\s+UDBAGameUIManager::EnsureLoginFlowBackgroundMusic\s*\(\s*\)\s*\{" `
    "`nvoid\s+UDBAGameUIManager::StopLoginFlowBackgroundMusic" `
    "EnsureLoginFlowBackgroundMusic"
Assert-GuardBeforeOperation $bgmBody "EnsureLoginFlowBackgroundMusic" "UGameplayStatics::SpawnSound2D"
Assert-True ($bgmBody.IndexOf("SpawnSound2D(World, LoginFlowBackgroundMusicSound") -gt $bgmBody.IndexOf("IsServerLikeRuntime(World)")) `
    "Expected EnsureLoginFlowBackgroundMusic to spawn UI audio only through the guarded World."

Write-Host "PASS: Game UI manager login flow server boundary" -ForegroundColor Green
