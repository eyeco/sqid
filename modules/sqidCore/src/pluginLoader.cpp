/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "pluginLoader.h"

#include <app.h>

#include <nlohmann\json.hpp>

#include <fstream>
#include <filesystem>

#include <dlfcn.h>

namespace sqid
{
	bool compiledWithGUI()
	{
#ifdef __SUPPORT_GUI
		return true;
#else
		return false;
#endif
	}

	bool compiledWithCompression()
	{
#ifdef __COMPRESSION_SUPPORT
		return true;
#else
		return false;
#endif
	}

	PluginLoader::PluginLoader()
	{}

	PluginLoader::~PluginLoader()
	{
		unload();
	}

	bool PluginLoader::checkCompatibility( const PluginInterface *pi ) 
	{
		//TODO: doublecheck compatibility here (compiler, etc., but also preprocessor defines like sqid version, GUI, COMPRESSION support...)

		//TODO: a bit mor sophisticated version checking would be good, allowing for backward-compatibility
		if( strcmp( pi->getBuiltForVersionString(), __VERSION_STRING ) )
		{
			std::cerr << "<error> plugin built for sqid version " << pi->getBuiltForVersionString() << " (this is " << __VERSION_STRING << ")" << std::endl;
			return false;
		}

		if( pi->compiledWithGUI() != compiledWithGUI() )
		{
			std::cerr << "<error> plugin '" << pi->getName() << "' incompatbile: compiled " << ( pi->compiledWithGUI() ? "with" : "without" ) << " GUI support" << std::endl;
			return false;
		}

		if( pi->compiledWithCompression() != compiledWithCompression() )
		{
			//NOTE: this actually does not mean it's completely incompatible, we just have to do some additional safety checks, but we don't
			// want to bother with this now... consider it a TODO
			std::cerr << "<error> plugin '" << pi->getName() << "' incompatbile: compiled " << ( pi->compiledWithCompression() ? "with" : "without" ) << " compression support" << std::endl;
			return false;
		}

		return true;
	}

	bool PluginLoader::rescan( const std::string &directory )
	{
		namespace fs = std::filesystem;

		_pluginPath = fs::path( directory );
		_pluginFiles.clear();

		std::ifstream i( "config.json" );
		if( !i.is_open() )
		{
			std::cerr << "<error> could not load config.json" << std::endl;
			return false;
		}

		using json = nlohmann::json;

		json root;
		i >> root;

		json pluginNames = root["plugins"];

		if( pluginNames.is_null() )
		{
			std::cerr << "<warning> could not find \"plugins\" section in config.json" << std::endl;
			return false;
		}

		try
		{
			if( fs::exists( _pluginPath ) && fs::is_directory( _pluginPath ) )
			{
				for( auto it = pluginNames.begin(); it != pluginNames.end(); ++it )
				{
					std::string name = it.value();

#ifdef _DEBUG
					name.append( "_d.dll" );
#else
					name.append( ".dll" );
#endif

					fs::path pluginPath = _pluginPath / fs::path( name );

					if( !fs::exists( pluginPath ) )
					{
						std::cerr << "<warning> plugin \"" << name << "\" not found in directory " << _pluginPath.string() << std::endl;
						continue;
					}

					if( fs::is_regular_file( pluginPath ) )
						_pluginFiles.push_back( pluginPath );
				}

				std::cout << "found " << _pluginFiles.size() << " plugin file(s):" << std::endl;

				for( auto it : _pluginFiles )
					std::cout << "  " << it.filename().string() << std::endl;
			}
			else
			{
				std::cerr << "<warning> plugin directory does not exist or is not accessible." << std::endl;
				return false;
			}
		}
		catch( const fs::filesystem_error& e )
		{
			std::cerr << e.what() << std::endl;
			return false;
		}
		return true;
	}

	bool PluginLoader::close( Plugin &plugin ) const
	{
		bool ret = true;

		//do not delete in this module (w/ delete), best practice is to delete in same module
		if( plugin.pi )
		{
			if( plugin.initialized )
			{
				plugin.pi->uninit();
				plugin.initialized = false;
			}

			if( plugin.deleteFunction )
				plugin.deleteFunction( plugin.pi );
			else
			{
				std::cerr << "<warning> apparently, plugin was created, but delete function was not found: mem-leak! (" << plugin.name << ")" << std::endl;
				ret = false;
			}
			plugin.pi = nullptr;
		}

		if( plugin.handle )
		{
			try
			{
				if( dlclose( plugin.handle ) )
				{
					char* errstr = dlerror();
					if( errstr != NULL )
						std::cerr << "<error> closing plugin handle failed (" << plugin.name << "): " << errstr << std::endl;
					else
						std::cerr << "<error> closing plugin handle failed (" << plugin.name << ")" << std::endl;
				}
			}
			catch( std::exception& e )
			{
				std::cerr << "<error> caught exception closing plugin dll (" << plugin.name << "): " << e.what() << std::endl;
			}
			plugin.handle = nullptr;
		}

		return ret;
	}

