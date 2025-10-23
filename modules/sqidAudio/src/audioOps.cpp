/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "audioOps.h"

#include <app.h>
#include <processing/opFactory.h>

#include <commonImGui.h>

#include <RtAudio.h>

namespace sqid
{
	namespace Plugins
	{
		DEFINE_OP_DESC( AudioIn, "audioIn", "/devices",
			"2EAD3DD7-9E04-4F88-9EAA-511ECF7B334B" );
		DEFINE_OP_DESC( AudioPower, "power", "/audio",
			"ED8C05D9-61F0-46E8-A944-E3C19969BCD1" );



		enum AudioAPI
		{
			AA_UNSPECIFIED,
			AA_DUMMY,
#if defined(_WIN32)
			AA_WASAPI,
			AA_ASIO,
			AA_DIRECT_SOUND,
#elif __APPLE__
			AA_CORE_AUDIO,
#elif __linux__
			AA_ALSA,
			AA_PULSE_AUDIO,
			AA_OSS,
#elif __unix__
			AA_JACK_CLIENT,
#endif

			AA_COUNT
		};

		enum AudioSampleFormat
		{
			ASF_8BIT,
			ASF_16BIT,
			ASF_24BIT,
			ASF_32BIT,
			ASF_FLOAT,
			ASF_DOUBLE,

			ASF_COUNT
		};

		const char* audioAPIToString( AudioAPI api )
		{
			switch( api )
			{
			case AA_UNSPECIFIED:
				return "UNSPECIFIED";
			case AA_DUMMY:
				return "DUMMY";
#if defined(_WIN32)
			case AA_WASAPI:
				return "WASAPI";
			case AA_ASIO:
				return "ASIO";
			case AA_DIRECT_SOUND:
				return "DS";
#elif __APPLE__
			case AA_CORE_AUDIO:
				return "CORE";
#elif __linux__
			case AA_ALSA:
				return "ALSA";
			case AA_PULSE_AUDIO:
				return "PULSE";
			case AA_OSS:
				return "OSS";
#elif __unix__
			case AA_JACK_CLIENT:
				return "JACK";
#endif
			}

			return "UNKNOWN";
		}

		AudioAPI audioAPIFromString( const char* s )
		{
			if( !s )
				return AA_COUNT;

			for( int i = 0; i < AA_COUNT; i++ )
				if( !_stricmp( s, audioAPIToString( (AudioAPI) i ) ) )
					return (AudioAPI) i;

			return AA_COUNT;
		}

		const char* audioSampleFormatToString( AudioSampleFormat format )
		{
			switch( format )
			{
			case ASF_8BIT:
				return "8BIT";
			case ASF_16BIT:
				return "16BIT";
			case ASF_24BIT:
				return "24BIT";
			case ASF_32BIT:
				return "32BIT";
			case ASF_FLOAT:
				return "FLOAT";
			case ASF_DOUBLE:
				return "DOUBLE";
			}

			return "UNKNOWN";
		}

		AudioSampleFormat audioSampleFormatFromString( const char* s )
		{
			if( !s )
				return ASF_COUNT;

			for( int i = 0; i < ASF_COUNT; i++ )
				if( !_stricmp( s, audioSampleFormatToString( (AudioSampleFormat) i ) ) )
					return (AudioSampleFormat) i;

			return ASF_COUNT;
		}

		enum AudioDeviceType
		{
			ADT_UNKNOWN = 0x00,

			ADT_IN = 0x01,
			ADT_OUT = 0x02,
			ADT_DUPLEX = 0x03,
		};

		const char* audioDeviceTypeToString( AudioDeviceType type )
		{
			switch( type )
			{
			case ADT_IN:
				return "IN";
			case ADT_OUT:
				return "OUT";
			case ADT_DUPLEX:
				return "DUPLEX";
			}
			return "UNKNOWN";
		}

