#!/usr/bin/env bash
set -euo pipefail

# 中文阅读说明：
# - 文件职责：在 Docker Compose PostgreSQL 服务上执行备份恢复演练。
# - 使用方式：bash scripts/rehearse-backup-restore.sh
# - 修改提示：脚本会创建并删除临时数据库，不会覆盖正式库，但仍建议在预发环境先跑。

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ENV_FILE="${BACKEND_DIR}/.env"

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

bash "${BACKEND_DIR}/scripts/backup-postgres.sh"
latest_backup="$(ls -t "${BACKUP_DIR}"/*.sql.gz | head -n 1)"
restore_db="${POSTGRES_DB}_restore_rehearsal_$(date +%Y%m%d%H%M%S)"

docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
  createdb -U "${POSTGRES_USER}" "${restore_db}"

gzip -dc "${latest_backup}" | docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
  psql -U "${POSTGRES_USER}" "${restore_db}" >/dev/null

docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
  psql -U "${POSTGRES_USER}" "${restore_db}" -c "select count(*) as tables from information_schema.tables where table_schema='public';"

docker compose "${COMPOSE_ENV_ARGS[@]}" -f "${BACKEND_DIR}/docker-compose.yml" exec -T postgres \
  dropdb -U "${POSTGRES_USER}" "${restore_db}"

echo "Backup restore rehearsal completed with ${latest_backup}."
