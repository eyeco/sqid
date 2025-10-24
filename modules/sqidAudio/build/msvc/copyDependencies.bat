@echo off

REM args:
REM %1 ... Configuration
REM %2 ... PlatformName

set moduledepdir=..\..\3rdparty\
set targetdir=..\..\..\..\apps\bin\%2\%1\

REM RtAudio
echo copy %moduledepdir%RtAudio-5.1.0\bin\msvc\%2\%1\rtaudio.dll %targetdir%
copy %moduledepdir%RtAudio-5.1.0\bin\msvc\%2\%1\rtaudio.dll %targetdir%
if "%1" == "Debug" (
    echo copy %moduledepdir%RtAudio-5.1.0\bin\msvc\%2\%1\rtaudio.pdb %targetdir%
    copy %moduledepdir%RtAudio-5.1.0\bin\msvc\%2\%1\rtaudio.pdb %targetdir%
)
