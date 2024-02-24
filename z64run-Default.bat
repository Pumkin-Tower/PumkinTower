@echo off
zzrtl.exe mm_build.rtl
if %ERRORLEVEL% neq 0 (
    echo Execution failed! Exiting...
    exit /b %ERRORLEVEL%
)
"YourEmulatorPathHere.exe" build.z64
