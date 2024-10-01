@echo off

cd /d "%~dp0"

call :checkAndExtract "bluetooth-serial-port"
call :checkAndExtract "bzlib2-1.0.6"
call :checkAndExtract "clipboardXX"
call :checkAndExtract "create.bat"
call :checkAndExtract "dlfcn-1.4.1"
call :checkAndExtract "fftw-3.3.8"
call :checkAndExtract "freetype-2.10.1"
call :checkAndExtract "libjpeg-turbo-1.5.3-1"
call :checkAndExtract "lz4-1.8.3"
call :checkAndExtract "minilzo-2.10"
call :checkAndExtract "quickLZ-1.5.0"
call :checkAndExtract "zlib-1.2.11-3"
call :checkAndExtract "zstd-1.4.0"

exit /b 0


:checkAndExtract lib
::           -- lib [in] - name of library (and archive)

set "lib=%~1"

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

EXIT /b