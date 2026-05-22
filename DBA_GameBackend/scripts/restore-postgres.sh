#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: restore-postgres.sh <backup.sql.gz>" >&2
  exit 64
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ENV_FILE="${BACKEND_DIR}/.env"
BACKUP_FILE="$1"

if [[ ! -f "${BACKUP_FILE}" ]]; then
  echo "Backup file not found: ${BACKUP_FILE}" >&2
  exit 66
fi

COMPOSE_ENV_ARGS=()
if [[ -f "${ENV_FILE}" ]]; then
  set -a
  # shellcheck disable=SC1090
  source "${ENV_FILE}"
  set +a
  COMPOSE_ENV_ARGS=(--env-file "${ENV_FILE}")
fi

POSTGRES_DB="${POSTGRES_DB:-game_platform}"
POSTGRES_USER="${POSTGRES_USER:-game_app}"

gzip -dc "${BACKUP_FILE}" | docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
  psql -U "${POSTGRES_USER}" "${POSTGRES_DB}"
