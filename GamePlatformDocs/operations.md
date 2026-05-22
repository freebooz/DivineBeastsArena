# 运维说明

## 健康检查

```powershell
Invoke-RestMethod http://localhost:8080/health/live
Invoke-RestMethod http://localhost:8080/health/ready
Invoke-RestMethod http://localhost:8080/api/platform/applications
```

## 常用维护命令

```powershell
.\GamePlatformScripts\check-platform.ps1
.\GamePlatformScripts\check-platform.ps1 -LiveApi -ApiBaseUrl http://localhost:8080
```

```powershell
cd GamePlatformOps\docker
docker compose up --build
docker compose down
```

## 数据库备份

```bash
./GamePlatformOps/scripts/backup-db.sh
```

## 数据库恢复

```bash
./GamePlatformOps/scripts/restore-db.sh backups/latest.sql
```

## 迁移

```bash
./GamePlatformOps/scripts/migrate-db.sh
```

## 日志与监控

- API 日志写入标准输出，容器环境由 Loki 收集。
- Prometheus 抓取 `/metrics`。
- Grafana 面板从 Prometheus 与 Loki 读取服务状态、延迟、错误率和资源指标。
