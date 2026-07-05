<#
Exercises scripts\import-fixed-skill-group-datatable.ps1 against lightweight fixtures.

The fixture validates the guarded command construction for importing the
FixedSkillGroups source CSV into a DataTable asset without launching Unreal.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$importer = Join-Path $repoRoot "scripts\import-fixed-skill-group-datatable.ps1"
$pythonImporter = Join-Path $repoRoot "scripts\unreal\import_fixed_skill_group_datatable.py"
$testRoot = Join-Path $repoRoot (".tmp\fixed-skill-group-datatable-import-tests-{0}" -f ([System.Guid]::NewGuid().ToString("N")))

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][object[]]$CodePoints)

    return -join ($CodePoints | ForEach-Object { [char]([int]$_) })
}

$missingProjectMessage = New-TextFromCodePoints -CodePoints @(0x9879, 0x76EE, 0x6587, 0x4EF6, 0x4E0D, 0x5B58, 0x5728)
$missingCsvMessage = (New-TextFromCodePoints -CodePoints @(0x6E90)) + " CSV " + (New-TextFromCodePoints -CodePoints @(0x4E0D, 0x5B58, 0x5728))
$missingEditorMessage = "UnrealEditor-Cmd.exe " + (New-TextFromCodePoints -CodePoints @(0x4E0D, 0x5B58, 0x5728))

function Invoke-ExpectFailure {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Action,
        [Parameter(Mandatory = $true)][string]$ExpectedMessage
    )

    try {
        & $Action
    }
    catch {
        if ($_.Exception.Message -notlike "*$ExpectedMessage*") {
            throw "$Name failed for the wrong reason: $($_.Exception.Message)"
        }

        Write-Host "PASS: $Name failed as expected" -ForegroundColor Green
        return
    }

    throw "$Name should have failed."
}

function Invoke-ExpectSuccess {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Action
    )

    & $Action
    Write-Host "PASS: $Name" -ForegroundColor Green
}

New-Item -ItemType Directory -Force -Path $testRoot | Out-Null

Invoke-ExpectSuccess `
    -Name "importer script exists" `
    -Action {
        if (-not (Test-Path -LiteralPath $importer)) {
            throw "Missing required importer script: $importer"
        }

        if (-not (Test-Path -LiteralPath $pythonImporter)) {
            throw "Missing required Unreal Python importer script: $pythonImporter"
        }
    }

Invoke-ExpectSuccess `
    -Name "import diagnostics use Chinese human-readable output" `
    -Action {
        $importerContent = Get-Content -LiteralPath $importer -Raw -Encoding UTF8
        $pythonImporterContent = Get-Content -LiteralPath $pythonImporter -Raw -Encoding UTF8

        $requiredChineseFragments = @(
            (New-TextFromCodePoints -CodePoints @(0x6E90, 0x20, 0x43, 0x53, 0x56, 0x20, 0x4E0D, 0x5B58, 0x5728)),
            (New-TextFromCodePoints -CodePoints @(0x884C, 0x7ED3, 0x6784, 0x4E0D, 0x5B58, 0x5728)),
            (New-TextFromCodePoints -CodePoints @(0x5DF2, 0x5BFC, 0x5165))
        )

        foreach ($fragment in $requiredChineseFragments) {
            if ($importerContent -notlike "*$fragment*" -and $pythonImporterContent -notlike "*$fragment*") {
                throw "Expected import diagnostics to contain Chinese fragment '$fragment'."
            }
        }

        $forbiddenEnglishDiagnostics = @(
            "Project file was not found",
            "Source CSV was not found",
            "Unreal import Python script was not found",
            "UnrealEditor-Cmd.exe was not found",
            "FixedSkillGroups DataTable import failed",
            "FixedSkillGroups DataTable import did not create the expected asset file",
            "Refusing to write outside the fixed skill group DataTable asset",
            "Row struct was not found",
            "Unreal asset import did not return an imported object path",
            "Imported DataTable asset was not found after import",
            "Imported asset is not a DataTable",
            "Imported DataTable row count mismatch",
            "Failed to save imported DataTable asset",
            "Imported {len(row_names)} rows into"
        )

        foreach ($diagnostic in $forbiddenEnglishDiagnostics) {
            if ($importerContent -like "*$diagnostic*" -or $pythonImporterContent -like "*$diagnostic*") {
                throw "Import diagnostics should not expose English message: $diagnostic"
            }
        }
    }

