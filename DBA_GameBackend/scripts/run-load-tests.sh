#!/usr/bin/env bash
set -euo pipefail

# 中文阅读说明：
# - 文件职责：顺序运行上线前 k6 压测脚本。
# - 使用方式：BASE_URL=http://localhost:8080 INTERNAL_API_KEY=xxx bash scripts/run-load-tests.sh
# - 修改提示：生产压测请先确认测试账号、端口范围和隔离数据库，避免污染正式玩家数据。

if ! command -v k6 >/dev/null 2>&1; then
  echo "k6 is required. Install from https://k6.io/docs/get-started/installation/"
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

k6 run "${BACKEND_DIR}/load-tests/k6-login.js"
k6 run "${BACKEND_DIR}/load-tests/k6-matchmaking.js"

if [[ -n "${INTERNAL_API_KEY:-}" ]]; then
  k6 run "${BACKEND_DIR}/load-tests/k6-server-manager.js"
else
  echo "Skipping server manager load test because INTERNAL_API_KEY is not set."
fi
