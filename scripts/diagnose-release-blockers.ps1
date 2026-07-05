<#
Writes an actionable release-blocker report from release-readiness-report.json.

Examples:
  .\scripts\diagnose-release-blockers.ps1
  .\scripts\diagnose-release-blockers.ps1 -ReportPath .\Artifacts\ProductionEvidence\release-readiness-report.json
#>

[CmdletBinding()]
param(
    [string]$ReportPath = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence\release-readiness-report.json"),
    [string]$OutputJsonPath = "",
    [string]$OutputMarkdownPath = "",
    [int]$MaxObservedReasonsPerBlocker = 5
)

$ErrorActionPreference = "Stop"
$RequiredAiShowcaseAutomationTestCount = 5

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

function Add-Reason {
    param(
        [System.Collections.Generic.List[string]]$Reasons,
        [string]$Reason
    )

    if (-not [string]::IsNullOrWhiteSpace($Reason) -and -not $Reasons.Contains($Reason)) {
        $Reasons.Add($Reason)
    }
}

function Get-JsonEvidenceDocuments {
    param(
        [Parameter(Mandatory = $true)]$Requirement,
        [Parameter(Mandatory = $true)][string]$EvidenceRoot
    )

    foreach ($relativePath in @($Requirement.files)) {
        if ([string]::IsNullOrWhiteSpace([string]$relativePath) -or -not ([string]$relativePath).EndsWith(".json", [System.StringComparison]::OrdinalIgnoreCase)) {
            continue
        }

        $evidencePath = Join-Path $EvidenceRoot ([string]$relativePath)
        if (-not (Test-Path -LiteralPath $evidencePath)) {
            continue
        }

        $evidenceItem = Get-Item -LiteralPath $evidencePath
        try {
            $document = Get-Content -Raw -Encoding UTF8 -LiteralPath $evidencePath | ConvertFrom-Json
            [pscustomobject]@{
                Path = [string]$relativePath
                ModifiedAtUtc = $evidenceItem.LastWriteTimeUtc
                Document = $document
            }
        }
        catch {
            [pscustomobject]@{
                Path = [string]$relativePath
                ModifiedAtUtc = $evidenceItem.LastWriteTimeUtc
                Document = $null
            }
        }
    }
}

