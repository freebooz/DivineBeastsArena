<#
Collects production release evidence files into a signed-by-hash manifest.

Examples:
  .\scripts\collect-production-evidence.ps1
  .\scripts\collect-production-evidence.ps1 -EvidenceRoot .\Artifacts\ProductionEvidence -RequireAll
#>

[CmdletBinding()]
param(
    [string]$EvidenceRoot = (Join-Path $PSScriptRoot "..\Artifacts\ProductionEvidence"),
    [string]$OutputPath,
    [string]$ReleaseId,
    [switch]$RequireAll
)

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

if (-not $ReleaseId) {
    $ReleaseId = "local-{0:yyyyMMddTHHmmssZ}" -f (Get-Date).ToUniversalTime()
}

if (-not $OutputPath) {
    $OutputPath = Join-Path $EvidenceRoot "production-evidence-manifest.json"
}

if (-not (Test-Path -LiteralPath $EvidenceRoot)) {
    New-Item -ItemType Directory -Path $EvidenceRoot -Force | Out-Null
}

$resolvedEvidenceRoot = (Resolve-Path -LiteralPath $EvidenceRoot).Path
$outputDirectory = Split-Path -Parent $OutputPath
if ($outputDirectory -and -not (Test-Path -LiteralPath $outputDirectory)) {
    New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
}

$resolvedOutputPath = $null
if (Test-Path -LiteralPath $OutputPath) {
    $resolvedOutputPath = (Resolve-Path -LiteralPath $OutputPath).Path
}

$RequiredAiShowcaseAutomationTestCount = 5

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    $builder = [System.Text.StringBuilder]::new()
    foreach ($codePoint in $CodePoints) {
        [void]$builder.Append([char]$codePoint)
    }
    return $builder.ToString()
}

