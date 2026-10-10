@echo off
setlocal
cd /d "%~dp0\.."

call scripts\build_native.bat
if errorlevel 1 exit /b 1

echo.
echo Running automated C tests...
tests\test_reliefq.exe