function Get-ObservedReasons {
    param(
        [Parameter(Mandatory = $true)]$Requirement,
        [Parameter(Mandatory = $true)][string]$EvidenceRoot
    )

    $reasons = New-Object System.Collections.Generic.List[string]
    $key = [string]$Requirement.key
    $documents = @(Get-JsonEvidenceDocuments -Requirement $Requirement -EvidenceRoot $EvidenceRoot | Sort-Object ModifiedAtUtc -Descending)

    foreach ($entry in $documents) {
        $doc = $entry.Document
        if ($null -eq $doc) {
            Add-Reason -Reasons $reasons -Reason ("{0}: JSON evidence could not be parsed." -f $entry.Path)
            continue
        }

        if ($key -eq "client.release_prerequisites") {
            if ($null -ne $doc.readyForReleaseInputs) {
                Add-Reason -Reasons $reasons -Reason ("{0}: readyForReleaseInputs={1}" -f $entry.Path, $doc.readyForReleaseInputs)
            }
            foreach ($issue in @($doc.blockingIssues)) {
                if (-not [string]::IsNullOrWhiteSpace([string]$issue.code) -or -not [string]::IsNullOrWhiteSpace([string]$issue.message)) {
                    Add-Reason -Reasons $reasons -Reason ("{0}: {1} - {2}" -f $entry.Path, $issue.code, $issue.message)
                }
            }
        }
        elseif ($key -eq "client.package_launcher") {
            if ($null -ne $doc.releaseReady) {
                Add-Reason -Reasons $reasons -Reason ("{0}: releaseReady={1}" -f $entry.Path, $doc.releaseReady)
            }
            foreach ($note in @($doc.releaseReadinessNotes)) {
                if (-not [string]::IsNullOrWhiteSpace([string]$note)) {
                    Add-Reason -Reasons $reasons -Reason ("{0}: {1}" -f $entry.Path, $note)
                }
            }
        }
        elseif ($key -eq "client.cdn_launcher_smoke") {
            if ($null -ne $doc.cdnReady) {
                Add-Reason -Reasons $reasons -Reason ("{0}: cdnReady={1}" -f $entry.Path, $doc.cdnReady)
            }
            foreach ($propertyName in @("manifestUrlIsHttps", "downloadUrlIsHttps", "manifestUrlIsExample", "downloadUrlIsExample", "AllowLocalHttp")) {
                if ($doc.PSObject.Properties.Name -contains $propertyName) {
                    Add-Reason -Reasons $reasons -Reason ("{0}: {1}={2}" -f $entry.Path, $propertyName, $doc.$propertyName)
                }
            }
        }
        elseif ($key -eq "client.code_signing") {
            if ($null -ne $doc.signingReady) {
                Add-Reason -Reasons $reasons -Reason ("{0}: signingReady={1}" -f $entry.Path, $doc.signingReady)
            }
            foreach ($propertyName in @("signableFileCount", "signedFileCount", "trustedSignedFileCount", "unsignedFileCount", "invalidSignedFileCount")) {
                if ($doc.PSObject.Properties.Name -contains $propertyName) {
                    Add-Reason -Reasons $reasons -Reason ("{0}: {1}={2}" -f $entry.Path, $propertyName, $doc.$propertyName)
                }
            }
            foreach ($note in @($doc.signingReadinessNotes)) {
                if (-not [string]::IsNullOrWhiteSpace([string]$note)) {
                    Add-Reason -Reasons $reasons -Reason ("{0}: {1}" -f $entry.Path, $note)
                }
            }
        }
        elseif ($key -eq "unreal.ai_showcase_automation") {
            if ($null -ne $doc.automationReady) {
                Add-Reason -Reasons $reasons -Reason ("{0}: automationReady={1}" -f $entry.Path, $doc.automationReady)
            }
            foreach ($propertyName in @("logErrorCount", "logWarningCount", "requestedTestCount", "passedTestCount")) {
                if ($doc.PSObject.Properties.Name -contains $propertyName) {
                    Add-Reason -Reasons $reasons -Reason ("{0}: {1}={2}" -f $entry.Path, $propertyName, $doc.$propertyName)
                }
            }
            if (-not [string]::IsNullOrWhiteSpace([string]$doc.testFilter)) {
                Add-Reason -Reasons $reasons -Reason ("{0}: testFilter={1}" -f $entry.Path, $doc.testFilter)
            }
        }
    }

    if ($reasons.Count -eq 0) {
        Add-Reason -Reasons $reasons -Reason "No parseable blocker-specific evidence details were found; inspect the listed files or regenerate evidence."
    }

    return @($reasons)
}

function Get-LatestEvidencePath {
    param(
        [Parameter(Mandatory = $true)]$Requirement,
        [Parameter(Mandatory = $true)][string]$EvidenceRoot
    )

    $latestEvidence = @(Get-JsonEvidenceDocuments -Requirement $Requirement -EvidenceRoot $EvidenceRoot | Sort-Object ModifiedAtUtc -Descending | Select-Object -First 1)
    if ($latestEvidence.Count -eq 0) {
        return ""
    }

    return [string]$latestEvidence[0].Path
}

function Get-MissingExternalInputs {
    param([Parameter(Mandatory = $true)][string]$RequirementKey)

    if ($RequirementKey -eq "client.release_prerequisites") {
        return @(
            "real HTTPS CDN download URL",
            "real HTTPS CDN manifest URL",
            "release package root",
            "trusted Authenticode signing identity",
            "signtool path"
        )
    }
    if ($RequirementKey -eq "client.package_launcher") {
        return @(
            "public Shipping package root",
            "real HTTPS CDN download URL",
            "real HTTPS CDN manifest URL"
        )
    }
    if ($RequirementKey -eq "client.cdn_launcher_smoke") {
        return @(
            "real HTTPS CDN manifest URL",
            "local smoke install root"
        )
    }
    if ($RequirementKey -eq "client.code_signing") {
        return @(
            "trusted Authenticode signing identity",
            "timestamp URL",
            "public package root containing signable binaries"
        )
    }

    return @()
}

