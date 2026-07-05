<#
Diagnoses client release inputs before signing, CDN upload, and release evidence.

This script does not sign, upload, download, or mutate the package. It writes a
small JSON report so local runs and CI can fail early on missing production
inputs such as example CDN URLs or absent signing identities.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$PackageRoot,
    [Parameter(Mandatory = $true)][string]$DownloadUrl,
    [string]$ManifestUrl = "",
    [switch]$RequireManifestUrl,
    [switch]$AllowLocalHttp,
    [switch]$RequireSigningIdentity,
    [string]$CertificateThumbprint = "",
    [string]$CertificateSubject = "",
    [string]$PfxPath = "",
    [string]$PfxPasswordEnvironmentVariable = "DBA_CODE_SIGNING_PFX_PASSWORD",
    [switch]$RequireSignTool,
    [string]$SignToolPath = "",
    [string]$OutputJsonPath = "",
    [switch]$FailOnBlockingIssues
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    $builder = [System.Text.StringBuilder]::new()
    foreach ($codePoint in $CodePoints) {
        [void]$builder.Append([char]$codePoint)
    }
    return $builder.ToString()
}

$reportWrittenMessage = New-TextFromCodePoints @(23458, 25143, 31471, 21457, 24067, 21069, 32622, 26465, 20214, 25253, 21578, 24050, 20889, 20837, 65306, 123, 48, 125)
$blockingIssuesTitleMessage = New-TextFromCodePoints @(23458, 25143, 31471, 21457, 24067, 36755, 20837, 23384, 22312, 38459, 22622, 39033, 65306)
$blockingFailureMessage = New-TextFromCodePoints @(23458, 25143, 31471, 21457, 24067, 21069, 32622, 26465, 20214, 26816, 26597, 21457, 29616, 32, 123, 48, 125, 32, 20010, 38459, 22622, 39033, 12290)
$successReadyMessage = New-TextFromCodePoints @(36890, 36807, 65306, 23458, 25143, 31471, 21457, 24067, 21069, 32622, 26465, 20214, 36755, 20837, 24050, 23601, 32490)
$urlRequiredMessageTemplate = New-TextFromCodePoints @(123, 48, 125, 32, 26159, 20505, 36873, 29256, 26412, 24517, 22635, 39033, 12290)
$urlAbsoluteHostMessageTemplate = New-TextFromCodePoints @(123, 48, 125, 32, 24517, 39035, 26159, 21253, 21547, 20027, 26426, 21517, 30340, 26377, 25928, 32477, 23545, 32, 85, 82, 76, 65306, 123, 49, 125)
$urlHttpsMessageTemplate = New-TextFromCodePoints @(123, 48, 125, 32, 24517, 39035, 20351, 29992, 32, 72, 84, 84, 80, 83, 32, 20316, 20026, 29983, 20135, 21457, 24067, 36755, 20837, 65306, 123, 49, 125)
$urlExampleCdnMessageTemplate = New-TextFromCodePoints @(123, 48, 125, 32, 20173, 25351, 21521, 31034, 20363, 32, 67, 68, 78, 65306, 123, 49, 125)
$packageMissingClientExeMessageTemplate = New-TextFromCodePoints @(80, 97, 99, 107, 97, 103, 101, 82, 111, 111, 116, 32, 19981, 21253, 21547, 32, 68, 105, 118, 105, 110, 101, 66, 101, 97, 115, 116, 115, 65, 114, 101, 110, 97, 46, 101, 120, 101, 65306, 123, 48, 125)
$packageRootMissingMessageTemplate = New-TextFromCodePoints @(80, 97, 99, 107, 97, 103, 101, 82, 111, 111, 116, 32, 19981, 23384, 22312, 65306, 123, 48, 125)
$signingIdentityInvalidMessage = New-TextFromCodePoints @(31614, 21517, 36523, 20221, 21442, 25968, 24517, 39035, 19988, 21482, 33021, 20256, 20837, 19968, 20010, 65306, 67, 101, 114, 116, 105, 102, 105, 99, 97, 116, 101, 84, 104, 117, 109, 98, 112, 114, 105, 110, 116, 12289, 67, 101, 114, 116, 105, 102, 105, 99, 97, 116, 101, 83, 117, 98, 106, 101, 99, 116, 32, 25110, 32, 80, 102, 120, 80, 97, 116, 104, 12290)
$pfxMissingMessageTemplate = New-TextFromCodePoints @(80, 102, 120, 80, 97, 116, 104, 32, 19981, 23384, 22312, 65306, 123, 48, 125)
$pfxPasswordMissingMessageTemplate = New-TextFromCodePoints @(80, 70, 88, 32, 23494, 30721, 29615, 22659, 21464, 37327, 26410, 35774, 32622, 65306, 123, 48, 125)
$certificateMissingMessage = New-TextFromCodePoints @(26410, 22312, 32, 67, 117, 114, 114, 101, 110, 116, 85, 115, 101, 114, 32, 25110, 32, 76, 111, 99, 97, 108, 77, 97, 99, 104, 105, 110, 101, 32, 35777, 20070, 23384, 20648, 20013, 25214, 21040, 20195, 30721, 31614, 21517, 35777, 20070, 12290)
$signToolMissingMessage = New-TextFromCodePoints @(26410, 25214, 21040, 32, 115, 105, 103, 110, 116, 111, 111, 108, 46, 101, 120, 101, 12290, 35831, 23433, 35013, 32, 87, 105, 110, 100, 111, 119, 115, 32, 83, 68, 75, 32, 25110, 20256, 20837, 32, 83, 105, 103, 110, 84, 111, 111, 108, 80, 97, 116, 104, 12290)

