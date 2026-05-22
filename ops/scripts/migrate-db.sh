#!/bin/bash
set -e

echo "Running database migrations..."

cd /app/DBA_GameBackend

dotnet ef database update \
  --project Game.Infrastructure/Game.Infrastructure.csproj \
  --startup-project Game.Api/Game.Api.csproj \
  --context GameDbContext

echo "Migrations completed"
