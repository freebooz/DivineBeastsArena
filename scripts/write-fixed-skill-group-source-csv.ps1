<#
Writes and validates the source CSV for /Game/DBA/Data/Tables/DT_FixedSkillGroups.

This script intentionally creates only a text import source. It does not create,
save, delete, or modify Unreal .uasset files.
#>

[CmdletBinding()]
param(
    [string]$OutputPath = "",
    [switch]$ValidateOnly
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $repoRoot "DBA_GameClient\Content\DBA\Data\Tables\Source\DT_FixedSkillGroups.csv"
}

$zodiacs = @(
    "Rat",
    "Ox",
    "Tiger",
    "Rabbit",
    "Dragon",
    "Snake",
    "Horse",
    "Goat",
    "Monkey",
    "Rooster",
    "Dog",
    "Pig"
)

$elements = @(
    "Gold",
    "Wood",
    "Water",
    "Fire",
    "Earth"
)

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][object[]]$CodePoints)

    return -join ($CodePoints | ForEach-Object { [char]([int]$_) })
}

$fixedSkillGroupSuffix = New-TextFromCodePoints @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4)
$fixedSkillGroupDescription = "MVP " + (New-TextFromCodePoints @(0x9636, 0x6BB5, 0x7531, 0x751F, 0x8096, 0x4E0E, 0x81EA, 0x7136, 0x5143, 0x7D20, 0x4E4B, 0x529B, 0x751F, 0x6210, 0x7684, 0x6807, 0x51C6, 0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4, 0x3002))
$fixedSkillGroupDesignerNotes = (New-TextFromCodePoints @(0x81EA, 0x52A8, 0x751F, 0x6210, 0x7684)) + " MVP " + (New-TextFromCodePoints @(0x6E90, 0x8868, 0x884C, 0xFF1B, 0x540E, 0x7EED, 0x901A, 0x8FC7, 0x7B56, 0x5212, 0x8BC4, 0x5BA1, 0x540E, 0x7684)) + " DataTable " + (New-TextFromCodePoints @(0x66F4, 0x65B0, 0x8865, 0x9F50, 0x6570, 0x503C, 0x4E0E, 0x8D44, 0x6E90, 0x3002))
$rowCountMismatchMessage = New-TextFromCodePoints @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4, 0x6E90, 0x20, 0x43, 0x53, 0x56, 0x20, 0x884C, 0x6570, 0x4E0D, 0x6B63, 0x786E)
$missingCsvHeaderMessage = New-TextFromCodePoints @(0x6E90, 0x20, 0x43, 0x53, 0x56, 0x20, 0x7F3A, 0x5C11, 0x8868, 0x5934)
$rowIdentityMismatchMessage = New-TextFromCodePoints @(0x6E90, 0x20, 0x43, 0x53, 0x56, 0x20, 0x884C, 0x8EAB, 0x4EFD, 0x4E0D, 0x5339, 0x914D)
$unsupportedZodiacTypeMessage = New-TextFromCodePoints @(0x4E0D, 0x652F, 0x6301, 0x7684, 0x751F, 0x8096, 0x7C7B, 0x578B)
$unsupportedElementTypeMessage = New-TextFromCodePoints @(0x4E0D, 0x652F, 0x6301, 0x7684, 0x81EA, 0x7136, 0x5143, 0x7D20, 0x7C7B, 0x578B)
$resonanceElementMismatchMessage = New-TextFromCodePoints @(0x5171, 0x9E23, 0x5143, 0x7D20, 0x5FC5, 0x987B, 0x4E0E, 0x81EA, 0x7136, 0x5143, 0x7D20, 0x7C7B, 0x578B, 0x4E00, 0x81F4)
$duplicateRowMessage = New-TextFromCodePoints @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4, 0x6E90, 0x20, 0x43, 0x53, 0x56, 0x20, 0x5B58, 0x5728, 0x91CD, 0x590D, 0x884C)
$missingRequiredRowMessage = New-TextFromCodePoints @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4, 0x6E90, 0x20, 0x43, 0x53, 0x56, 0x20, 0x7F3A, 0x5C11, 0x5FC5, 0x9700, 0x884C)
$missingSourceCsvMessage = (New-TextFromCodePoints @(0x6E90)) + " CSV " + (New-TextFromCodePoints @(0x4E0D, 0x5B58, 0x5728))

$zodiacDisplayNames = @{
    Rat = New-TextFromCodePoints @(0x9F20)
    Ox = New-TextFromCodePoints @(0x725B)
    Tiger = New-TextFromCodePoints @(0x864E)
    Rabbit = New-TextFromCodePoints @(0x5154)
    Dragon = New-TextFromCodePoints @(0x9F99)
    Snake = New-TextFromCodePoints @(0x86C7)
    Horse = New-TextFromCodePoints @(0x9A6C)
    Goat = New-TextFromCodePoints @(0x7F8A)
    Monkey = New-TextFromCodePoints @(0x7334)
    Rooster = New-TextFromCodePoints @(0x9E21)
    Dog = New-TextFromCodePoints @(0x72AC)
    Pig = New-TextFromCodePoints @(0x732A)
}

