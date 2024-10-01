@echo off

cd /d "%~dp0"

call :checkAndExtract "cxxopts-3.2.0"
call :checkAndExtract "glew-2.1.0-1"
call :checkAndExtract "glfw-3.3"
call :checkAndExtract "glm-0.9.9.3"
call :checkAndExtract "imgui-1.91.1"
call :checkAndExtract "liblo-0.31"
call :checkAndExtract "nlohmann-json-3.5.0-5"
call :checkAndExtract "openCV-4.1.0"
call :checkAndExtract "serial-1.2.1"

exit /b 1


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