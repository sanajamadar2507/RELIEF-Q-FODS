@echo off
setlocal
cd /d "%~dp0\.."

echo ====================================================
echo  Compiling RELIEF-Q Native C Targets (GCC C11)
echo ====================================================

where gcc >nul 2>&1
if errorlevel 1 (
    echo [ERROR] gcc compiler not found on PATH.
    echo Please install MinGW or GCC to compile native binaries.
    exit /b 1
)

echo Compiling reliefq_cli.exe...
gcc -std=c11 -Wall -Wextra -O2 -Iinclude c_src\main_cli.c c_src\memory_pool.c c_src\string_utils.c c_src\resource_array.c c_src\linked_list.c c_src\queue.c c_src\priority_queue.c c_src\stack.c c_src\search_sort.c c_src\reliefq_core.c c_src\json_export.c -o reliefq_cli.exe
if errorlevel 1 (
    echo [ERROR] Failed to compile reliefq_cli.exe
    exit /b 1
)

echo Compiling tests\test_reliefq.exe...
gcc -std=c11 -Wall -Wextra -Iinclude tests\test_reliefq.c c_src\memory_pool.c c_src\string_utils.c c_src\resource_array.c c_src\linked_list.c c_src\queue.c c_src\priority_queue.c c_src\stack.c c_src\search_sort.c c_src\reliefq_core.c c_src\json_export.c -o tests\test_reliefq.exe
if errorlevel 1 (
    echo [ERROR] Failed to compile test_reliefq.exe
    exit /b 1
)

echo [SUCCESS] Native C compilation complete: reliefq_cli.exe, tests\test_reliefq.exe