function Add-BlockingIssue {
    param(
        [Parameter(Mandatory = $true)][AllowEmptyCollection()][System.Collections.Generic.List[object]]$Issues,
        [Parameter(Mandatory = $true)][string]$Code,
        [Parameter(Mandatory = $true)][string]$Message
    )

    $Issues.Add([ordered]@{
        code = $Code
        message = $Message
    }) | Out-Null
}

function Test-LocalHttpUrl {
    param([Parameter(Mandatory = $true)][uri]$Uri)

    if ($Uri.Scheme -ne "http") {
        return $false
    }
    return $Uri.Host -in @("localhost", "127.0.0.1", "::1")
}

function Test-ExampleCdnUrl {
    param([Parameter(Mandatory = $true)][string]$Url)
    return $Url -match "^https://cdn\.example\.com(/|$)"
}

function Test-UrlPolicy {
    param(
        [Parameter(Mandatory = $true)][AllowEmptyCollection()][System.Collections.Generic.List[object]]$Issues,
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][string]$MissingCode,
        [Parameter(Mandatory = $true)][string]$InvalidCode,
        [Parameter(Mandatory = $true)][string]$ExampleCode
    )

    if ([string]::IsNullOrWhiteSpace($Url)) {
        Add-BlockingIssue -Issues $Issues -Code $MissingCode -Message ($urlRequiredMessageTemplate -f $Name)
        return [ordered]@{
            value = $Url
            valid = $false
            isHttps = $false
            isExample = $false
            isAllowedLocalHttp = $false
        }
    }

    $uri = $null
    if (-not [uri]::TryCreate($Url, [System.UriKind]::Absolute, [ref]$uri) -or [string]::IsNullOrWhiteSpace($uri.Host)) {
        Add-BlockingIssue -Issues $Issues -Code $InvalidCode -Message ($urlAbsoluteHostMessageTemplate -f $Name, $Url)
        return [ordered]@{
            value = $Url
            valid = $false
            isHttps = $false
            isExample = $false
            isAllowedLocalHttp = $false
        }
    }

    $isHttps = $uri.Scheme -eq "https"
    $isAllowedLocalHttp = $AllowLocalHttp -and (Test-LocalHttpUrl -Uri $uri)
    $isExample = Test-ExampleCdnUrl -Url $Url

    if (-not $isHttps -and -not $isAllowedLocalHttp) {
        Add-BlockingIssue -Issues $Issues -Code $InvalidCode -Message ($urlHttpsMessageTemplate -f $Name, $Url)
    }
    if ($isExample) {
        Add-BlockingIssue -Issues $Issues -Code $ExampleCode -Message ($urlExampleCdnMessageTemplate -f $Name, $Url)
    }

    return [ordered]@{
        value = $Url
        valid = ($isHttps -or $isAllowedLocalHttp) -and -not $isExample
        isHttps = $isHttps
        isExample = $isExample
        isAllowedLocalHttp = $isAllowedLocalHttp
    }
}

