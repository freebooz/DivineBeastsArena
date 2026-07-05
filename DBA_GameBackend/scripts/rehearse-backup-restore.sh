#!/usr/bin/env bash
set -euo pipefail

# Chinese notes:
# - Runs a PostgreSQL backup/restore rehearsal against Docker Compose.
# - Creates and drops a temporary restore database; it does not overwrite the source DB.
# - Writes manifest-ready evidence under Artifacts/ProductionEvidence/ops by default.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ENV_FILE="${BACKEND_DIR}/.env"
RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)}"
EVIDENCE_DIR="${EVIDENCE_DIR:-${BACKEND_DIR}/../Artifacts/ProductionEvidence/ops}"
LOG_PATH="${EVIDENCE_DIR}/backup-restore-rehearsal-${RUN_ID}.log"
SUMMARY_PATH="${EVIDENCE_DIR}/backup-restore-rehearsal-${RUN_ID}.json"

mkdir -p "${EVIDENCE_DIR}"

COMPOSE_ENV_ARGS=()
if [[ -f "${ENV_FILE}" ]]; then
  COMPOSE_ENV_ARGS=(--env-file "${ENV_FILE}")
fi

set -a
if [[ -f "${ENV_FILE}" ]]; then
  # shellcheck disable=SC1090
  source "${ENV_FILE}"
fi
set +a

POSTGRES_DB="${POSTGRES_DB:-game_platform}"
POSTGRES_USER="${POSTGRES_USER:-game_app}"
BACKUP_DIR="${BACKUP_DIR:-${BACKEND_DIR}/backups}"

latest_backup=""
restore_db=""
restore_db_dropped="true"
table_count=""

json_escape() {
  local value="$1"
  value="${value//\\/\\\\}"
  value="${value//\"/\\\"}"
  value="${value//$'\n'/\\n}"
  value="${value//$'\r'/}"
  printf '%s' "${value}"
}

write_summary() {
  local status="$1"
  local exit_code="$2"
  local generated_at
  generated_at="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

  cat >"${SUMMARY_PATH}" <<JSON
{
  "schemaVersion": "1.0",
  "runId": "$(json_escape "${RUN_ID}")",
  "status": "$(json_escape "${status}")",
  "exitCode": ${exit_code},
  "generatedAtUtc": "$(json_escape "${generated_at}")",
  "postgresDb": "$(json_escape "${POSTGRES_DB}")",
  "postgresUser": "$(json_escape "${POSTGRES_USER}")",
  "backupFile": "$(json_escape "${latest_backup}")",
  "restoreDatabase": "$(json_escape "${restore_db}")",
  "publicTableCount": "$(json_escape "${table_count}")",
  "logFile": "$(json_escape "${LOG_PATH}")"
}
JSON
}

drop_restore_db_if_needed() {
  if [[ -n "${restore_db}" && "${restore_db_dropped}" != "true" ]]; then
    docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
      dropdb -U "${POSTGRES_USER}" --if-exists "${restore_db}" >/dev/null 2>&1 || true
    restore_db_dropped="true"
  fi
}

run_rehearsal() {
  echo "Backup restore rehearsal run: ${RUN_ID}"
  echo "Evidence log: ${LOG_PATH}"
  echo "Evidence summary: ${SUMMARY_PATH}"

  bash "${BACKEND_DIR}/scripts/backup-postgres.sh"
  latest_backup="$(ls -t "${BACKUP_DIR}"/*.sql.gz | head -n 1)"
  restore_db="${POSTGRES_DB}_restore_rehearsal_$(date +%Y%m%d%H%M%S)"
  restore_db_dropped="false"

  docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
    createdb -U "${POSTGRES_USER}" "${restore_db}"

  gzip -dc "${latest_backup}" | docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
    psql -U "${POSTGRES_USER}" "${restore_db}" >/dev/null

  table_count="$(docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
    psql -U "${POSTGRES_USER}" "${restore_db}" -t -A -c "select count(*) from information_schema.tables where table_schema='public';" | tr -d '[:space:]')"
  echo "Restored public table count: ${table_count}"

  drop_restore_db_if_needed

  echo "Backup restore rehearsal completed with ${latest_backup}."
}

set +e
run_rehearsal > >(tee "${LOG_PATH}") 2>&1
status_code=$?
set -e

if [[ ${status_code} -eq 0 ]]; then
  write_summary "passed" "${status_code}"
  echo "Backup restore rehearsal evidence written: ${SUMMARY_PATH}"
else
  drop_restore_db_if_needed
  write_summary "failed" "${status_code}"
  echo "Backup restore rehearsal failed. Evidence written: ${SUMMARY_PATH}" >&2
  exit "${status_code}"
fi
