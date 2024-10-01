/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "midiOps.h"

#include <app.h>
#include <processing/opFactory.h>

#include <commonImGui.h>

#include <rtmidi.h>


namespace sqid
{
	namespace Plugins
	{
		DEFINE_OP_DESC( MIDIIn, "midiIn", "/devices",
			"277947B4-898B-48FE-8B2B-5D76369A785A" );
		DEFINE_OP_DESC( MIDIOut, "midiOut", "/devices",
			"A9DE901E-F53D-44FC-A34A-1FFCA2BE2B18" );

		/*
			Note off                      8x      Key number          Note Off velocity
			Note on                       9x      Key number          Note on velocity
			Polyphonic Key Pressure       Ax      Key number          Amount of pressure
			Control Change                Bx      Controller number   Controller value
			Program Change                Cx      Program number      None
			Channel Pressure              Dx      Pressure value      None
			Pitch Bend                    Ex      MSB                 LSB
			*/
		enum VoiceMessage
		{
			VM_NOTE_OFF = 0x00,
			VM_NOTE_ON = 0x01,
			VM_POLYPHONIC_KEY_PRESSURE = 0x02,
			VM_CONTROL_CHANGE = 0x03,
			VM_PROGRAM_CHANGE = 0x04,
			VM_CHANNEL_PRESSURE = 0x05,
			VM_PITCH_BEND = 0x06,

			VM_COUNT = 0x07
		};

		const char* voiceMessageToString( VoiceMessage msg )
		{
			switch( msg )
			{
			case VM_NOTE_OFF:
				return "note off";
			case VM_NOTE_ON:
				return "note on";
			case VM_POLYPHONIC_KEY_PRESSURE:
				return "polyphonic key pressure";
			case VM_CONTROL_CHANGE:
				return "control change";
			case VM_PROGRAM_CHANGE:
				return "program change";
			case VM_CHANNEL_PRESSURE:
				return "channel pressure";
			case VM_PITCH_BEND:
				return "pitch bend";
			}

			return "UNKNOWN";
		}

		VoiceMessage voiceMessageFromString( const char* s )
		{
			if( !s )
				return VM_COUNT;

			for( int i = 0; i < VM_COUNT; i++ )
				if( !_stricmp( s, voiceMessageToString( (VoiceMessage) i ) ) )
					return (VoiceMessage) i;

			return VM_COUNT;
		}

		VoiceMessage voiceMessageFromString( const std::string& s )
		{
			return voiceMessageFromString( s.c_str() );
		}

		class MIDISourceImpl
		{
		private:
			bool isStarted;

			unsigned char port;
			unsigned int midiQueueSizeLimit;

			std::vector<std::string> ports;

			RtMidiIn* midiIn;

			unsigned int maxControllers;

			SampleFrame* values;

		public:
			MIDISourceImpl( unsigned int midiQueueSizeLimit = 100 ) :
				isStarted( false ),
				port( 0 ),
				midiQueueSizeLimit( midiQueueSizeLimit ),
				midiIn( nullptr ),
				maxControllers( 121 ),
				values( nullptr )
			{
				try
				{
					midiIn = new RtMidiIn( RtMidi::Api::WINDOWS_MM, "RtMidi Input Client", midiQueueSizeLimit );
				}
				catch( RtMidiError& e )
				{
					std::cerr << "error instantiating RtMidi input: " << e.what() << std::endl;
					midiIn = nullptr;
				}

				rescan();
			}

			~MIDISourceImpl()
			{
				close();

				safeDelete( midiIn );
			}

			void rescan()
			{
				ports.clear();

				if( !midiIn )
				{
					std::cerr << "<error> unable to enumerate" << std::endl;
					return;
				}

				size_t portCount = midiIn->getPortCount();
				std::cout << (int) portCount << " MIDI input sources available" << std::endl;
				for( unsigned int i = 0; i < portCount; i++ )
				{
					try
					{
						ports.push_back( midiIn->getPortName( i ) );
						std::cout << "  port #" << i << ": " << ports.back() << std::endl;
					}
					catch( RtMidiError& e )
					{
						std::cerr << "error getting RtMidi port name: " << e.what() << std::endl;
						continue;
					}
				}
			}

			const std::vector<std::string>& getPorts() const
			{
				return ports;
			}

