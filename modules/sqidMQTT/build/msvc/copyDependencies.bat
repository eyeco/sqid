@echo off

REM args:
REM %1 ... Configuration
REM %2 ... PlatformName

set moduledepdir=..\..\3rdparty\
set targetdir=..\..\..\..\apps\bin\%2\%1\

REM Mosquitto
echo copy %moduledepdir%mosquitto-2.0.14\bin\msvc\%2\mosquitto.dll %targetdir%
copy %moduledepdir%mosquitto-2.0.14\bin\msvc\%2\mosquitto.dll %targetdir%
