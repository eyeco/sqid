/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "MQTTOps.h"
#include "sqidMQTT.h"
#include "MQTTContext.h"

#include <app.h>
#include <processing/opFactory.h>

#include <commonImGui.h>

namespace sqid
{
	namespace Plugins
	{
		DEFINE_OP_DESC( MQTTIn, "mqttIn", "/networking",
			"55045E5A-47C9-4895-A98A-42919AE902F8" );
		DEFINE_OP_DESC( MQTTOut, "mqttOut", "/networking",
			"87126F36-818A-4BA3-8627-120960AAAEB3" );


		MQTTIn::MQTTIn( const std::string& address, unsigned short port, const std::string& topic, int qos, bool cleanSession, int keepAlive ) :
			Op(),
			_handle( 0 ),
			_id( "" ),
			_address( address ), _port( port ),
			_cleanSession( cleanSession ), _keepAlive( keepAlive ),
			_topic( topic ), _qos( qos ),
			_tryReconnect( true ),
			_reconnectTimeout( 5 ),
			_inputBufferID( 64 ),
			_inputBufferAddress( 128 ),
			_inputBufferTopic( 128 )
		{
			//initiate creation of singleton
			MQTTSingleton::get();

			updateBuffers();
		}

		MQTTIn::~MQTTIn()
		{
			close();
		}

		void MQTTIn::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool MQTTIn::drawUI()
		{
			if( !Op::drawUI() )
				return false;


			if( ImGui::Button( _handle ? "stop" : "start" ) )
			{
				if( _handle )
					close();
				else
					open();
			}

			{
				ScopedImGuiDisable disable( _handle );

				if( ImGui::InputText( "id", &_inputBufferID[0], _inputBufferID.size() ) )
					_id.assign( &_inputBufferID[0] );

				if( ImGui::InputText( "address", &_inputBufferAddress[0], _inputBufferAddress.size() ) )
					_address.assign( &_inputBufferAddress[0] );

				int i = _port;
				if( ImGui::InputInt( "port", &i ) )
					_port = clamp<int>( i, 0x0000, 0xffff );

				ImGui::Checkbox( "clean session", &_cleanSession );

				i = _keepAlive;
				if( ImGui::InputInt( "keep alive", &i ) )
					_keepAlive = max<int>( i, 5 );

				if( ImGui::InputText( "topic", &_inputBufferTopic[0], _inputBufferTopic.size() ) )
					_topic.assign( &_inputBufferTopic[0] );

				i = _qos;
				if( ImGui::SliderInt( "qos", &i, 0, 2 ) )
					_qos = i;

				ImGui::Checkbox( "auto-reconnect", &_tryReconnect );

				{
					ScopedImGuiDisable disalbe2( !_tryReconnect );
					ImGui::InputInt( "timeout", &_reconnectTimeout );
				}
			}

			return true;
		}
#endif

		//TODO: when dealing with multiple inputs, this does not really make sense
		// think of a better way to do this
		bool MQTTIn::process()
		{
			auto ctxt = MQTTSingleton::get();
			if( _handle && ctxt )
			{
				std::vector<SampleFrame*> frames;

				if( !ctxt->fetchFrames( _handle, frames ) )
					_handle = 0;

				if( frames.size() )
				{
					//TODO: buffer -- for now, we just use the most recent one
					SampleFrame* ret = frames.back();

					if( ret )
					{
						drawFrame( ret );

						pushOutput( "out", ret );
						ret = nullptr;
					}

					for( int i = 0; i < frames.size(); i++ )
						safeDelete( frames[i] );
					frames.clear();
				}
			}

			return false;
		}

		bool MQTTIn::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			bool sending = false;
			load<bool>( j, "sending", sending );
			load<std::string>( j, "id", _id );

