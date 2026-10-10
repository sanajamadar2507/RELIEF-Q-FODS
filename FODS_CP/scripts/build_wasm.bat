@echo off
setlocal
cd /d "%~dp0\.."

echo ====================================================
echo  Compiling RELIEF-Q to WebAssembly (C11 -> WASM)
echo ====================================================

set CLANG_CMD=clang
where clang >nul 2>&1
if errorlevel 1 (
    if exist "C:\Users\%USERNAME%\AppData\Local\Android\Sdk\ndk\28.2.13676358\toolchains\llvm\prebuilt\windows-x86_64\bin\clang.exe" (
        set CLANG_CMD="C:\Users\%USERNAME%\AppData\Local\Android\Sdk\ndk\28.2.13676358\toolchains\llvm\prebuilt\windows-x86_64\bin\clang.exe"
    ) else (
        echo [ERROR] clang compiler not found on PATH or in Android NDK.
        echo Please ensure clang or LLVM is installed.
        exit /b 1
    )
)

%CLANG_CMD% --target=wasm32 -nostdlib -Iinclude -Wl,--no-entry -Wl,--export-all -O3 -o public\reliefq.wasm c_src\memory_pool.c c_src\string_utils.c c_src\resource_array.c c_src\linked_list.c c_src\queue.c c_src\priority_queue.c c_src\stack.c c_src\search_sort.c c_src\reliefq_core.c c_src\json_export.c c_src\wasm_api.c
if errorlevel 1 (
    echo [ERROR] WebAssembly compilation failed.
    exit /b 1
)

copy /y public\reliefq.wasm reliefq.wasm >nul
node scripts\gen_wasm_bytes.js
echo [SUCCESS] WebAssembly build complete: public\reliefq.wasm