			bool run( unsigned char port, unsigned char maxControllers )
			{
				if( isStarted )
					return false;

				close();

				if( !maxControllers )
					std::cerr << "<warning> number of controllers cannot be 0" << std::endl;
				else if( maxControllers > 121 )
					std::cerr << "<warning> maximum controllers possible as specified for MIDI protocol is 121" << std::endl;

				if( port >= ports.size() )
					return false;

				midiIn->openPort( port );
				midiIn->ignoreTypes( false, false, false );

				isStarted = midiIn->isPortOpen();

				values = new SampleFrame( maxControllers, 1, 0 );

				return isStarted;
			}

			void close()
			{
				if( midiIn )
					midiIn->closePort();

				safeDelete( values );

				isStarted = false;
			}

			void fetchFrames( std::vector<SampleFrame*>& frames )
			{
				if( frames.size() )
					std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory!" << std::endl;

				//TODO: read asynchronously and put into ring buffer
				// https://www.music.mcgill.ca/~gary/rtmidi/
				// "These messages are then either queued and read by the user via calls to the RtMidiIn::getMessage() function or immediately passed to a user-specified callback function (which must be "registered" using 
				// the RtMidiIn::setCallback() function)"
				if( isStarted )
				{
					if( !midiIn || !midiIn->isPortOpen() )
						return;

					double time = getAppTime();
					uint32_t ts = time * 1000;

					std::vector<unsigned char> message;

					do
					{
						message.clear();

						double deltaTime = midiIn->getMessage( &message );

						if( message.size() )
						{
							SampleFrame* sf = nullptr;

							unsigned char status = message[0];

							bool isStatusByte = ( status & 0x80 );
							unsigned char channelID = ( status & 0x0f );
							unsigned char messageID = ( status & 0x70 ) >> 4;

							switch( messageID )
							{
							case VM_CONTROL_CHANGE:
							{
								if( message.size() != 3 )
								{
									std::cerr << "<error> invalid MIDI controller message (size: " << message.size() << ")" << std::endl;
									break;
								}

								unsigned char controllerNr = message[1];

								//handle channel mode messages
								switch( controllerNr )
								{
									/*
									1st Data Byte      Description                Meaning of 2nd Data Byte
									-------------   ----------------------        ------------------------
											79        Reset all  controllers            None; set to 0
											7A        Local control                     0 = off; 127  = on
											7B        All notes off                     None; set to 0
											7C        Omni mode off                     None; set to 0
											7D        Omni mode on                      None; set to 0
											7E        Mono mode on (Poly mode off)      **
											7F        Poly mode on (Mono mode off)      None; set to 0
									*/
								case 0x79:
									std::cout << "resetting all controllers" << std::endl;
									values->set( 0.0f );
									sf = new SampleFrame( values->width(), values->height(), values->values(), ts, values->depth() );
									break;
								case 0x7A:
								case 0x7B:
								case 0x7C:
								case 0x7D:
								case 0x7E:
								case 0x7F:
									std::cerr << "channel mode message " << (int) controllerNr << " not implemented" << std::endl;
									break;
								default:
									unsigned char controllerValue = message[2];

									if( controllerNr > maxControllers )
										std::cout << "<warning> MIDI from " << (int) controllerNr << " out of specified bounds -- message dropped" << std::endl;
									else
									{
										values->values()[controllerNr] = controllerValue / 127.0f;
										sf = new SampleFrame( values->width(), values->height(), values->values(), ts, values->depth() );
									}
									break;
								}

								break;
							}
							default:
								std::cerr << "MIDI message type " << (int) messageID << " not in use" << std::endl;
							}

							if( sf )
								frames.push_back( sf );
						}

					} while( message.size() );
				}
			}
		};

		class MIDISinkImpl
		{
		private:
			bool isStarted;

			unsigned char port;

			std::vector<std::string> ports;

			RtMidiOut* midiOut;

			unsigned int maxControllers;

		public:
			MIDISinkImpl( unsigned int midiQueueSizeLimit = 100 ) :
				isStarted( false ),
				port( 0 ),
				midiOut( nullptr ),
				maxControllers( 121 )
			{
				try
				{
					midiOut = new RtMidiOut( RtMidi::Api::WINDOWS_MM, "RtMidi Output Client" );
				}
				catch( RtMidiError& e )
				{
					std::cerr << "error instantiating RtMidi output: " << e.what() << std::endl;
					midiOut = nullptr;
				}

				rescan();
			}

			~MIDISinkImpl()
			{
				close();

				safeDelete( midiOut );
			}

