<#
Tests the internal API route protection validator.

The fixtures are small C# endpoint snippets. They keep the validator honest
without booting ASP.NET, Unreal, databases, or external services.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$validator = Join-Path $repoRoot "scripts\validate-internal-api-route-protection.ps1"

if (-not (Test-Path -LiteralPath $validator)) {
    throw "Missing validator: scripts\validate-internal-api-route-protection.ps1"
}

$fixtureRoot = Join-Path $repoRoot (".tmp\internal-api-route-protection-fixtures\{0}" -f [guid]::NewGuid().ToString("N"))
if (Test-Path -LiteralPath $fixtureRoot) {
    Remove-Item -LiteralPath $fixtureRoot -Recurse -Force
}
New-Item -ItemType Directory -Path $fixtureRoot | Out-Null

try {
    $protectedByFilter = @'
using Game.Api.Extensions;

public static class ProtectedByFilterEndpoints
{
    public static void Map(IEndpointRouteBuilder app)
    {
        var internalGroup = app.MapGroup("/internal/filter-protected");
        internalGroup.AddEndpointFilter(InternalApiKeyEndpointFilter.RequireInternalApiKey);
        internalGroup.MapGet("/", List);
    }

    private static IResult List() => Results.Ok();
}
'@

    $protectedByHandler = @'
using Game.Api.Extensions;

public static class ProtectedByHandlerEndpoints
{
    public static void Map(IEndpointRouteBuilder app)
    {
        var internalGroup = app.MapGroup("/internal/handler-protected");
        internalGroup.MapPost("/{id}", Update);
    }

    private static IResult Update(Guid id, HttpContext httpContext)
    {
        var unauthorized = InternalApiKeyEndpointFilter.Validate(httpContext);
        if (unauthorized is not null) return unauthorized;
        return Results.Ok();
    }
}
'@

    $directProtectedByFilter = @'
using Game.Api.Extensions;

public static class DirectProtectedByFilterEndpoints
{
    public static void Map(IEndpointRouteBuilder app)
    {
        app.MapGet("/internal/direct-filter-protected", List)
            .AddEndpointFilter(InternalApiKeyEndpointFilter.RequireInternalApiKey);
    }

    private static IResult List() => Results.Ok();
}
'@

    $directLambdaProtectedByFilter = @'
using Game.Api.Extensions;

public static class DirectLambdaProtectedByFilterEndpoints
{
    public static void Map(IEndpointRouteBuilder app)
    {
        app.MapGet("/internal/direct-lambda-filter-protected", () => Results.Ok())
            .AddEndpointFilter(InternalApiKeyEndpointFilter.RequireInternalApiKey);
    }
}
'@

    $unprotected = @'
public static class UnprotectedInternalEndpoints
{
    public static void Map(IEndpointRouteBuilder app)
    {
        var internalGroup = app.MapGroup("/internal/unprotected");
        internalGroup.MapGet("/", List);
    }

    private static IResult List() => Results.Ok();
}
'@

    $directUnprotected = @'
public static class DirectUnprotectedInternalEndpoints
{
    public static void Map(IEndpointRouteBuilder app)
    {
        app.MapGet("/internal/direct-unprotected", List);
    }

    private static IResult List() => Results.Ok();
}
'@

    $directLambdaUnprotected = @'
public static class DirectLambdaUnprotectedInternalEndpoints
{
    public static void Map(IEndpointRouteBuilder app)
    {
        app.MapGet("/internal/direct-lambda-unprotected", () => Results.Ok());
    }
}
'@

    $chainedUnprotected = @'
public static class ChainedUnprotectedInternalEndpoints
{
    public static void Map(IEndpointRouteBuilder app)
    {
        app.MapGroup("/internal/chained-unprotected")
            .MapGet("/", List);
    }

    private static IResult List() => Results.Ok();
}
'@

    function New-ProtectedFixtureSet {
        param([Parameter(Mandatory = $true)][string]$Root)

        New-Item -ItemType Directory -Force -Path $Root | Out-Null
        Set-Content -LiteralPath (Join-Path $Root "ProtectedByFilterEndpoints.cs") -Encoding UTF8 -Value $protectedByFilter
        Set-Content -LiteralPath (Join-Path $Root "ProtectedByHandlerEndpoints.cs") -Encoding UTF8 -Value $protectedByHandler
        Set-Content -LiteralPath (Join-Path $Root "DirectProtectedByFilterEndpoints.cs") -Encoding UTF8 -Value $directProtectedByFilter
        Set-Content -LiteralPath (Join-Path $Root "DirectLambdaProtectedByFilterEndpoints.cs") -Encoding UTF8 -Value $directLambdaProtectedByFilter
    }

    $validFixtureRoot = Join-Path $fixtureRoot "valid"
    New-ProtectedFixtureSet -Root $validFixtureRoot
    & $validator -EndpointRoot $validFixtureRoot

    $unprotectedFixtureRoot = Join-Path $fixtureRoot "unprotected"
    New-ProtectedFixtureSet -Root $unprotectedFixtureRoot
    Set-Content -LiteralPath (Join-Path $unprotectedFixtureRoot "UnprotectedInternalEndpoints.cs") -Encoding UTF8 -Value $unprotected

    $failedAsExpected = $false
    try {
        & $validator -EndpointRoot $unprotectedFixtureRoot
    }
    catch {
        $failedAsExpected = $true
        if ($_.Exception.Message -notmatch "/internal/unprotected") {
            throw "Expected failure message to name the unprotected route, got: $($_.Exception.Message)"
        }
    }

    if (-not $failedAsExpected) {
        throw "Expected validator to reject an unprotected /internal route fixture."
    }

    $directUnprotectedFixtureRoot = Join-Path $fixtureRoot "direct-unprotected"
    New-ProtectedFixtureSet -Root $directUnprotectedFixtureRoot
    Set-Content -LiteralPath (Join-Path $directUnprotectedFixtureRoot "DirectUnprotectedInternalEndpoints.cs") -Encoding UTF8 -Value $directUnprotected

    $directFailedAsExpected = $false
    try {
        & $validator -EndpointRoot $directUnprotectedFixtureRoot
    }
    catch {
        $directFailedAsExpected = $true
        if ($_.Exception.Message -notmatch "/internal/direct-unprotected") {
            throw "Expected failure message to name the direct unprotected route, got: $($_.Exception.Message)"
        }
    }

    if (-not $directFailedAsExpected) {
        throw "Expected validator to reject an unprotected direct /internal route fixture."
    }

    $directLambdaUnprotectedFixtureRoot = Join-Path $fixtureRoot "direct-lambda-unprotected"
    New-ProtectedFixtureSet -Root $directLambdaUnprotectedFixtureRoot
    Set-Content -LiteralPath (Join-Path $directLambdaUnprotectedFixtureRoot "DirectLambdaUnprotectedInternalEndpoints.cs") -Encoding UTF8 -Value $directLambdaUnprotected

    $directLambdaFailedAsExpected = $false
    try {
        & $validator -EndpointRoot $directLambdaUnprotectedFixtureRoot
    }
    catch {
        $directLambdaFailedAsExpected = $true
        if ($_.Exception.Message -notmatch "/internal/direct-lambda-unprotected") {
            throw "Expected failure message to name the direct lambda unprotected route, got: $($_.Exception.Message)"
        }
    }

    if (-not $directLambdaFailedAsExpected) {
        throw "Expected validator to reject an unprotected direct lambda /internal route fixture."
    }

    $chainedUnprotectedFixtureRoot = Join-Path $fixtureRoot "chained-unprotected"
    New-ProtectedFixtureSet -Root $chainedUnprotectedFixtureRoot
    Set-Content -LiteralPath (Join-Path $chainedUnprotectedFixtureRoot "ChainedUnprotectedInternalEndpoints.cs") -Encoding UTF8 -Value $chainedUnprotected

    $chainedFailedAsExpected = $false
    try {
        & $validator -EndpointRoot $chainedUnprotectedFixtureRoot
    }
    catch {
        $chainedFailedAsExpected = $true
        if ($_.Exception.Message -notmatch "/internal/chained-unprotected") {
            throw "Expected failure message to name the chained unprotected route, got: $($_.Exception.Message)"
        }
    }

    if (-not $chainedFailedAsExpected) {
        throw "Expected validator to reject an unprotected chained /internal route fixture."
    }
}
finally {
    if (Test-Path -LiteralPath $fixtureRoot) {
        try {
            Remove-Item -LiteralPath $fixtureRoot -Recurse -Force -ErrorAction SilentlyContinue
        }
        catch {
        }
    }
}

Write-Host "PASS: internal API route protection contract" -ForegroundColor Green