$manifestWrittenMessage = New-TextFromCodePoints @(29983, 20135, 35777, 25454, 28165, 21333, 24050, 20889, 20837, 65306, 123, 48, 125)
$missingEvidenceTitleMessage = New-TextFromCodePoints @(32570, 22833, 25110, 26410, 23436, 25104, 30340, 29983, 20135, 35777, 25454, 20998, 31867, 65306)
$allEvidencePresentMessage = New-TextFromCodePoints @(25152, 26377, 29983, 20135, 35777, 25454, 20998, 31867, 22343, 24050, 23601, 32490, 12290)
$missingEvidenceFailureMessage = New-TextFromCodePoints @(32570, 22833, 25110, 26410, 23436, 25104, 32, 123, 48, 125, 32, 20010, 24517, 38656, 29983, 20135, 35777, 25454, 20998, 31867, 12290)
$nugetEvidenceDescription = New-TextFromCodePoints @(78, 117, 71, 101, 116, 32, 28431, 27934, 21253, 25253, 21578, 26469, 33258, 32, 115, 101, 99, 117, 114, 105, 116, 121, 45, 99, 105, 32, 25110, 32, 112, 114, 111, 100, 117, 99, 116, 105, 111, 110, 45, 115, 101, 99, 117, 114, 105, 116, 121, 45, 97, 117, 100, 105, 116, 12290)
$npmEvidenceDescription = New-TextFromCodePoints @(65, 100, 109, 105, 110, 12289, 87, 101, 98, 115, 105, 116, 101, 12289, 76, 97, 117, 110, 99, 104, 101, 114, 32, 30340, 29983, 20135, 32, 110, 112, 109, 32, 97, 117, 100, 105, 116, 32, 74, 83, 79, 78, 12290)
$trivyEvidenceDescription = New-TextFromCodePoints @(65, 80, 73, 32, 21644, 32, 87, 111, 114, 107, 101, 114, 32, 23481, 22120, 32, 84, 114, 105, 118, 121, 32, 83, 65, 82, 73, 70, 32, 25110, 25195, 25551, 25253, 21578, 12290)
$k6EvidenceDescription = New-TextFromCodePoints @(107, 54, 32, 30331, 24405, 12289, 21305, 37197, 25110, 19987, 29992, 26381, 21153, 22120, 32534, 25490, 21387, 27979, 36755, 20986, 12290)
$backupRestoreEvidenceDescription = New-TextFromCodePoints @(80, 111, 115, 116, 103, 114, 101, 83, 81, 76, 32, 22791, 20221, 19982, 24674, 22797, 28436, 32451, 35777, 25454, 12290)
$deployRollbackEvidenceDescription = New-TextFromCodePoints @(37096, 32626, 12289, 20882, 28895, 27979, 35797, 21644, 22238, 28378, 35777, 25454, 12290)
$ueOnlineEvidenceDescription = New-TextFromCodePoints @(85, 69, 32, 25171, 21253, 25110, 32534, 36753, 22120, 22312, 32447, 39564, 35777, 35777, 25454, 65292, 35206, 30422, 21518, 31471, 20998, 37197, 12289, 68, 101, 100, 105, 99, 97, 116, 101, 100, 32, 83, 101, 114, 118, 101, 114, 32, 21551, 21160, 21644, 23458, 25143, 31471, 21152, 20837, 12290)
$aiShowcaseEvidenceDescription = New-TextFromCodePoints @(77, 67, 80, 32, 29983, 25104, 32, 65, 73, 95, 83, 104, 111, 119, 99, 97, 115, 101, 32, 36164, 20135, 30340, 32, 85, 73, 47, 86, 70, 88, 32, 33258, 21160, 21270, 22238, 24402, 35777, 25454, 12290)
$clientPrerequisiteEvidenceDescription = New-TextFromCodePoints @(23458, 25143, 31471, 21457, 24067, 36755, 20837, 21069, 32622, 35777, 25454, 65292, 35777, 26126, 21253, 20307, 26681, 30446, 24405, 12289, 29983, 20135, 32, 67, 68, 78, 32, 85, 82, 76, 32, 21644, 31614, 21517, 36755, 20837, 24050, 23601, 32490, 12290)
$clientPackageEvidenceDescription = New-TextFromCodePoints @(23458, 25143, 31471, 21253, 12289, 21551, 21160, 22120, 28165, 21333, 12289, 83, 72, 65, 50, 53, 54, 32, 25991, 20214, 21015, 34920, 21644, 26412, 22320, 23433, 35013, 47, 26356, 26032, 20462, 22797, 20882, 28895, 35777, 25454, 12290)
$cdnSmokeEvidenceDescription = New-TextFromCodePoints @(21551, 21160, 22120, 32, 67, 68, 78, 32, 20882, 28895, 35777, 25454, 65292, 35777, 26126, 28165, 21333, 33719, 21462, 12289, 21253, 19979, 36733, 12289, 83, 72, 65, 50, 53, 54, 32, 26657, 39564, 21644, 26412, 22320, 23433, 35013, 29256, 26412, 25345, 20037, 21270, 12290)
$codeSigningEvidenceDescription = New-TextFromCodePoints @(87, 105, 110, 100, 111, 119, 115, 32, 65, 117, 116, 104, 101, 110, 116, 105, 99, 111, 100, 101, 32, 20195, 30721, 31614, 21517, 35777, 25454, 65292, 35777, 26126, 20844, 24320, 23458, 25143, 31471, 21487, 25191, 34892, 25991, 20214, 21644, 24211, 24050, 21487, 20449, 31614, 21517, 12290)
$launcherInstallEvidenceDescription = New-TextFromCodePoints @(21551, 21160, 22120, 23433, 35013, 47, 26356, 26032, 20882, 28895, 35777, 25454, 65292, 35777, 26126, 28165, 21333, 33719, 21462, 12289, 20462, 22797, 19979, 36733, 12289, 83, 72, 65, 50, 53, 54, 32, 26657, 39564, 21644, 29256, 26412, 25345, 20037, 21270, 12290)
$launcherUiEvidenceDescription = New-TextFromCodePoints @(21551, 21160, 22120, 32, 85, 73, 32, 35270, 35273, 35777, 25454, 65292, 35777, 26126, 29609, 23478, 21487, 35265, 21551, 21160, 22120, 30028, 38754, 21487, 28210, 26579, 24182, 26292, 38706, 23433, 35013, 47, 26356, 26032, 25805, 20316, 12290)

function Get-GitValue {
    param([Parameter(Mandatory = $true)][string[]]$Arguments)

    Push-Location $repoRoot
    try {
        $output = & git @Arguments 2>$null
        if ($LASTEXITCODE -ne 0) {
            return $null
        }
        return (($output | Out-String).Trim())
    }
    finally {
        Pop-Location
    }
}

function Test-RelativePathMatch {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$Patterns
    )

    foreach ($pattern in $Patterns) {
        if ($RelativePath -match $pattern) {
            return $true
        }
    }

    return $false
}

function Test-ClientPackageReleaseReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)client-package-launcher.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $downloadUrlHasHost = if ($json.PSObject.Properties.Name -contains "downloadUrlHasHost") {
                [bool]$json.downloadUrlHasHost
            }
            else {
                $uri = $null
                [System.Uri]::TryCreate([string]$json.downloadUrl, [System.UriKind]::Absolute, [ref]$uri) -and
                    $null -ne $uri -and
                    -not [string]::IsNullOrWhiteSpace($uri.Host)
            }
            $downloadUrlIsExample = if ($json.PSObject.Properties.Name -contains "downloadUrlIsExample") {
                [bool]$json.downloadUrlIsExample
            }
            else {
                [string]$json.downloadUrl -like "https://cdn.example.com/*"
            }
            $downloadUrlIsHttps = if ($json.PSObject.Properties.Name -contains "downloadUrlIsHttps") {
                [bool]$json.downloadUrlIsHttps
            }
            else {
                [string]$json.downloadUrl -match "^https://"
            }
            if ($json.kind -eq "client-package-launcher" -and $json.releaseReady -eq $true -and $downloadUrlHasHost -and $downloadUrlIsHttps -and -not $downloadUrlIsExample) {
                return $true
            }
        }
        catch {
            continue
        }
    }

    return $false
}

