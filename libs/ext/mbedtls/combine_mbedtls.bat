@echo off
setlocal EnableDelayedExpansion

rem echo combine_mbedtls %1 %2
rem lib.exe /LIST "!LIBFILE!" /NOLOGO 

rmdir /S /Q %2
mkdir %2

for %%F in (%1) do (
    set "LIBFILE=%%F"
    for /F "tokens=*" %%O in ('lib.exe /LIST "!LIBFILE!" /NOLOGO') do (
        set "OBJFILE=%%O"
        rem echo Extracting !OBJFILE! to mbedtls_%%~nxO
        lib.exe /NOLOGO "!LIBFILE!" "/EXTRACT:!OBJFILE!" "/OUT:mbedtls_%%~nxO"
    )
)
