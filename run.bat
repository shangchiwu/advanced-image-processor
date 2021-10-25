@echo off
setlocal enabledelayedexpansion

:: config

:: printColor     : 1/0 to enable/disable print color
:: buildBeforeRun : 1/0 to enable/disable build before run
:: runDir         : the directory to run the run command
:: runCommand     : the command to be run

set printColor=1
set buildBeforeRun=1
set runDir=.\build\bin
set runCommand=.\HW3_61047014S.exe

:: print colors by control chars

if !printColor! equ 1 (
    set colorRed=[91m
    set colorGreen=[92m
    set colorYellow=[93m
    set colorReset=[0m
)

:: build

if !buildBeforeRun! equ 1 (
    echo ========== !colorYellow!Build!colorReset! ========= [%TIME%]
    call .\build.bat

    if !errorlevel! neq 0 (
        call :decToHex !errorlevel!
        echo !colorRed!Build error: Exit code: 0x!decToHexVal! ^(!errorlevel!^)!colorReset!
        goto :exit
    )
)

:: run

echo ========== !colorYellow!Run!colorReset! =========== [%TIME%]
pushd %runDir%

if [%~1] neq [] (
    set runCommand=%*
)

!runCommand!

popd

echo ========== !colorYellow!Done!colorReset! ========== [%TIME%]
call :decToHex !errorlevel!

if !errorlevel! equ 0 ( set colorResult=!colorGreen!
) else ( set colorResult=!colorRed!)

echo !colorResult!Exit code: 0x!decToHexVal! ^(!errorlevel!^)!colorReset!
goto :exit

:: decToHex

:decToHex
cmd /C exit %1
set "decToHexVal=%=ExitCode%"
if not defined decToHexVal set "decToHexVal=0"
goto :eof

:: exit

:exit
exit /b !errorlevel!
