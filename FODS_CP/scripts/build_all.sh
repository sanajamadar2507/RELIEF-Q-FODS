#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

bash scripts/build_native.sh
bash scripts/build_wasm.sh

echo "===================================================="
echo " All RELIEF-Q C targets built successfully!"
echo "===================================================="
