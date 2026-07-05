<#
Validates URL-policy failures for the launcher CDN smoke script.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$smokeScript = Join-Path $repoRoot "scripts\run-launcher-cdn-smoke.ps1"
$scratchRoot = Join-Path $repoRoot (".tmp\launcher-cdn-smoke-url-policy-tests-{0}" -f [guid]::NewGuid().ToString("N"))

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Invoke-ExpectFailure {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$ManifestUrl,
        [Parameter(Mandatory = $true)][string]$ExpectedMessage
    )

    try {
        & $smokeScript `
            -ManifestUrl $ManifestUrl `
            -EvidenceDir (Join-Path $scratchRoot $Name) `
            -InstallRoot (Join-Path $scratchRoot "$Name-install") `
            -RunId $Name
        throw "Expected launcher CDN smoke to fail for fixture: $Name"
    }
    catch {
        $message = $_.Exception.Message
        Assert-True ($message.Contains($ExpectedMessage)) "Expected '$Name' failure to contain '$ExpectedMessage', got: $message"
    }
}

if (Test-Path -LiteralPath $scratchRoot) {
    Remove-Item -LiteralPath $scratchRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $scratchRoot | Out-Null

Invoke-ExpectFailure `
    -Name "hostless-https" `
    -ManifestUrl "https://" `
    -ExpectedMessage "ManifestUrl must be a valid absolute URL"

Invoke-ExpectFailure `
    -Name "external-http" `
    -ManifestUrl "http://download.example.com/manifest.json" `
    -ExpectedMessage "ManifestUrl must be HTTPS unless -AllowLocalHttp is used for localhost smoke"

Write-Host "PASS: launcher CDN smoke URL policy fixtures" -ForegroundColor Green
