#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ENV_FILE="${BACKEND_DIR}/.env"

COMPOSE_ENV_ARGS=()
if [[ -f "${ENV_FILE}" ]]; then
  set -a
  # shellcheck disable=SC1090
  source "${ENV_FILE}"
  set +a
  COMPOSE_ENV_ARGS=(--env-file "${ENV_FILE}")
fi

BACKUP_DIR="${BACKUP_DIR:-${BACKEND_DIR}/backups}"
BACKUP_KEEP_DAYS="${BACKUP_KEEP_DAYS:-14}"
POSTGRES_DB="${POSTGRES_DB:-game_platform}"
POSTGRES_USER="${POSTGRES_USER:-game_app}"

mkdir -p "${BACKUP_DIR}"
BACKUP_FILE="${BACKUP_DIR}/postgres-${POSTGRES_DB}-$(date -u +%Y%m%dT%H%M%SZ).sql.gz"

docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
  pg_dump -U "${POSTGRES_USER}" "${POSTGRES_DB}" | gzip -9 > "${BACKUP_FILE}"

find "${BACKUP_DIR}" -type f -name "postgres-${POSTGRES_DB}-*.sql.gz" -mtime "+${BACKUP_KEEP_DAYS}" -delete
echo "PostgreSQL backup written to ${BACKUP_FILE}"
