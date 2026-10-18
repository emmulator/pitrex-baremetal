@echo off
rem Double-click wrapper is not useful (the script needs the folder), so run it from a command prompt:
rem   install.bat [--settings tuned] [--arcade-tweaks] [--dry-run] C:\path\to\PiTrex_sd_Jul29_25
rem   install.bat --restore C:\path\to\PiTrex_sd_Jul29_25
rem   install.bat E:\        (the SD card itself)
setlocal EnableDelayedExpansion
set "ARGS="
set "NEXTSET="
:next
if "%~1"=="" goto run
set "A=%~1"
rem a path ending in \ (E:\) would end in \" and PowerShell would read the quote as part of the path: E:\ becomes E:\.
if "!A:~-1!"=="\" set "A=!A!."
if defined NEXTSET ( set "ARGS=!ARGS! -Settings !A!" & set "NEXTSET=" & shift & goto next )
if /I "!A!"=="--settings" ( set "NEXTSET=1" & shift & goto next )
if /I "!A!"=="--arcade-tweaks" ( set "ARGS=!ARGS! -ArcadeTweaks" ) else if /I "!A!"=="--dry-run" ( set "ARGS=!ARGS! -DryRun" ) else if /I "!A!"=="--restore" ( set "ARGS=!ARGS! -Restore" ) else ( set "ARGS=!ARGS! "!A!"" )
shift
goto next
:run
if "!ARGS!"=="" (
  echo usage: install.bat [--settings tuned] [--arcade-tweaks] [--dry-run] [--restore] folder
  exit /b 2
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1" !ARGS!
exit /b %ERRORLEVEL%
