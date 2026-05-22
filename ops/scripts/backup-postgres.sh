#!/bin/bash
set -e

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BACKUP_DIR="/var/backups/gameplatform"
DB_NAME="gameplatform"
DB_USER="gameuser"

mkdir -p "$BACKUP_DIR"

echo "Starting PostgreSQL backup at $TIMESTAMP"

PGPASSWORD="${POSTGRES_PASSWORD}" pg_dump -h postgres -U "$DB_USER" -d "$DB_NAME" -Fc > "$BACKUP_DIR/db_backup_$TIMESTAMP.dump"

find "$BACKUP_DIR" -name "db_backup_*.dump" -mtime +7 -delete

echo "Backup completed: db_backup_$TIMESTAMP.dump"
gzip "$BACKUP_DIR/db_backup_$TIMESTAMP.dump"

echo "Backup compressed and ready: $BACKUP_DIR/db_backup_$TIMESTAMP.dump.gz"