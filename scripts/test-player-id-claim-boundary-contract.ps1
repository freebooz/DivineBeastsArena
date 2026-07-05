param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

$endpointRoot = Join-Path $RepoRoot "DBA_GameBackend\Game.Api\Endpoints"
if (-not (Test-Path $endpointRoot)) {
    throw "Endpoint root not found: $endpointRoot"
}

$forbiddenPattern = 'FindFirst\("player_id"\)\s*\?\?\s*ctx\.User\.FindFirst\(ClaimTypes\.NameIdentifier\)'
$matches = Get-ChildItem -Path $endpointRoot -Recurse -Filter *.cs |
    Where-Object { $_.FullName -notmatch '\\Endpoints\\Auth\\|\\Endpoints\\Admin\\' } |
    Select-String -Pattern $forbiddenPattern

if ($matches) {
    $summary = ($matches | ForEach-Object {
        "$($_.Path.Substring($RepoRoot.Length + 1)):$($_.LineNumber)"
    }) -join ", "
    throw "Player-scoped endpoints must not fall back from player_id to account NameIdentifier: $summary"
}

Write-Host "PASS: player_id claim boundary contract"