		size_t getSampleSizeFromFormat( AudioSampleFormat format )
		{
			switch( format )
			{
			case ASF_8BIT:
				return 1;
				break;
			case ASF_16BIT:
				return 2;
				break;
			case ASF_24BIT:
				return 3;
				break;
			case ASF_32BIT:
				return 4;
				break;
			case ASF_FLOAT:
				return 4;
				break;
			case ASF_DOUBLE:
				return 8;
				break;
			}

			throw std::runtime_error( "invalid device format" );
		}

		size_t getSampleBitsFromFormat( AudioSampleFormat format )
		{
			switch( format )
			{
			case ASF_8BIT: // 8-bit signed integer.
				return 8;
			case ASF_16BIT: // 16-bit signed integer.
				return 16;
			case ASF_24BIT: // 24-bit signed integer.
				return 24;
			case ASF_32BIT: // 32-bit signed integer.
				return 32;
			case ASF_FLOAT: // Normalized between plus/minus 1.0.
				return 32;
			case ASF_DOUBLE: // Normalized between plus/minus 1.0.
				return 64;
			}

			throw std::runtime_error( "invalid device format" );
		}

		size_t getSampleDepthFromFormat( AudioSampleFormat format )
		{
			switch( format )
			{
			case ASF_8BIT: // 8-bit signed integer.
				return std::numeric_limits<char>::max();
			case ASF_16BIT: // 16-bit signed integer.
				return std::numeric_limits<short>::max();
			case ASF_24BIT: // 24-bit signed integer.
				throw std::runtime_error( "audio format signed int 24 not implemented" );
			case ASF_32BIT: // 32-bit signed integer.
				return std::numeric_limits<int>::max();
			case ASF_FLOAT: // Normalized between plus/minus 1.0.
				return 1.0f;
			case ASF_DOUBLE: // Normalized between plus/minus 1.0.
				return 1.0;
			}

			throw std::runtime_error( "invalid device format" );
		}

		RtAudio::Api rtaApiFromApi( AudioAPI api )
		{
			switch( api )
			{
			case AA_UNSPECIFIED:
				return RtAudio::UNSPECIFIED;
			case AA_DUMMY:
				return RtAudio::RTAUDIO_DUMMY;
#if defined(_WIN32)
			case AA_WASAPI:
				return RtAudio::WINDOWS_WASAPI;
			case AA_ASIO:
				return RtAudio::WINDOWS_ASIO;
			case AA_DIRECT_SOUND:
				return RtAudio::WINDOWS_DS;
#elif __APPLE__
			case AA_CORE_AUDIO:
				return RtAudio::MACOSX_CORE;
#elif __linux__
			case AA_ALSA:
				return RtAudio::LINUX_ALSA;
			case AA_PULSE_AUDIO:
				return RtAudio::LINUX_PULSE;
			case AA_OSS:
				return RtAudio::LINUX_OSS;
#elif __unix__
			case AA_JACK_CLIENT:
				return RtAudio::UNIX_JACK;
#endif
			}

			std::cerr << "<warning> unknown Audio API, trying \"unspecified\"..." << std::endl;
			return RtAudio::UNSPECIFIED;
		}

		class AudioSourceImpl
		{
		private:
			struct AudioChunk
			{
				float* ptr;
				size_t size;
				uint32_t ts;
			};

			unsigned int maxQueueSize;
			std::list<AudioChunk> sampleBuffer;
			std::mutex sampleMutex;

			AudioAPI api;

			bool isStarted;

			//TODO: having this instance here probably won't work when we have multiple audio sources -- figure out and implement
			RtAudio audio;

			unsigned int	deviceID;

			unsigned int	channels;

			unsigned int	sampleRate;
			size_t			sampleSize;

			unsigned int	_bufferFrames;

			AudioDeviceType deviceType;
			AudioDeviceType capability;

			AudioSampleFormat sampleFormat;

			RtAudio::DeviceInfo info;

