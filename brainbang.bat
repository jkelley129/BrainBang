@echo off
setlocal enabledelayedexpansion

:: Flag: by default, run the compiled file
set "run_compiled=true"

:: First non-flag argument (assumed to be source filename)
set "filename="

:: Parse arguments
:parse
if "%~1"=="" goto done

if "%~1"=="-c" (
    set "run_compiled=false"
) else (
    if not defined filename (
        set "filename=%~1"
    ) else (
        echo Unexpected argument: %~1
    )
)
shift
goto parse

:done

:: Check if filename is provided
if not defined filename (
    echo Usage: %~nx0 [-c] source.bb
    exit /b 1
)

:: Compile the BrainBang source file
python compilers/brainbang_compiler.py %filename%

:: Trim .bb extension to get base name
set "cutFilename=%filename:~0,-3%"

:: Conditionally run the compiled .bf file
if "%run_compiled%"=="true" (
    python compilers/brainfk_interpreter.py %cutFilename%.bf
)