# Ops

Operational files for the platform.

## Docker Compose

```bash
cd ops/docker
cp .env.example .env
docker compose config
docker compose up -d --build
```

PostgreSQL and Redis are internal services and are not published to public host ports.

## Scripts

- `scripts/deploy.sh`
- `scripts/backup-postgres.sh`
- `scripts/restore-postgres.sh`
- `scripts/migrate-db.sh`

## Observability

- Prometheus: `prometheus/prometheus.yml`
- Alert rules: `prometheus/alert-rules.yml`
- Loki: `loki/loki-config.yml`
- Grafana provisioning: `grafana/provisioning`
