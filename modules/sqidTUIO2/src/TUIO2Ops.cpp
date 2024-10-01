/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "TUIO2Ops.h"
#include "frameDrawerTUIO2.h"

#include <app.h>
#include <plugins.h>
#include <processing/pin.h>
#include <processing/opFactory.h>

#include <commonImGui.h>

#include <TUIO2/TuioListener.h>
#include <TUIO2/TuioClient.h>
#include <TUIO2/UdpReceiver.h>


namespace sqid
{
	LOCAL_CONTAINER( Plugins::TUIOPointers, "TUIO pointers", "tp", "235E25FF-2D58-457F-A26B-C28FFEAFB240" )


	namespace Plugins
	{
		DEFINE_OP_DESC( TUIO, "tuio", "/networking/tuio",
			"F24E263F-7DCA-4B8D-91CF-7EBA47AF485B" );
		DEFINE_OP_DESC( TUIO2SF, "toSampleFrame", "/networking/tuio",
			"232926CD-91FF-455E-B1CF-3D40D686AC48" );


		using namespace TUIO2;

		inline glm::vec2 TUIOglm( const TuioPoint& pt )
		{
			return glm::vec2( pt.getX(), pt.getY() );
		}

		class TUIOSourceImpl : protected TuioListener
		{
		private:
			std::mutex _msgMutex;

			TuioClient* _tuioClient;
			OscReceiver* _oscReceiver;

			bool _update;
			std::map<unsigned int, TUIOPointerExt> _pointers;

		protected:
			void tuioAdd( TuioObject* tobj )
			{
				if( tobj->containsNewTuioPointer() )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					TuioPointer* ptr = tobj->getTuioPointer();

					_pointers.insert(
						std::make_pair(
							ptr->getPointerID(),
							TUIOPointerExt( ptr->getPointerID(), TUIOglm( ptr->getPosition() ) )
						) );

					_update = true;
				}
				//TODO: implement objects and blobs
			}

			void tuioUpdate( TuioObject* tobj )
			{
				if( tobj->containsTuioPointer() )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					TuioPointer* ptr = tobj->getTuioPointer();

					_pointers[ptr->getPointerID()].update( TUIOglm( ptr->getPosition() ) );

					_update = true;
				}
				//TODO: implement objects and blobs
			}

			void tuioRemove( TuioObject* tobj )
			{
				if( tobj->containsTuioPointer() )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					TuioPointer* ptr = tobj->getTuioPointer();

					_pointers.erase( ptr->getPointerID() );

					_update = true;
				}
				//TODO: implement objects and blobs
			}

			void tuioRefresh( TuioTime frameTime )
			{}

		public:
			TUIOSourceImpl() :
				_tuioClient( nullptr ),
				_oscReceiver( nullptr ),
				_update( false )
			{}

			~TUIOSourceImpl()
			{
				close();
			}

			bool run( unsigned short port )
			{
				close();

				if( _oscReceiver )
					return false;

				_oscReceiver = new UdpReceiver( port );

				_tuioClient = new TuioClient( _oscReceiver );
				_tuioClient->addTuioListener( this );
				_tuioClient->connect();

				if( !_tuioClient->isConnected() )
				{
					this->close();
					return false;
				}

				return true;
			}

			void close()
			{
				std::lock_guard<std::mutex> lock( _msgMutex );

				if( _tuioClient )
				{
					_tuioClient->disconnect();
					safeDelete( _tuioClient );
				}
				if( _oscReceiver )
				{
					_oscReceiver->disconnect();
					safeDelete( _oscReceiver );
				}
			}

			bool fetchFrames( std::vector<TUIOPointers*>& ptrs )
			{
				if( ptrs.size() )
					std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory here!" << std::endl;

				if( _update )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					_update = false;

					if( _pointers.size() )
					{
						TUIOPointers* p = new TUIOPointers( _pointers.size(), getAppTime() * 1000.0f );
						ptrs.push_back( p );

						int cntr = 0;
						for( auto& it : _pointers )
						{
							p->id[cntr] = it.second.id;
							p->pos[cntr] = it.second.pos;

							cntr++;
						}
					}

					return true;
				}

				return false;
			}

			void fetchObjects( std::vector<TUIOPointerExt>& pointers )
			{
				pointers.clear();

				if( _pointers.size() )
				{
					std::lock_guard<std::mutex> lock( _msgMutex );

					for( auto& it : _pointers )
						pointers.push_back( it.second );
				}
			}
		};


		TUIO::TUIO( unsigned short port ) :
			Op(),
			_connected( false ),
			_port( port ),
			_impl( new TUIOSourceImpl() )
		{
		}

		TUIO::~TUIO()
		{
			stop();

			safeDelete( _impl );
		}

		void TUIO::createPins()
		{
			//addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "ptrID", this ) );
			//addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "ptrPos", this ) );

			addOutlet( new OutletPin( new DataContainer<TUIOPointers>(), "ptrs", this ) );
		}

