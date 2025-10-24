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

REM ZeroMQ
if "%1" == "Debug" (
    echo copy %moduledepdir%zeromq-4.3.2\bin\msvc\%2\%1\libzmq-v141-mt-gd-4_3_2.dll %targetdir%
    copy %moduledepdir%zeromq-4.3.2\bin\msvc\%2\%1\libzmq-v141-mt-gd-4_3_2.dll %targetdir%

    echo copy %moduledepdir%zeromq-4.3.2\bin\msvc\%2\%1\libzmq-v141-mt-gd-4_3_2.pdb %targetdir%
    copy %moduledepdir%zeromq-4.3.2\bin\msvc\%2\%1\libzmq-v141-mt-gd-4_3_2.pdb %targetdir%
) else if "%1" == "Release" (
    echo copy %moduledepdir%zeromq-4.3.2\bin\msvc\%2\%1\libzmq-v141-mt-4_3_2.dll %targetdir%
    copy %moduledepdir%zeromq-4.3.2\bin\msvc\%2\%1\libzmq-v141-mt-4_3_2.dll %targetdir%
) else (
    echo ERROR: invalid Configuration specified
)

exit /b 0