			static int audioCallback( void* outputBuffer, void* inputBuffer, unsigned int bufferFrames, double streamTime, RtAudioStreamStatus status, void* data )
			{
				if( status == RTAUDIO_INPUT_OVERFLOW )
					std::cerr << "<error> audio stream input overflow detected" << std::endl;
				else if( status == RTAUDIO_OUTPUT_UNDERFLOW )
					std::cerr << "<error> audio stream output underflow detected" << std::endl;
				else if( status )
					std::cerr << "<error> unknown stream error status" << std::endl;

				AudioSourceImpl* source = reinterpret_cast<AudioSourceImpl*>( data );

				bool success = false;

				switch( source->sampleFormat )
				{
				case ASF_8BIT: // 8-bit signed integer.
					success = source->onData<char>( (const char*) inputBuffer, bufferFrames, streamTime );
					break;
				case ASF_16BIT: // 16-bit signed integer.
					success = source->onData<short>( (const short*) inputBuffer, bufferFrames, streamTime );
					break;
				case ASF_24BIT: // 24-bit signed integer.
					throw std::runtime_error( "audio format signed int 24 not implemented" );
					break;
				case ASF_32BIT: // 32-bit signed integer.
					success = source->onData<int>( (const int*) inputBuffer, bufferFrames, streamTime );
					break;
				case ASF_FLOAT: // Normalized between plus/minus 1.0.
					success = source->onData<float>( (const float*) inputBuffer, bufferFrames, streamTime );
					break;
				case ASF_DOUBLE: // Normalized between plus/minus 1.0.
					success = source->onData<double>( (const double*) inputBuffer, bufferFrames, streamTime );
					break;
				default:
					throw std::runtime_error( "invalid device format" );
				}

				return ( success ? 0 : 1 );
			}

			template<typename T>
			bool onData( const T* inputBuffer, unsigned int bufferFrames, double streamTime )
			{
				if( !isStarted )
					return false;

				if( deviceType & ADT_IN )
				{
					AudioChunk ac;
					ac.size = bufferFrames * channels;
					ac.ptr = new float[ac.size];
					ac.ts = streamTime * 1e6;

					float s = 1.0f / getSampleDepthFromFormat( sampleFormat );
					for( int i = 0; i < ac.size; i++ )
						ac.ptr[i] = inputBuffer[i] * s;

					{
						std::lock_guard lock( sampleMutex );

						sampleBuffer.push_back( ac );

						while( sampleBuffer.size() > maxQueueSize )
						{
							safeDeleteArray( sampleBuffer.front().ptr );
							sampleBuffer.pop_front();
						}
					}
				}

				return true;
			}

