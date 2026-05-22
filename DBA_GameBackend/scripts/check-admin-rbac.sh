#!/usr/bin/env bash
set -euo pipefail

# 中文阅读说明：
# - 文件职责：用真实 HTTP 请求检查后台 RBAC 是否阻止低权限角色执行高危操作。
# - 使用方式：BASE_URL=http://localhost:8080 bash scripts/check-admin-rbac.sh
# - 修改提示：如开发账号或接口路径变化，请同步本脚本。

BASE_URL="${BASE_URL:-http://localhost:8080}"
VIEWER_USERNAME="${VIEWER_USERNAME:-viewer_admin}"
VIEWER_PASSWORD="${VIEWER_PASSWORD:-Viewer@123456}"

login_payload=$(printf '{"username":"%s","password":"%s"}' "$VIEWER_USERNAME" "$VIEWER_PASSWORD")
login_response=$(curl -fsS -H 'Content-Type: application/json' -d "$login_payload" "$BASE_URL/api/admin/auth/login")
token=$(printf '%s' "$login_response" | python -c "import json,sys; print(json.load(sys.stdin)['data']['accessToken'])")

status=$(curl -sS -o /tmp/dba-rbac-response.json -w '%{http_code}' \
  -H "Authorization: Bearer $token" \
  -H 'Content-Type: application/json' \
  -d '{"reason":"rbac smoke test"}' \
  "$BASE_URL/api/admin/servers/00000000-0000-0000-0000-000000000000/kill")

if [[ "$status" != "403" ]]; then
  echo "RBAC check failed: expected 403 for VIEWER high-risk operation, got $status"
  cat /tmp/dba-rbac-response.json || true
  exit 1
fi

echo "RBAC check completed with HTTP $status."