function Test-AbsoluteUrlWithHost {
    param([string]$Url)

    $uri = $null
    return [System.Uri]::TryCreate([string]$Url, [System.UriKind]::Absolute, [ref]$uri) -and
        $null -ne $uri -and
        -not [string]::IsNullOrWhiteSpace($uri.Host)
}

function Test-LauncherCdnReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)launcher-cdn-smoke.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $manifestUrl = if ($json.PSObject.Properties.Name -contains "ManifestUrl") { [string]$json.ManifestUrl } else { [string]$json.manifestUrl }
            $downloadUrl = [string]$json.downloadUrl
            $manifestUrlHasHost = Test-AbsoluteUrlWithHost -Url $manifestUrl
            $downloadUrlHasHost = Test-AbsoluteUrlWithHost -Url $downloadUrl
            $manifestUrlIsHttps = if ($json.PSObject.Properties.Name -contains "manifestUrlIsHttps") {
                [bool]$json.manifestUrlIsHttps
            }
            else {
                $manifestUrl -match "^https://"
            }
            $downloadUrlIsHttps = if ($json.PSObject.Properties.Name -contains "downloadUrlIsHttps") {
                [bool]$json.downloadUrlIsHttps
            }
            else {
                $downloadUrl -match "^https://"
            }
            $manifestUrlIsExample = if ($json.PSObject.Properties.Name -contains "manifestUrlIsExample") {
                [bool]$json.manifestUrlIsExample
            }
            else {
                $manifestUrl -like "https://cdn.example.com/*"
            }
            $downloadUrlIsExample = if ($json.PSObject.Properties.Name -contains "downloadUrlIsExample") {
                [bool]$json.downloadUrlIsExample
            }
            else {
                $downloadUrl -like "https://cdn.example.com/*"
            }

            if ($json.kind -eq "launcher-cdn-smoke" -and
                $json.cdnReady -eq $true -and
                $manifestUrlHasHost -and
                $downloadUrlHasHost -and
                $manifestUrlIsHttps -and
                $downloadUrlIsHttps -and
                -not $manifestUrlIsExample -and
                -not $downloadUrlIsExample) {
                return $true
            }
        }
        catch {
            continue
        }
    }

    return $false
}

function Test-CodeSigningReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)code-signing.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $signableFileCount = [int]$json.signableFileCount
            $signedFileCount = [int]$json.signedFileCount
            $trustedSignedFileCount = [int]$json.trustedSignedFileCount
            $unsignedFileCount = [int]$json.unsignedFileCount
            $invalidSignedFileCount = [int]$json.invalidSignedFileCount
            if ($json.kind -eq "code-signing" -and
                $json.signingReady -eq $true -and
                $signableFileCount -gt 0 -and
                $unsignedFileCount -eq 0 -and
                $invalidSignedFileCount -eq 0 -and
                $trustedSignedFileCount -eq $signableFileCount -and
                $signedFileCount -eq $signableFileCount) {
                return $true
            }
        }
        catch {
            continue
        }
    }

    return $false
}

function Get-LauncherInstallUpdateReadyEvidencePaths {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $readyPaths = New-Object System.Collections.Generic.List[string]

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)launcher-install-update-smoke.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $expectedLauncherInstallUpdateTest = "repair_game_downloads_local_package_and_persists_version"
            if ($json.kind -eq "launcher-install-update-smoke" -and
                $json.installUpdateReady -eq $true -and
                $json.hashVerified -eq $true -and
                $json.versionPersisted -eq $true -and
                [int]$json.exitCode -eq 0 -and
                [string]$json.testName -eq $expectedLauncherInstallUpdateTest) {
                $readyPaths.Add($relativePath) | Out-Null

                $jsonLeaf = Split-Path -Leaf ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
                $logLeaf = [System.IO.Path]::ChangeExtension($jsonLeaf, ".log")
                $matchingLogPath = @(
                    $RelativePaths |
                        Where-Object {
                            $_ -match "\.log$" -and
                            $_ -notmatch "\.stderr\.log$" -and
                            (Split-Path -Leaf ($_ -replace "/", [System.IO.Path]::DirectorySeparatorChar)) -eq $logLeaf
                        } |
                        Select-Object -First 1
                )
                if ($matchingLogPath.Count -gt 0) {
                    $readyPaths.Add([string]$matchingLogPath[0]) | Out-Null
                }
            }
        }
        catch {
            continue
        }
    }

    return @($readyPaths | Select-Object -Unique)
}

function Test-LauncherInstallUpdateReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    return @(Get-LauncherInstallUpdateReadyEvidencePaths -EvidenceRoot $EvidenceRoot -RelativePaths $RelativePaths).Count -gt 0
}

