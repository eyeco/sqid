#pragma once

/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <config.h>
#include <plugins.h>

#include <list>
#include <filesystem>

namespace sqid
{
	class PluginLoader
	{
	private:
		struct Plugin
		{
			void *handle;

			bool initialized;

			PluginCreateFunction		createFunction;
			PluginDeleteFunction		deleteFunction;
			//PluginRegisterOpsFunction	registerOpsFunction;

			PluginInterface *pi;

			std::string name;
		};

		std::filesystem::path				_pluginPath;
		std::list<std::filesystem::path>	_pluginFiles;

		std::vector<Plugin>	_plugins;

		bool checkCompatibility( const PluginInterface *pi );

		bool close( Plugin &plugin ) const;

	public:
		PluginLoader();
		~PluginLoader();

		bool rescan( const std::string &directory );

		bool load();
		void unload();

		//void initImGui();

		bool registerOps();
	};
}