#!/bin/bash
set -e

echo "Bootstrapping MyGamePlatform repository..."

# Create directories
mkdir -p ops/docker ops/nginx ops/prometheus ops/grafana ops/loki ops/scripts
mkdir -p configs/character configs/skill configs/match configs/reward configs/map
mkdir -p docs
mkdir -p scripts

echo "Repository bootstrapped successfully!"
echo ""
echo "Next steps:"
echo "  cd DBA_GameBackend && dotnet restore && dotnet build"
echo "  cd DBA_GameAdmin && dotnet build"
echo "  cd DBA_GameWebsite && npm install"
echo "  cd DBA_GameLauncher && npm install"
