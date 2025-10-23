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

#include <sources/dataSource.h>

namespace sqid
{
	class SceneGraph;

	namespace Internal
	{
		class COMSourceImpl;
	}

	class SQID_API COMSource : public DataSource
	{
	private:
		unsigned short _portNr;

		Internal::COMSourceImpl *_impl;

		unsigned int _baud;
		unsigned int _queueSize;
		unsigned int _dropdownSelected;

		bool run( const std::string &portName, unsigned int baud, unsigned int queueSize );
		void close();

	public:
		COMSource();
		virtual ~COMSource();

		virtual void fetchFrames( std::vector<SampleFrameContainer> &frames );

		virtual std::string getDesc() const;

		virtual unsigned short getDevicePort() const { return _portNr; }
		virtual DeviceInterface getDeviceInterface() const { return DI_COM; }

#ifdef __SUPPORT_GUI
		virtual bool drawUI();
#endif

		bool write( const unsigned char *data, size_t size );
		bool write( const std::string &str );
		bool write( const std::vector<unsigned char> &data );

		static bool init();
		static bool isInitialized();

		static void rescan();
		static void enumerate();

		bool loadFromJSON( const nlohmann::json &j );
		void saveToJSON( nlohmann::json &j ) const;
	};
}