function Get-LauncherUiVisualReadyEvidencePaths {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $readyPaths = New-Object System.Collections.Generic.List[string]

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)launcher-ui-visual-evidence.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $requiredMarkers = @($json.requiredMarkers)
            $missingMarkers = @($json.missingMarkers)
            if ($json.kind -eq "launcher-ui-visual-evidence" -and
                $json.uiEvidenceReady -eq $true -and
                $json.screenshotReady -eq $true -and
                $json.uiMarkersReady -eq $true -and
                [int]$json.buildExitCode -eq 0 -and
                $json.previewStarted -eq $true -and
                [int]$json.screenshotExitCode -eq 0 -and
                [int]$json.domExitCode -eq 0 -and
                $requiredMarkers.Count -gt 0 -and
                $missingMarkers.Count -eq 0) {
                $readyPaths.Add($relativePath) | Out-Null

                $jsonLeaf = Split-Path -Leaf ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
                $baseLeaf = [System.IO.Path]::GetFileNameWithoutExtension($jsonLeaf)
                $allowedLeaves = @(
                    "$baseLeaf.png",
                    "$baseLeaf.browser.log",
                    "$baseLeaf.build.log",
                    "$baseLeaf.preview.log"
                )
                foreach ($supportPath in $RelativePaths) {
                    if ($supportPath -match "\.stderr\.log$") {
                        continue
                    }

                    $supportLeaf = Split-Path -Leaf ($supportPath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
                    if ($allowedLeaves -contains $supportLeaf) {
                        $readyPaths.Add([string]$supportPath) | Out-Null
                    }
                }
            }
        }
        catch {
            continue
        }
    }

    return @($readyPaths | Select-Object -Unique)
}

function Test-LauncherUiVisualReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    return @(Get-LauncherUiVisualReadyEvidencePaths -EvidenceRoot $EvidenceRoot -RelativePaths $RelativePaths).Count -gt 0
}

function Test-NpmAuditReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $requiredApps = @("admin", "website", "launcher")
    $cleanApps = @{}

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "\.json$") {
            continue
        }

        $normalizedPath = $relativePath.ToLowerInvariant()
        $appKey = $null
        if ($normalizedPath -match "(^|[-_/])admin([-_/]|\.|$)" -or $normalizedPath -match "dba_gameadmin") {
            $appKey = "admin"
        }
        elseif ($normalizedPath -match "(^|[-_/])website([-_/]|\.|$)" -or $normalizedPath -match "dba_gamewebsite") {
            $appKey = "website"
        }
        elseif ($normalizedPath -match "(^|[-_/])launcher([-_/]|\.|$)" -or $normalizedPath -match "dba_gamelauncher") {
            $appKey = "launcher"
        }
        if ([string]::IsNullOrWhiteSpace($appKey)) {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            if (-not $json.metadata -or -not $json.metadata.vulnerabilities) {
                continue
            }

            $high = [int]$json.metadata.vulnerabilities.high
            $critical = [int]$json.metadata.vulnerabilities.critical
            if ($high -eq 0 -and $critical -eq 0) {
                $cleanApps[$appKey] = $true
            }
        }
        catch {
            continue
        }
    }

    foreach ($requiredApp in $requiredApps) {
        if (-not $cleanApps.ContainsKey($requiredApp)) {
            return $false
        }
    }

    return $true
}

function Test-NuGetVulnerabilityReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)vulnerability-report\.txt$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $reportText = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath
            if ([string]::IsNullOrWhiteSpace($reportText)) {
                continue
            }

            if ($reportText -notmatch "(?i)has the following vulnerable packages|critical|high|moderate|low") {
                return $true
            }
        }
        catch {
            continue
        }
    }

    return $false
}

function Test-TrivyReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $requiredImages = @("api", "worker")
    $cleanImages = @{}

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "\.sarif$") {
            continue
        }

        $normalizedPath = $relativePath.ToLowerInvariant()
        $imageKey = $null
        if ($normalizedPath -match "(^|[-_/])api([-_/]|\.|$)") {
            $imageKey = "api"
        }
        elseif ($normalizedPath -match "(^|[-_/])worker([-_/]|\.|$)") {
            $imageKey = "worker"
        }
        if ([string]::IsNullOrWhiteSpace($imageKey)) {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            if ($json.version -ne "2.1.0" -or -not $json.runs) {
                continue
            }

            $resultCount = 0
            foreach ($run in @($json.runs)) {
                $resultCount += @($run.results).Count
            }
            if ($resultCount -eq 0) {
                $cleanImages[$imageKey] = $true
            }
        }
        catch {
            continue
        }
    }

    foreach ($requiredImage in $requiredImages) {
        if (-not $cleanImages.ContainsKey($requiredImage)) {
            return $false
        }
    }

    return $true
}