function Resolve-SignToolCandidate {
    param([string]$ExplicitPath)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitPath)) {
        if (Test-Path -LiteralPath $ExplicitPath) {
            return (Resolve-Path -LiteralPath $ExplicitPath).ProviderPath
        }
        return ""
    }

    $command = Get-Command "signtool.exe" -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $kitRoot = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
    if (Test-Path -LiteralPath $kitRoot) {
        $candidate = Get-ChildItem -Path $kitRoot -Recurse -Filter "signtool.exe" -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "\\x64\\signtool\.exe$" } |
            Sort-Object FullName -Descending |
            Select-Object -First 1
        if ($candidate) {
            return $candidate.FullName
        }
    }

    return ""
}

function Find-CodeSigningCertificate {
    param(
        [string]$Thumbprint,
        [string]$Subject
    )

    $stores = @("Cert:\CurrentUser\My", "Cert:\LocalMachine\My")
    foreach ($store in $stores) {
        if (-not (Test-Path $store)) {
            continue
        }

        if (-not [string]::IsNullOrWhiteSpace($Thumbprint)) {
            $normalizedThumbprint = ($Thumbprint -replace "\s", "").ToUpperInvariant()
            $match = Get-ChildItem $store -ErrorAction SilentlyContinue |
                Where-Object { ($_.Thumbprint -replace "\s", "").ToUpperInvariant() -eq $normalizedThumbprint } |
                Select-Object -First 1
            if ($match) {
                return $match
            }
        }

        if (-not [string]::IsNullOrWhiteSpace($Subject)) {
            $match = Get-ChildItem $store -ErrorAction SilentlyContinue |
                Where-Object { $_.Subject -like "*$Subject*" } |
                Select-Object -First 1
            if ($match) {
                return $match
            }
        }
    }

    return $null
}

$issues = [System.Collections.Generic.List[object]]::new()
$resolvedPackageRoot = ""
$clientExePath = ""
$packageFileCount = 0

if (Test-Path -LiteralPath $PackageRoot) {
    $resolvedPackageRoot = (Resolve-Path -LiteralPath $PackageRoot).ProviderPath
    $clientExe = Get-ChildItem -LiteralPath $resolvedPackageRoot -Recurse -File -Filter "DivineBeastsArena.exe" -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($clientExe) {
        $clientExePath = $clientExe.FullName
    }
    else {
        Add-BlockingIssue -Issues $issues -Code "package_missing_client_exe" -Message ($packageMissingClientExeMessageTemplate -f $resolvedPackageRoot)
    }
    $packageFileCount = @(Get-ChildItem -LiteralPath $resolvedPackageRoot -Recurse -File -ErrorAction SilentlyContinue).Count
}
else {
    Add-BlockingIssue -Issues $issues -Code "package_root_missing" -Message ($packageRootMissingMessageTemplate -f $PackageRoot)
}

$downloadUrlReport = Test-UrlPolicy `
    -Issues $issues `
    -Name "DownloadUrl" `
    -Url $DownloadUrl `
    -MissingCode "download_url_missing" `
    -InvalidCode "download_url_invalid" `
    -ExampleCode "download_url_example"

$manifestUrlReport = [ordered]@{
    value = $ManifestUrl
    valid = $true
    isHttps = $false
    isExample = $false
    isAllowedLocalHttp = $false
}
if ($RequireManifestUrl -or -not [string]::IsNullOrWhiteSpace($ManifestUrl)) {
    $manifestUrlReport = Test-UrlPolicy `
        -Issues $issues `
        -Name "ManifestUrl" `
        -Url $ManifestUrl `
        -MissingCode "manifest_url_missing" `
        -InvalidCode "manifest_url_invalid" `
        -ExampleCode "manifest_url_example"
}

