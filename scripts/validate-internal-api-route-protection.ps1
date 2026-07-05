<#
Validates that every Minimal API /internal route group has internal API key
protection.

Accepted protection shapes:
- group.AddEndpointFilter(InternalApiKeyEndpointFilter.RequireInternalApiKey)
- every mapped handler in the group accepts HttpContext and calls
  InternalApiKeyEndpointFilter.Validate(httpContext)
#>

[CmdletBinding()]
param(
    [string]$EndpointRoot = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")) "DBA_GameBackend\Game.Api\Endpoints")
)

$ErrorActionPreference = "Stop"

function Get-HandlerBody {
    param(
        [Parameter(Mandatory = $true)][string]$Content,
        [Parameter(Mandatory = $true)][string]$HandlerName
    )

    $escapedName = [regex]::Escape($HandlerName)
    $pattern = "(?s)(?:private|public|internal)\s+static\s+.*?\s+$escapedName\s*\((?<params>.*?)\)\s*\{(?<body>.*?)(?=\r?\n\s*(?:private|public|internal)\s+static|\r?\n\})"
    $match = [regex]::Match($Content, $pattern)
    if (-not $match.Success) {
        return $null
    }

    [pscustomobject]@{
        Parameters = $match.Groups["params"].Value
        Body = $match.Groups["body"].Value
    }
}

function Test-HandlerHasInternalApiValidation {
    param(
        [Parameter(Mandatory = $true)][string]$Content,
        [Parameter(Mandatory = $true)][string]$HandlerName
    )

    $handler = Get-HandlerBody -Content $Content -HandlerName $HandlerName
    if ($null -eq $handler) {
        return $false
    }

    return $handler.Parameters -match "\bHttpContext\s+httpContext\b" -and
        $handler.Body -match "InternalApiKeyEndpointFilter\.Validate\s*\(\s*httpContext\s*\)"
}

function Get-InternalRouteGroups {
    param([Parameter(Mandatory = $true)][string]$Content)

    $groupPattern = "(?m)^\s*var\s+(?<var>\w+)\s*=\s*app\.MapGroup\s*\(\s*`"(?<route>/internal[^`"]*)`"\s*\)"
    foreach ($match in [regex]::Matches($Content, $groupPattern)) {
        [pscustomobject]@{
            Variable = $match.Groups["var"].Value
            Route = $match.Groups["route"].Value
        }
    }
}

function Get-MappedHandlers {
    param(
        [Parameter(Mandatory = $true)][string]$Content,
        [Parameter(Mandatory = $true)][string]$GroupVariable
    )

    $escapedVariable = [regex]::Escape($GroupVariable)
    $mapPattern = "(?m)^\s*$escapedVariable\.Map(?:Get|Post|Put|Delete|Patch)\s*\(\s*[^,\r\n]+,\s*(?<handler>\w+)"
    foreach ($match in [regex]::Matches($Content, $mapPattern)) {
        $match.Groups["handler"].Value
    }
}

function Get-DirectInternalRouteHandlers {
    param([Parameter(Mandatory = $true)][string]$Content)

    $mapPattern = "(?ms)^\s*app\.Map(?:Get|Post|Put|Delete|Patch)\s*\(\s*`"(?<route>/internal[^`"]*)`"\s*,\s*(?<handler>.*?)\)(?<chain>(?:\s*\.[^;]+)?)\s*;"
    foreach ($match in [regex]::Matches($Content, $mapPattern)) {
        [pscustomobject]@{
            Route = $match.Groups["route"].Value
            Handler = $match.Groups["handler"].Value.Trim()
            Chain = $match.Groups["chain"].Value
        }
    }
}

function Get-ChainedInternalRouteGroups {
    param([Parameter(Mandatory = $true)][string]$Content)

    $groupPattern = "(?ms)app\.MapGroup\s*\(\s*`"(?<route>/internal[^`"]*)`"\s*\)(?<chain>(?:\s*\.[^;]+)+)\s*;"
    foreach ($match in [regex]::Matches($Content, $groupPattern)) {
        [pscustomobject]@{
            Route = $match.Groups["route"].Value
            Chain = $match.Groups["chain"].Value
        }
    }
}

function Get-ChainedMappedHandlers {
    param([Parameter(Mandatory = $true)][string]$Chain)

    $mapPattern = "\.Map(?:Get|Post|Put|Delete|Patch)\s*\(\s*[^,\r\n]+,\s*(?<handler>\w+)"
    foreach ($match in [regex]::Matches($Chain, $mapPattern)) {
        $match.Groups["handler"].Value
    }
}

$endpointRootPath = Resolve-Path -LiteralPath $EndpointRoot
$files = @(Get-ChildItem -LiteralPath $endpointRootPath -Recurse -Filter *.cs)
$violations = New-Object System.Collections.Generic.List[string]

foreach ($file in $files) {
    $content = Get-Content -Raw -Encoding UTF8 -LiteralPath $file.FullName
    $groups = @(Get-InternalRouteGroups -Content $content)

    foreach ($group in $groups) {
        $filterPattern = [regex]::Escape($group.Variable) + "\.AddEndpointFilter\s*\(\s*InternalApiKeyEndpointFilter\.RequireInternalApiKey\s*\)"
        if ($content -match $filterPattern) {
            continue
        }

        $handlers = @(Get-MappedHandlers -Content $content -GroupVariable $group.Variable)
        if ($handlers.Count -eq 0) {
            $violations.Add("$($file.FullName): $($group.Route) has no detected mapped handlers and no internal API key filter.")
            continue
        }

        foreach ($handler in $handlers) {
            if (-not (Test-HandlerHasInternalApiValidation -Content $content -HandlerName $handler)) {
                $violations.Add("$($file.FullName): $($group.Route) handler $handler is missing HttpContext/internal API key validation.")
            }
        }
    }

    $directHandlers = @(Get-DirectInternalRouteHandlers -Content $content)
    foreach ($direct in $directHandlers) {
        if ($direct.Chain -match "InternalApiKeyEndpointFilter\.RequireInternalApiKey") {
            continue
        }

        if ($direct.Handler -notmatch "^\w+$") {
            $violations.Add("$($file.FullName): $($direct.Route) direct handler expression is missing endpoint/internal API key validation.")
            continue
        }

        if (-not (Test-HandlerHasInternalApiValidation -Content $content -HandlerName $direct.Handler)) {
            $violations.Add("$($file.FullName): $($direct.Route) handler $($direct.Handler) is missing HttpContext/internal API key validation.")
        }
    }

    $chainedGroups = @(Get-ChainedInternalRouteGroups -Content $content)
    foreach ($chained in $chainedGroups) {
        if ($chained.Chain -notmatch "\.Map(?:Get|Post|Put|Delete|Patch)\s*\(") {
            continue
        }

        if ($chained.Chain -match "InternalApiKeyEndpointFilter\.RequireInternalApiKey") {
            continue
        }

        $handlers = @(Get-ChainedMappedHandlers -Chain $chained.Chain)
        if ($handlers.Count -eq 0) {
            $violations.Add("$($file.FullName): $($chained.Route) has no detected chained mapped handlers and no internal API key filter.")
            continue
        }

        foreach ($handler in $handlers) {
            if (-not (Test-HandlerHasInternalApiValidation -Content $content -HandlerName $handler)) {
                $violations.Add("$($file.FullName): $($chained.Route) handler $handler is missing HttpContext/internal API key validation.")
            }
        }
    }
}

if ($violations.Count -gt 0) {
    throw "Unprotected /internal route groups detected:`n$($violations -join "`n")"
}

Write-Host "PASS: internal API route protection validation" -ForegroundColor Green