function Get-InputResolutionHints {
    param($InputNames)

    $hints = New-Object System.Collections.Generic.List[object]
    $hintTemplates = @{
        "real HTTPS CDN download URL" = [ordered]@{
            parameters = @("-DownloadUrl")
            environmentVariables = @()
        }
        "real HTTPS CDN manifest URL" = [ordered]@{
            parameters = @("-ManifestUrl")
            environmentVariables = @()
        }
        "release package root" = [ordered]@{
            parameters = @("-PackageRoot")
            environmentVariables = @()
        }
        "public Shipping package root" = [ordered]@{
            parameters = @("-PackageRoot", "-StagedPackageRoot")
            environmentVariables = @()
        }
        "trusted Authenticode signing identity" = [ordered]@{
            parameters = @("-CertificateThumbprint", "-CertificateSubject", "-PfxPath")
            environmentVariables = @()
        }
        "signtool path" = [ordered]@{
            parameters = @("-SignToolPath")
            environmentVariables = @("WindowsSdkDir")
        }
        "local smoke install root" = [ordered]@{
            parameters = @("-InstallRoot")
            environmentVariables = @()
        }
        "timestamp URL" = [ordered]@{
            parameters = @("-TimestampUrl")
            environmentVariables = @()
        }
        "public package root containing signable binaries" = [ordered]@{
            parameters = @("-PackageRoot")
            environmentVariables = @()
        }
        "release run id" = [ordered]@{
            parameters = @("-RunId")
            environmentVariables = @("GITHUB_RUN_ID")
        }
    }

    foreach ($rawInputName in @($InputNames)) {
        $inputName = [string]$rawInputName
        $template = if ($hintTemplates.ContainsKey($inputName)) {
            $hintTemplates[$inputName]
        }
        else {
            [ordered]@{
                parameters = @()
                environmentVariables = @()
            }
        }

        $hints.Add([ordered]@{
            input = $inputName
            parameters = @($template.parameters)
            environmentVariables = @($template.environmentVariables)
        })
    }

    return $hints.ToArray()
}

function Get-ReleaseBlockerAction {
    param(
        [Parameter(Mandatory = $true)]$Requirement,
        [Parameter(Mandatory = $true)][string]$EvidenceRoot
    )

    $key = [string]$Requirement.key
    $status = [string]$Requirement.status
    $allObservedReasons = @(Get-ObservedReasons -Requirement $Requirement -EvidenceRoot $EvidenceRoot)
    $visibleObservedReasons = @($allObservedReasons | Select-Object -First $MaxObservedReasonsPerBlocker)
    $latestEvidencePath = if ($allObservedReasons.Count -gt 0 -and ([string]$allObservedReasons[0]).Contains(":")) {
        ([string]$allObservedReasons[0]).Split(":", 2)[0]
    }
    else {
        Get-LatestEvidencePath -Requirement $Requirement -EvidenceRoot $EvidenceRoot
    }
    $missingExternalInputs = @(Get-MissingExternalInputs -RequirementKey $key)
    $automationBlocked = $missingExternalInputs.Count -gt 0
    $blockingExternalInputs = if ($automationBlocked) { $missingExternalInputs } else { @() }
    $hintInputs = if ($key -eq "unreal.ai_showcase_automation") {
        @("release run id")
    }
    else {
        $blockingExternalInputs
    }
    $inputResolutionHints = @(Get-InputResolutionHints -InputNames $hintInputs)

    $knownActions = @{
        "unreal.ai_showcase_automation" = [ordered]@{
            owner = "unreal-automation"
            script = "scripts\run-ai-showcase-automation.ps1"
            nextCommand = ".\scripts\run-ai-showcase-automation.ps1 -EvidenceDir .\Artifacts\ProductionEvidence -RunId <release-run-id>"
            unblockCriteria = "Generate ai-showcase-automation evidence with automationReady=true, logErrorCount=0, requestedTestCount=$RequiredAiShowcaseAutomationTestCount, and passedTestCount=$RequiredAiShowcaseAutomationTestCount for DivineBeastsArena.Showcase.AIShowcase."
        }
        "client.release_prerequisites" = [ordered]@{
            owner = "release-engineering"
            script = "scripts\diagnose-client-release-prerequisites.ps1"
            nextCommand = ".\scripts\diagnose-client-release-prerequisites.ps1 -PackageRoot <release-package-root> -DownloadUrl <real-https-cdn-download-url> -ManifestUrl <real-https-cdn-manifest-url> -RequireManifestUrl -RequireSigningIdentity -CertificateThumbprint <trusted-authenticode-signing-identity> -SignToolPath <signtool-path> -RequireSignTool -FailOnBlockingIssues"
            unblockCriteria = "Generate client-release-prerequisites evidence with readyForReleaseInputs=true using a real HTTPS CDN URL, non-example manifest URL, package root, signing identity, and signtool."
        }
        "client.package_launcher" = [ordered]@{
            owner = "client-release"
            script = "scripts\run-client-release-evidence.ps1"
            nextCommand = ".\scripts\run-client-release-evidence.ps1 -PackageRoot <public-shipping-package-root> -DownloadUrl <real-https-cdn-download-url> -ManifestUrl <real-https-cdn-manifest-url> -BuildConfiguration Shipping"
            unblockCriteria = "Generate client-package-launcher evidence with releaseReady=true, public Shipping package, no debug symbols, and non-example HTTPS CDN URLs."
        }
        "client.cdn_launcher_smoke" = [ordered]@{
            owner = "release-ops"
            script = "scripts\run-launcher-cdn-smoke.ps1"
            nextCommand = ".\scripts\run-launcher-cdn-smoke.ps1 -ManifestUrl <real-https-cdn-manifest-url> -InstallRoot <local-smoke-install-root>"
            unblockCriteria = "Generate launcher CDN smoke evidence with cdnReady=true after downloading every manifest file and validating SHA256, size, and version persistence from real HTTPS CDN."
        }
        "client.code_signing" = [ordered]@{
            owner = "release-security"
            script = "scripts\sign-client-release-package.ps1"
            nextCommand = ".\scripts\sign-client-release-package.ps1 -PackageRoot <public-package-root-containing-signable-binaries> -CertificateThumbprint <trusted-authenticode-signing-identity> -TimestampUrl <timestamp-url> -RequireSigned"
            unblockCriteria = "Generate code-signing evidence with signingReady=true after every public .exe, .dll, .msi, .msix, and .appx has a trusted Authenticode signature."
        }
    }

    if ($knownActions.ContainsKey($key)) {
        $template = $knownActions[$key]
        return [ordered]@{
            key = $key
            status = $status
            owner = $template.owner
            script = $template.script
            nextCommand = $template.nextCommand
            unblockCriteria = $template.unblockCriteria
            latestEvidencePath = $latestEvidencePath
            missingExternalInputs = $missingExternalInputs
            automationBlocked = $automationBlocked
            blockingExternalInputs = $blockingExternalInputs
            inputResolutionHints = $inputResolutionHints
            observedReasonCount = $allObservedReasons.Count
            observedReasons = $visibleObservedReasons
            description = [string]$Requirement.description
            fileCount = [int]$Requirement.fileCount
            files = @($Requirement.files)
        }
    }

    return [ordered]@{
        key = $key
        status = $status
        owner = "unassigned"
        script = ""
        nextCommand = "Inspect release-readiness-report.json and add a blocker-specific release action."
        unblockCriteria = "Provide evidence that marks this requirement present."
        latestEvidencePath = $latestEvidencePath
        missingExternalInputs = $missingExternalInputs
        automationBlocked = $automationBlocked
        blockingExternalInputs = $blockingExternalInputs
        inputResolutionHints = $inputResolutionHints
        observedReasonCount = $allObservedReasons.Count
        observedReasons = $visibleObservedReasons
        description = [string]$Requirement.description
        fileCount = [int]$Requirement.fileCount
        files = @($Requirement.files)
    }
}

$resolvedReportPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ReportPath)
if (-not (Test-Path -LiteralPath $resolvedReportPath)) {
    throw "release-readiness-report.json was not found: $resolvedReportPath"
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedReportPath) -ChildPath "release-blocker-actions.json"
}
if ([string]::IsNullOrWhiteSpace($OutputMarkdownPath)) {
    $OutputMarkdownPath = Resolve-DefaultPath -BasePath (Split-Path -Parent $resolvedReportPath) -ChildPath "release-blocker-actions.md"
}

$report = Get-Content -Raw -Encoding UTF8 -LiteralPath $resolvedReportPath | ConvertFrom-Json
$reportDirectory = Split-Path -Parent $resolvedReportPath
$evidenceRoot = if (-not [string]::IsNullOrWhiteSpace([string]$report.evidenceRoot)) {
    $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath([string]$report.evidenceRoot)
}
else {
    $reportDirectory
}
$blockingRequirements = @($report.blockingRequirements)
$actions = @($blockingRequirements | ForEach-Object { Get-ReleaseBlockerAction -Requirement $_ -EvidenceRoot $evidenceRoot })

$actionReport = [ordered]@{
    schemaVersion = "1.0"
    kind = "release-blocker-actions"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    releaseId = $report.releaseId
    readyForRelease = [bool]$report.readyForRelease
    reportPath = $resolvedReportPath
    evidenceRoot = $evidenceRoot
    blockerCount = $actions.Count
    actions = $actions
}

