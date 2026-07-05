<#
Writes a human-readable release readiness report from production evidence.

Examples:
  .\scripts\write-release-readiness-report.ps1
  .\scripts\write-release-readiness-report.ps1 -RequireReady
#>

[CmdletBinding()]
param(
    [string]$EvidenceRoot = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence"),
    [string]$ManifestPath = "",
    [string]$OutputJsonPath = "",
    [string]$OutputMarkdownPath = "",
    [switch]$RequireReady
)

$ErrorActionPreference = "Stop"

function Resolve-DefaultPath {
    param(
        [Parameter(Mandatory = $true)][string]$BasePath,
        [Parameter(Mandatory = $true)][string]$ChildPath
    )

    return $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath((Join-Path $BasePath $ChildPath))
}

function Escape-MarkdownCell {
    param([string]$Value)

    if ($null -eq $Value) {
        return ""
    }

    return ($Value -replace "\|", "\|") -replace "`r?`n", " "
}

$resolvedEvidenceRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EvidenceRoot)
if ([string]::IsNullOrWhiteSpace($ManifestPath)) {
    $ManifestPath = Resolve-DefaultPath -BasePath $resolvedEvidenceRoot -ChildPath "production-evidence-manifest.json"
}
if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath $resolvedEvidenceRoot -ChildPath "release-readiness-report.json"
}
if ([string]::IsNullOrWhiteSpace($OutputMarkdownPath)) {
    $OutputMarkdownPath = Resolve-DefaultPath -BasePath $resolvedEvidenceRoot -ChildPath "release-readiness-report.md"
}

if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "production-evidence-manifest.json was not found: $ManifestPath"
}

$manifest = Get-Content -Raw -Encoding UTF8 -LiteralPath $ManifestPath | ConvertFrom-Json
$requirements = @($manifest.requirements)
$blockingRequirements = @($requirements | Where-Object { $_.status -ne "present" })
$readyForRelease = $blockingRequirements.Count -eq 0
$releaseBlockerPosturePath = Resolve-DefaultPath -BasePath $resolvedEvidenceRoot -ChildPath "release-blockers-external-only-validation.json"
$releaseBlockerPosture = $null
if (Test-Path -LiteralPath $releaseBlockerPosturePath) {
    $postureDocument = Get-Content -Raw -Encoding UTF8 -LiteralPath $releaseBlockerPosturePath | ConvertFrom-Json
    $postureKind = [string]$postureDocument.kind
    $postureReleaseId = [string]$postureDocument.releaseId
    $postureBlockerCount = [int]$postureDocument.blockerCount
    $postureExternalBlockerCount = [int]$postureDocument.externalBlockerCount
    $postureKindIsValid = $postureKind -eq "release-blockers-external-only-validation"
    $postureReleaseIdMatchesManifest = -not [string]::IsNullOrWhiteSpace($postureReleaseId) -and $postureReleaseId -eq [string]$manifest.releaseId
    $postureBlockerCountMatchesManifest = $postureBlockerCount -eq $blockingRequirements.Count -and $postureExternalBlockerCount -eq $blockingRequirements.Count
    $releaseBlockerPosture = [ordered]@{
        kind = $postureKind
        path = $releaseBlockerPosturePath
        releaseId = $postureReleaseId
        kindIsValid = $postureKindIsValid
        releaseIdMatchesManifest = $postureReleaseIdMatchesManifest
        blockerCountMatchesManifest = $postureBlockerCountMatchesManifest
        externalOnly = [bool]$postureDocument.externalOnly
        blockerCount = $postureBlockerCount
        externalBlockerCount = $postureExternalBlockerCount
        externalBlockers = @($postureDocument.externalBlockers)
        localAutomationBlockerCount = [int]$postureDocument.localAutomationBlockerCount
        localAutomationBlockers = @($postureDocument.localAutomationBlockers)
        emptyExternalInputBlockerCount = [int]$postureDocument.emptyExternalInputBlockerCount
        emptyExternalInputBlockers = @($postureDocument.emptyExternalInputBlockers)
    }
}
$developmentContinuationReady = $readyForRelease -or (
    $null -ne $releaseBlockerPosture -and
    $releaseBlockerPosture.kindIsValid -and
    $releaseBlockerPosture.releaseIdMatchesManifest -and
    $releaseBlockerPosture.blockerCountMatchesManifest -and
    $releaseBlockerPosture.externalOnly -and
    $releaseBlockerPosture.localAutomationBlockerCount -eq 0 -and
    $releaseBlockerPosture.emptyExternalInputBlockerCount -eq 0
)