			void rescan()
			{
				ports.clear();

				if( !midiOut )
				{
					std::cerr << "<error> unable to enumerate" << std::endl;
					return;
				}

				size_t portCount = midiOut->getPortCount();
				std::cout << (int) portCount << " MIDI output sources available" << std::endl;
				for( unsigned int i = 0; i < portCount; i++ )
				{
					try
					{
						ports.push_back( midiOut->getPortName( i ) );
						std::cout << "  port #" << i << ": " << ports.back() << std::endl;
					}
					catch( RtMidiError& e )
					{
						std::cerr << "error getting RtMidi port name: " << e.what() << std::endl;
						continue;
					}
				}
			}

			const std::vector<std::string>& getPorts() const
			{
				return ports;
			}

			bool run( unsigned char port, unsigned char maxControllers )
			{
				if( isStarted )
					return false;

				close();

				if( !maxControllers )
					std::cerr << "<warning> number of controllers cannot be 0" << std::endl;
				else if( maxControllers > 121 )
					std::cerr << "<warning> maximum controllers possible as specified for MIDI protocol is 121" << std::endl;

				if( port >= ports.size() )
					return false;

				midiOut->openPort( port );

				isStarted = midiOut->isPortOpen();

				return isStarted;
			}

			void close()
			{
				if( midiOut )
					midiOut->closePort();

				isStarted = false;
			}

			void sendFrame( const SampleFrame* frames )
			{
				if( !frames )
					return;

				//TODO: read asynchronously and put into ring buffer
				// https://www.music.mcgill.ca/~gary/rtmidi/
				// "These messages are then either queued and read by the user via calls to the RtMidiIn::getMessage() function or immediately passed to a user-specified callback function (which must be "registered" using 
				// the RtMidiIn::setCallback() function)"
				if( isStarted )
				{
					if( !midiOut || !midiOut->isPortOpen() )
						return;


				}
			}
		};


		MIDIIn::MIDIIn( unsigned char maxControllers ) :
			Op(),
			_connected( false ),
			_port( -1 ),
			_maxControllers( maxControllers ),
			_impl( new MIDISourceImpl() )
		{
		}

		MIDIIn::~MIDIIn()
		{
			safeDelete( _impl );
		}

		void MIDIIn::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "midi", this ) );
		}

#ifdef __SUPPORT_GUI
		bool MIDIIn::drawUI()
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

				if( _impl )
				{
					std::vector<std::string> ports = _impl->getPorts();

					if( !ports.size() )
					{
						ports.push_back( "<no devices found>" );
						_port = 0;
					}
					else
						_port = clamp<int>( _port, 0, ports.size() - 1 );

					ImGui::Text( "device" );
					ImGui::SameLine();
					const char* currentItem = ports[_port].c_str();
					if( ImGui::BeginCombo( "##combo", currentItem, ImGuiComboFlags_None ) )
					{
						for( int i = 0; i < ports.size(); i++ )
						{
							bool isSelected = ( i == _port );
							if( ImGui::Selectable( ports[i].c_str(), isSelected ) )
							{
								currentItem = ports[i].c_str();
								_port = i;
							}
							if( isSelected )
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if( !_impl->getPorts().size() )
						_port = -1;
				}

				int c = _maxControllers;
				if( ImGui::SliderInt( "max Controllers", &c, 1, 121 ) )
					_maxControllers = c;
			}

			return true;
		}
