@echo off

set "lib=tuio-2.0"

cd /d "%~dp0"

if not exist "%lib%" (
    echo extracting %lib%
    if exist "%lib%.tar.gz" (
        tar -xzf "%lib%.tar.gz"
        echo done
    ) else (
        echo ERROR: archive %lib%.tar.gz not fround
    )
) else (
    echo found "%lib%", nothing to do
)