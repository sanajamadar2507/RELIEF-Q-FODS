#!/usr/bin/env bash
set -e
cd "$(dirname "$0")/.."

echo "===================================================="
echo " Compiling RELIEF-Q Native C Targets (GCC C11)"
echo "===================================================="

if ! command -v gcc &> /dev/null; then
    echo "[ERROR] gcc is required to compile native targets."
    exit 1
fi

echo "Compiling reliefq_cli..."
gcc -std=c11 -Wall -Wextra -O2 -Iinclude c_src/main_cli.c c_src/memory_pool.c c_src/string_utils.c \
    c_src/resource_array.c c_src/linked_list.c c_src/queue.c c_src/priority_queue.c \
    c_src/stack.c c_src/search_sort.c c_src/reliefq_core.c c_src/json_export.c -o reliefq_cli

echo "Compiling tests/test_reliefq..."
gcc -std=c11 -Wall -Wextra -Iinclude tests/test_reliefq.c c_src/memory_pool.c c_src/string_utils.c \
    c_src/resource_array.c c_src/linked_list.c c_src/queue.c c_src/priority_queue.c \
    c_src/stack.c c_src/search_sort.c c_src/reliefq_core.c c_src/json_export.c -o tests/test_reliefq

echo "[SUCCESS] Native C compilation complete: reliefq_cli, tests/test_reliefq"
