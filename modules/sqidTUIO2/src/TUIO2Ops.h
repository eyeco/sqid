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
		struct TUIOPointers
		{
			std::vector<uint> id;
			std::vector<glm::vec2> pos;

			uint32_t ts;

			TUIOPointers() :
				ts( 0 )
			{}

			TUIOPointers( size_t size, uint32_t ts ) :
				id( size ),
				pos( size ),
				ts( ts )
			{}
		};

		struct TUIOPointerExt
		{
			const uint id;

			glm::vec2 pos;

			std::vector<glm::vec2> history;

			TUIOPointerExt() :
				id( -1 ),
				pos( 0 )
			{}

			explicit TUIOPointerExt( uint id, const glm::vec2& p = glm::vec2( 0 ) ) :
				id( id ),
				pos( p )
			{}

			void update( const glm::vec2& p )
			{
				history.push_back( pos );
				pos = p;
			}
		};

		class TUIOSourceImpl;

		class TUIO : public Op
		{
		private:
			bool _connected;
			unsigned short _port;

			TUIOSourceImpl* _impl;

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			explicit TUIO( unsigned short port = 3333 );
			virtual ~TUIO();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();

			virtual std::vector<FrameDrawer*> createDrawers();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			const std::vector<TUIOPointerExt> getPointers() const;

			DECLARE_OP_DESC;
		};

		class TUIO2SF : public Op
		{
		private:
			unsigned int _width;
			unsigned int _height;

		protected:
			virtual bool process();

		public:
			explicit TUIO2SF( unsigned int width = 16, unsigned int height = 16 );
			virtual ~TUIO2SF();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json& j );
			virtual bool saveToJSON( nlohmann::json& j ) const;

			DECLARE_OP_DESC;
		};
	}

	template<> std::string toString<Plugins::TUIOPointers>( const Plugins::TUIOPointers& tp );
}