			bool tryOpen()
			{
				closeAudio();

				if( deviceID >= audio.getDeviceCount() )
				{
					std::cerr << "<error> audio device #" << deviceID << " not present" << std::endl;
					return false;
				}

				RtAudioFormat format = 0;
				switch( sampleFormat )
				{
				case ASF_8BIT:
					format = RTAUDIO_SINT8;
					break;
				case ASF_16BIT:
					format = RTAUDIO_SINT16;
					break;
				case ASF_24BIT:
					format = RTAUDIO_SINT24;
					break;
				case ASF_32BIT:
					format = RTAUDIO_SINT32;
					break;
				case ASF_FLOAT:
					format = RTAUDIO_FLOAT32;
					break;
				case ASF_DOUBLE:
					format = RTAUDIO_FLOAT64;
					break;
				}

				if( !format )
				{
					std::cerr << "<error> format equivalent not found for " << audioSampleFormatToString( this->sampleFormat ) << std::endl;
					return false;
				}

				info = audio.getDeviceInfo( deviceID );

				if( info.inputChannels > 0 && this->info.outputChannels > 0 )
					capability = ADT_DUPLEX;
				else if( info.inputChannels > 0 )
					capability = ADT_IN;
				else if( info.outputChannels > 0 )
					capability = ADT_OUT;

				switch( deviceType )
				{
				case ADT_IN:
					if( !( capability & deviceType ) )
					{
						std::cerr << "<error> device " << this->info.name << " with type " << audioDeviceTypeToString( capability ) << " not suitable for device type " << audioDeviceTypeToString( deviceType ) << std::endl;
						return false;
					}
					break;
				case ADT_DUPLEX:
				case ADT_OUT:
					throw std::runtime_error( "audioSource must be of devicetype IN" );
				default:
					throw std::runtime_error( "invalid device type passed" );
				}

				//TODO: check if sample rate even supported

				//TODO: check if sample format even supported
				this->sampleSize = getSampleSizeFromFormat( this->sampleFormat );

				if( !this->sampleSize )
					throw std::runtime_error( "unknown samplesize" );

				std::cout << "creating audio input from device #" << deviceID << ", " << channels << " ch, " << audioSampleFormatToString( sampleFormat ) << ", sample rate " << sampleRate << ", sample size " << sampleSize * 8 << " bits" << std::endl;

				//TODO: check if channels even supported
				RtAudio::StreamParameters params;
				params.deviceId = deviceID;
				params.firstChannel = 0;
				params.nChannels = channels;

				try
				{
					RtAudio::StreamOptions opts;
					opts.flags = RTAUDIO_MINIMIZE_LATENCY;

					std::cout << "opening audio stream" << std::endl;

					unsigned int bufferFrames = _bufferFrames;
					audio.openStream( nullptr, &params, format, sampleRate, &bufferFrames, &audioCallback, (void*) this, &opts );

					if( !audio.isStreamOpen() )
						throw std::runtime_error( "failed to open audio stream" );

					if( opts.flags )
						std::cout << "opened audio stream with flags: "
						<< ( opts.flags & RTAUDIO_NONINTERLEAVED ? "noninterlaced; " : "" )
						<< ( opts.flags & RTAUDIO_MINIMIZE_LATENCY ? "minimize latency; " : "" )
						<< ( opts.flags & RTAUDIO_HOG_DEVICE ? "exclusive mode; " : "" )
						<< ( opts.flags & RTAUDIO_SCHEDULE_REALTIME ? "realtime scheluling; " : "" )
						<< ( opts.flags & RTAUDIO_ALSA_USE_DEFAULT ? "default ALSA PCM device; " : "" )
						<< std::endl;
					else
						std::cout << "opened audio stream" << std::endl;

					std::cout << "buffer: " << bufferFrames << " frames" << std::endl;

					isStarted = true;

					audio.startStream();
					std::cout << "audio stream started" << std::endl;
				}
				catch( std::exception& e )
				{
					std::cerr << "<error> failed creating audio input device: \"" << e.what() << "\"" << std::endl;
					return false;
				}

				std::cout << "successfully initialized audio device #" << deviceID << std::endl;

				return true;
			}

			void closeAudio()
			{
				if( audio.isStreamOpen() )
					audio.closeStream();
				isStarted = false;
			}

