#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ENV_FILE="${BACKEND_DIR}/.env"

COMPOSE_ENV_ARGS=()
if [[ -f "${ENV_FILE}" ]]; then
  COMPOSE_ENV_ARGS=(--env-file "${ENV_FILE}")
fi

docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" run --rm \
  -e Database__RunMigrationsAndExit=true \
  game-api
