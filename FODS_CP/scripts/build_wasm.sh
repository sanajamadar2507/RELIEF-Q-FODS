#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

echo "===================================================="
echo " Compiling RELIEF-Q to WebAssembly (C11 -> WASM)"
echo "===================================================="

CLANG_CMD="clang"
if ! command -v clang &> /dev/null; then
    echo "[ERROR] clang is required to compile to WebAssembly."
    exit 1
fi

$CLANG_CMD --target=wasm32 -nostdlib -Iinclude -Wl,--no-entry -Wl,--export-all -O3 -o public/reliefq.wasm \
    c_src/memory_pool.c c_src/string_utils.c c_src/resource_array.c c_src/linked_list.c \
    c_src/queue.c c_src/priority_queue.c c_src/stack.c c_src/search_sort.c \
    c_src/reliefq_core.c c_src/json_export.c c_src/wasm_api.c

cp public/reliefq.wasm reliefq.wasm
node scripts/gen_wasm_bytes.js
echo "[SUCCESS] WebAssembly build complete: public/reliefq.wasm"
