<#
使用临时夹具验证客户端发布运行器诊断链路。
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$diagnosticScript = Join-Path $repoRoot "scripts\diagnose-client-release-runner.ps1"
$fixtureRoot = Join-Path $repoRoot (".tmp\client-release-runner-diagnostic-tests-{0}" -f [guid]::NewGuid().ToString("N"))

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][int[]]$CodePoints)

    $builder = [System.Text.StringBuilder]::new()
    foreach ($codePoint in $CodePoints) {
        [void]$builder.Append([char]$codePoint)
    }
    return $builder.ToString()
}

$successMessage = New-TextFromCodePoints @(36890, 36807, 65306, 23458, 25143, 31471, 21457, 24067, 36816, 34892, 22120, 35786, 26029, 22865, 32422)
$expectedUniqueFixtureRootMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 24517, 39035, 20351, 29992, 21807, 19968, 20020, 26102, 30446, 24405, 65292, 36991, 20813, 22797, 29992, 26087, 22841, 20855, 30446, 24405, 12290)
$forbiddenFixedFixtureRootMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 19981, 24471, 22797, 29992, 22266, 23450, 23458, 25143, 31471, 21457, 24067, 36816, 34892, 22120, 35786, 26029, 22841, 20855, 30446, 24405, 12290)
$expectedChineseSuccessMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 38656, 35201, 23450, 20041, 20013, 25991, 25104, 21151, 28040, 24687, 12290)
$forbiddenOldSuccessMessage = New-TextFromCodePoints @(27979, 35797, 33050, 26412, 19981, 24471, 36755, 20986, 26087, 33521, 25991, 25104, 21151, 28040, 24687, 12290)
$forbiddenFixedFixtureRootLiteral = New-TextFromCodePoints @(36, 102, 105, 120, 116, 117, 114, 101, 82, 111, 111, 116, 32, 61, 32, 74, 111, 105, 110, 45, 80, 97, 116, 104, 32, 36, 114, 101, 112, 111, 82, 111, 111, 116, 32, 34, 46, 116, 109, 112, 92, 99, 108, 105, 101, 110, 116, 45, 114, 101, 108, 101, 97, 115, 101, 45, 114, 117, 110, 110, 101, 114, 45, 100, 105, 97, 103, 110, 111, 115, 116, 105, 99, 45, 116, 101, 115, 116, 115, 34)
$forbiddenOldSuccessLiteral = New-TextFromCodePoints @(80, 65, 83, 83, 58, 32, 99, 108, 105, 101, 110, 116, 32, 114, 101, 108, 101, 97, 115, 101, 32, 114, 117, 110, 110, 101, 114, 32, 100, 105, 97, 103, 110, 111, 115, 116, 105, 99)
$expectedMissingPackageFailureMessage = New-TextFromCodePoints @(39044, 26399, 32, 80, 97, 99, 107, 97, 103, 101, 82, 111, 111, 116, 32, 32570, 22833, 26102, 36816, 34892, 22120, 35786, 26029, 22833, 36133, 12290)
$expectedMissingKindMessage = New-TextFromCodePoints @(39044, 26399, 32570, 22833, 21253, 20307, 25253, 21578, 32, 107, 105, 110, 100, 61, 99, 108, 105, 101, 110, 116, 45, 114, 101, 108, 101, 97, 115, 101, 45, 114, 117, 110, 110, 101, 114, 45, 100, 105, 97, 103, 110, 111, 115, 116, 105, 99, 12290)
$expectedMissingPackageRootCheckMessage = New-TextFromCodePoints @(39044, 26399, 32570, 22833, 21253, 20307, 25253, 21578, 21253, 21547, 22833, 36133, 30340, 32, 99, 108, 105, 101, 110, 116, 46, 112, 97, 99, 107, 97, 103, 101, 95, 114, 111, 111, 116, 32, 26816, 26597, 12290)
$expectedValidExitCodeMessage = New-TextFromCodePoints @(39044, 26399, 26377, 25928, 36816, 34892, 22120, 35786, 26029, 36864, 20986, 30721, 20026, 32, 48, 65292, 23454, 38469, 20026)
$expectedValidRunnerPassMessage = New-TextFromCodePoints @(39044, 26399, 26377, 25928, 36816, 34892, 22120, 22841, 20855, 36890, 36807, 65307, 22833, 36133, 26816, 26597, 65306)
$expectedPrerequisitePassMessage = New-TextFromCodePoints @(39044, 26399, 26377, 25928, 36816, 34892, 22120, 22841, 20855, 36890, 36807, 32, 114, 101, 108, 101, 97, 115, 101, 46, 112, 114, 101, 114, 101, 113, 117, 105, 115, 105, 116, 101, 115, 12290)

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function New-FixturePackage {
    param([Parameter(Mandatory = $true)][string]$Root)

    if (Test-Path -LiteralPath $Root) {
        Remove-Item -LiteralPath $Root -Recurse -Force
    }

    New-Item -ItemType Directory -Force -Path (Join-Path $Root "DivineBeastsArena\Content\Paks") | Out-Null
    Set-Content -LiteralPath (Join-Path $Root "DivineBeastsArena.exe") -Value "fixture exe" -Encoding ASCII
    Set-Content -LiteralPath (Join-Path $Root "DivineBeastsArena\Content\Paks\DivineBeastsArena-Windows.pak") -Value "fixture pak" -Encoding ASCII
}