$fixtureRoot = Join-Path $testRoot "valid-fixture"
$unrealRoot = Join-Path $fixtureRoot "Unreal"
$projectRoot = Join-Path $fixtureRoot "Project"
$editorCmd = Join-Path $unrealRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$projectPath = Join-Path $projectRoot "DBA_GameClient\DivineBeastsArena.uproject"
$csvPath = Join-Path $fixtureRoot "DT_FixedSkillGroups.csv"

New-Item -ItemType Directory -Force -Path (Split-Path $editorCmd -Parent), (Split-Path $projectPath -Parent), (Split-Path $csvPath -Parent) | Out-Null
Set-Content -LiteralPath $editorCmd -Encoding ASCII -Value "fixture"
Set-Content -LiteralPath $projectPath -Encoding UTF8 -Value "{}"
Set-Content -LiteralPath $csvPath -Encoding UTF8 -Value "Name,RowId,ZodiacType,ElementType`nRat_Water,Rat_Water,Rat,Water"

Invoke-ExpectSuccess `
    -Name "command-only output includes guarded import command" `
    -Action {
        $result = & $importer `
            -UnrealRoot $unrealRoot `
            -ProjectPath $projectPath `
            -CsvPath $csvPath `
            -CommandOnly

        if ($result.FilePath -ne $editorCmd) {
            throw "Expected FilePath '$editorCmd' but got '$($result.FilePath)'."
        }

        if ($result.WorkingDirectory -ne $repoRoot) {
            throw "Expected WorkingDirectory '$repoRoot' but got '$($result.WorkingDirectory)'."
        }

        if ($result.AssetPackagePath -ne "/Game/DBA/Data/Tables/DT_FixedSkillGroups") {
            throw "Unexpected asset package path: $($result.AssetPackagePath)"
        }

        if ($result.RowStructPath -ne "/Script/DivineBeastsArena.DBAZodiacElementFixedSkillGroupRow") {
            throw "Unexpected row struct path: $($result.RowStructPath)"
        }

        if ($result.EnvironmentVariables.DBA_FIXED_SKILL_GROUP_CSV -ne $csvPath) {
            throw "Importer must pass the CSV path through DBA_FIXED_SKILL_GROUP_CSV."
        }

        if ($result.EnvironmentVariables.DBA_FIXED_SKILL_GROUP_ASSET -ne "/Game/DBA/Data/Tables/DT_FixedSkillGroups") {
            throw "Importer must pass the asset path through DBA_FIXED_SKILL_GROUP_ASSET."
        }

        if ($result.EnvironmentVariables.DBA_FIXED_SKILL_GROUP_ROW_STRUCT -ne "/Script/DivineBeastsArena.DBAZodiacElementFixedSkillGroupRow") {
            throw "Importer must pass the row struct path through DBA_FIXED_SKILL_GROUP_ROW_STRUCT."
        }

        $pythonArg = @($result.Arguments) | Where-Object { $_ -like "-ExecutePythonScript=*" } | Select-Object -First 1
        if ($pythonArg -notlike "*import_fixed_skill_group_datatable.py*") {
            throw "Import command must execute import_fixed_skill_group_datatable.py."
        }

        if ($result.WillWriteAsset -ne $false) {
            throw "CommandOnly mode must report WillWriteAsset=False."
        }
    }

Invoke-ExpectFailure `
    -Name "missing csv fails fast" `
    -ExpectedMessage $missingCsvMessage `
    -Action {
        & $importer `
            -UnrealRoot $unrealRoot `
            -ProjectPath $projectPath `
            -CsvPath (Join-Path $fixtureRoot "Missing.csv") `
            -CommandOnly | Out-Null
    }

Invoke-ExpectFailure `
    -Name "missing project path fails fast" `
    -ExpectedMessage $missingProjectMessage `
    -Action {
        & $importer `
            -UnrealRoot $unrealRoot `
            -ProjectPath (Join-Path $projectRoot "Missing\DivineBeastsArena.uproject") `
            -CsvPath $csvPath `
            -CommandOnly | Out-Null
    }

Invoke-ExpectFailure `
    -Name "missing UnrealEditor-Cmd fails fast" `
    -ExpectedMessage $missingEditorMessage `
    -Action {
        & $importer `
            -UnrealRoot (Join-Path $fixtureRoot "MissingUnreal") `
            -ProjectPath $projectPath `
            -CsvPath $csvPath `
            -CommandOnly | Out-Null
    }