		public:
			AudioSourceImpl( AudioAPI api, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int bufferFrames, AudioSampleFormat sampleFormat, unsigned int maxQueueSize ) :
				maxQueueSize( maxQueueSize ),
				api( api ),
				isStarted( false ),
				audio( rtaApiFromApi( api ) ),
				deviceID( deviceID ),
				channels( channels ),
				sampleRate( sampleRate ),
				sampleSize( 0 ),
				_bufferFrames( bufferFrames ),
				deviceType( ADT_IN ),
				capability( ADT_UNKNOWN ),
				sampleFormat( sampleFormat )
			{
				audio.showWarnings( true );

				// Create an api map.
				std::map<int, std::string> apiMap;
				apiMap[RtAudio::UNSPECIFIED] = "UNSPECIFIED";
				apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
				apiMap[RtAudio::LINUX_PULSE] = "Linux PulseAudio";
				apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
				apiMap[RtAudio::UNIX_JACK] = "Jack Client";
				apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
				apiMap[RtAudio::WINDOWS_WASAPI] = "Windows WASAPI";
				apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
				apiMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
				apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

				std::vector<RtAudio::Api> apis;
				RtAudio::getCompiledApi( apis );

				std::cout << "-----------------  RTAUDIO CAPS  ------------------------" << std::endl;

				std::cout << "RtAudio Version " << RtAudio::getVersion() << std::endl;

				std::cout << "Compiled APIs:" << std::endl;
				for( int i = 0; i < apis.size(); i++ )
					std::cout << "  " << apiMap[apis[i]] << std::endl;

				RtAudio::DeviceInfo info;

				//TODO: provide possibility to set API (had problems with ASIO where i couldn't create or even probe 
				// an audio device -- specifically this was named "Blackmagic Audio" and the only possibility was to 
				// uninstall ASIO since I couldn't find a possibility in Windows to switch from ASIO to DirectSound)
				std::cout << "Current API: " << apiMap[audio.getCurrentApi()] << std::endl;

				unsigned int devices = audio.getDeviceCount();
				std::cout << "Found " << devices << " device(s) ..." << std::endl;

				for( unsigned int i = 0; i < devices; i++ )
				{
					info = audio.getDeviceInfo( i );

					std::cout << "Device #" << i << std::endl;
					std::cout << "   Device Name = " << info.name << std::endl;
					if( info.probed == false )
						std::cerr << "<warning>   Probe Status = UNsuccessful for " << info.name << std::endl;
					else
					{
						if( info.isDefaultOutput )
							std::cout << "   This is the default output device" << std::endl;
						else
							std::cout << "   This is NOT the default output device" << std::endl;

						if( info.isDefaultInput )
							std::cout << "   This is the default input device." << std::endl;
						else
							std::cout << "   This is NOT the default input device." << std::endl;

						std::cout << "   Probe Status = Successful" << std::endl;
						std::cout << "   Output Channels = " << info.outputChannels << std::endl;
						std::cout << "   Input Channels = " << info.inputChannels << std::endl;
						std::cout << "   Duplex Channels = " << info.duplexChannels << std::endl;

						if( info.nativeFormats == 0 )
							std::cerr << "<warning>   No natively supported data formats for " << info.name << "!" << std::endl;
						else
						{
							std::cout << "   Natively supported data formats:" << std::endl;
							if( info.nativeFormats & RTAUDIO_SINT8 )
								std::cout << "     8-bit int" << std::endl;
							if( info.nativeFormats & RTAUDIO_SINT16 )
								std::cout << "     16-bit int" << std::endl;
							if( info.nativeFormats & RTAUDIO_SINT24 )
								std::cout << "     24-bit int" << std::endl;
							if( info.nativeFormats & RTAUDIO_SINT32 )
								std::cout << "     32-bit int" << std::endl;
							if( info.nativeFormats & RTAUDIO_FLOAT32 )
								std::cout << "     32-bit float" << std::endl;
							if( info.nativeFormats & RTAUDIO_FLOAT64 )
								std::cout << "     64-bit float" << std::endl;
						}

						if( info.sampleRates.size() < 1 )
							std::cerr << "<error>   No supported sample rates found for " << info.name << "!" << std::endl;
						else
						{
							std::stringstream sstr;
							for( int j = 0; j < info.sampleRates.size(); j++ )
								sstr << info.sampleRates[j] << " ";

							std::cout << "   Supported sample rates: " << sstr.str() << std::endl;
						}

						std::cout << "   Preferred sample rate: " << info.preferredSampleRate << std::endl;
					}
				}

				std::cout << "---------------------------------------------------------" << std::endl;
			}

			~AudioSourceImpl()
			{
				this->close();
			}

			bool run()
			{
				if( isStarted )
					return false;

				return tryOpen();
			}

			void close()
			{
				this->closeAudio();

				if( sampleBuffer.size() )
				{
					std::lock_guard lock( sampleMutex );

					for( auto& it : sampleBuffer )
						safeDeleteArray( it.ptr );
					sampleBuffer.clear();
				}
			}

			bool fetchFrames( std::vector<SampleFrame*>& frames )
			{
				if( frames.size() )
					std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory!" << std::endl;

				if( sampleBuffer.size() )
				{
					std::lock_guard lock( sampleMutex );
					for( auto& it : sampleBuffer )
					{
						frames.push_back( new SampleFrame( channels, it.size / channels, it.ptr, it.ts, 1 ) );
						safeDeleteArray( it.ptr );
					}

					sampleBuffer.clear();

					return true;
				}

				return false;
			}
		};

