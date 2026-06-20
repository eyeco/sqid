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
	class Sink : public Op
	{
	private:
		DataInterfaceType _interface;
		unsigned short _port;

		unsigned char _deviceID;
		unsigned char _sensorID;

		std::string _msgOSC;

		std::vector<char> _inputBufferMsg;

		double _lastUpdateTime;

		float _sampleRate;
		unsigned int _sampleCntr;

		float _frameRate;
		unsigned int _frameCntr;

		uint64_t _dataRate;
		uint64_t _dataCntr;

		float _statsTimeAccu;

		unsigned int _maxBufferSize;
		std::list<SampleFrame*> _bufferedFrames;

		void updateMsgFilter();

		void queue( const SampleFrame *sf );

	protected:
		virtual bool process();

	public:
		explicit Sink( unsigned short port = 0, unsigned int maxBufferSize = 128 );
		virtual ~Sink();

		virtual void createPins();

		void updateStats( float dt );
		bool fetchFrames( std::vector<SampleFrameContainer>& frames );

		//void setSinkDesc( const std::string& desc ) { _sinkDesc = desc; }

		DataInterfaceType getInterface() const { return _interface; }
		unsigned short getPort() const { return _port; }

#ifdef __SUPPORT_GUI
		virtual bool drawUI();
#endif

		virtual bool loadFromJSON( const nlohmann::json& j );
		virtual bool saveToJSON( nlohmann::json& j ) const;

		DECLARE_OP_DESC;
	};
}