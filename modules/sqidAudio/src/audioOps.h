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

#include <processing/op.h>

#include <fileIO/json.h>

namespace sqid
{
	namespace Plugins
	{
		class AudioSourceImpl;

		/*
		//TODO: change AudioIn's output from SampleFrames to custom type AudioFrames
		struct AudioFrames
		{
			std::vector<float> samples;

			uint32_t frameSize;
			uint8_t channels;

			uint32_t ts;

			AudioFrames() :
				frameSize( 0 ),
				channels( 0 ),
				ts( 0 )
			{}

			AudioFrames( size_t frameSize, uint8_t channels, uint32_t ts ) :
				samples( frameSize * channels ),
				frameSize( frameSize ),
				channels( channels ),
				ts( ts )
			{}
		};
		*/

		class AudioIn : public Op
		{
		private:
			int _deviceID;
			int _channels;
			int _bufferFrames;
			int _sampleRate;

			AudioSourceImpl* _impl;

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			AudioIn();
			virtual ~AudioIn();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			DECLARE_OP_DESC;
		};

		class AudioPower : public Op
		{
		private:

		protected:
			virtual bool process();

		public:
			AudioPower();
			virtual ~AudioPower();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			DECLARE_OP_DESC;
		};
	}
}