function Read-Json {
    param([Parameter(Mandatory = $true)][string]$Path)
    Get-Content -Raw -Encoding UTF8 -LiteralPath $Path | ConvertFrom-Json
}

$testSource = Get-Content -Raw -Encoding UTF8 -LiteralPath $PSCommandPath
Assert-True ($testSource.Contains("client-release-runner-diagnostic-tests-{0}")) $expectedUniqueFixtureRootMessage
Assert-True (-not $testSource.Contains($forbiddenFixedFixtureRootLiteral)) $forbiddenFixedFixtureRootMessage
Assert-True ($testSource.Contains('$successMessage')) $expectedChineseSuccessMessage
Assert-True (-not $testSource.Contains($forbiddenOldSuccessLiteral)) $forbiddenOldSuccessMessage

New-Item -ItemType Directory -Force -Path $fixtureRoot | Out-Null
$missingOutput = Join-Path $fixtureRoot "missing-package.json"
& $diagnosticScript `
    -PackageRoot (Join-Path $fixtureRoot "missing-package") `
    -DownloadUrl "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/" `
    -ManifestUrl "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/launcher-manifest.json" `
    -JsonOutputPath $missingOutput `
    -SkipSigningProbe | Out-Null
$missingExitCode = $LASTEXITCODE
if ($missingExitCode -eq 0) {
    throw $expectedMissingPackageFailureMessage
}

$missingReport = Read-Json -Path $missingOutput
if ($missingReport.kind -ne "client-release-runner-diagnostic") {
    throw $expectedMissingKindMessage
}
if (@($missingReport.checks | Where-Object { $_.name -eq "client.package_root" -and $_.status -eq "FAIL" }).Count -ne 1) {
    throw $expectedMissingPackageRootCheckMessage
}

$packageRoot = Join-Path $fixtureRoot "package"
New-FixturePackage -Root $packageRoot
$validOutput = Join-Path $fixtureRoot "valid-runner.json"
& $diagnosticScript `
    -PackageRoot $packageRoot `
    -DownloadUrl "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/" `
    -ManifestUrl "https://cdn.divinebeastsarena.invalid/releases/0.1.0.0/launcher-manifest.json" `
    -JsonOutputPath $validOutput `
    -SkipSigningProbe | Out-Null
$validExitCode = $LASTEXITCODE
if ($validExitCode -ne 0) {
    throw ("{0} {1}" -f $expectedValidExitCodeMessage, $validExitCode)
}

$validReport = Read-Json -Path $validOutput
$failedChecks = @($validReport.checks | Where-Object { $_.status -eq "FAIL" })
if ($failedChecks.Count -ne 0) {
    throw ("{0} {1}" -f $expectedValidRunnerPassMessage, ($failedChecks.name -join ', '))
}
if (@($validReport.checks | Where-Object { $_.name -eq "release.prerequisites" -and $_.status -eq "PASS" }).Count -ne 1) {
    throw $expectedPrerequisitePassMessage
}

Write-Host $successMessage -ForegroundColor Green
