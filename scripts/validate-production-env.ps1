<#
Validate a DBA_GameBackend production .env file before deployment.
Example:
  .\scripts\validate-production-env.ps1 -EnvFile .\DBA_GameBackend\.env
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$EnvFile
)

$ErrorActionPreference = "Stop"
$resolved = Resolve-Path $EnvFile
$values = @{}

Get-Content $resolved | ForEach-Object {
    $line = $_.Trim()
    if (-not $line -or $line.StartsWith("#")) {
        return
    }

    $idx = $line.IndexOf("=")
    if ($idx -le 0) {
        return
    }

    $key = $line.Substring(0, $idx).Trim()
    $value = $line.Substring($idx + 1).Trim().Trim('"').Trim("'")
    $values[$key] = $value
}

$errors = New-Object System.Collections.Generic.List[string]

function Require-Key {
    param([string]$Key)
    if (-not $values.ContainsKey($Key) -or [string]::IsNullOrWhiteSpace($values[$Key])) {
        $errors.Add("$Key is required.")
    }
}

function Reject-Placeholder {
    param([string]$Key)
    if ($values.ContainsKey($Key) -and $values[$Key] -match "change-me|your-github-org-or-user|example\.com|admin@example\.com") {
        $errors.Add("$Key still contains a placeholder value.")
    }
}

@(
    "IMAGE_NAMESPACE",
    "POSTGRES_PASSWORD",
    "REDIS_PASSWORD",
    "JWT_SECRET",
    "INTERNAL_API_KEY",
    "PUBLIC_DOMAIN",
    "ACME_EMAIL",
    "GRAFANA_ADMIN_PASSWORD"
) | ForEach-Object {
    Require-Key $_
    Reject-Placeholder $_
}

if ($values.ContainsKey("JWT_SECRET") -and $values["JWT_SECRET"].Length -lt 32) {
    $errors.Add("JWT_SECRET must be at least 32 characters.")
}

if ($values.ContainsKey("ASPNETCORE_ENVIRONMENT") -and $values["ASPNETCORE_ENVIRONMENT"] -ne "Production") {
    $errors.Add("ASPNETCORE_ENVIRONMENT must be Production.")
}

if ($values.ContainsKey("DOTNET_ENVIRONMENT") -and $values["DOTNET_ENVIRONMENT"] -ne "Production") {
    $errors.Add("DOTNET_ENVIRONMENT must be Production.")
}

if ($values.ContainsKey("SEED_DATA_ENABLED") -and $values["SEED_DATA_ENABLED"].ToLowerInvariant() -ne "false") {
    $errors.Add("SEED_DATA_ENABLED must be false in production.")
}

if ($values.ContainsKey("SWAGGER_ENABLED") -and $values["SWAGGER_ENABLED"].ToLowerInvariant() -ne "false") {
    $errors.Add("SWAGGER_ENABLED must be false in production unless protected by an external gateway.")
}

if ($values.ContainsKey("DATABASE_RUN_MIGRATIONS_AND_EXIT") -and $values["DATABASE_RUN_MIGRATIONS_AND_EXIT"].ToLowerInvariant() -ne "false") {
    $errors.Add("DATABASE_RUN_MIGRATIONS_AND_EXIT must be false for normal runtime containers.")
}

if ($values.ContainsKey("API_HTTP_BIND") -and $values["API_HTTP_BIND"] -notin @("127.0.0.1", "localhost")) {
    Write-Host "Warning: API_HTTP_BIND is not loopback. Ensure this is intentional and protected by a firewall or gateway." -ForegroundColor Yellow
}

if ($values.ContainsKey("GAME_SERVER_PORT_START") -and $values.ContainsKey("GAME_SERVER_PORT_END")) {
    $start = 0
    $end = 0
    if (-not [int]::TryParse($values["GAME_SERVER_PORT_START"], [ref]$start) -or -not [int]::TryParse($values["GAME_SERVER_PORT_END"], [ref]$end)) {
        $errors.Add("GAME_SERVER_PORT_START and GAME_SERVER_PORT_END must be integers.")
    }
    elseif ($start -le 0 -or $end -lt $start) {
        $errors.Add("Game server port range is invalid.")
    }
}

if ($errors.Count -gt 0) {
    Write-Host "Production env validation failed:" -ForegroundColor Red
    foreach ($error in $errors) {
        Write-Host "- $error" -ForegroundColor Red
    }
    exit 1
}

Write-Host "Production env validation passed: $resolved" -ForegroundColor Green
