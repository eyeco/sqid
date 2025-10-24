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

REM RtAudio
echo copy %moduledepdir%RtAudio-5.1.0\bin\msvc\%2\%1\rtaudio.dll %targetdir%
copy %moduledepdir%RtAudio-5.1.0\bin\msvc\%2\%1\rtaudio.dll %targetdir%
if "%1" == "Debug" (
    echo copy %moduledepdir%RtAudio-5.1.0\bin\msvc\%2\%1\rtaudio.pdb %targetdir%
    copy %moduledepdir%RtAudio-5.1.0\bin\msvc\%2\%1\rtaudio.pdb %targetdir%
)

exit /b 0
