#!/bin/bash
set -e

cd "$(dirname "$0")/.."

echo "Creating DBA_GameBackend .NET solution..."

# Create projects
dotnet new sln -n DBA_GameBackend
dotnet new webapi -n Game.Api --no-openapi
dotnet new worker -n Game.Worker
dotnet new classlib -n Game.Shared
dotnet new classlib -n Game.Infrastructure
dotnet new xunit -n Game.Api.Tests
dotnet new xunit -n Game.IntegrationTests

# Add to solution
dotnet sln add Game.Api/Game.Api.csproj
dotnet sln add Game.Worker/Game.Worker.csproj
dotnet sln add Game.Shared/Game.Shared.csproj
dotnet sln add Game.Infrastructure/Game.Infrastructure.csproj
dotnet sln add Game.Api.Tests/Game.Api.Tests.csproj
dotnet sln add Game.IntegrationTests/Game.IntegrationTests.csproj

echo "Done!"