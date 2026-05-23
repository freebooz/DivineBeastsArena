#!/usr/bin/env bash
set -euo pipefail

env_file="${1:-}"
if [[ -z "${env_file}" ]]; then
  echo "Usage: scripts/validate-production-env.sh <path-to-env-file>" >&2
  exit 2
fi

if [[ ! -f "${env_file}" ]]; then
  echo "Env file not found: ${env_file}" >&2
  exit 2
fi

declare -A values=()
while IFS= read -r raw_line || [[ -n "${raw_line}" ]]; do
  line="$(printf '%s' "${raw_line}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
  [[ -z "${line}" || "${line}" == \#* ]] && continue
  [[ "${line}" != *=* ]] && continue
  key="${line%%=*}"
  value="${line#*=}"
  value="${value%\"}"
  value="${value#\"}"
  value="${value%\'}"
  value="${value#\'}"
  values["${key}"]="${value}"
done < "${env_file}"

errors=()

require_key() {
  local key="$1"
  if [[ -z "${values[$key]:-}" ]]; then
    errors+=("${key} is required.")
  fi
}

reject_placeholder() {
  local key="$1"
  local value="${values[$key]:-}"
  if [[ "${value}" =~ change-me|your-github-org-or-user|example\.com|admin@example\.com ]]; then
    errors+=("${key} still contains a placeholder value.")
  fi
}

required_keys=(
  IMAGE_NAMESPACE
  POSTGRES_PASSWORD
  REDIS_PASSWORD
  JWT_SECRET
  INTERNAL_API_KEY
  PUBLIC_DOMAIN
  ACME_EMAIL
  GRAFANA_ADMIN_PASSWORD
)

for key in "${required_keys[@]}"; do
  require_key "${key}"
  reject_placeholder "${key}"
done

if [[ -n "${values[JWT_SECRET]:-}" && "${#values[JWT_SECRET]}" -lt 32 ]]; then
  errors+=("JWT_SECRET must be at least 32 characters.")
fi

if [[ "${values[ASPNETCORE_ENVIRONMENT]:-Production}" != "Production" ]]; then
  errors+=("ASPNETCORE_ENVIRONMENT must be Production.")
fi

if [[ "${values[DOTNET_ENVIRONMENT]:-Production}" != "Production" ]]; then
  errors+=("DOTNET_ENVIRONMENT must be Production.")
fi

if [[ "${values[SEED_DATA_ENABLED]:-false}" != "false" ]]; then
  errors+=("SEED_DATA_ENABLED must be false in production.")
fi

if [[ "${values[SWAGGER_ENABLED]:-false}" != "false" ]]; then
  errors+=("SWAGGER_ENABLED must be false in production unless protected by an external gateway.")
fi

if [[ "${values[DATABASE_RUN_MIGRATIONS_AND_EXIT]:-false}" != "false" ]]; then
  errors+=("DATABASE_RUN_MIGRATIONS_AND_EXIT must be false for normal runtime containers.")
fi

if [[ -n "${values[GAME_SERVER_PORT_START]:-}" && -n "${values[GAME_SERVER_PORT_END]:-}" ]]; then
  if ! [[ "${values[GAME_SERVER_PORT_START]}" =~ ^[0-9]+$ && "${values[GAME_SERVER_PORT_END]}" =~ ^[0-9]+$ ]]; then
    errors+=("GAME_SERVER_PORT_START and GAME_SERVER_PORT_END must be integers.")
  else
    port_start="${values[GAME_SERVER_PORT_START]}"
    port_end="${values[GAME_SERVER_PORT_END]}"
    if (( port_start <= 0 || port_end < port_start )); then
    errors+=("Game server port range is invalid.")
    fi
  fi
fi

if (( ${#errors[@]} > 0 )); then
  echo "Production env validation failed:" >&2
  for error in "${errors[@]}"; do
    echo "- ${error}" >&2
  done
  exit 1
fi

echo "Production env validation passed: ${env_file}"