$identityModes = @(
    -not [string]::IsNullOrWhiteSpace($CertificateThumbprint),
    -not [string]::IsNullOrWhiteSpace($CertificateSubject),
    -not [string]::IsNullOrWhiteSpace($PfxPath)
) | Where-Object { $_ }
$signingIdentityRequired = $RequireSigningIdentity -or $identityModes.Count -gt 0
$certificateFound = $false
$resolvedPfxPath = ""

if ($signingIdentityRequired) {
    if ($identityModes.Count -ne 1) {
        Add-BlockingIssue -Issues $issues -Code "signing_identity_invalid" -Message $signingIdentityInvalidMessage
    }
    elseif (-not [string]::IsNullOrWhiteSpace($PfxPath)) {
        if (Test-Path -LiteralPath $PfxPath) {
            $resolvedPfxPath = (Resolve-Path -LiteralPath $PfxPath).ProviderPath
        }
        else {
            Add-BlockingIssue -Issues $issues -Code "pfx_missing" -Message ($pfxMissingMessageTemplate -f $PfxPath)
        }
        if ([string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable($PfxPasswordEnvironmentVariable))) {
            Add-BlockingIssue -Issues $issues -Code "pfx_password_missing" -Message ($pfxPasswordMissingMessageTemplate -f $PfxPasswordEnvironmentVariable)
        }
    }
    elseif ($identityModes.Count -eq 1) {
        $certificate = Find-CodeSigningCertificate -Thumbprint $CertificateThumbprint -Subject $CertificateSubject
        if ($certificate) {
            $certificateFound = $true
        }
        else {
            Add-BlockingIssue -Issues $issues -Code "certificate_not_found" -Message $certificateMissingMessage
        }
    }
}

$signToolPath = ""
if ($RequireSignTool -or $signingIdentityRequired) {
    $signToolPath = Resolve-SignToolCandidate -ExplicitPath $SignToolPath
    if ([string]::IsNullOrWhiteSpace($signToolPath)) {
        Add-BlockingIssue -Issues $issues -Code "signtool_missing" -Message $signToolMissingMessage
    }
}

if ([string]::IsNullOrWhiteSpace($OutputJsonPath)) {
    $OutputJsonPath = Join-Path $repoRoot "Artifacts\ProductionEvidence\client\client-release-prerequisites-{0:yyyyMMddTHHmmssZ}.json" -f (Get-Date).ToUniversalTime()
}
$resolvedOutputJsonPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputJsonPath)
$outputDir = Split-Path -Parent $resolvedOutputJsonPath
if ($outputDir -and -not (Test-Path -LiteralPath $outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

$report = [ordered]@{
    schemaVersion = "1.0"
    kind = "client-release-prerequisites"
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    readyForReleaseInputs = ($issues.Count -eq 0)
    blockingIssueCount = $issues.Count
    blockingIssues = @($issues)
    package = [ordered]@{
        root = $resolvedPackageRoot
        requestedRoot = $PackageRoot
        clientExePath = $clientExePath
        fileCount = $packageFileCount
    }
    urls = [ordered]@{
        downloadUrl = $downloadUrlReport
        manifestUrl = $manifestUrlReport
        allowLocalHttp = [bool]$AllowLocalHttp
    }
    signing = [ordered]@{
        required = [bool]$signingIdentityRequired
        certificateThumbprintProvided = -not [string]::IsNullOrWhiteSpace($CertificateThumbprint)
        certificateSubjectProvided = -not [string]::IsNullOrWhiteSpace($CertificateSubject)
        pfxPath = $resolvedPfxPath
        pfxPasswordEnvironmentVariable = $PfxPasswordEnvironmentVariable
        certificateFound = $certificateFound
        signToolPath = $signToolPath
    }
}

$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $resolvedOutputJsonPath -Encoding UTF8
Write-Host ($reportWrittenMessage -f $resolvedOutputJsonPath)

if ($issues.Count -gt 0) {
    Write-Host $blockingIssuesTitleMessage -ForegroundColor Yellow
    foreach ($issue in $issues) {
        Write-Host ("- {0}: {1}" -f $issue.code, $issue.message) -ForegroundColor Yellow
    }
    if ($FailOnBlockingIssues) {
        throw ($blockingFailureMessage -f $issues.Count)
    }
}
else {
    Write-Host $successReadyMessage -ForegroundColor Green
}
