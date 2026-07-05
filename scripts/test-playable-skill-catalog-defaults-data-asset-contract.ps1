<#
验证可玩技能默认目录必须由数据资产或配置驱动，不能继续在 C++ 中硬编码默认技能数值、资源路径和展示文本。
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$componentHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBAPlayableSkillComponent.h"
$componentCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAPlayableSkillComponent.cpp"

function New-CodePointText {
    param([int[]]$CodePoints)

    return -join ($CodePoints | ForEach-Object { [string][char]$_ })
}

function Assert-Contains {
    param(
        [string]$Content,
        [string]$Token,
        [string]$Message
    )

    if (-not $Content.Contains($Token)) {
        throw $Message
    }
}

function Assert-NotContains {
    param(
        [string]$Content,
        [string]$Token,
        [string]$Message
    )

    if ($Content.Contains($Token)) {
        throw $Message
    }
}

function Get-FunctionBody {
    param(
        [string]$Content,
        [string]$Signature
    )

    $start = $Content.IndexOf($Signature)
    if ($start -lt 0) {
        throw "未找到函数：$Signature"
    }

    $braceStart = $Content.IndexOf("{", $start)
    if ($braceStart -lt 0) {
        throw "未找到函数体：$Signature"
    }

    $depth = 0
    for ($index = $braceStart; $index -lt $Content.Length; $index++) {
        $char = $Content[$index]
        if ($char -eq "{") {
            $depth++
        }
        elseif ($char -eq "}") {
            $depth--
            if ($depth -eq 0) {
                return $Content.Substring($braceStart, $index - $braceStart + 1)
            }
        }
    }

    throw "函数体未闭合：$Signature"
}

$componentHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $componentHeaderPath
$componentCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $componentCppPath
$resetBody = Get-FunctionBody -Content $componentCpp -Signature 'void UDBAPlayableSkillComponent::ResetToDefaultSkillSpecs()'

Assert-Contains -Content $componentHeader -Token 'DefaultSkillCatalog' -Message "技能组件必须暴露默认技能目录数据资产软引用。"
Assert-Contains -Content $componentHeader -Token 'RequestDefaultSkillCatalogAsync' -Message "技能组件必须通过异步路径请求默认技能目录。"
Assert-Contains -Content $componentCpp -Token 'GetDefault<UDBAPlayableSkillDeveloperSettings>()' -Message "技能组件必须读取 DeveloperSettings 中的默认技能目录配置。"
Assert-Contains -Content $componentCpp -Token 'DBAAsyncAssetLoader::RequestAsyncAsset<UDBAPlayableSkillCatalogDataAsset>' -Message "默认技能目录必须异步加载，不能阻塞加载资源。"

Assert-NotContains -Content $resetBody -Token 'MakeSkill(' -Message "ResetToDefaultSkillSpecs 不得在 C++ 中构造硬编码技能。"
Assert-NotContains -Content $resetBody -Token 'NSLOCTEXT(' -Message "ResetToDefaultSkillSpecs 不得在 C++ 中写死默认技能展示文本。"
Assert-NotContains -Content $resetBody -Token 'TEXT("/Game/' -Message "ResetToDefaultSkillSpecs 不得在 C++ 中写死表现资源路径。"
Assert-NotContains -Content $resetBody -Token 'ADBAFireballProjectile::StaticClass()' -Message "ResetToDefaultSkillSpecs 不得在 C++ 中写死投射物技能类。"
Assert-NotContains -Content $resetBody -Token 'ADBAFrostShardProjectile::StaticClass()' -Message "ResetToDefaultSkillSpecs 不得在 C++ 中写死投射物技能类。"
Assert-NotContains -Content $resetBody -Token 'ADBABloomHealingSpell::StaticClass()' -Message "ResetToDefaultSkillSpecs 不得在 C++ 中写死治疗技能类。"
Assert-NotContains -Content $resetBody -Token 'ADBAChainLightningSpell::StaticClass()' -Message "ResetToDefaultSkillSpecs 不得在 C++ 中写死连锁技能类。"
Assert-NotContains -Content $resetBody -Token 'ADBAHolyShieldSpell::StaticClass()' -Message "ResetToDefaultSkillSpecs 不得在 C++ 中写死护盾技能类。"

$successMessage = New-CodePointText @(0x901A, 0x8FC7, 0xFF1A, 0x53EF, 0x73A9, 0x6280, 0x80FD, 0x9ED8, 0x8BA4, 0x76EE, 0x5F55, 0x5DF2, 0x6539, 0x4E3A, 0x6570, 0x636E, 0x8D44, 0x4EA7, 0x9A71, 0x52A8)
Write-Host $successMessage -ForegroundColor Green
