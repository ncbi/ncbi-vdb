@echo off
setlocal EnableDelayedExpansion

rem echo combine_mbedtls %1 %2
rem lib.exe /LIST "!LIBFILE!" /NOLOGO 

for %%F in (%1) do (
    set "LIBFILE=%%F"
    for /F "tokens=*" %%O in ('lib.exe /LIST "!LIBFILE!" /NOLOGO') do (
        set "OBJFILE=%%O"
        rem echo Extracting !OBJFILE! to %2\\mbedtls_%%~nxO
        lib.exe /NOLOGO "!LIBFILE!" "/EXTRACT:!OBJFILE!" "/OUT:%2\\mbedtls_%%~nxO"
    )
)
