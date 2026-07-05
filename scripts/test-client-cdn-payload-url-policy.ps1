<#
Validates URL-policy failures for the client CDN payload preparation script.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$payloadScript = Join-Path $repoRoot "scripts\prepare-client-cdn-payload.ps1"
$scratchRoot = Join-Path $repoRoot (".tmp\client-cdn-payload-url-policy-tests-{0}" -f [guid]::NewGuid().ToString("N"))
$packageRoot = Join-Path $scratchRoot "package"

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
        [Parameter(Mandatory = $true)][string]$DownloadUrl,
        [Parameter(Mandatory = $true)][string]$ExpectedMessage
    )

    try {
        & $payloadScript `
            -PackageRoot $packageRoot `
            -PayloadRoot (Join-Path $scratchRoot "$Name-payload") `
            -EvidenceDir (Join-Path $scratchRoot "$Name-evidence") `
            -RunId $Name `
            -DownloadUrl $DownloadUrl
        throw "Expected client CDN payload preparation to fail for fixture: $Name"
    }
    catch {
        $message = $_.Exception.Message
        Assert-True ($message.Contains($ExpectedMessage)) "Expected '$Name' failure to contain '$ExpectedMessage', got: $message"
    }
}

if (Test-Path -LiteralPath $scratchRoot) {
    Remove-Item -LiteralPath $scratchRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $packageRoot | Out-Null
Set-Content -LiteralPath (Join-Path $packageRoot "DivineBeastsArena.exe") -Value "fixture exe" -Encoding UTF8

Invoke-ExpectFailure `
    -Name "hostless-https" `
    -DownloadUrl "https://" `
    -ExpectedMessage "DownloadUrl must be a valid absolute URL"

Invoke-ExpectFailure `
    -Name "external-http" `
    -DownloadUrl "http://download.example.com/releases/1.0.0/" `
    -ExpectedMessage "DownloadUrl must be HTTPS unless -AllowLocalHttp is used for localhost payload validation"

Write-Host "PASS: client CDN payload URL policy fixtures" -ForegroundColor Green
