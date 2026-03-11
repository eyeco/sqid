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

#include <processing/op.h>

namespace sqid
{
	class Source : public Op
	{
	private:
		DataInterfaceType _interface;
		unsigned short _port;

		unsigned char _deviceID;
		unsigned char _sensorID;

		std::string _msgFilterOSC;
		bool _exactOSC;
		
		std::vector<char> _inputBufferMsg;

		float _timeout;
		double _lastUpdateTime;

		float _sampleRate;
		unsigned int _sampleCntr;

		uint64_t _dataRate;
		uint64_t _dataCntr;

		float _statsTimeAccu;

		std::string _sourceDesc;

		unsigned int _maxBufferSize;
		std::list<SampleFrame*> _bufferedFrames;

		void updateMsgFilter();

	protected:
		virtual bool process();

	public:
		explicit Source( unsigned short port = 0, float timeout = 15.0f, unsigned int maxBufferSize = 128 );
		virtual ~Source();

		//void display( int position, int count );
		virtual void createPins();

		void updateStats( float dt );

		bool isOffline();
		void setSourceDesc( const std::string &desc ) { _sourceDesc = desc; }

		DataInterfaceType getInterface() const { return _interface; }
		unsigned short getPort() const { return _port; }

		bool doesWant( const SampleFrameContainer *sfc, const std::string &senderDesc );
		//unsigned char getSensorID() const { return _sensorID; }
		//unsigned char getDeviceID() const { return _deviceID; }

		float getSampleRate() const { return _sampleRate; }
		uint64_t getDataRate() const { return _dataRate; }

		bool feed( const SampleFrame *sf );

#ifdef __SUPPORT_GUI
		virtual bool drawUI();
#endif

		virtual bool loadFromJSON( const nlohmann::json &j );
		virtual bool saveToJSON( nlohmann::json &j ) const;

		DECLARE_OP_DESC;
	};
}