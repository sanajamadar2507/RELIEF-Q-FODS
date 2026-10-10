#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

bash scripts/build_native.sh

echo ""
echo "Running automated C tests..."
./tests/test_reliefq
