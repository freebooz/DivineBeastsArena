#!/bin/bash
set -e

BACKUP_DIR="/var/backups/gameplatform"
BACKUP_FILE="$1"

if [ -z "$BACKUP_FILE" ]; then
    echo "Usage: $0 <backup_file>"
    exit 1
fi

FULL_PATH="$BACKUP_DIR/$BACKUP_FILE"
if [ ! -f "$FULL_PATH" ]; then
    echo "Backup file not found: $FULL_PATH"
    exit 1
fi

DB_NAME="gameplatform"
DB_USER="gameuser"

echo "Restoring PostgreSQL from $BACKUP_FILE"

PGPASSWORD="${POSTGRES_PASSWORD}" pg_restore -h postgres -U "$DB_USER" -d "$DB_NAME" -c "$FULL_PATH"

echo "Restore completed successfully"