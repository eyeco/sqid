@echo off

chcp 65001

setlocal enabledelayedexpansion

:: Check if an argument is provided
if "%~1"=="" (
    echo Usage: %0 pluginName
    exit /b 1
)

tar -xf sqidPluginTemplate.zip

:: Remove any surrounding quotes from the input argument
set "orig=%~1"

:: Ensure the first character is a letter
for /f "tokens=* delims=0123456789" %%a in ("%orig%") do set "orig=%%a"


:: Check if the resulting string starts with a letter, if not exit
if not defined orig (
    echo The provided argument must start with a letter.
    exit /b 1
)

:: Capitalize the first letter and concatenate with the rest
set "firstLetter=%orig:~0,1%"
set "rest=%orig:~1%"

call :toUpper firstLetter
set "pluginNameUC=%firstLetter%%rest%"

call :toLower firstLetter
set "pluginNameLC=%firstLetter%%rest%"


set "pluginFullNameLC=sqid%pluginNameUC%"
set "pluginFullNameUC=Sqid%pluginNameUC%"

:: create project files
set "searchLC=pluginTemplate"
set "searchFullLC=sqidPluginTemplate"
set "searchUC=PluginTemplate"
set "searchFullUC=SqidPluginTemplate"

cd .\%searchFullLC%\build\msvc

:: generate project file
set "inFile=%searchFullLC%.vcxproj"
set "outfile=%pluginFullNameLC%.vcxproj"
if not exist "%inFile%" (
     echo File not found: %inFile%
     exit /b 1
)
echo creating %outFile% from %inFile%
copy %inFile% %outFile%

echo replacing %searchFullLC% by %pluginFullNameLC%
..\..\..\..\utils\fart %outFile% %searchFullLC% %pluginFullNameLC%
echo replacing %searchLC% by %pluginNameLC%
..\..\..\..\utils\fart %outFile% %searchLC% %pluginNameLC%

:: generate filter file
set "inFile=%searchFullLC%.vcxproj.filters"
set "outfile=%pluginFullNameLC%.vcxproj.filters"
if not exist "%inFile%" (
     echo File not found: %inFile%
     exit /b 1
)
echo creating %outFile% from %inFile%

copy %inFile% %outFile%
echo replacing %searchFullLC% by %pluginFullNameLC%
..\..\..\..\utils\fart %outFile% %searchFullLC% %pluginFullNameLC%
echo replacing %searchLC% by %pluginNameLC%
..\..\..\..\utils\fart %outFile% %searchLC% %pluginNameLC%

del sqidPluginTemplate.*

:: create source files
cd ..\..\src

:: generate plugin interface header and cpp files
set "inFile=%searchFullLC%.h"
set "outFile=%pluginFullNameLC%.h"
if not exist "%inFile%" (
     echo File not found: %inFile%
     exit /b 1
)
echo creating %outFile% from %inFile%

copy %inFile% %outFile%
echo replacing %searchFullUC% by %pluginFullNameUC%
..\..\..\utils\fart %outFile% %searchFullUC% %pluginFullNameUC%
echo replacing %searchLC% by %pluginNameLC%
..\..\..\utils\fart %outFile% %searchLC% %pluginNameLC%

set "inFile=%searchFullLC%.cpp"
set "outFile=%pluginFullNameLC%.cpp"
if not exist "%inFile%" (
     echo File not found: %inFile%
     exit /b 1
)
echo creating %outFile% from %inFile%

copy %inFile% %outFile%
echo replacing %searchUC% by %pluginNameUC%
..\..\..\utils\fart %outFile% %searchUC% %pluginNameUC%
echo replacing %searchLC% by %pluginNameLC%
..\..\..\utils\fart %outFile% %searchLC% %pluginNameLC%

:: generate operator implementation header and cpp files
set "inFile=%searchLC%Ops.h"
set "outFile=%pluginNameLC%Ops.h"
if not exist "%inFile%" (
     echo File not found: %inFile%
     exit /b 1
)
echo creating %outFile% from %inFile%

copy %inFile% %outFile%
echo replacing %searchUC% by %pluginNameUC%
..\..\..\utils\fart %outFile% %searchUC% %pluginNameUC%

set "inFile=%searchLC%Ops.cpp"
set "outFile=%pluginNameLC%Ops.cpp"
if not exist "%inFile%" (
     echo File not found: %inFile%
     exit /b 1
)
echo creating %outFile% from %inFile%

copy %inFile% %outFile%
echo replacing %searchUC% by %pluginNameUC%
..\..\..\utils\fart %outFile% %searchUC% %pluginNameUC%
echo replacing %searchLC% by %pluginNameLC%
..\..\..\utils\fart %outFile% %searchLC% %pluginNameLC%

del pluginTemplateOps.**
del sqidPluginTemplate.*

cd ..\..\

ren %searchFullLC% %pluginFullNameLC%

exit /b 0



:toUpper str -- converts lowercase character to uppercase
::           -- str [in,out] - valref of string variable to be converted
:$created 20060101 :$changed 20080219 :$categories StringManipulation
:$source https://www.dostips.com
if not defined %~1 EXIT /b
for %%a in ("a=A" "b=B" "c=C" "d=D" "e=E" "f=F" "g=G" "h=H" "i=I"
            "j=J" "k=K" "l=L" "m=M" "n=N" "o=O" "p=P" "q=Q" "r=R"
            "s=S" "t=T" "u=U" "v=V" "w=W" "x=X" "y=Y" "z=Z" "ä=Ä"
            "ö=Ö" "ü=Ü" "ß=ẞ") do (
    call set %~1=%%%~1:%%~a%%
)
EXIT /b

:toLower str -- converts uppercase character to lowercase
::           -- str [in,out] - valref of string variable to be converted
:$created 20060101 :$changed 20080219 :$categories StringManipulation
:$source https://www.dostips.com
if not defined %~1 EXIT /b
for %%a in ("A=a" "B=b" "C=c" "D=d" "E=e" "F=f" "G=g" "H=h" "I=i"
            "J=j" "K=k" "L=l" "M=m" "N=n" "O=o" "P=p" "Q=q" "R=r"
            "S=s" "T=t" "U=u" "V=v" "W=w" "X=x" "Y=y" "Z=z" "Ä=ä"
            "Ö=ö" "Ü=ü" "ẞ=ß") do (
    call set %~1=%%%~1:%%~a%%
)
EXIT /b