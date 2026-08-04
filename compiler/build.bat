@echo off
setlocal EnableDelayedExpansion
set "RED="
set "BLUE="
set "RESET="
if defined WT_SESSION set "ANSI=1"
if defined ConEmuANSI set "ANSI=1"
if defined ANSICON set "ANSI=1"
if not defined ANSI (
    for /f "tokens=3" %%A in ('reg query HKCU\Console /v VirtualTerminalLevel 2^>nul') do (
        if "%%A"=="0x1" set "ANSI=1"
    )
)
if defined ANSI (
    for /f %%A in ('echo prompt $E ^| cmd') do set "ESC=%%A"
    set "RED=!ESC![31m"
    set "BLUE=!ESC![34m"
    set "RESET=!ESC![0m"
)
echo !BLUE!Building fvmC statically...!RESET!
cmake -S . -B build
if errorlevel 1 exit /b 1
cmake --build build
if errorlevel 1 exit /b 1
echo !BLUE!Built fvmC!!RESET!
endlocal