	bool PluginLoader::load()
	{
		for( auto it : _pluginFiles )
		{
			std::cout << "loading plugin " << it.filename().string() << std::endl;

			Plugin plugin = { 0 };

			plugin.name = it.filename().string();
			plugin.handle = dlopen( it.string().c_str(), RTLD_GLOBAL | RTLD_LAZY );

			if( plugin.handle )
			{
				if( !( plugin.createFunction = (PluginCreateFunction) dlsym( plugin.handle, "createPlugin" ) ) )
				{
					char *errstr = dlerror();
					if( errstr != NULL )
						std::cerr << "getting function 'createPlugin' failed: " << errstr << std::endl;
					else
						std::cerr << "getting function 'createPlugin' failed" << std::endl;
				}

				if( !( plugin.deleteFunction = (PluginDeleteFunction) dlsym( plugin.handle, "deletePlugin" ) ) )
				{
					char *errstr = dlerror();
					if( errstr != NULL )
						std::cerr << "getting function 'deletePlugin' failed: " << errstr << std::endl;
					else
						std::cerr << "getting function 'deletePlugin' failed" << std::endl;
				}

				/*if( !( plugin.registerOpsFunction = (PluginRegisterOpsFunction) dlsym( plugin.handle, "registerOps" ) ) )
				{
					char* errstr = dlerror();
					if( errstr != NULL )
						std::cerr << "getting function 'registerOps' failed: " << errstr << std::endl;
					else
						std::cerr << "getting function 'registerOps' failed" << std::endl;
				}*/

				if( plugin.createFunction && plugin.deleteFunction )// && plugin.registerOpsFunction )
				{
					plugin.pi = plugin.createFunction();
					if( plugin.pi )
					{
						if( !checkCompatibility( plugin.pi ) )
						{
							std::cerr << "<error> loading '" << plugin.pi->getName() << "' failed (incompatible code)" << std::endl;

							close( plugin );
						}
						else
						{
							std::cout << "name: " << plugin.pi->getName() << std::endl;

							if( plugin.pi->init() )
							{
								plugin.initialized = true;
								_plugins.push_back( plugin );
							}
							else
							{
								std::cerr << "<error> initializing '" << plugin.pi->getName() << "' failed" << std::endl;

								close( plugin );
							}
						}
					}
					else
					{
						std::cerr << "<error> failed to create plugin" << std::endl;
						close( plugin );
					}
				}
				else
					close( plugin );
			}
			else
			{
				char *errstr = dlerror();
				if( errstr != NULL )
					std::cerr << "loading failed: " << errstr << std::endl;
				else
					std::cerr << "loading failed" << std::endl;
			}
		}

		return true;
	}

	void PluginLoader::unload()
	{
		for( auto it : _plugins )
		{
			std::cout << "unloading plugin " << it.pi->getName() << std::endl;
			close( it );
		}
		_plugins.clear();
	}

	/*
	void PluginLoader::initImGui()
	{
		ImGuiContext *ctx = ImGui::GetCurrentContext();
		ImGuiMemAllocFunc imGuiAllocFunc = nullptr;
		ImGuiMemFreeFunc imGuiFreeFunc = nullptr;
		void *imGuiUserData = nullptr;
		ImGui::GetAllocatorFunctions( &imGuiAllocFunc, &imGuiFreeFunc, &imGuiUserData );

		for( auto it : _plugins )
		{
			std::cout << "initializing ImGui for plugin " << it.pi->getName() << std::endl;
			it.pi->initImGui( ctx, imGuiAllocFunc, imGuiFreeFunc, imGuiUserData );
		}
	}
	*/

	bool PluginLoader::registerOps()
	{
		bool ret = true;

		for( auto it : _plugins )
		{
			std::cout << "registering Ops of plugin " << it.pi->getName() << std::endl;
			//ret &= it.registerOpsFunction();
			ret &= it.pi->registerOps();
		}

		return ret;
	}
}
