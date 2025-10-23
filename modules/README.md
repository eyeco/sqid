# sqid Submodules & Plugins

This folder contains all modules (i.e., [sqid core](./sqidCore/) as well as several plugins). Several plugins are contained in this repository, while others are found in separate ones. 

## Adding external modules

To add new modules follow these steps:
1. Add them as git submodules right here, e.g., with ```git submodule add https://github.com/mySubmodule.git```. 
2. New plugins/modules may need to be added into the VS solution file, use the Solution Explorer and place them at modules (for new modules) or modules/plugins (for new plugins). 
3. Don't forget to configure or update the build dependencies for the correct buil order: usually, plugins will depend on the sqidCore project.

## Generator script for plugin VSVC project and source files

To create additional plugins, use the provided [template](sqidPluginTemplate.zip) and follow these steps to prepare for your custom plugin:
1. run script [generate.bat](./generate.bat), passing the name of your plugin. naming convention is camel case, 1st character will be lower-cased for some files, plugin name, etc. so avoid all-uppercase names
2. add your project to the sqid solution in the folder "plugins"
3. adjust build dependencies, set your plugin to depend on squidCore, so it is always built against up-to-date libraries and include files
4. add 3rd party dependencies into the 3rdparty folder of your plugin, try to stick to the usual directory layout and naming conventions
5. add include directories, library directories, and library dependencies in the project file within MSVS. Use VS macros wherever possible when doing so. Most importantly: avoid absolute paths.
6. for sqid to actually load your plugin, add its name (filename of dll, without extension) to the ```plugins``` array in config.json.

## Providing Dependencies
It is best practice to put all your dependencies in tar.gz archives using ```tar -czf myPluginDependency.tar.gz myPluginDependency``` and place them in a folder ```3rdparty```, along with a script to unpack the archive if this has not been done before. See [the TUIO plugin 3rdparty folder](./sqidTUIO2/3rdparty/) for an example:

```
@echo off

:: set library name
set "lib=tuio-2.0"

:: change into 3rdparty directory
cd /d "%~dp0"

:: check if directory tuio-2.0 already exists
if not exist "%lib%" (
    echo extracting %lib%
    if exist "%lib%.tar.gz" (
        ::extract tuio-2.0.tar.gz
        tar -xzf "%lib%.tar.gz"
        echo done
    ) else (
        echo ERROR: archive %lib%.tar.gz not fround
    )
) else (
    echo found "%lib%", nothing to do
)
```

The batch script extracts the archive into the 3rdparty folder when it is not yet to be found. The sqidTUIO2 VS project is configured so it starts this script as a pre-build event.
As can be seen, the dependencies archive must contain a folder with the library name that exactly matches the archive name itself, e.g.:

```
content of 'myLibraryDependency-0.1.0.tar.gz'
> myLibraryDependency-0.1.0/
        bin/
            msvc/
                x64/
                    Debug/
                    MinSizeRel/
                    Release/
                    RelWithDebugInfo/
                x86/
                    Debug/
                    MinSizeRel/
                    Release/
                    RelWithDebugInfo/
        include/
            ...
        lib/
            msvc/
                x64/
                    Debug/
                    MinSizeRel/
                    Release/
                    RelWithDebugInfo/
                x86/
                    Debug/
                    MinSizeRel/
                    Release/
                    RelWithDebugInfo/
        LICENSE
```

Try to stick to the folder structure above. Place all static library files into the ```lib``` branch and all executable/shared library files into ```bin``` and separate build system, architecture, and build configuration, as in the example, whenever possible. Don't provide superfluous code, stick to the basic required files (i.e., header files, pre-compiled binaries and libs as well as dependencies, debug symbols, etc.), don't add source code or surplus binaries. In Visual Studio, use only paths relative to the project directories, and make use of VS macros to do so and to locate the correct directories (such as ```$(ProjectDir)```, ```$(ConfigurationName)```, ```$(PlatformName)```, ```$(PlatformShortName)```, etc.). Again, see the provided examples as a guideline.

## Placing compiled plugin binaries for use by the sqid application 

The available plugins are set up with post-build events that copy the generated files into the sqid output folder:

```
copy $(OutDir)$(TargetName)$(TargetExt) $(ProjectDir)..\..\..\..\apps\bin\$(PlatformName)\$(Configuration)\
copy $(OutDir)$(TargetName).pdb $(ProjectDir)..\..\..\..\apps\bin\$(PlatformName)\$(Configuration)\
```

Note that 3rdparty dependencies are not yet copied in this fashion, thus they have to be copied manually (or the post-build event needs to be adjusted accordingly).