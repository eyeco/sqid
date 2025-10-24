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

REM Mosquitto
echo copy %moduledepdir%mosquitto-2.0.14\bin\msvc\%2\mosquitto.dll %targetdir%
copy %moduledepdir%mosquitto-2.0.14\bin\msvc\%2\mosquitto.dll %targetdir%

exit /b 0