function Get-K6ReadyEvidencePaths {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $requiredTests = @("login", "matchmaking")
    $cleanTests = @{}
    $readyPathsByTest = @{}

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "\.json$") {
            continue
        }

        $normalizedPath = $relativePath.ToLowerInvariant()
        $testKey = $null
        if ($normalizedPath -match "(^|[-_/])login([-_/]|\.|$)") {
            $testKey = "login"
        }
        elseif ($normalizedPath -match "(^|[-_/])matchmaking([-_/]|\.|$)") {
            $testKey = "matchmaking"
        }
        elseif ($normalizedPath -match "dedicated-server-orchestration") {
            $testKey = "dedicated-server-orchestration"
        }
        if ([string]::IsNullOrWhiteSpace($testKey)) {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            if (-not $json.metrics) {
                continue
            }

            $checkFails = [int]$json.metrics.checks.fails
            $httpFailedRate = [double]$json.metrics.http_req_failed.value
            $httpRequestCount = [int]$json.metrics.http_reqs.count
            $iterationCount = [int]$json.metrics.iterations.count
            if ($checkFails -eq 0 -and $httpFailedRate -eq 0 -and $httpRequestCount -gt 0 -and $iterationCount -gt 0) {
                $cleanTests[$testKey] = $true
                if (-not $readyPathsByTest.ContainsKey($testKey)) {
                    $paths = New-Object System.Collections.Generic.List[string]
                    $paths.Add($relativePath) | Out-Null

                    $jsonLeaf = Split-Path -Leaf ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
                    $logLeaf = [System.IO.Path]::ChangeExtension($jsonLeaf, ".log")
                    $matchingLogPath = @(
                        $RelativePaths |
                            Where-Object {
                                $_ -match "\.log$" -and
                                (Split-Path -Leaf ($_ -replace "/", [System.IO.Path]::DirectorySeparatorChar)) -eq $logLeaf
                            } |
                            Select-Object -First 1
                    )
                    if ($matchingLogPath.Count -gt 0) {
                        $paths.Add([string]$matchingLogPath[0]) | Out-Null
                    }

                    $readyPathsByTest[$testKey] = $paths
                }
            }
        }
        catch {
            continue
        }
    }

    foreach ($requiredTest in $requiredTests) {
        if (-not $cleanTests.ContainsKey($requiredTest)) {
            return @()
        }
    }

    $readyPaths = New-Object System.Collections.Generic.List[string]
    foreach ($requiredTest in $requiredTests) {
        foreach ($path in @($readyPathsByTest[$requiredTest])) {
            $readyPaths.Add([string]$path) | Out-Null
        }
    }

    return @($readyPaths | Select-Object -Unique)
}

function Test-K6ReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    return @(Get-K6ReadyEvidencePaths -EvidenceRoot $EvidenceRoot -RelativePaths $RelativePaths).Count -gt 0
}

function Get-BackupRestoreReadyEvidencePaths {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $readyPaths = New-Object System.Collections.Generic.List[string]

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)backup-restore-rehearsal.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $publicTableCount = [int]$json.publicTableCount
            if ($json.schemaVersion -eq "1.0" -and
                $json.status -eq "passed" -and
                [int]$json.exitCode -eq 0 -and
                -not [string]::IsNullOrWhiteSpace([string]$json.runId) -and
                -not [string]::IsNullOrWhiteSpace([string]$json.backupFile) -and
                -not [string]::IsNullOrWhiteSpace([string]$json.restoreDatabase) -and
                $publicTableCount -gt 0 -and
                -not [string]::IsNullOrWhiteSpace([string]$json.logFile)) {
                $readyPaths.Add($relativePath) | Out-Null

                $logLeaf = Split-Path -Leaf ([string]$json.logFile)
                if (-not [string]::IsNullOrWhiteSpace($logLeaf)) {
                    $matchingLogPath = @(
                        $RelativePaths |
                            Where-Object {
                                $_ -match "\.log$" -and
                                (Split-Path -Leaf ($_ -replace "/", [System.IO.Path]::DirectorySeparatorChar)) -eq $logLeaf
                            } |
                            Select-Object -First 1
                    )
                    if ($matchingLogPath.Count -gt 0) {
                        $readyPaths.Add([string]$matchingLogPath[0]) | Out-Null
                    }
                }
            }
        }
        catch {
            continue
        }
    }

    return @($readyPaths | Select-Object -Unique)
}

function Test-BackupRestoreReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    return @(Get-BackupRestoreReadyEvidencePaths -EvidenceRoot $EvidenceRoot -RelativePaths $RelativePaths).Count -gt 0
}

