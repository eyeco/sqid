@echo off

REM args:
REM %1 ... Configuration
REM %2 ... PlatformName

if not "%1" == "Debug" if not "%1" == "Release" (
	echo ERROR: invalid Configuration specified: "%1"
	exit /b 1
)

if not "%2" == "x64" if not "%2" == "Win32" (
	echo ERROR: invalid Configuration specified: "%2"
	exit /b 1
)

set moduledepdir=..\..\3rdparty\
set targetdir=..\..\..\..\apps\bin\%2\%1\

REM libTUIO2
if "%1" == "Debug" (
    set name=libTUIO2_d
) else if "%1" == "Release" (
    set name=libTUIO2
) else (
    echo ERROR: invalid Configuration specified
)

echo copy %moduledepdir%tuio-2.0\bin\msvc\%2\%1\%name%.dll %targetdir%
copy %moduledepdir%tuio-2.0\bin\msvc\%2\%1\%name%.dll %targetdir%

echo copy %moduledepdir%tuio-2.0\bin\msvc\%2\%1\%name%.pdb %targetdir%
copy %moduledepdir%tuio-2.0\bin\msvc\%2\%1\%name%.pdb %targetdir%

exit /b 0
