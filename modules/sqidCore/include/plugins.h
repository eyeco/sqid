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

//instantiate and define container for local use, within module boundaries
#define LOCAL_CONTAINER( _DATA_TYPE, _NAME, _SHORT_NAME, _GUID ) \
	template class DataContainer<_DATA_TYPE>; \
	template<> const GUID DataContainer<_DATA_TYPE>::typeGUID = guidFromString( _GUID ); \
	template<> const std::string DataContainer<_DATA_TYPE>::typeName( _NAME ); \
	template<> const std::string DataContainer<_DATA_TYPE>::typeShortName( _SHORT_NAME );

namespace sqid
{
	class SQID_API PluginInterface
	{
	public:
		virtual ~PluginInterface() {}

		virtual const char *getName() const = 0;

		virtual void getVersion( int &major, int &minor, int &patch ) const = 0;
		virtual const char *getVersionString() const = 0;

		void getBuiltForVersion( int &major, int &minor, int &patch ) const { major = __VERSION_MAJOR; minor = __VERSION_MINOR; patch = __VERSION_PATCH; }
		const char *getBuiltForVersionString() const { return __VERSION_STRING; }

		virtual void printBuildInfo() const = 0;

		virtual bool init() = 0;
		virtual void uninit() = 0;

		//virtual void initImGui( ImGuiContext *imGuiContext, ImGuiMemAllocFunc imGuiAllocFunc, ImGuiMemFreeFunc imGuiFreeFunc, void* imGuiUserData ) = 0;

		virtual bool registerOps() = 0;
		virtual void enumerateCaps() = 0;

		virtual bool compiledWithGUI() const = 0;
		virtual bool compiledWithCompression() const = 0;
	};

	typedef PluginInterface* ( *PluginCreateFunction )();
	typedef void ( *PluginDeleteFunction )( PluginInterface* );
	//typedef bool ( *PluginRegisterOpsFunction )();
}