#ifdef __SUPPORT_GUI
		bool TUIO::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _connected ? "disconnect" : "connect" ) )
			{
				if( _connected )
					stop();
				else
					start();
			}

			{
				ScopedImGuiDisable disable( _connected );

				int p = _port;
				if( ImGui::InputInt( "port", &p ) )
					_port = p;
			}

			return true;
		}

		std::vector<FrameDrawer*> TUIO::createDrawers()
		{
			std::vector<FrameDrawer*> drawers;

			drawers.push_back( new FrameDrawerTUIO2( this ) );

			return drawers;
		}
#endif

		bool TUIO::process()
		{
			if( _impl )
			{
				std::vector<TUIOPointers*> ptrs;

				_impl->fetchFrames( ptrs );

				if( ptrs.size() )
				{
					//TODO: buffer -- for now, we just use the most recent one
					TUIOPointers* ret = ptrs.back();
					if( ret )
					{
						//drawFrame( ret );

						pushOutput( "ptrs", ret );
						ret = nullptr;
					}

					//delete all unused ones
					for( int i = 0; i < ptrs.size(); i++ )
						safeDelete( ptrs[i] );
					ptrs.clear();
				}
			}

			return false;
		}

		bool TUIO::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			load<bool>( j, "connected", _connected );
			load<uint16_t>( j, "port", _port );

			if( _connected )
				start();

			return ret;
		}

		bool TUIO::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "connected", _connected );
			save( j, "port", _port );

			return ret;
		}

		void TUIO::start()
		{
			stop();

			if( _impl->run( _port ) )
				_connected = true;
			else
			{
				std::cerr << "<error> failed to run TUIO on port " << _port << std::endl;
				stop();
			}
		}

		void TUIO::stop()
		{
			_impl->close();
			_connected = false;
		}

		const std::vector<TUIOPointerExt> TUIO::getPointers() const
		{
			std::vector<TUIOPointerExt> ret;

			_impl->fetchObjects( ret );

			return ret;
		}




		TUIO2SF::TUIO2SF( unsigned int width, unsigned int height ) :
			Op(),
			_width( width ),
			_height( height )
		{}

		TUIO2SF::~TUIO2SF()
		{}

		void TUIO2SF::createPins()
		{
			addInlet( new InletPin( new DataContainer<TUIOPointers>(), "ptrs", this ) );

			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool TUIO2SF::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int i[2] = {
				(int) _width,
				(int) _height };
			if( ImGui::InputInt2( "size", i ) )
			{
				_width = max( 1, i[0] );
				_height = max( 1, i[1] );
			}

			return true;
		}
#endif

		bool TUIO2SF::process()
		{
			if( !_width || !_height )
				throw std::runtime_error( "width or height is 0" );

			//SampleFrame *ids = fetchInput<SampleFrame>( "ptrID" );
			//SampleFrame *pos = fetchInput<SampleFrame>( "ptrPos" );
			TUIOPointers* ptrs = fetchInput<TUIOPointers>( "ptrs" );

			//TODO: make use of IDs
			if( /*ids &&*/ ptrs )
			{
				SampleFrame* ret = new SampleFrame( _width, _height, ptrs->ts );

				if( ptrs->id.size() != ptrs->pos.size() )
					std::cerr << "<warning> numbers of IDs and ptrs differ!" << std::endl;

				for( auto& p : ptrs->pos )
				{
					size_t posX = clamp<size_t>( ( _width - 1 ) * p.x, 0, _width );
					size_t posY = clamp<size_t>( ( _height - 1 ) * p.y, 0, _height );

					ret->values()[posX + posY * _width] = 1.0f;
				}

				safeDelete( ptrs );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "ptrs" );
		}

		bool TUIO2SF::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "width", _width );
			load<unsigned int>( j, "height", _height );

			return ret;
		}

		bool TUIO2SF::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "width", _width );
			save( j, "height", _height );

			return ret;
		}
	}

	template<>
	std::string toString<Plugins::TUIOPointers>( const Plugins::TUIOPointers& tp )
	{
		std::stringstream sstr;

		sstr.precision( 3 );
		sstr << tp.pos.size() << ", t:" << tp.ts;

		return sstr.str();
	}
}