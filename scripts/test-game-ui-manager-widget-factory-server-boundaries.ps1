<#
Validates DBAGameUIManager protected widget factories no-op before CreateWidget in server-like runtimes.
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
    param([Parameter(Mandatory = $true)][string]$FunctionName)

    $pattern = "void\s+UDBAGameUIManager::$FunctionName\s*\(\s*\)\s*\{"
    $match = [regex]::Match($cpp, $pattern)
    Assert-True $match.Success "Expected $FunctionName implementation."

    $nextMatch = [regex]::Match($cpp.Substring($match.Index + 1), "`nvoid\s+UDBAGameUIManager::")
    if ($nextMatch.Success) {
        $end = $match.Index + 1 + $nextMatch.Index
        return $cpp.Substring($match.Index, $end - $match.Index)
    }

    return $cpp.Substring($match.Index)
}

$factoryFunctions = @(
    "CreateMainLobbyWidget",
    "CreateGameSettingsWidget",
    "CreateInventoryWidget",
    "CreatePartyPanelWidget",
    "CreateInvitePanelWidget",
    "CreateQueueModeSelectWidget",
    "CreateQueueStatusWidget",
    "CreateReadyCheckWidget",
    "CreateMatchFoundWidget",
    "CreatePortalConfirmWidget",
    "CreateInteractionPromptWidget",
    "CreateNewbieVillageMainWidget",
    "CreateNewbieTaskTrackerWidget"
)

foreach ($functionName in $factoryFunctions) {
    $body = Get-FunctionBody $functionName
    $guardIndex = $body.IndexOf("IsWorldSafeForWidgetCreation(World)")
    $serverGuardIndex = $body.IndexOf("IsServerLikeRuntime(World)")
    $returnIndex = $body.IndexOf("return")
    $createIndex = $body.IndexOf("CreateWidget<")

    Assert-True ($guardIndex -ge 0) "Expected $functionName to verify the world is safe before widget creation."
    Assert-True ($serverGuardIndex -ge 0) "Expected $functionName to no-op in server-like runtime."
    Assert-True ($returnIndex -gt $serverGuardIndex) "Expected $functionName server-like guard to return before widget creation."
    Assert-True ($createIndex -gt $returnIndex) "Expected $functionName to run CreateWidget only after runtime guards."
}

Write-Host "PASS: Game UI manager widget factory server boundaries" -ForegroundColor Green
