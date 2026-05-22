#!/bin/bash
set -e

echo "Bootstrapping MyGamePlatform repository..."

# Create directories
mkdir -p GamePlatformOps/docker GamePlatformOps/nginx GamePlatformOps/prometheus GamePlatformOps/grafana GamePlatformOps/loki GamePlatformOps/scripts
mkdir -p GamePlatformConfigs/character GamePlatformConfigs/skill GamePlatformConfigs/match GamePlatformConfigs/reward GamePlatformConfigs/map
mkdir -p GamePlatformDocs
mkdir -p GamePlatformScripts

echo "Repository bootstrapped successfully!"
echo ""
echo "Next steps:"
echo "  cd DBA_GameBackend && dotnet restore && dotnet build"
echo "  cd DBA_GameAdmin && dotnet build"
echo "  cd DBA_GameWebsite && npm install"
echo "  cd DBA_GameLauncher && npm install"
