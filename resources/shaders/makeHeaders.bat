@echo off

cd /d "%~dp0"

echo creating header files from shaders

if not exist "./include" (
    echo creatig include
    mkdir include
)

for %%f in (*.frag,*.vert) do (

    echo R^"^( > ./include/%%f.h

    for /f "delims=" %%A in ('type %%f') do (
        echo %%A>> ./include/%%f.h
    )

    echo ^)^" >> ./include/%%f.h
)