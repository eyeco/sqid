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

// include nlohmann JSON if you plan to load/save your custom parameters
#include <fileIO/json.h>

namespace sqid
{
	namespace Plugins
	{
		class MIDISourceImpl;
		class MIDISinkImpl;

		class MIDIIn : public Op
		{
		private:
			bool _connected;

			int _port;
			unsigned char _maxControllers;

			MIDISourceImpl* _impl;

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			explicit MIDIIn( unsigned char maxControllers = 121 );
			virtual ~MIDIIn();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			DECLARE_OP_DESC;
		};

		class MIDIOut : public Op
		{
		private:
			bool _connected;

			int _port;
			unsigned char _maxControllers;

			MIDISinkImpl* _impl;

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			explicit MIDIOut( unsigned char maxControllers = 121 );
			virtual ~MIDIOut();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			DECLARE_OP_DESC;
		};
	}
}
