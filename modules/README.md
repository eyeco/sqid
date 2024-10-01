# sqid Submodules

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