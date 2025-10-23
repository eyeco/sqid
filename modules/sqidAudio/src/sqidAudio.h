/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <config.h>

#include <plugins.h>

#define __PLUGIN_VERSION_MAJOR		0
#define __PLUGIN_VERSION_MINOR		0
#define __PLUGIN_VERSION_PATCH		1

#define __PLUGIN_VERSION_STRING	STR( __PLUGIN_VERSION_MAJOR ) "." STR( __PLUGIN_VERSION_MINOR ) "." STR( __PLUGIN_VERSION_PATCH )

#ifdef __SUPPORT_GUI
#define GUI_SUPPORT true
#else
#define GUI_SUPPORT false
#endif

#ifdef __COMPRESSION_SUPPORT
#define COMPRESSION_SUPPORT true
#else
#define COMPRESSION_SUPPORT false
#endif

namespace sqid
{
	namespace Plugins
	{
		class SqidAudio : public PluginInterface
		{
		public:
			SqidAudio();
			virtual ~SqidAudio();

			virtual const char *getName() const { return "audio"; }

			virtual void getVersion( int &major, int &minor, int &patch ) const { major = __PLUGIN_VERSION_MAJOR; minor = __PLUGIN_VERSION_MINOR; patch = __PLUGIN_VERSION_PATCH; }
			virtual const char *getVersionString() const { return __PLUGIN_VERSION_STRING; }

			virtual void printBuildInfo() const;

			virtual bool init();
			virtual void uninit();

			virtual bool registerOps();
			virtual void enumerateCaps();

			virtual bool compiledWithGUI() const { return GUI_SUPPORT; }
			virtual bool compiledWithCompression() const { return COMPRESSION_SUPPORT; }
		};
	}
}