#endif

		//TODO: when dealing with multiple inputs, this does not really make sense
		// think of a better way to do this
		bool MIDIIn::process()
		{
			if( _impl && _connected )
			{
				std::vector<SampleFrame*> frames;

				_impl->fetchFrames( frames );

				if( frames.size() )
				{
					//TODO: buffer -- for now, we just use the most recent one
					SampleFrame* ret = frames.back();

					drawFrame( ret );

					pushOutput( "midi", ret );
					ret = nullptr;

					for( int i = 0; i < frames.size(); i++ )
						safeDelete( frames[i] );
					frames.clear();
				}
			}

			return false;
		}

		bool MIDIIn::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			load<bool>( j, "connected", _connected );
			load<uint8_t>( j, "maxControllers", _maxControllers );

			auto prt = j.find( "port" );
			if( prt != j.end() )
			{
				_port = -1;
				std::string p = *prt;
				if( _impl )
				{
					auto& ports = _impl->getPorts();
					for( int i = 0; i < ports.size(); i++ )
						if( !toLower( p ).compare( toLower( ports[i] ) ) )
						{
							_port = i;
							break;
						}
					if( _port < 0 )
						std::cerr << "<warning> MIDI device \"" << p << "\" not found" << std::endl;
				}
				else
					std::cerr << "<warning> unable to locate MIDI device \"" << p << "\" as RtMidi is not created" << std::endl;
			}

			if( _connected )
				start();

			return ret;
		}

		bool MIDIIn::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "connected", _connected );
			save( j, "maxControllers", (int) _maxControllers );
			if( _impl )
				if( _port >= 0 && _port < _impl->getPorts().size() )
					save( j, "port", _impl->getPorts()[_port] );

			return ret;
		}

		void MIDIIn::start()
		{
			stop();

			std::cerr << "opening MIDI input on port " << (int) _port << std::endl;

			if( _impl->run( _port, _maxControllers ) )
				_connected = true;
			else
			{
				std::cerr << "<error> failed to open MIDI input on port " << (int) _port << std::endl;
				stop();
			}
		}

		void MIDIIn::stop()
		{
			if( _impl )
				_impl->close();

			_connected = false;
		}



		MIDIOut::MIDIOut( unsigned char maxControllers ) :
			Op(),
			_connected( false ),
			_port( -1 ),
			_maxControllers( maxControllers ),
			_impl( new MIDISinkImpl() )
		{
		}

		MIDIOut::~MIDIOut()
		{
			safeDelete( _impl );
		}

		void MIDIOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

#ifdef __SUPPORT_GUI
		bool MIDIOut::drawUI()
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

				if( _impl )
				{
					std::vector<std::string> ports = _impl->getPorts();

					if( !ports.size() )
					{
						ports.push_back( "<no devices found>" );
						_port = 0;
					}
					else
						_port = clamp<int>( _port, 0, ports.size() - 1 );

					ImGui::Text( "device" );
					ImGui::SameLine();
					const char* currentItem = ports[_port].c_str();
					if( ImGui::BeginCombo( "##combo", currentItem, ImGuiComboFlags_None ) )
					{
						for( int i = 0; i < ports.size(); i++ )
						{
							bool isSelected = ( i == _port );
							if( ImGui::Selectable( ports[i].c_str(), isSelected ) )
							{
								currentItem = ports[i].c_str();
								_port = i;
							}
							if( isSelected )
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					if( !_impl->getPorts().size() )
						_port = -1;
				}

				//int c = _maxControllers;
				//if( ImGui::SliderInt( "max Controllers", &c, 1, 121 ) )
				//	_maxControllers = c;
			}

			return true;
		}
#endif

		bool MIDIOut::process()
		{
			SampleFrame* sf = fetchInput<SampleFrame>( "in" );

			if( _impl && _connected && sf )
			{
				_impl->sendFrame( sf );
				drawFrame( sf );
			}

			safeDelete( sf );

			return inputPending( "in" );
		}

		bool MIDIOut::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			load<bool>( j, "connected", _connected );
			load<uint8_t>( j, "maxControllers", _maxControllers );

			auto prt = j.find( "port" );
			if( prt != j.end() )
			{
				_port = -1;
				std::string p = *prt;
				if( _impl )
				{
					auto& ports = _impl->getPorts();
					for( int i = 0; i < ports.size(); i++ )
						if( !toLower( p ).compare( toLower( ports[i] ) ) )
						{
							_port = i;
							break;
						}
					if( _port < 0 )
						std::cerr << "<warning> MIDI device \"" << p << "\" not found" << std::endl;
				}
				else
					std::cerr << "<warning> unable to locate MIDI device \"" << p << "\" as RtMidi is not created" << std::endl;
			}

			if( _connected )
				start();

			return ret;
		}

		bool MIDIOut::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "connected", _connected );
			save( j, "maxControllers", (int) _maxControllers );
			if( _impl )
				if( _port >= 0 && _port < _impl->getPorts().size() )
					save( j, "port", _impl->getPorts()[_port] );

			return ret;
		}

		void MIDIOut::start()
		{
			stop();

			std::cerr << "opening MIDI output on port " << (int) _port << std::endl;

			if( _impl->run( _port, _maxControllers ) )
				_connected = true;
			else
			{
				std::cerr << "<error> failed to open MIDI output on port " << (int) _port << std::endl;
				stop();
			}
		}

		void MIDIOut::stop()
		{
			if( _impl )
				_impl->close();

			_connected = false;
		}
	}
}