#!/usr/bin/env bash
set -euo pipefail

# 中文阅读说明：
# - 文件职责：通过 GitHub REST API 配置 main 分支保护，要求 solution-ci 通过后才能合并。
# - 使用方式：GH_TOKEN=<repo admin token> bash scripts/configure-branch-protection.sh freebooz DivineBeastsArena
# - 修改提示：如 CI 名称变化，请同步 REQUIRED_CONTEXTS。

OWNER="${1:-freebooz}"
REPO="${2:-DivineBeastsArena}"
BRANCH="${BRANCH:-main}"
REQUIRED_CONTEXTS_JSON="${REQUIRED_CONTEXTS_JSON:-[\"solution-ci\"]}"

if [[ -z "${GH_TOKEN:-}" ]]; then
  echo "GH_TOKEN is required and must have repo administration permission."
  exit 1
fi

payload="$(cat <<JSON
{
  "required_status_checks": {
    "strict": true,
    "contexts": ${REQUIRED_CONTEXTS_JSON}
  },
  "enforce_admins": true,
  "required_pull_request_reviews": {
    "required_approving_review_count": 1,
    "dismiss_stale_reviews": true,
    "require_code_owner_reviews": false
  },
  "restrictions": null,
  "allow_force_pushes": false,
  "allow_deletions": false,
  "required_linear_history": true
}
JSON
)"

curl -fsS -X PUT \
  -H "Accept: application/vnd.github+json" \
  -H "Authorization: Bearer ${GH_TOKEN}" \
  -H "X-GitHub-Api-Version: 2022-11-28" \
  "https://api.github.com/repos/${OWNER}/${REPO}/branches/${BRANCH}/protection" \
  -d "${payload}"

echo "Branch protection configured for ${OWNER}/${REPO}:${BRANCH}."
