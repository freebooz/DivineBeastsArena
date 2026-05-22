#!/bin/bash
set -e

REGISTRY="${REGISTRY:-ghcr.io}"
IMAGE_NAME="${IMAGE_NAME:-gameplatform}"
IMAGE_TAG="${IMAGE_TAG:-latest}"
SERVER_HOST="${SERVER_HOST:-}"
SERVER_USER="${SERVER_USER:-}"
SSH_KEY="${SSH_KEY:-}"

if [ -z "$SERVER_HOST" ] || [ -z "$SERVER_USER" ]; then
    echo "ERROR: SERVER_HOST and SERVER_USER must be set"
    exit 1
fi

echo "Deploying $REGISTRY/$IMAGE_NAME/api:$IMAGE_TAG to $SERVER_HOST"

ssh -i "$SSH_KEY" "$SERVER_USER@$SERVER_HOST" << 'ENDSSH'
    set -e
    cd /opt/gameplatform

    echo "Backing up database..."
    ./scripts/backup-postgres.sh

    echo "Pulling new images..."
    docker compose pull

    echo "Stopping old containers..."
    docker compose down

    echo "Starting new containers..."
    docker compose up -d

    echo "Checking health..."
    sleep 10
    curl -f http://localhost:8080/health/live || exit 1

    echo "Deployment completed successfully"
ENDSSH

echo "Deployment completed"