$report = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-readiness-report"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    readyForRelease = $readyForRelease
    developmentContinuationReady = $developmentContinuationReady
    releaseId = $manifest.releaseId
    manifestPath = $ManifestPath
    evidenceRoot = $manifest.evidenceRoot
    gitCommit = $manifest.gitCommit
    gitIsDirty = $manifest.gitIsDirty
    requirementCount = $requirements.Count
    presentRequirementCount = @($requirements | Where-Object { $_.status -eq "present" }).Count
    blockingRequirementCount = $blockingRequirements.Count
    releaseBlockerPosture = $releaseBlockerPosture
    blockingRequirements = @($blockingRequirements | ForEach-Object {
        [ordered]@{
            key = $_.key
            status = $_.status
            description = $_.description
            fileCount = $_.fileCount
            files = $_.files
        }
    })
    requirements = @($requirements | ForEach-Object {
        [ordered]@{
            key = $_.key
            status = $_.status
            description = $_.description
            fileCount = $_.fileCount
            files = $_.files
        }
    })
}

$jsonDirectory = Split-Path -Parent $OutputJsonPath
if ($jsonDirectory -and -not (Test-Path -LiteralPath $jsonDirectory)) {
    New-Item -ItemType Directory -Force -Path $jsonDirectory | Out-Null
}
$markdownDirectory = Split-Path -Parent $OutputMarkdownPath
if ($markdownDirectory -and -not (Test-Path -LiteralPath $markdownDirectory)) {
    New-Item -ItemType Directory -Force -Path $markdownDirectory | Out-Null
}

$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# Release readiness report")
$lines.Add("")
$lines.Add("- kind: release-readiness-report")
$lines.Add("- releaseId: $($manifest.releaseId)")
$lines.Add("- generatedAtUtc: $($report.generatedAtUtc)")
$lines.Add("- readyForRelease: $readyForRelease")
$lines.Add("- developmentContinuationReady: $developmentContinuationReady")
$lines.Add("- manifest: $ManifestPath")
$lines.Add("- evidenceRoot: $($manifest.evidenceRoot)")
$lines.Add("- gitCommit: $($manifest.gitCommit)")
$lines.Add("- gitIsDirty: $($manifest.gitIsDirty)")
$lines.Add("- presentRequirements: $($report.presentRequirementCount) / $($report.requirementCount)")
$lines.Add("- blockingRequirements: $($blockingRequirements.Count)")
if ($null -ne $releaseBlockerPosture) {
    $lines.Add("- externalOnlyReleaseBlockers: $($releaseBlockerPosture.externalOnly)")
    $lines.Add("- releaseBlockerPostureReleaseIdMatchesManifest: $($releaseBlockerPosture.releaseIdMatchesManifest)")
    $lines.Add("- releaseBlockerPostureBlockerCountMatchesManifest: $($releaseBlockerPosture.blockerCountMatchesManifest)")
    $lines.Add("- externalBlockers: $($releaseBlockerPosture.externalBlockerCount)")
    $lines.Add("- localAutomationBlockers: $($releaseBlockerPosture.localAutomationBlockerCount)")
    $lines.Add("- emptyExternalInputBlockers: $($releaseBlockerPosture.emptyExternalInputBlockerCount)")
}
$lines.Add("")

if ($blockingRequirements.Count -gt 0) {
    $lines.Add("## Missing or incomplete production evidence")
    $lines.Add("")
    $lines.Add("| Key | Status | Files | Description |")
    $lines.Add("| --- | --- | ---: | --- |")
    foreach ($requirement in $blockingRequirements) {
        $lines.Add("| $(Escape-MarkdownCell $requirement.key) | $(Escape-MarkdownCell $requirement.status) | $($requirement.fileCount) | $(Escape-MarkdownCell $requirement.description) |")
    }
    $lines.Add("")
}

$lines.Add("## All production evidence requirements")
$lines.Add("")
$lines.Add("| Key | Status | Files | Description |")
$lines.Add("| --- | --- | ---: | --- |")
foreach ($requirement in $requirements) {
    $lines.Add("| $(Escape-MarkdownCell $requirement.key) | $(Escape-MarkdownCell $requirement.status) | $($requirement.fileCount) | $(Escape-MarkdownCell $requirement.description) |")
}
$lines.Add("")
$lines.Add("## Evidence file index")
$lines.Add("")
foreach ($file in @($manifest.files | Sort-Object path)) {
    $filePath = [string]$file.path
    $fileSha256 = [string]$file.sha256
    $lines.Add("- ``$filePath`` ($($file.sizeBytes) bytes, sha256 ``$fileSha256``)")
}

$lines | Set-Content -LiteralPath $OutputMarkdownPath -Encoding UTF8

Write-Host ("Release readiness report written: {0}" -f $OutputMarkdownPath)
Write-Host ("Release readiness JSON written: {0}" -f $OutputJsonPath)

if ($blockingRequirements.Count -gt 0) {
    Write-Host "Missing or incomplete production evidence:" -ForegroundColor Yellow
    foreach ($requirement in $blockingRequirements) {
        Write-Host ("- {0} [{1}]" -f $requirement.key, $requirement.status) -ForegroundColor Yellow
    }

    if ($RequireReady) {
        throw ("Release readiness report found {0} blockingRequirements." -f $blockingRequirements.Count)
    }
}
else {
    Write-Host "Release readiness report: readyForRelease=true" -ForegroundColor Green
}
