#!/usr/bin/env bash
set -euo pipefail

# 中文阅读说明：
# - 文件职责：顺序运行上线前 k6 压测脚本。
# - 使用方式：BASE_URL=http://localhost:8080 INTERNAL_API_KEY=xxx EVIDENCE_DIR=../Artifacts/ProductionEvidence/load bash scripts/run-load-tests.sh
# - Docker 用法：USE_DOCKER_K6=1 AUTH_MODE=guest bash scripts/run-load-tests.sh
# - 修改提示：生产压测请先确认测试账号、端口范围和隔离数据库，避免污染正式玩家数据。

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)}"
EVIDENCE_DIR="${EVIDENCE_DIR:-${BACKEND_DIR}/../Artifacts/ProductionEvidence/load}"
K6_DOCKER_IMAGE="${K6_DOCKER_IMAGE:-grafana/k6:latest}"
USE_DOCKER_K6="${USE_DOCKER_K6:-auto}"
BASE_URL="${BASE_URL:-http://localhost:8080}"
K6_BASE_URL="${K6_BASE_URL:-${BASE_URL}}"

if [[ "${USE_DOCKER_K6}" == "auto" ]]; then
  if command -v k6 >/dev/null 2>&1; then
    K6_RUNNER="native"
  elif command -v docker >/dev/null 2>&1; then
    K6_RUNNER="docker"
  else
    echo "k6 is required. Install k6 or Docker with ${K6_DOCKER_IMAGE} available."
    exit 1
  fi
elif [[ "${USE_DOCKER_K6}" == "1" || "${USE_DOCKER_K6}" == "true" ]]; then
  if ! command -v docker >/dev/null 2>&1; then
    echo "Docker is required when USE_DOCKER_K6=${USE_DOCKER_K6}."
    exit 1
  fi
  K6_RUNNER="docker"
else
  if ! command -v k6 >/dev/null 2>&1; then
    echo "k6 is required. Install from https://k6.io/docs/get-started/installation/ or set USE_DOCKER_K6=1."
    exit 1
  fi
  K6_RUNNER="native"
fi

if [[ "${K6_RUNNER}" == "docker" ]]; then
  case "${K6_BASE_URL}" in
    http://localhost:*|https://localhost:*|http://127.0.0.1:*|https://127.0.0.1:*)
      K6_BASE_URL="${K6_DOCKER_BASE_URL:-${K6_BASE_URL/localhost/host.docker.internal}}"
      K6_BASE_URL="${K6_BASE_URL/127.0.0.1/host.docker.internal}"
      ;;
  esac
fi

mkdir -p "${EVIDENCE_DIR}"

write_run_meta() {
  local test_name="$1"
  local meta_path="${EVIDENCE_DIR}/k6-${test_name}-${RUN_ID}.meta.txt"

  cat > "${meta_path}" <<EOF
RUN_ID=${RUN_ID}
K6_RUNNER=${K6_RUNNER}
K6_DOCKER_IMAGE=${K6_DOCKER_IMAGE}
BASE_URL=${K6_BASE_URL}
AUTH_MODE=${AUTH_MODE:-}
EVIDENCE_DIR=${EVIDENCE_DIR}
EOF
}

run_k6_command() {
  local script_path="$1"
  local summary_path="$2"

  if [[ "${K6_RUNNER}" == "docker" ]]; then
    local script_name
    script_name="$(basename "${script_path}")"
    local summary_name
    summary_name="$(basename "${summary_path}")"
    docker run --rm \
      -v "${BACKEND_DIR}/load-tests:/scripts:ro" \
      -v "${EVIDENCE_DIR}:/evidence" \
      "${K6_DOCKER_IMAGE}" run \
      -e "BASE_URL=${K6_BASE_URL}" \
      -e "AUTH_MODE=${AUTH_MODE:-}" \
      -e "DEV_USERNAME=${DEV_USERNAME:-}" \
      -e "DEV_PASSWORD=${DEV_PASSWORD:-}" \
      -e "ACCOUNT_USERNAME=${ACCOUNT_USERNAME:-}" \
      -e "ACCOUNT_EMAIL=${ACCOUNT_EMAIL:-}" \
      -e "ACCOUNT_PASSWORD=${ACCOUNT_PASSWORD:-}" \
      -e "INTERNAL_API_KEY=${INTERNAL_API_KEY:-}" \
      -e "LOGIN_RAMP_TARGET_VUS=${LOGIN_RAMP_TARGET_VUS:-}" \
      -e "LOGIN_RAMP_UP_DURATION=${LOGIN_RAMP_UP_DURATION:-}" \
      -e "LOGIN_RAMP_HOLD_DURATION=${LOGIN_RAMP_HOLD_DURATION:-}" \
      -e "LOGIN_RAMP_DOWN_DURATION=${LOGIN_RAMP_DOWN_DURATION:-}" \
      -e "LOGIN_SLEEP_SECONDS=${LOGIN_SLEEP_SECONDS:-}" \
      -e "MATCHMAKING_VUS=${MATCHMAKING_VUS:-}" \
      -e "MATCHMAKING_DURATION=${MATCHMAKING_DURATION:-}" \
      -e "MATCHMAKING_SLEEP_SECONDS=${MATCHMAKING_SLEEP_SECONDS:-}" \
      --summary-export "/evidence/${summary_name}" "/scripts/${script_name}"
  else
    BASE_URL="${K6_BASE_URL}" k6 run --summary-export "${summary_path}" "${script_path}"
  fi
}

run_k6_test() {
  local test_name="$1"
  local script_path="$2"
  local summary_path="${EVIDENCE_DIR}/k6-${test_name}-${RUN_ID}.json"
  local log_path="${EVIDENCE_DIR}/k6-${test_name}-${RUN_ID}.log"

  write_run_meta "${test_name}"
  echo "Running k6 ${test_name} with ${K6_RUNNER}; summary=${summary_path}; log=${log_path}"
  run_k6_command "${script_path}" "${summary_path}" 2>&1 | tee "${log_path}"
}

run_k6_test "login" "${BACKEND_DIR}/load-tests/k6-login.js"
run_k6_test "matchmaking" "${BACKEND_DIR}/load-tests/k6-matchmaking.js"

if [[ -n "${INTERNAL_API_KEY:-}" ]]; then
  run_k6_test "dedicated-server-orchestration" "${BACKEND_DIR}/load-tests/k6-dedicated-server-orchestration.js"
else
  skip_path="${EVIDENCE_DIR}/dedicated-server-orchestration-skipped-${RUN_ID}.txt"
  echo "Skipping dedicated server orchestration load test because INTERNAL_API_KEY is not set." | tee "${skip_path}"
fi

