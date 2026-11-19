@echo off
setlocal
pushd "%~dp0"

if not exist "nefconc.exe" (
    echo ERROR: nefconc.exe was not found in %~dp0
    popd
    exit /b 1
)

nefconc.exe --remove-device-node --hardware-id root\sudomaker\sudovda --class-guid "4D36E968-E325-11CE-BFC1-08002BE10318"
set "EXIT_CODE=%ERRORLEVEL%"

popd
endlocal & exit /b %EXIT_CODE%