function Test-DeployRollbackReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    $requiredChecks = @(
        "live health",
        "ready health",
        "version api",
        "launcher manifest",
        "metrics endpoint"
    )

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)production-smoke-backend.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            if ($json.schemaVersion -ne "1.0" -or
                $json.status -ne "passed" -or
                [int]$json.exitCode -ne 0 -or
                [string]::IsNullOrWhiteSpace([string]$json.runId) -or
                [string]::IsNullOrWhiteSpace([string]$json.baseUrl) -or
                [string]::IsNullOrWhiteSpace([string]$json.logFile)) {
                continue
            }

            $passedChecks = @{}
            foreach ($check in @($json.checks)) {
                $checkName = [string]$check.name
                if ([string]::IsNullOrWhiteSpace($checkName)) {
                    continue
                }
                if ($check.status -eq "passed") {
                    $passedChecks[$checkName] = $true
                }
            }

            $allRequiredChecksPassed = $true
            foreach ($requiredCheck in $requiredChecks) {
                if (-not $passedChecks.ContainsKey($requiredCheck)) {
                    $allRequiredChecksPassed = $false
                    break
                }
            }
            if (-not $allRequiredChecksPassed) {
                continue
            }

            if ($json.guestLogin -eq $true -and -not $passedChecks.ContainsKey("guest login")) {
                continue
            }

            return $true
        }
        catch {
            continue
        }
    }

    return $false
}

function Test-UeOnlineValidationReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)ue-online-validation.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $runtimeOkCount = @($json.safeLogEvidence.runtimePlayerJoinedOk).Count
            $clientATravelCount = @($json.safeLogEvidence.clientATravelCompleted).Count
            $clientBTravelCount = @($json.safeLogEvidence.clientBTravelCompleted).Count
            $hasRuntimeContext = -not [string]::IsNullOrWhiteSpace([string]$json.roomId) -and
                -not [string]::IsNullOrWhiteSpace([string]$json.sessionId) -and
                -not [string]::IsNullOrWhiteSpace([string]$json.serverId) -and
                [int]$json.allocatedPort -gt 0 -and
                [int]$json.clientConnectPort -gt 0 -and
                [int]$json.processIds.server -gt 0 -and
                [int]$json.processIds.clientA -gt 0 -and
                [int]$json.processIds.clientB -gt 0
            if ($json.kind -eq "ue-online-validation" -and $json.status -eq "passed" -and -not [bool]$json.skipClientLaunch -and $hasRuntimeContext -and $runtimeOkCount -ge 2 -and $clientATravelCount -ge 1 -and $clientBTravelCount -ge 1) {
                return $true
            }
        }
        catch {
            continue
        }
    }

    return $false
}

function Test-AiShowcaseAutomationReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)ai-showcase-automation.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $hasRequiredTestCounts = (
                ($json.PSObject.Properties.Name -contains "requestedTestCount") -and
                ($json.PSObject.Properties.Name -contains "passedTestCount") -and
                [int]$json.requestedTestCount -eq $RequiredAiShowcaseAutomationTestCount -and
                [int]$json.passedTestCount -eq $RequiredAiShowcaseAutomationTestCount
            )
            if ($json.kind -eq "ai-showcase-automation" -and $json.automationReady -eq $true -and ($json.PSObject.Properties.Name -contains "logErrorCount") -and [int]$json.logErrorCount -eq 0 -and $hasRequiredTestCounts) {
                return $true
            }
        }
        catch {
            continue
        }
    }

    return $false
}

