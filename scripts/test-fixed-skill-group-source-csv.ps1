<#
Exercises scripts/write-fixed-skill-group-source-csv.ps1.

This keeps the FixedSkillGroups DataTable import source deterministic before
the actual .uasset is created through the Unreal Editor.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$writer = Join-Path $repoRoot "scripts\write-fixed-skill-group-source-csv.ps1"
$testRoot = Join-Path $repoRoot (".tmp\fixed-skill-group-source-csv-tests-{0}" -f ([System.Guid]::NewGuid().ToString("N")))

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][object[]]$CodePoints)

    return -join ($CodePoints | ForEach-Object { [char]([int]$_) })
}

$missingSourceCsvMessage = (New-TextFromCodePoints -CodePoints @(0x6E90)) + " CSV " + (New-TextFromCodePoints -CodePoints @(0x4E0D, 0x5B58, 0x5728))

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
    -Name "writer script exists" `
    -Action {
        if (-not (Test-Path -LiteralPath $writer)) {
            throw "Missing required writer script: $writer"
        }
    }

Invoke-ExpectSuccess `
    -Name "writer validation diagnostics use Chinese human-readable output" `
    -Action {
        $writerContent = Get-Content -LiteralPath $writer -Raw -Encoding UTF8

        $requiredMessageTokens = @(
            "rowCountMismatchMessage",
            "missingCsvHeaderMessage",
            "rowIdentityMismatchMessage",
            "unsupportedZodiacTypeMessage",
            "unsupportedElementTypeMessage",
            "resonanceElementMismatchMessage",
            "duplicateRowMessage",
            "missingRequiredRowMessage",
            "missingSourceCsvMessage"
        )

        foreach ($token in $requiredMessageTokens) {
            if ($writerContent -notlike "*$token*") {
                throw "Expected writer script to define Chinese runtime message token '$token'."
            }
        }

        $forbiddenEnglishDiagnostics = @(
            "Expected 60 fixed skill group rows but found",
            "Missing CSV header",
            "Row identity mismatch",
            "Unsupported ZodiacType",
            "Unsupported ElementType",
            "ResonanceElement must match ElementType",
            "Duplicate fixed skill group row",
            "Missing fixed skill group row",
            "Source CSV was not found"
        )

        foreach ($diagnostic in $forbiddenEnglishDiagnostics) {
            if ($writerContent -like "*$diagnostic*") {
                throw "Writer script should not expose English diagnostic: $diagnostic"
            }
        }
    }

$csvPath = Join-Path $testRoot "DT_FixedSkillGroups.csv"

Invoke-ExpectSuccess `
    -Name "writer creates deterministic 60-row source csv" `
    -Action {
        $result = & $writer -OutputPath $csvPath

        if ($result.OutputPath -ne $csvPath) {
            throw "Expected OutputPath '$csvPath' but got '$($result.OutputPath)'."
        }

        if ($result.RowCount -ne 60) {
            throw "Expected 60 generated rows but got '$($result.RowCount)'."
        }

        $rows = Import-Csv -LiteralPath $csvPath
        if (@($rows).Count -ne 60) {
            throw "Expected CSV to contain 60 rows but got '$(@($rows).Count)'."
        }

        $expectedHeaders = @(
            "Name",
            "RowId",
            "DataVersion",
            "ZodiacType",
            "ElementType",
            "ElementPassiveSkillId",
            "ElementSkill1Id",
            "ElementSkill2Id",
            "ElementSkill3Id",
            "ElementSkill4Id",
            "ZodiacUltimateSkillId",
            "ElementPassiveInputKey",
            "ElementSkill1InputKey",
            "ElementSkill2InputKey",
            "ElementSkill3InputKey",
            "ElementSkill4InputKey",
            "ZodiacUltimateInputKey",
            "ResonanceInputKey",
            "ElementResonanceLevel",
            "ResonanceElement",
            "ResonanceControlTimeBonus",
            "ResonanceShieldBonus",
            "AbilitySetAsset",
            "DisplayName",
            "Description",
            "Icon",
            "OriginalSourceName",
            "SanitizedName",
            "SanitizedAssetName",
            "bEnabled",
            "bIsInDevelopment",
            "DesignerNotes"
        )

        $actualHeaders = @($rows[0].PSObject.Properties.Name)
        foreach ($header in $expectedHeaders) {
            if ($actualHeaders -notcontains $header) {
                throw "Missing CSV header: $header"
            }
        }

        $ratWater = $rows | Where-Object { $_.Name -eq "Rat_Water" } | Select-Object -First 1
        if (-not $ratWater) {
            throw "Missing Rat_Water row."
        }

        if ($ratWater.RowId -ne "Rat_Water" -or $ratWater.ZodiacType -ne "Rat" -or $ratWater.ElementType -ne "Water") {
            throw "Rat_Water identity fields are incorrect."
        }

        if ($ratWater.ElementSkill4Id -ne "Rat_Water_Skill04" -or $ratWater.ZodiacUltimateSkillId -ne "Rat_Ultimate") {
            throw "Rat_Water skill ids are incorrect."
        }

        $snakeGold = $rows | Where-Object { $_.Name -eq "Snake_Gold" } | Select-Object -First 1
        if (-not $snakeGold -or $snakeGold.ResonanceElement -ne "Gold") {
            throw "Snake_Gold row is missing or has the wrong resonance element."
        }

        $expectedRatWaterDisplayName = New-TextFromCodePoints @(0x9F20, 0x6C34, 0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4)
        $requiredDescriptionText = New-TextFromCodePoints @(0x751F, 0x8096, 0x4E0E, 0x81EA, 0x7136, 0x5143, 0x7D20, 0x4E4B, 0x529B)
        $requiredDesignerNotesText = New-TextFromCodePoints @(0x7B56, 0x5212)

        if ($ratWater.DisplayName -ne $expectedRatWaterDisplayName) {
            throw "Rat_Water DisplayName should use Chinese UI text, got '$($ratWater.DisplayName)'."
        }

        if ($ratWater.Description -notlike "*$requiredDescriptionText*") {
            throw "Rat_Water Description should use Chinese DataTable text, got '$($ratWater.Description)'."
        }

        if ($ratWater.DesignerNotes -notlike "*$requiredDesignerNotesText*") {
            throw "Rat_Water DesignerNotes should use Chinese notes, got '$($ratWater.DesignerNotes)'."
        }

        $forbiddenEnglishText = @(
            "Fixed Skill Group",
            "MVP canonical fixed skill group generated from Zodiac + Element.",
            "Generated MVP source row. Replace balance and assets through reviewed DataTable updates."
        )

        foreach ($row in $rows) {
            foreach ($forbiddenText in $forbiddenEnglishText) {
                if ($row.DisplayName -like "*$forbiddenText*" -or $row.Description -like "*$forbiddenText*" -or $row.DesignerNotes -like "*$forbiddenText*") {
                    throw "FixedSkillGroups source CSV should not expose English UI/DataTable text: row '$($row.Name)' contains '$forbiddenText'."
                }
            }
        }
    }

Invoke-ExpectSuccess `
    -Name "validate-only accepts generated csv" `
    -Action {
        $result = & $writer -OutputPath $csvPath -ValidateOnly
        if ($result.RowCount -ne 60 -or -not $result.Valid) {
            throw "ValidateOnly should accept the generated CSV."
        }
    }

Invoke-ExpectFailure `
    -Name "validate-only rejects missing csv" `
    -ExpectedMessage $missingSourceCsvMessage `
    -Action {
        & $writer -OutputPath (Join-Path $testRoot "Missing.csv") -ValidateOnly | Out-Null
    }
