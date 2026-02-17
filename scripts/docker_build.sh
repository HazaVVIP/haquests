#!/bin/bash
set -e

echo "Building Docker image..."

docker build -t haquests:latest .

echo "Docker image built successfully!"
echo "Run with: docker-compose up -d"