function Test-ClientReleasePrerequisiteReadyEvidence {
    param(
        [Parameter(Mandatory = $true)][string]$EvidenceRoot,
        [Parameter(Mandatory = $true)][string[]]$RelativePaths
    )

    foreach ($relativePath in $RelativePaths) {
        if ($relativePath -notmatch "(^|/)client-release-prerequisites.*\.json$") {
            continue
        }

        $fullPath = Join-Path $EvidenceRoot ($relativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
        if (-not (Test-Path -LiteralPath $fullPath)) {
            continue
        }

        try {
            $json = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath | ConvertFrom-Json
            $blockingIssues = @($json.blockingIssues)
            $hasNoBlockingIssues = ($json.PSObject.Properties.Name -contains "blockingIssueCount") -and
                [int]$json.blockingIssueCount -eq 0 -and
                $blockingIssues.Count -eq 0
            $hasPackage = -not [string]::IsNullOrWhiteSpace([string]$json.package.clientExePath) -and
                [int]$json.package.fileCount -gt 0
            $downloadUrl = [string]$json.urls.downloadUrl.value
            $downloadUrlReady = -not [string]::IsNullOrWhiteSpace($downloadUrl) -and
                (Test-AbsoluteUrlWithHost -Url $downloadUrl) -and
                [bool]$json.urls.downloadUrl.valid -and
                ([bool]$json.urls.downloadUrl.isHttps -or [bool]$json.urls.downloadUrl.isAllowedLocalHttp) -and
                -not [bool]$json.urls.downloadUrl.isExample
            $manifestUrl = [string]$json.urls.manifestUrl.value
            $manifestUrlReady = -not [string]::IsNullOrWhiteSpace($manifestUrl) -and
                (Test-AbsoluteUrlWithHost -Url $manifestUrl) -and
                [bool]$json.urls.manifestUrl.valid -and
                ([bool]$json.urls.manifestUrl.isHttps -or [bool]$json.urls.manifestUrl.isAllowedLocalHttp) -and
                -not [bool]$json.urls.manifestUrl.isExample
            $signingRequired = [bool]$json.signing.required
            $hasSigningIdentity = [bool]$json.signing.certificateFound -or
                -not [string]::IsNullOrWhiteSpace([string]$json.signing.pfxPath)
            $signingReady = -not $signingRequired -or
                ($hasSigningIdentity -and -not [string]::IsNullOrWhiteSpace([string]$json.signing.signToolPath))

            if ($json.kind -eq "client-release-prerequisites" -and $json.readyForReleaseInputs -eq $true -and $hasNoBlockingIssues -and $hasPackage -and $downloadUrlReady -and $manifestUrlReady -and $signingReady) {
                return $true
            }
        }
        catch {
            continue
        }
    }

    return $false
}

function Get-PortableRelativePath {
    param(
        [Parameter(Mandatory = $true)][string]$RootPath,
        [Parameter(Mandatory = $true)][string]$FilePath
    )

    $normalizedRoot = [System.IO.Path]::GetFullPath($RootPath)
    if (-not $normalizedRoot.EndsWith([System.IO.Path]::DirectorySeparatorChar.ToString())) {
        $normalizedRoot = $normalizedRoot + [System.IO.Path]::DirectorySeparatorChar
    }

    $rootUri = New-Object System.Uri($normalizedRoot)
    $fileUri = New-Object System.Uri([System.IO.Path]::GetFullPath($FilePath))
    return [System.Uri]::UnescapeDataString($rootUri.MakeRelativeUri($fileUri).ToString()).Replace("\", "/")
}

function Test-DerivedEvidenceOutput {
    param([Parameter(Mandatory = $true)][string]$RelativePath)

    $leafName = Split-Path -Leaf ($RelativePath -replace "/", [System.IO.Path]::DirectorySeparatorChar)
    return $leafName -in @(
        "production-evidence-manifest.json",
        "release-readiness-report.json",
        "release-readiness-report.md",
        "release-blocker-actions.json",
        "release-blocker-actions.md",
        "release-blocker-action-validation.json",
        "release-blockers-external-only-validation.json",
        "development-continuation-readiness-validation.json",
        "release-input-template.json",
        "release-input-template.md",
        "release-input-template-validation.json",
        "release-input-values.template.json",
        "release-input-values.template.md",
        "release-input-values-validation.json",
        "release-command-plan.template-check.json",
        "release-command-plan.template-check.md"
    )
}

$requirements = @(
    [ordered]@{
        key = "security.nuget"
        description = $nugetEvidenceDescription
        patterns = @("(^|/)vulnerability-report\.txt$")
    },
    [ordered]@{
        key = "security.npm"
        description = $npmEvidenceDescription
        patterns = @("(^|/)npm-audit-report\.json$", "npm-audit.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "security.trivy"
        description = $trivyEvidenceDescription
        patterns = @("trivy.*\.(sarif|json|txt|log)$", "security-ci-trivy.*\.(sarif|json|txt|log)$")
    },
    [ordered]@{
        key = "load.k6"
        description = $k6EvidenceDescription
        patterns = @("(^|/)k6[-_/].*\.(json|txt|log)$", "(^|/)load/.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "ops.backup_restore"
        description = $backupRestoreEvidenceDescription
        patterns = @("(backup|restore|rehearsal).*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "ops.deploy_rollback"
        description = $deployRollbackEvidenceDescription
        patterns = @("(deploy|rollback|production-smoke|smoke-backend).*\.(json|txt|log|md)$")
    },
    [ordered]@{
        key = "unreal.online_validation"
        description = $ueOnlineEvidenceDescription
        patterns = @("ue-online-validation.*\.(json|txt|log)$", "local-ue-validation.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "unreal.ai_showcase_automation"
        description = $aiShowcaseEvidenceDescription
        patterns = @("ai-showcase-automation.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "client.release_prerequisites"
        description = $clientPrerequisiteEvidenceDescription
        patterns = @("client-release-prerequisites.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "client.package_launcher"
        description = $clientPackageEvidenceDescription
        patterns = @("client-package-launcher.*\.(json|txt|log)$", "launcher-manifest.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "client.cdn_launcher_smoke"
        description = $cdnSmokeEvidenceDescription
        patterns = @("launcher-cdn-smoke.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "client.code_signing"
        description = $codeSigningEvidenceDescription
        patterns = @("code-signing.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "client.launcher_install_update"
        description = $launcherInstallEvidenceDescription
        patterns = @("launcher-install-update-smoke.*\.(json|txt|log)$")
    },
    [ordered]@{
        key = "client.launcher_ui_visual"
        description = $launcherUiEvidenceDescription
        patterns = @("launcher-ui-visual-evidence.*\.(json|txt|log|png)$")
    }
)

$evidenceFiles = @()
Get-ChildItem -LiteralPath $resolvedEvidenceRoot -Recurse -File | ForEach-Object {
    if ($resolvedOutputPath -and $_.FullName -eq $resolvedOutputPath) {
        return
    }

    $relativePath = Get-PortableRelativePath -RootPath $resolvedEvidenceRoot -FilePath $_.FullName
    if (Test-DerivedEvidenceOutput -RelativePath $relativePath) {
        return
    }

    $hash = Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256
    $evidenceFiles += [ordered]@{
        path = $relativePath
        sizeBytes = $_.Length
        modifiedAtUtc = $_.LastWriteTimeUtc.ToString("o")
        sha256 = $hash.Hash.ToLowerInvariant()
    }
}

$requirementResults = @()
foreach ($requirement in $requirements) {
    $matches = @(
        $evidenceFiles |
            Where-Object { Test-RelativePathMatch -RelativePath $_.path -Patterns $requirement.patterns } |
            ForEach-Object { $_.path }
    )
    $status = if ($matches.Count -gt 0) { "present" } else { "missing" }
    if ($requirement.key -eq "security.nuget" -and $matches.Count -gt 0) {
        $status = if (Test-NuGetVulnerabilityReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "security.npm" -and $matches.Count -gt 0) {
        $status = if (Test-NpmAuditReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "security.trivy" -and $matches.Count -gt 0) {
        $status = if (Test-TrivyReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "load.k6" -and $matches.Count -gt 0) {
        $readyMatches = @(Get-K6ReadyEvidencePaths -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches)
        if ($readyMatches.Count -gt 0) {
            $status = "present"
            $matches = $readyMatches
        }
        else {
            $status = "incomplete"
        }
    }
    if ($requirement.key -eq "ops.backup_restore" -and $matches.Count -gt 0) {
        $readyMatches = @(Get-BackupRestoreReadyEvidencePaths -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches)
        if ($readyMatches.Count -gt 0) {
            $status = "present"
            $matches = $readyMatches
        }
        else {
            $status = "incomplete"
        }
    }
    if ($requirement.key -eq "ops.deploy_rollback" -and $matches.Count -gt 0) {
        $status = if (Test-DeployRollbackReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "client.package_launcher" -and $matches.Count -gt 0) {
        $status = if (Test-ClientPackageReleaseReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "client.release_prerequisites" -and $matches.Count -gt 0) {
        $status = if (Test-ClientReleasePrerequisiteReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "unreal.online_validation" -and $matches.Count -gt 0) {
        $status = if (Test-UeOnlineValidationReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "unreal.ai_showcase_automation" -and $matches.Count -gt 0) {
        $status = if (Test-AiShowcaseAutomationReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "client.cdn_launcher_smoke" -and $matches.Count -gt 0) {
        $status = if (Test-LauncherCdnReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "client.code_signing" -and $matches.Count -gt 0) {
        $status = if (Test-CodeSigningReadyEvidence -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches) {
            "present"
        }
        else {
            "incomplete"
        }
    }
    if ($requirement.key -eq "client.launcher_install_update" -and $matches.Count -gt 0) {
        $readyMatches = @(Get-LauncherInstallUpdateReadyEvidencePaths -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches)
        $status = if ($readyMatches.Count -gt 0) {
            "present"
        }
        else {
            "incomplete"
        }
        if ($readyMatches.Count -gt 0) {
            $matches = $readyMatches
        }
    }
    if ($requirement.key -eq "client.launcher_ui_visual" -and $matches.Count -gt 0) {
        $readyMatches = @(Get-LauncherUiVisualReadyEvidencePaths -EvidenceRoot $resolvedEvidenceRoot -RelativePaths $matches)
        $status = if ($readyMatches.Count -gt 0) {
            "present"
        }
        else {
            "incomplete"
        }
        if ($readyMatches.Count -gt 0) {
            $matches = $readyMatches
        }
    }

    $requirementResults += [ordered]@{
        key = $requirement.key
        description = $requirement.description
        status = $status
        fileCount = $matches.Count
        files = $matches
    }
}

$gitStatus = Get-GitValue @("status", "--short")
$manifest = [ordered]@{
    schemaVersion = "1.0"
    releaseId = $ReleaseId
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    repoRoot = $repoRoot.Path
    gitCommit = Get-GitValue @("rev-parse", "HEAD")
    gitIsDirty = -not [string]::IsNullOrWhiteSpace($gitStatus)
    evidenceRoot = $resolvedEvidenceRoot
    requirements = $requirementResults
    files = $evidenceFiles
}

$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputPath -Encoding UTF8
Write-Host ($manifestWrittenMessage -f $OutputPath)

$missingRequirements = @($requirementResults | Where-Object { $_.status -ne "present" })
if ($missingRequirements.Count -gt 0) {
    Write-Host $missingEvidenceTitleMessage -ForegroundColor Yellow
    $missingRequirements | ForEach-Object {
        Write-Host ("- {0} [{1}]: {2}" -f $_.key, $_.status, $_.description) -ForegroundColor Yellow
    }

    if ($RequireAll) {
        throw ($missingEvidenceFailureMessage -f $missingRequirements.Count)
    }
}
else {
    Write-Host $allEvidencePresentMessage -ForegroundColor Green
}