			load<std::string>( j, "address", _address );
			load<unsigned short>( j, "port", _port );
			load<bool>( j, "cleanSession", _cleanSession );
			load<int>( j, "keepAlive", _keepAlive );
			load<std::string>( j, "topic", _topic );
			load<int>( j, "qos", _qos );
			load<bool>( j, "tryReconnect", _tryReconnect );
			load<int>( j, "reconnectTimeout", _reconnectTimeout );

			updateBuffers();

			if( sending )
				open();

			return ret;
		}

		bool MQTTIn::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "sending", _handle ? true : false );

			save( j, "id", _id );
			save( j, "address", _address );
			save( j, "port", _port );
			save( j, "cleanSession", _cleanSession );
			save( j, "keepAlive", _keepAlive );
			save( j, "topic", _topic );
			save( j, "qos", _qos );
			save( j, "tryReconnect", _tryReconnect );
			save( j, "reconnectTimeout", _reconnectTimeout );

			return ret;
		}

		void MQTTIn::updateBuffers()
		{
			strncpy( &_inputBufferID[0], _id.c_str(), _id.size() + 1 );
			strncpy( &_inputBufferAddress[0], _address.c_str(), _address.size() + 1 );
			strncpy( &_inputBufferTopic[0], _topic.c_str(), _topic.size() + 1 );
		}

		bool MQTTIn::open()
		{
			close();

			if( !_address.size() )
			{
				std::cerr << "<error> address not set" << std::endl;
				return false;
			}

			if( !_topic.size() )
			{
				std::cerr << "<error> topic not set" << std::endl;
				return false;
			}

			auto ctxt = MQTTSingleton::get();
			if( !ctxt )
			{
				std::cerr << "<error> MQTT context not created" << std::endl;
				return false;
			}

			_handle = ctxt->createSubscriber(
				_id,
				_cleanSession,
				_address.c_str(), _port, _keepAlive,
				_topic, _qos,
				_tryReconnect, _reconnectTimeout
			);

			if( !_handle )
			{
				std::cerr << "<error> open failed" << std::endl;
				return false;
			}

			return true;
		}

		void MQTTIn::close()
		{
			if( _handle )
			{
				auto ctxt = MQTTSingleton::get();
				if( ctxt )
					ctxt->destroy( _handle );
				else
				{
					std::cerr << "<error> MQTT context already destroyed?" << std::endl;
					return;
				}

				_handle = 0;
			}
		}














		MQTTOut::MQTTOut( const std::string& address, unsigned short port, const std::string& topic, int qos, bool retain, bool cleanSession, int keepAlive ) :
			Op(),
			_handle( 0 ),
			_id( "" ),
			_address( address ), _port( port ),
			_cleanSession( cleanSession ), _keepAlive( keepAlive ),
			_topic( topic ), _qos( qos ),
			_retain( retain ),
			_tryReconnect( true ),
			_reconnectTimeout( 5 ),
			_inputBufferID( 64 ),
			_inputBufferAddress( 128 ),
			_inputBufferTopic( 128 )
		{
			//initiate creation of singleton
			MQTTSingleton::get();

			updateBuffers();
		}

		MQTTOut::~MQTTOut()
		{
			close();
		}

		bool MQTTOut::process()
		{
			SampleFrame* sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				auto ctxt = MQTTSingleton::get();
				if( _handle && ctxt )
				{
					if( sf->size() > PACKAGE_MAX_SAMPLES )
						std::cout << "<warning> sampleframe with " << sf->size() << " samples is seems quite tall to be sent via MQTT -- package will not be sent" << std::endl;
					else
					{
						if( sf->size() > 100 )
						{
							std::cout << "<warning> sampleframe size is borderline for sending MQTT packages with " << sf->size() << " samples." << std::endl;
						}

						if( !ctxt->publish( sf, _handle, _topic, _qos, _retain ) )
							_handle = 0;
					}
				}

				drawFrame( sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool MQTTOut::drawUI()
		{
			if( !Op::drawUI() )
				return false;


			if( ImGui::Button( _handle ? "stop" : "start" ) )
			{
				if( _handle )
					close();
				else
					open();
			}

			{
				ScopedImGuiDisable disable( _handle );

				if( ImGui::InputText( "id", &_inputBufferID[0], _inputBufferID.size() ) )
					_id.assign( &_inputBufferID[0] );

				if( ImGui::InputText( "address", &_inputBufferAddress[0], _inputBufferAddress.size() ) )
					_address.assign( &_inputBufferAddress[0] );

				int i = _port;
				if( ImGui::InputInt( "port", &i ) )
					_port = clamp<int>( i, 0x0000, 0xffff );

				ImGui::Checkbox( "clean session", &_cleanSession );

				i = _keepAlive;
				if( ImGui::InputInt( "keep alive", &i ) )
					_keepAlive = max<int>( i, 5 );

				if( ImGui::InputText( "topic", &_inputBufferTopic[0], _inputBufferTopic.size() ) )
					_topic.assign( &_inputBufferTopic[0] );

				i = _qos;
				if( ImGui::SliderInt( "qos", &i, 0, 2 ) )
					_qos = i;

				ImGui::Checkbox( "retain", &_retain );

				ImGui::Checkbox( "auto-reconnect", &_tryReconnect );

				{
					ScopedImGuiDisable disalbe2( !_tryReconnect );
					ImGui::InputInt( "timeout", &_reconnectTimeout );
				}
			}

			return true;
		}
#endif

		void MQTTOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

		bool MQTTOut::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			bool sending = false;

			load<bool>( j, "sending", sending );
			load<std::string>( j, "id", _id );

			load<std::string>( j, "address", _address );
			load<unsigned short>( j, "port", _port );
			load<bool>( j, "cleanSession", _cleanSession );
			load<int>( j, "keepAlive", _keepAlive );
			load<std::string>( j, "topic", _topic );
			load<int>( j, "qos", _qos );
			load<bool>( j, "retain", _retain );
			load<bool>( j, "tryReconnect", _tryReconnect );
			load<int>( j, "reconnectTimeout", _reconnectTimeout );

			updateBuffers();

			if( sending )
				open();

			return ret;
		}

		bool MQTTOut::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "sending", _handle ? true : false );

			save( j, "id", _id );
			save( j, "address", _address );
			save( j, "port", _port );
			save( j, "cleanSession", _cleanSession );
			save( j, "keepAlive", _keepAlive );
			save( j, "topic", _topic );
			save( j, "qos", _qos );
			save( j, "retain", _retain );
			save( j, "tryReconnect", _tryReconnect );
			save( j, "reconnectTimeout", _reconnectTimeout );

			return ret;
		}

		void MQTTOut::updateBuffers()
		{
			strncpy( &_inputBufferID[0], _id.c_str(), _id.size() + 1 );
			strncpy( &_inputBufferAddress[0], _address.c_str(), _address.size() + 1 );
			strncpy( &_inputBufferTopic[0], _topic.c_str(), _topic.size() + 1 );
		}

		bool MQTTOut::open()
		{
			close();

			if( !_address.size() )
			{
				std::cerr << "<error> address not set" << std::endl;
				return false;
			}

			if( !_topic.size() )
			{
				std::cerr << "<error> topic not set" << std::endl;
				return false;
			}

			auto ctxt = MQTTSingleton::get();
			if( !ctxt )
			{
				std::cerr << "<error> MQTT context not created" << std::endl;
				return false;
			}

			_handle = ctxt->createPublisher(
				_id,
				_cleanSession,
				_address.c_str(), _port, _keepAlive,
				_tryReconnect, _reconnectTimeout
			);

			if( !_handle )
			{
				std::cerr << "<error> open failed" << std::endl;
				return false;
			}

			return true;
		}

		void MQTTOut::close()
		{
			if( _handle )
			{
				auto ctxt = MQTTSingleton::get();
				if( ctxt )
					ctxt->destroy( _handle );
				else
				{
					std::cerr << "<error> MQTT context already destroyed?" << std::endl;
					return;
				}

				_handle = 0;
			}
		}
	}
}