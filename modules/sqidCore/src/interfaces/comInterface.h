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

#include <interfaces/dataInterface.h>

namespace sqid
{
	class SceneGraph;

	namespace Internal
	{
		class COMInterfaceImpl;
	}

	enum IOMode : unsigned char
	{
		IOM_NONE			= 0x00,
		IOM_INPUT			= 0x01,
		IOM_OUTPUT			= 0x02,
		IOM_INPUT_OUTPUT	= IOM_INPUT | IOM_OUTPUT
	};

	class SQID_API COMInterface : public DataInterface
	{
	private:
		IOMode _ioMode;

		unsigned char _deviceID;
		unsigned char _sensorID;

		bool _clamp;
		bool _normalize;

		int _dataType;

		unsigned short _portNr;

		Internal::COMInterfaceImpl *_impl;

		unsigned int _baud;
		unsigned int _queueSize;
		unsigned int _dropdownSelected;

		bool run( const std::string &portName, unsigned int baud, unsigned int queueSize );
		void close();

	public:
		COMInterface();
		virtual ~COMInterface();

		virtual bool doesWant( const SampleFrameContainer *sfc ) const;

		virtual void fetchFrames( std::vector<SampleFrameContainer> &frames );
		virtual bool queueFrame( const SampleFrameContainer& sfc );

		virtual std::string getDesc() const;

		virtual unsigned short getDevicePort() const { return _portNr; }
		virtual DataInterfaceType getDataInterfaceType() const { return DIT_COM; }

#ifdef __SUPPORT_GUI
		virtual bool drawUI();
#endif

		static bool init();
		static bool isInitialized();

		static void rescan();
		static void enumerate();

		bool loadFromJSON( const nlohmann::json &j );
		void saveToJSON( nlohmann::json &j ) const;
	};
}