		AudioIn::AudioIn() :
			Op(),
			_deviceID( -1 ),
			_channels( 1 ),
			_sampleRate( 22050 ),
			_bufferFrames( 0 ),
			_impl( nullptr )
		{
		}

		AudioIn::~AudioIn()
		{
			safeDelete( _impl );
		}

		void AudioIn::createPins()
		{
			//TODO: change this to AudioFrame type
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool AudioIn::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _impl ? "stop" : "start" ) )
			{
				if( !_impl )
					start();
				else
					stop();
			}

			{
				ScopedImGuiDisable disable( _impl != nullptr );

				//TODO: do this properly, provide selection of compiled APIs, available devices, supported sample rates, etc.
				int i = _deviceID;
				if( ImGui::InputInt( "device", &i ) )
					_deviceID = i;

				i = _channels;
				if( ImGui::InputInt( "channels", &i ) )
					_channels = clamp<int>( i, 1, 2 );

				i = _bufferFrames;
				if( ImGui::InputInt( "bufferFrames", &i ) )
					_bufferFrames = max( i, 0 );

				i = _sampleRate;
				if( ImGui::InputInt( "sampleRate", &i ) )
					_sampleRate = i;
			}

			return true;
		}
#endif

		bool AudioIn::process()
		{
			if( _impl )
			{
				std::vector<SampleFrame*> frames;

				if( _impl->fetchFrames( frames ) )
				{
					if( frames.size() )
					{
						//TODO: buffer -- for now, we just use the most recent one
						SampleFrame* ret = frames.back()->transpose();

						drawFrame( ret );

						//TODO: change this to AudioFrame type
						pushOutput( "out", ret );
						ret = nullptr;

						for( int i = 0; i < frames.size(); i++ )
							safeDelete( frames[i] );
						frames.clear();
					}
				}
			}

			return false;
		}

		bool AudioIn::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			load<int>( j, "deviceID", _deviceID );
			load<int>( j, "channels", _channels );
			load<int>( j, "bufferFrames", _bufferFrames );
			load<int>( j, "sampleRate", _sampleRate );

			bool started = false;
			load<bool>( j, "started", started );
			if( started )
				start();

			return ret;
		}

		bool AudioIn::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "deviceID", _deviceID );
			save( j, "channels", _channels );
			save( j, "bufferFrames", _bufferFrames );
			save( j, "sampleRate", _sampleRate );

			save( j, "started", ( _impl ? true : false ) );

			return ret;
		}

		void AudioIn::start()
		{
			stop();

			AudioSampleFormat asf = ASF_FLOAT;
			_impl = new AudioSourceImpl( AA_DIRECT_SOUND, _deviceID, _channels, _sampleRate, _bufferFrames, asf, 256 );
			if( !_impl->run() )
			{
				std::cerr << "failed to start audio device #" << _deviceID << " with " << _channels << " channels, " << _sampleRate << " Hz, format: " << audioSampleFormatToString( asf ) << std::endl;
				stop();
			}
		}

		void AudioIn::stop()
		{
			safeDelete( _impl );
		}




		AudioPower::AudioPower() :
			Op()
		{}

		AudioPower::~AudioPower()
		{}

		bool AudioPower::process()
		{
			SampleFrame* sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame* ret = new SampleFrame( 1, sf->height(), sf->timeStamp(), sf->depth() );

				const float* inPtr = sf->values();
				float* outPtr = ret->values();
				int stride = sf->width() * sf->depth();

				for( int j = 0; j < sf->height(); j++ )
					for( int k = 0; k < sf->depth(); k++ )
					{
						float sum = 0;
						for( int i = 0; i < sf->width(); i++ )
						{
							float val = inPtr[j * stride + i * sf->depth() + k];
							sum += val * val;
						}
						outPtr[j * sf->depth() + k] = sum /= sf->height();
					}

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool AudioPower::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			return ret;
		}

		bool AudioPower::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			return ret;
		}
	}
}