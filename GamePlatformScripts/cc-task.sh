#!/bin/bash
# Claude Code task runner

TASK_TYPE="${1:-default}"
shift

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

case "$TASK_TYPE" in
  build)
    cd "$ROOT_DIR/DBA_GameBackend" && dotnet build GameBackend.slnx
    ;;
  test)
    cd "$ROOT_DIR/DBA_GameBackend" && dotnet test GameBackend.slnx
    ;;
  admin)
    cd "$ROOT_DIR/DBA_GameAdmin" && dotnet build
    ;;
  website)
    cd "$ROOT_DIR/DBA_GameWebsite" && npm install && npm run build
    ;;
  launcher)
    cd "$ROOT_DIR/DBA_GameLauncher" && npm install && cargo check --manifest-path src-tauri/Cargo.toml
    ;;
  *)
    echo "Usage: $0 {build|test|admin|website|launcher}"
    ;;
esac