$elementDisplayNames = @{
    Gold = New-TextFromCodePoints @(0x91D1)
    Wood = New-TextFromCodePoints @(0x6728)
    Water = New-TextFromCodePoints @(0x6C34)
    Fire = New-TextFromCodePoints @(0x706B)
    Earth = New-TextFromCodePoints @(0x571F)
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

function New-FixedSkillGroupRows {
    $rows = foreach ($zodiac in $zodiacs) {
        foreach ($element in $elements) {
            $rowId = "{0}_{1}" -f $zodiac, $element
            $zodiacDisplayName = $zodiacDisplayNames[$zodiac]
            $elementDisplayName = $elementDisplayNames[$element]
            [PSCustomObject]@{
                Name = $rowId
                RowId = $rowId
                DataVersion = 1
                ZodiacType = $zodiac
                ElementType = $element
                ElementPassiveSkillId = "{0}_{1}_Passive" -f $zodiac, $element
                ElementSkill1Id = "{0}_{1}_Skill01" -f $zodiac, $element
                ElementSkill2Id = "{0}_{1}_Skill02" -f $zodiac, $element
                ElementSkill3Id = "{0}_{1}_Skill03" -f $zodiac, $element
                ElementSkill4Id = "{0}_{1}_Skill04" -f $zodiac, $element
                ZodiacUltimateSkillId = "{0}_Ultimate" -f $zodiac
                ElementPassiveInputKey = ""
                ElementSkill1InputKey = "One"
                ElementSkill2InputKey = "Two"
                ElementSkill3InputKey = "Three"
                ElementSkill4InputKey = "Four"
                ZodiacUltimateInputKey = "Five"
                ResonanceInputKey = ""
                ElementResonanceLevel = 4
                ResonanceElement = $element
                ResonanceControlTimeBonus = 1.0
                ResonanceShieldBonus = 20.0
                AbilitySetAsset = ""
                DisplayName = "{0}{1}{2}" -f $zodiacDisplayName, $elementDisplayName, $fixedSkillGroupSuffix
                Description = $fixedSkillGroupDescription
                Icon = ""
                OriginalSourceName = $rowId
                SanitizedName = $rowId
                SanitizedAssetName = $rowId
                bEnabled = "True"
                bIsInDevelopment = "False"
                DesignerNotes = $fixedSkillGroupDesignerNotes
            }
        }
    }

    return @($rows)
}

function Test-FixedSkillGroupRows {
    param([Parameter(Mandatory = $true)][object[]]$Rows)

    if ($Rows.Count -ne ($zodiacs.Count * $elements.Count)) {
        throw ("{0}: {1}/{2}" -f $rowCountMismatchMessage, ($zodiacs.Count * $elements.Count), $Rows.Count)
    }

    $actualHeaders = @($Rows[0].PSObject.Properties.Name)
    foreach ($header in $expectedHeaders) {
        if ($actualHeaders -notcontains $header) {
            throw ("{0}: {1}" -f $missingCsvHeaderMessage, $header)
        }
    }

    $seen = @{}
    foreach ($row in $Rows) {
        $expectedRowId = "{0}_{1}" -f $row.ZodiacType, $row.ElementType
        if ($row.Name -ne $expectedRowId -or $row.RowId -ne $expectedRowId) {
            throw ("{0}: {1}/{2}" -f $rowIdentityMismatchMessage, $row.Name, $expectedRowId)
        }

        if ($zodiacs -notcontains $row.ZodiacType) {
            throw ("{0}: {1}/{2}" -f $unsupportedZodiacTypeMessage, $row.ZodiacType, $row.Name)
        }

        if ($elements -notcontains $row.ElementType) {
            throw ("{0}: {1}/{2}" -f $unsupportedElementTypeMessage, $row.ElementType, $row.Name)
        }

        if ($row.ResonanceElement -ne $row.ElementType) {
            throw ("{0}: {1}" -f $resonanceElementMismatchMessage, $row.Name)
        }

        if ($seen.ContainsKey($row.Name)) {
            throw ("{0}: {1}" -f $duplicateRowMessage, $row.Name)
        }

        $seen[$row.Name] = $true
    }

    foreach ($zodiac in $zodiacs) {
        foreach ($element in $elements) {
            $requiredRow = "{0}_{1}" -f $zodiac, $element
            if (-not $seen.ContainsKey($requiredRow)) {
                throw ("{0}: {1}" -f $missingRequiredRowMessage, $requiredRow)
            }
        }
    }
}

if ($ValidateOnly) {
    if (-not (Test-Path -LiteralPath $OutputPath)) {
        throw ("{0}: {1}" -f $missingSourceCsvMessage, $OutputPath)
    }

    $rows = @(Import-Csv -LiteralPath $OutputPath)
    Test-FixedSkillGroupRows -Rows $rows

    return [PSCustomObject]@{
        OutputPath = $OutputPath
        RowCount = $rows.Count
        Valid = $true
    }
}

$rowsToWrite = New-FixedSkillGroupRows
Test-FixedSkillGroupRows -Rows $rowsToWrite

$outputDirectory = Split-Path -Parent $OutputPath
if (-not (Test-Path -LiteralPath $outputDirectory)) {
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
}

$rowsToWrite |
    Select-Object $expectedHeaders |
    Export-Csv -LiteralPath $OutputPath -NoTypeInformation -Encoding UTF8

return [PSCustomObject]@{
    OutputPath = $OutputPath
    RowCount = $rowsToWrite.Count
    Valid = $true
}
