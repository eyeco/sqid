@echo off

REM args:
REM %1 ... Configuration
REM %2 ... PlatformName

set depdir=..\..\..\..\3rdparty\
set moduledepdir=..\..\3rdparty\
set targetdir=..\..\..\..\apps\bin\%2\%1\

REM imgui
echo copy %depdir%imgui-1.91.1\bin\msvc\%2\%1\imgui.dll %targetdir%
copy %depdir%imgui-1.91.1\bin\msvc\%2\%1\imgui.dll %targetdir%

REM glfw3
echo copy %depdir%glfw-3.3\lib\msvc\%2\glfw3.dll %targetdir%
copy %depdir%glfw-3.3\lib\msvc\%2\glfw3.dll %targetdir%

REM OpenCV
if "%1" == "Debug" (
    set file=opencv_world410d.dll
) else if "%1" == "Release" (
    set file=opencv_world410.dll
) else (
    echo ERROR: invalid Configuration specified
)
echo copy %depdir%openCV-4.1.0\bin\msvc\%2\%file% %targetdir%
copy %depdir%openCV-4.1.0\bin\msvc\%2\%file% %targetdir%

REM glew
if "%1" == "Debug" (
    set file=glew32d.dll
) else if "%1" == "Release" (
    set file=glew32.dll
) else (
    echo ERROR: invalid Configuration specified
)
echo copy %depdir%glew-2.1.0-1\bin\msvc\%2\%1\%file% %targetdir%
copy %depdir%glew-2.1.0-1\bin\msvc\%2\%1\%file% %targetdir%

REM freetype
echo copy %moduledepdir%freetype-2.10.1\lib\msvc\%2\freetype.dll %targetdir%
copy %moduledepdir%freetype-2.10.1\lib\msvc\%2\freetype.dll %targetdir%

REM dl
echo copy %moduledepdir%dlfcn-1.4.1\bin\msvc\%2\%1\dl.dll %targetdir%
copy %moduledepdir%dlfcn-1.4.1\bin\msvc\%2\%1\dl.dll %targetdir%
if "%1" == "Debug" (
    echo copy %moduledepdir%dlfcn-1.4.1\bin\msvc\%2\%1\dl.pdb %targetdir%
    copy %moduledepdir%dlfcn-1.4.1\bin\msvc\%2\%1\dl.pdb %targetdir%
)

REM FFTW3
echo copy %moduledepdir%fftw-3.3.8\bin\msvc\%2\%1\fftw3f.dll %targetdir%
copy %moduledepdir%fftw-3.3.8\bin\msvc\%2\%1\fftw3f.dll %targetdir%
if "%1" == "Debug" (
    echo copy %moduledepdir%fftw-3.3.8\bin\msvc\%2\%1\fftw3f.pdb %targetdir%
    copy %moduledepdir%fftw-3.3.8\bin\msvc\%2\%1\fftw3f.pdb %targetdir%
)