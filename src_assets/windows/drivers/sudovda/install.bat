@echo off
setlocal
pushd "%~dp0"

set "CERTUTIL=certutil"
where certutil >nul 2>&1 || set "CERTUTIL=%SystemRoot%\System32\certutil.exe"

if not exist "nefconc.exe" (
    echo ERROR: nefconc.exe was not found in %~dp0
    goto :fail
)
if not exist "SudoVDA.dll" (
    echo ERROR: SudoVDA.dll was not found in %~dp0
    goto :fail
)
if not exist "SudoVDA.inf" (
    echo ERROR: SudoVDA.inf was not found in %~dp0
    goto :fail
)
if not exist "sudovda.cer" (
    echo ERROR: sudovda.cer was not found in %~dp0
    goto :fail
)

echo ================
echo Installing cert for the SudoVDA driver...

%CERTUTIL% -addstore -f root "sudovda.cer"
if errorlevel 1 goto :fail
%CERTUTIL% -addstore -f TrustedPublisher "sudovda.cer"
if errorlevel 1 goto :fail

echo ================
echo Removing the old driver... It's OK to show an error if this is the first install.

nefconc.exe --remove-device-node --hardware-id root\sudomaker\sudovda --class-guid "4D36E968-E325-11CE-BFC1-08002BE10318"

echo ================
echo Creating the virtual display device node...

nefconc.exe --create-device-node --class-name Display --class-guid "4D36E968-E325-11CE-BFC1-08002BE10318" --hardware-id root\sudomaker\sudovda
if errorlevel 1 goto :fail

echo ================
echo Installing the SudoVDA driver...

nefconc.exe --install-driver --inf-path "SudoVDA.inf"
if errorlevel 1 goto :fail

echo ================
echo Done!
set "EXIT_CODE=0"
goto :finish

:fail
echo ERROR: SudoVDA driver installation failed.
set "EXIT_CODE=1"

:finish
popd
endlocal & exit /b %EXIT_CODE%
