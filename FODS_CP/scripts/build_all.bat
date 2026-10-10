@echo off
setlocal
cd /d "%~dp0\.."

call scripts\build_native.bat
if errorlevel 1 exit /b 1

call scripts\build_wasm.bat
if errorlevel 1 exit /b 1

echo ====================================================
echo  All RELIEF-Q C targets built successfully!
echo ====================================================