$jsonDirectory = Split-Path -Parent $OutputJsonPath
if ($jsonDirectory -and -not (Test-Path -LiteralPath $jsonDirectory)) {
    New-Item -ItemType Directory -Force -Path $jsonDirectory | Out-Null
}
$markdownDirectory = Split-Path -Parent $OutputMarkdownPath
if ($markdownDirectory -and -not (Test-Path -LiteralPath $markdownDirectory)) {
    New-Item -ItemType Directory -Force -Path $markdownDirectory | Out-Null
}

$actionReport | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputJsonPath -Encoding UTF8

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# Release blocker actions")
$lines.Add("")
$lines.Add("- kind: release-blocker-actions")
$lines.Add("- releaseId: $($report.releaseId)")
$lines.Add("- generatedAtUtc: $($actionReport.generatedAtUtc)")
$lines.Add("- readyForRelease: $($actionReport.readyForRelease)")
$lines.Add("- blockerCount: $($actions.Count)")
$lines.Add("")

if ($actions.Count -gt 0) {
    $lines.Add("| Key | Status | Owner | Script | Next command |")
    $lines.Add("| --- | --- | --- | --- | --- |")
    foreach ($action in $actions) {
        $lines.Add("| $(Escape-MarkdownCell $action.key) | $(Escape-MarkdownCell $action.status) | $(Escape-MarkdownCell $action.owner) | $(Escape-MarkdownCell $action.script) | ``$(Escape-MarkdownCell $action.nextCommand)`` |")
    }
    $lines.Add("")
    $lines.Add("## Unblock criteria")
    $lines.Add("")
    foreach ($action in $actions) {
        $lines.Add("- ``$($action.key)``: $($action.unblockCriteria)")
    }
    $lines.Add("")
    $lines.Add("## Automation status")
    $lines.Add("")
    foreach ($action in $actions) {
        $lines.Add("### $($action.key)")
        $lines.Add("- automationBlocked: $($action.automationBlocked)")
        foreach ($inputName in @($action.blockingExternalInputs)) {
            $lines.Add("- blockedBy: $inputName")
        }
        if (@($action.blockingExternalInputs).Count -eq 0) {
            $lines.Add("- blockedBy: none")
        }
        $lines.Add("")
    }
    $lines.Add("## Input resolution hints")
    $lines.Add("")
    foreach ($action in $actions) {
        $lines.Add("### $($action.key)")
        foreach ($hint in @($action.inputResolutionHints)) {
            $parameters = if (@($hint.parameters).Count -gt 0) { @($hint.parameters) -join ", " } else { "none" }
            $environmentVariables = if (@($hint.environmentVariables).Count -gt 0) { @($hint.environmentVariables) -join ", " } else { "none" }
            $lines.Add("- $($hint.input): parameters: $parameters; env: $environmentVariables")
        }
        if (@($action.inputResolutionHints).Count -eq 0) {
            $lines.Add("- No input resolution hints are defined.")
        }
        $lines.Add("")
    }
    $lines.Add("## Missing external inputs")
    $lines.Add("")
    foreach ($action in $actions) {
        $lines.Add("### $($action.key)")
        if (-not [string]::IsNullOrWhiteSpace([string]$action.latestEvidencePath)) {
            $lines.Add("- latest evidence: $($action.latestEvidencePath)")
        }
        foreach ($inputName in @($action.missingExternalInputs)) {
            $lines.Add("- $inputName")
        }
        if (@($action.missingExternalInputs).Count -eq 0) {
            $lines.Add("- No blocker-specific external input template is defined yet.")
        }
        $lines.Add("")
    }
    $lines.Add("## Observed reasons")
    $lines.Add("")
    foreach ($action in $actions) {
        $lines.Add("### $($action.key)")
        if ($action.observedReasonCount -gt @($action.observedReasons).Count) {
            $lines.Add("_showing $(@($action.observedReasons).Count) of $($action.observedReasonCount) observed reasons_")
            $lines.Add("")
        }
        foreach ($reason in @($action.observedReasons)) {
            $lines.Add("- $(Escape-MarkdownCell $reason)")
        }
        $lines.Add("")
    }
}
else {
    $lines.Add("No release blockers. The release readiness report is ready.")
}

$lines | Set-Content -LiteralPath $OutputMarkdownPath -Encoding UTF8

Write-Host ("Release blocker action JSON written: {0}" -f $OutputJsonPath)
Write-Host ("Release blocker action report written: {0}" -f $OutputMarkdownPath)
if ($actions.Count -gt 0) {
    Write-Host ("Release blocker actions: {0}" -f $actions.Count) -ForegroundColor Yellow
}
else {
    Write-Host "Release blocker actions: none" -ForegroundColor Green
}
