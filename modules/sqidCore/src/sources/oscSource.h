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

#include <sources/dataSource.h>

namespace sqid
{
	class SceneGraph;

	namespace Internal
	{
		class OSCSourceImpl;
	}

	class SQID_API OSCSource : public DataSource
	{
	private:
		Protocol _proto;

		std::string _ip;
		unsigned short _port;

		unsigned int _queueSize;

		bool _connected;

		std::vector<char> _inputBufferIP;

		Internal::OSCSourceImpl *_impl;

		bool run();
		void close();

		void updateBuffers();

	public:
		OSCSource();
		virtual ~OSCSource();

		virtual void fetchFrames( std::vector<SampleFrameContainer> &frames );

		virtual std::string getDesc() const;

		virtual unsigned short getDevicePort() const { return _port; }
		virtual DeviceInterface getDeviceInterface() const { return DI_OSC; }

#ifdef __SUPPORT_GUI
		virtual bool drawUI();
#endif

		//static bool init();
		//static bool isInitialized();

		bool loadFromJSON( const nlohmann::json &j );
		void saveToJSON( nlohmann::json &j ) const;
	};
}
