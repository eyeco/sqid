/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "util.h"

#ifdef __COMPRESSION_SUPPORT
#  include <blobFrame.h>
#  include <compression/compressor.h>
#  include <compression/decompressor.h>
#endif

#include <app.h>
#include <fileIO/json.h>

#include <commonImGui.h>

namespace sqid
{
	namespace Util
	{
		DEFINE_OP_DESC( Time, "time", "/util",
			"609563ED-1007-4DF7-A53A-21D76454BCCB" );
		DEFINE_OP_DESC( TimeStamp, "timestamp", "/util",
			"36A26F0F-16FD-4B03-95D2-C2EF4946AFED");
		DEFINE_OP_DESC( Sync, "sync", "/util",
			"6637087B-12A4-486B-8052-4948EB98EF19" );
		DEFINE_OP_DESC( Sampler, "sampler", "/util",
			"690A3AE0-4074-4FB3-B038-2C258D98B442" );
		DEFINE_OP_DESC( Buffer, "buffer", "/util",
			"F11EFB21-3C05-47BF-8DC3-A3AAA6C87B5A" );
		//DEFINE_OP_DESC( StaticOffset, "staticOffset", "/util/deprecated",
		//	"F37E0A17-A01A-4799-B875-EEC42681BD20" );
		DEFINE_OP_DESC( Split, "split", "/util",
			"9FE922B0-DF53-4460-B78B-64586E06CB5D" );
		DEFINE_OP_DESC( Merge, "merge", "/util",
			"A040325F-6B57-4C16-97B4-A643796336DB" );
		DEFINE_OP_DESC( Crop, "crop", "/util",
			"680BD180-5D9C-4A88-9D93-8420E97B040D" );
		DEFINE_OP_DESC( Resize, "resize", "/util",
			"75D11FB7-8BCC-4984-B492-ECA730A16781" );
		DEFINE_OP_DESC( Join, "join", "/util",
			"63C90FA8-D7AF-41E2-9566-15DBD704A00E" );
		DEFINE_OP_DESC( Transpose, "transpose", "/util",
			"69D52D7C-2CE0-480A-B0AD-83D8DC8FA5B5" );
		DEFINE_OP_DESC( Flatten, "flatten", "/util",
			"C08E1CA9-402F-4B0C-B619-E9646624DBC1" );
		DEFINE_OP_DESC( Reshape, "reshape", "/util",
			"177C4CCC-5D3D-4B6A-AF57-B343F1AB7D6E" );
		DEFINE_OP_DESC( Flip, "flip", "/util",
			"78F966FE-8E2B-4E4E-A6D0-98F5A8DC76A6" );
		DEFINE_OP_DESC( OnOff, "onOff", "/util",
			"D398DBB1-60E8-4867-ACFB-0D0356890F44" );
		DEFINE_OP_DESC( FlipFlop, "flipFlop", "/util",
			"EF924231-C33C-40AF-AB05-2F4E286B2204" );
		DEFINE_OP_DESC( SampleAndHold, "s+h", "/util",
			"31D14E08-DB80-40F5-A1A7-B7B7F856A354" );
		DEFINE_OP_DESC( MaxPooling, "maxPooling", "/util",
			"353B47FF-78F5-45F3-8666-4732FAC08319");

#ifdef __COMPRESSION_SUPPORT
		DEFINE_OP_DESC( Compress, "compress", "/util/compression",
			"D06E3264-802C-4A19-880A-12C868CF5723" );
		DEFINE_OP_DESC( Decompress, "decompress", "/util/compression",
			"A41DD941-7116-4B6E-97D7-4B76C999DA87" );
#endif



		Time::Time() :
			Op(),
			_global( false ),
			_refTime( getAppTime() )
		{}

		Time::~Time()
		{}

		void Time::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Time::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "global", &_global );

			{
				ScopedImGuiDisable disable( _global );
				if( ImGui::Button( "reset" ) )
					_refTime = getAppTime();
			}

			return true;
		}
#endif

		bool Time::process()
		{
			float gt = getAppTime();
			SampleFrame *sf = new SampleFrame( 1, 1, gt * 1000.0f, 1 );
			sf->set( _global ? gt : gt - _refTime );

			drawFrame( sf );

			bool ret = pushOutput( "out", sf );
			safeDelete( sf );

			return false;
		}

		bool Time::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "global", _global );

			return ret;
		}

		bool Time::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "global", _global );

			return ret;
		}




		TimeStamp::TimeStamp() :
			Op()
		{}

		TimeStamp::~TimeStamp()
		{}

		bool TimeStamp::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				SampleFrame *ret = new SampleFrame( 1, 1, sf->timeStamp(), 1 );
				ret->set( sf->timeStamp() / 1000.0f );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );

				safeDelete( sf );
			}

			return inputPending( "in" );
		}




		Sync::Sync() :
			Op(),
			_lastA( nullptr ),
			_lastB( nullptr ),
			_mode( M_OR )
		{}

		Sync::~Sync()
		{
			safeDelete( _lastA );
			safeDelete( _lastB );
		}

		void Sync::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );

			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "b", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Sync::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			for( int i = 0; i < (int) Sync::M_COUNT; i++ )
				if( ImGui::RadioButton( Sync::modeToString( (Sync::Mode) i ), i == _mode ) )
					_mode = (Sync::Mode) i;

			return true;
		}
#endif

		bool Sync::process()
		{
			SampleFrame *a = nullptr;
			SampleFrame *b = nullptr;

			//TODO: provide other sync modes as well, this one should do it for starters....
			//get last in queue in input pin A
			do
			{
				safeDelete( a );
				a = fetchInput<SampleFrame>( "a" );
			} while( inputPending( "a" ) );
			//get last in queue in input pin B
			do
			{
				safeDelete( b );
				b = fetchInput<SampleFrame>( "b" );
			} while( inputPending( "b" ) );


			if( a )
			{
				safeDelete( _lastA );
				_lastA = a;
			}
			if( b )
			{
				safeDelete( _lastB );
				_lastB = b;
			}

			if( _mode == M_OR )
			{
				//if there was activity at any input
				if( a || b )
				{
					if( _lastA && _lastB )
					{
						//TODO: idk yet what a good visualization would be for this, maybe draw both side-by-side...
						drawFrame( _lastA );

						pushOutput( "a", _lastA );
						pushOutput( "b", _lastB );
					}
				}
			}
			else if( _mode == M_AND )
			{
				//if there was activity at both inputs
				if( a && b )
				{
					//TODO: idk yet what a good visualization would be for this, maybe draw both side-by-side...
					drawFrame( _lastA );

					pushOutput( "a", _lastA );
					pushOutput( "b", _lastB );
				}
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		bool Sync::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string str;
			if( load<std::string>( j, "mode", str ) )
				_mode = modeFromString( str );

			return ret;
		}

		bool Sync::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "mode", modeToString( _mode ) );

			return ret;
		}

		const char *Sync::modeToString( Sync::Mode mode )
		{
			switch( mode )
			{
			case M_OR:
				return "or";
			case M_AND:
				return "and";
			}
			return "UNKNOWN";
		}

		Sync::Mode Sync::modeFromString( const char *s )
		{
			if( !s )
				return Sync::M_COUNT;

			for( int i = 0; i < Reshape::M_COUNT; i++ )
				if( !_stricmp( s, modeToString( (Sync::Mode) i ) ) )
					return (Sync::Mode) i;

			return Sync::M_COUNT;
		}

		Sync::Mode Sync::modeFromString( const std::string &s )
		{
			return Sync::modeFromString( s.c_str() );
		}



		Sampler::Sampler() :
			Op(),
			_nx( 0 ), _ny( 0 ), _nz( 0 ),
			_useZ( false ),
			_normalized( true ),
			_x( 0 ), _y( 0 ), _z( 0 )
		{}

		Sampler::~Sampler()
		{}

#ifdef __SUPPORT_GUI
		bool Sampler::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "normalized", &_normalized );
			if( _normalized )
			{
				ImGui::SliderFloat( "x", &_nx, 0, 1, "%.2f" );
				ImGui::SliderFloat( "y", &_ny, 0, 1, "%.2f" );

				ImGui::Checkbox( "use z", &_useZ );
				{
					ScopedImGuiDisable disable( !_useZ );
					ImGui::SliderFloat( "z", &_nz, 0, 1, "%.2f" );
				}
			}
			else
			{
				ImGui::Checkbox( "use z", &_useZ );
				if( _useZ )
				{
					float f[3] = { _x, _y, _z };
					ImGui::InputFloat3( "x/y/z", f, "%.2f" );
					_x = f[0];
					_y = f[1];
					_z = f[2];
				}
				else
				{
					float f[2] = { _x, _y };
					ImGui::InputFloat2( "x/y", f, "%.2f" );
					_x = f[0];
					_y = f[1];
				}
			}

			return true;
		}
#endif

		bool Sampler::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				if( sf->width() && sf->height() && sf->depth() )
				{
					int x = (int) ( _normalized ? _nx * ( sf->width() - 1 ) : _x );
					int y = (int) ( _normalized ? _ny * ( sf->height() - 1 ) : _y );
					int z = 0;

					size_t count = 0;
					if( _useZ )
					{
						//get a single value at x/y/z
						count = 1;
						z = (int) ( _normalized ? _nz * ( sf->depth() - 1 ) : _z );
					}
					else
					{
						//standard behavior: get a vector at x/y
						count = sf->depth();
					}

					x = clamp<int>( x, 0, sf->width() - 1 );
					y = clamp<int>( y, 0, sf->height() - 1 );
					z = clamp<int>( z, 0, sf->depth() - 1 );


					SampleFrame *ret = new SampleFrame( 1, 1, sf->timeStamp(), count );
					const float *s0 = sf->values() + sf->depth() * ( x + y * sf->width() ) + z;
					float *d0 = ret->values();

					for( int i = 0; i < sf->depth(); i++ )
						*( d0++ ) = *( s0++ );

					drawFrame( ret );

					pushOutput( "out", ret );
					safeDelete( ret );
				}

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Sampler::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "nx", _nx );
			load<float>( j, "ny", _ny );
			load<float>( j, "nz", _nz );

			load<bool>( j, "useZ", _useZ );

			load<bool>( j, "normalized", _normalized );

			load<float>( j, "x", _x );
			load<float>( j, "y", _y );
			load<float>( j, "z", _z );

			return ret;
		}

		bool Sampler::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "nx", _nx );
			save( j, "ny", _ny );
			save( j, "nz", _nz );

			save<bool>( j, "useZ", _useZ );
			save<bool>( j, "normalized", _normalized );

			save( j, "x", _x );
			save( j, "y", _y );
			save( j, "z", _z );

			return ret;
		}







		Buffer::Buffer() :
			Op(),
			_size( 256 ),
			_buffer( nullptr )
		{}

		Buffer::~Buffer()
		{
			clear();
		}

		void Buffer::clear()
		{
			safeDelete( _buffer );
		}

		void Buffer::checkBuffer( const SampleFrame *sf )
		{
			if( !sf )
				return;

			if( _buffer && sf->height() != _buffer->height() )
				safeDelete( _buffer );

			if( !_buffer )
				_buffer = new SampleFrame( _size, sf->height(), 0, sf->depth() );
		}

#ifdef __SUPPORT_GUI
		bool Buffer::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int size = _size;
			if( ImGui::SliderInt( "size", &size, 1, 512 ) )
			{
				if( _size != size )
				{
					_size = size;
					clear();
				}
			}

			return true;
		}
#endif

		bool Buffer::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				checkBuffer( sf );

				SampleFrame *ret = new SampleFrame( _buffer->width(), _buffer->height(), sf->timeStamp(), _buffer->depth() );

				//TODO: add options to buffer along Y and Z
				size_t blockSize = ( _buffer->width() * _buffer->depth() < sf->width() * sf->depth() ? 0 : _buffer->width() * _buffer->depth() - sf->width() * sf->depth() ) * sizeof( float );
				if( blockSize )
				{
					size_t stride = _buffer->width() * _buffer->depth();
					size_t offset = sf->width() * sf->depth();

					float *dst = ret->values();
					const float *src = _buffer->values() + offset;

					for( int j = 0; j < sf->height(); j++, dst += stride, src += stride )
						memcpy( dst, src, blockSize );
				}

				blockSize = min<int>( sf->width() * sf->depth(), ret->width() * ret->depth() ) * sizeof( float );
				if( blockSize )
				{
					size_t dstStride = ret->width() * ret->depth();
					size_t srcStride = sf->width() * sf->depth();

					size_t offset = max<int>( 0, dstStride - srcStride );

					float *dst = ret->values() + offset;
					const float *src = sf->values();

					for( int j = 0; j < sf->height(); j++, dst += dstStride, src += srcStride )
						memcpy( dst, src, blockSize );
				}

				safeDelete( sf );
				safeDelete( _buffer );

				_buffer = ret;

				drawFrame( ret );

				pushOutput( "out", ret );
			}

			return inputPending( "in" );
		}

		bool Buffer::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<size_t>( j, "size", _size );
			clear();

			return ret;
		}

		bool Buffer::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "size", _size );

			return ret;
		}







		/*
		StaticOffset::StaticOffset() :
			Op(),
			//showOffset( false ),
			_clamp( false ),
			_offset( nullptr ),
			_lastInput( nullptr )
		{
		}

		StaticOffset::~StaticOffset()
		{
			this->clear();
		}

#ifdef __SUPPORT_GUI
		bool StaticOffset::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "clamp", &_clamp );

			if( ImGui::Button( "snapshot" ) )
				offsetFromSnapshot();
			ImGui::SameLine();
			if( ImGui::Button( "reset" ) )
				this->setOffset( nullptr );

			//int r = ( showOffset ? 1 : 0 );
			//ImGui::RadioButton( "data", &r, 0 );
			//ImGui::RadioButton( "offset", &r, 1 );
			//showOffset = ( r == 1 );

			return true;
		}
#endif

		bool StaticOffset::offsetFromSnapshot()
		{
			if( !_lastInput )
				return false;

			this->setOffset( _lastInput );
			return true;
		}

		void StaticOffset::setOffset( const SampleFrame *f )
		{
			if( f )
			{
				if( !_offset )
					_offset = new SampleFrame( *f );
				else
					_offset->set( f );
			}
			else
				safeDelete( _offset );
		}

		bool StaticOffset::clear()
		{
			safeDelete( _offset );
			safeDelete( _lastInput );

			return true;
		}

		bool StaticOffset::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				safeDelete( _lastInput );
				_lastInput = new SampleFrame( *sf );

				if( _offset )
				{
					if( _clamp )
						sf->sub( _offset )->clamp01();
					else
						sf->sub( _offset );
				}

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool StaticOffset::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "clamp", _clamp );

			SampleFrame *sf = nullptr;
			load<SampleFrame*>( j, "offset", sf );
			setOffset( sf );
			safeDelete( sf );

			return ret;
		}

		bool StaticOffset::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "clamp", _clamp );
			save( j, "offset", *_offset );

			return ret;
		}
		*/






		Split::Split() :
			Op()
		{}

		Split::~Split()
		{}

		void Split::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );

			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "x", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "y", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "z", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "w", this ) );
		}

		bool Split::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( sf->depth() == 1 )
				{
					drawFrame( sf );

					pushOutput( "x", sf );
					safeDelete( sf );
				}
				else
				{
					std::vector<cv::Mat> layers;
					cv::split( sf->mat(), layers );

					size_t size = layers.size();
					if( size > 4 )
					{
						std::cerr << "<warning> more than 4 channels not supported" << std::endl;
						size = 4;
					}

					static const char *outNames[4] = { "x", "y", "z", "w" };
					for( int i = 0; i < size; i++ )
					{
						SampleFrame *ret = new SampleFrame( layers[i], sf->timeStamp() );
						if( !i )
							drawFrame( ret );

						pushOutput( outNames[i], ret );
						safeDelete( ret );
					}

					safeDelete( sf );
				}
			}

			return inputPending( "in" );
		}

		bool Split::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			return ret;
		}

		bool Split::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			return ret;
		}





		Merge::Merge() :
			Op()
		{}

		Merge::~Merge()
		{}

		void Merge::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "x", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "y", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "z", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "w", this ) );

			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

		bool Merge::process()
		{
			static const char *inNames[4] = { "x", "y", "z", "w" };
			SampleFrame *sf[4] = { nullptr, nullptr, nullptr, nullptr };

			size_t w = 0;
			size_t h = 0;

			uint32_t ts = 0;

			size_t size = 0;
			for( int i = 0; i < 4; i++ )
			{
				sf[i] = fetchInput<SampleFrame>( inNames[i] );
				if( sf[i] )
				{
					w = sf[i]->width();
					h = sf[i]->height();

					ts = sf[i]->timeStamp();

					size = i + 1;
				}
			}

			std::stringstream sstr;
			bool invalid = false;
			for( int i = 0; i < 4; i++ )
				if( sf[i] )
				{
					if( sf[i]->depth() > 1 )
					{
						sstr << "<error> can only merge frames with depths of 1";
						invalid = true;
						break;
					}
					if( sf[i]->width() != w || sf[i]->height() != h )
					{
						sstr << "<error> input frames differ in size";
						invalid = true;
						break;
					}
				}

			if( invalid )
			{
				for( int i = 0; i < 4; i++ )
					if( sf[i] )
						safeDelete( sf[i] );
			
				throw std::runtime_error( sstr.str() );
			}

			if( size )
			{
				std::vector<cv::Mat> layers;

				for( int i = 0; i < size; i++ )
				{
					if( sf[i] )
						layers.push_back( sf[i]->mat() );
					else
						layers.push_back( cv::Mat::zeros( h, w, CV_MAKETYPE( CV_32F, 1 ) ) );
				}

				cv::Mat m;
				cv::merge( layers, m );

				SampleFrame *ret = new SampleFrame( m, ts );

				for( int i = 0; i < 4; i++ )
					if( sf[i] )
						safeDelete( sf[i] );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return (
				inputPending( inNames[0] ) ||
				inputPending( inNames[1] ) ||
				inputPending( inNames[2] ) ||
				inputPending( inNames[3] ) );
		}

		bool Merge::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			return ret;
		}

		bool Merge::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			return ret;
		}





		Crop::Crop() :
			Op(),
			_left( 0 ),
			_right( 1 ),
			_bottom( 1 ),
			_top( 0 ),
			_normalized( true ),
			_refWidth( 0 ),
			_refHeight( 0 )
		{}

		Crop::~Crop()
		{}

#ifdef __SUPPORT_GUI
		bool Crop::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "normalized", &_normalized );

			if( _normalized )
			{
				ImGui::SliderFloat( "left", &_left, 0, 1 );
				ImGui::SliderFloat( "right", &_right, 0, 1 );
				ImGui::SliderFloat( "top", &_top, 0, 1 );
				ImGui::SliderFloat( "bottom", &_bottom, 0, 1 );
			}
			else
			{
				ScopedImGuiDisable disable( _refWidth * _refHeight == 0 );

				int l = _left * _refWidth;
				int r = _right * _refWidth;
				int b = _bottom * _refHeight;
				int t = _top * _refHeight;

				ImGui::SliderInt( "left", &l, 0, _refWidth );
				ImGui::SliderInt( "right", &r, 0, _refWidth );
				ImGui::SliderInt( "top", &t, 0, _refHeight );
				ImGui::SliderInt( "bottom", &b, 0, _refHeight );

				if( _refWidth * _refHeight )
				{
					_left = (float) l / _refWidth;
					_right = (float) r / _refWidth;
					_bottom = (float) b / _refHeight;
					_top = (float) t / _refHeight;
				}
			}

			if( _left > _right )
				_left = _right;
			if( _top > _bottom )
				_top = _bottom;

			return true;
		}
#endif

		bool Crop::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				_refWidth = sf->width();
				_refHeight = sf->height();

				//int width = ( _right - _left ) * sf->width();
				//int height = ( _bottom - _top ) * sf->height();

				//int x = width - _right * sf->width();
				//int y = height - _bottom * sf->height();
				int x0 = _left * sf->width();
				int y0 = _top * sf->height();

				int x1 = _right * sf->width();
				int y1 = _bottom * sf->height();

				int width = x1 - x0;
				int height = y1 - y0;

				SampleFrame *ret = new SampleFrame( width, height, sf->timeStamp(), sf->depth() );
				ret->setCropped( sf, x0, y0, width, height );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Crop::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "left", _left );
			load<float>( j, "right", _right );
			load<float>( j, "bottom", _bottom );
			load<float>( j, "top", _top );
			load<bool>( j, "normalized", _normalized );

			return ret;
		}

		bool Crop::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "left", _left );
			save( j, "right", _right );
			save( j, "bottom", _bottom );
			save( j, "top", _top );
			save( j, "normalized", _normalized );

			return ret;
		}






		Resize::Resize() :
			Op(),
			_width( 1 ),
			_height( 1 ),
			_keepAspect( false ),
			_method( IM_CUBIC ),
			_warp( false )
		{}

		Resize::~Resize()
		{}

#ifdef __SUPPORT_GUI
		bool Resize::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "keepAspect", &_keepAspect );

			int v = _width;
			if( ImGui::SliderInt( "width", &v, 1, 64 ) )
				_width = v;

			{
				ScopedImGuiDisable disable( _keepAspect );

				v = _height;
				if( ImGui::SliderInt( "height", &v, 1, 64 ) )
					_height = v;
			}

			int m = _method;
			for( int i = 0; i < IM_COUNT; i++ )
				ImGui::RadioButton( interpolationMethodToString( (InterpolationMethod) i ), &m, i );
			_method = (InterpolationMethod) m;

			//TODO: figure out what effect this has -- didn't notice one
			//ImGui::Checkbox( "warp outliers", &_warp );

			return true;
		}
#endif

		bool Resize::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( _keepAspect )
					_height = sqid::max( 1, (int) ( _width * sf->height() / sf->width() ) );

				SampleFrame *ret = new SampleFrame( _width, _height, sf->timeStamp(), sf->depth() );

				ret->setResized( sf, _method, _warp );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Resize::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string str;
			if( load<std::string>( j, "method", str ) )
				_method = interpolationMethodFromString( str );
			load<unsigned int>( j, "width", _width );
			load<unsigned int>( j, "height", _height );
			load<bool>( j, "keepAspect", _keepAspect );
			load<bool>( j, "warp", _warp );

			return ret;
		}

		bool Resize::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "method", interpolationMethodToString( _method ) );
			save( j, "width", _width );
			save( j, "height", _height );
			save( j, "keepAspect", _keepAspect );
			save( j, "warp", _warp );

			return ret;
		}




		MaxPooling::MaxPooling() :
			Op(),
			_width( 1 ),
			_height( 1 )
		{}

		MaxPooling::~MaxPooling()
		{}

#ifdef __SUPPORT_GUI
		bool MaxPooling::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int v = _width;
			if( ImGui::SliderInt( "width", &v, 1, 64 ) )
				_width = v;


			v = _height;
			if( ImGui::SliderInt( "height", &v, 1, 64 ) )
				_height = v;

			return true;
		}
#endif

		bool MaxPooling::process()
		{
			SampleFrame* sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				int xFrameResolution = sf->width();
				int yFrameResolution = sf->height();

				int xResolution = ceil( xFrameResolution / (double) _width );
				int yResolution = ceil( yFrameResolution / (double) _height );

				SampleFrame* ret = new SampleFrame( xResolution, yResolution, sf->timeStamp(), sf->depth() );

				int xIndex = 0;
				int yIndex = 0;

				//TODO: this seems to not consider matrices' depth -- it only operates along X and Y
				// -> complete implementation
				for( int x = 0; x < sf->width(); x++ )
				{
					xIndex = x / _width;
					for( int y = 0; y < sf->height(); y++ )
					{
						yIndex = y / _height;
						ret->values()[yIndex * xResolution + xIndex] = x % _width == 0 && y % _height == 0 ?
							sf->values()[y * xFrameResolution + x] : max( ret->values()[yIndex * xResolution + xIndex], sf->values()[y * xFrameResolution + x] );
					}
				}

				safeDelete( sf );

				drawFrame( ret );
				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool MaxPooling::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "width", _width );
			load<unsigned int>( j, "height", _height );

			return ret;
		}

		bool MaxPooling::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "width", _width );
			save( j, "height", _height );

			return ret;
		}

		



		Join::Join() :
			Op(),
			_ver( false )
		{}

		Join::~Join()
		{}

		void Join::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );

			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Join::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "vertically", &_ver );

			return true;
		}
#endif

		bool Join::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				if( a && b && a->depth() != b->depth() )
				{
					safeDelete( a );
					safeDelete( b );

					throw std::runtime_error( "depths must be equal" );
				}

				unsigned int width = 0;
				unsigned int height = 0;
				unsigned int depth = ( a ? a->depth() : b->depth() );
				uint32_t ts = ( a ? a->timeStamp() : b->timeStamp() );

				if( _ver )
				{
					if( a && b && a->width() != b->width() )
					{
						safeDelete( a );
						safeDelete( b );

						throw std::runtime_error( "widths must be equal for joining vertically" );
					}

					width = ( a ? a->width() : b->width() );
					height = ( a ? a->height() : 0 ) + ( b ? b->height() : 0 );
				}
				else
				{
					if( a && b && a->height() != b->height() )
					{
						safeDelete( a );
						safeDelete( b );

						throw std::runtime_error( "heights must be equal for joining vertically" );
					}

					width = ( a ? a->width() : 0 ) + ( b ? b->width() : 0 );
					height = ( a ? a->height() : b->height() );
				}

				SampleFrame *sf = new SampleFrame( width, height, ts, depth );

				int offX = 0;
				int offY = 0;

				if( a )
				{
					sf->paste( a, offX, offY );
					if( _ver )
						offY += a->height();
					else
						offX += a->width();
				}
				if( b )
					sf->paste( b, offX, offY );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			safeDelete( a );
			safeDelete( b );

			return inputPending( "a" ) || inputPending( "b" );
		}

		bool Join::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "ver", _ver );

			return ret;
		}

		bool Join::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "ver", _ver );

			return ret;
		}






		Transpose::Transpose() :
			Op()
		{}

		Transpose::~Transpose()
		{}

		bool Transpose::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				sf->transpose();

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Transpose::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			return ret;
		}

		bool Transpose::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			return ret;
		}





		Flatten::Flatten() :
			Op()
		{}

		Flatten::~Flatten()
		{}

		bool Flatten::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame *ret = new SampleFrame( sf->size(), 1, sf->values(), sf->timeStamp() );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			safeDelete( sf );

			return inputPending( "in" );
		}





		Reshape::Reshape() :
			Op(),
			_autoMode( M_AUTO_X ),
			_x( 1 ), _y( 1 ), _z( 1 ),
			_autoX( 1 ), _autoY( 1 ), _autoZ( 1 )
		{}

		Reshape::~Reshape()
		{}

#ifdef __SUPPORT_GUI
		bool Reshape::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( _autoMode == M_AUTO_X )
			{
				ScopedImGuiDisable disable;
				int i = _autoX;
				ImGui::InputInt( "x", &i );
			}
			else
			{
				int i = _x;
				if( ImGui::InputInt( "x", &i ) )
					_x = std::max<int>( 1, i );
			}

			if( _autoMode == M_AUTO_Y )
			{
				ScopedImGuiDisable disable;
				int i = _autoY;
				ImGui::InputInt( "y", &i );
			}
			else
			{
				int i = _y;
				if( ImGui::InputInt( "y", &i ) )
					_y = std::max<int>( 1, i );
			}

			if( _autoMode == M_AUTO_Z )
			{
				ScopedImGuiDisable disable;
				int i = _autoZ;
				ImGui::InputInt( "z", &i );
			}
			else
			{
				int i = _z;
				if( ImGui::InputInt( "z", &i ) )
					_z = std::max<int>( 1, i );
			}

			for( int i = 0; i < (int)Reshape::M_COUNT; i++ )
				if( ImGui::RadioButton( Reshape::modeToString( (Reshape::Mode)i ), i == _autoMode ) )
					_autoMode = (Reshape::Mode)i;

			return true;
		}
#endif

		bool Reshape::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				int width = _x;
				int height = _y;
				int depth = _z;

				SampleFrame *ret = nullptr;

				if( width * height * depth )
				{
					switch( _autoMode )
					{
					case M_STRICT:
						break;
					case M_AUTO_X:
						width = ceil( sf->size() / (float)( height * depth ) );
						_autoX = width;
						break;
					case M_AUTO_Y:
						height = ceil( sf->size() / (float) ( width * depth ) );
						_autoY = height;
						break;
					case M_AUTO_Z:
						depth = ceil( sf->size() / (float) ( width * height ) );
						_autoZ = depth;
						break;
					}

					ret = new SampleFrame( width, height, sf->timeStamp(), depth );

					if( sf->size() && ret->size() )
						memcpy( ret->values(), sf->values(), std::min( ret->size(), sf->size() ) * sizeof( float ) );
				}
				else
					ret = new SampleFrame();


				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			safeDelete( sf );

			return inputPending( "in" );
		}

		bool Reshape::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<uint32_t>( j, "x", _x );
			load<uint32_t>( j, "y", _y );
			load<uint32_t>( j, "z", _z );

			return ret;
		}

		bool Reshape::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "x", _x );
			save( j, "y", _y );
			save( j, "z", _z );

			return ret;
		}

		const char *Reshape::modeToString( Reshape::Mode mode )
		{
			switch( mode )
			{
			case M_STRICT:
				return "strict";
			case M_AUTO_X:
				return "autoX";
			case M_AUTO_Y:
				return "autoY";
			case M_AUTO_Z:
				return "autoZ";
			}
			return "UNKNOWN";
		}

		Reshape::Mode Reshape::modeFromString( const char *s )
		{
			if( !s )
				return Reshape::M_COUNT;

			for( int i = 0; i < Reshape::M_COUNT; i++ )
				if( !_stricmp( s, modeToString( (Reshape::Mode) i ) ) )
					return (Reshape::Mode) i;

			return Reshape::M_COUNT;
		}

		Reshape::Mode Reshape::modeFromString( const std::string &s )
		{
			return Reshape::modeFromString( s.c_str() );
		}




		Flip::Flip() :
			Op(),
			_horizontally( false ),
			_vertically( false )
		{}

		Flip::~Flip()
		{}

#ifdef __SUPPORT_GUI
		bool Flip::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "hor", &_horizontally );
			ImGui::Checkbox( "ver", &_vertically );

			return true;
		}
#endif

		bool Flip::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );
				ret->setFlipped( sf, _horizontally, _vertically );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Flip::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "hor", _horizontally );
			load<bool>( j, "ver", _vertically );

			return ret;
		}

		bool Flip::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "hor", _horizontally );
			save( j, "ver", _vertically );

			return ret;
		}



		
	


		OnOff::OnOff( float threshold ) :
			Op(),
			_hysteresis( false ),
			_threshold( threshold ),
			_lowerThreshold( threshold ),
			_continuous( true ),
			_tempFrame( nullptr )
		{}

		OnOff::~OnOff()
		{
			safeDelete( _tempFrame );
		}

		void OnOff::clear()
		{
			safeDelete( _tempFrame );
		}

#ifdef __SUPPORT_GUI
		bool OnOff::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Checkbox( "hysteresis", &_hysteresis ) )
				clear();

			if( _hysteresis )
			{
				ImGui::SliderFloat( "on", &_threshold, 0.0f, 1.0f );
				ImGui::SliderFloat( "off", &_lowerThreshold, 0.0f, 1.0f );

				_lowerThreshold = std::min( _lowerThreshold, _threshold );
			}
			else
			{
				ImGui::SliderFloat( "threshold", &_threshold, 0.0f, 1.0f );
			}

			if( ImGui::Checkbox( "continuous", &_continuous ) )
				clear();

			return true;
		}
#endif

		bool OnOff::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				bool sendFrame = _continuous;
				SampleFrame *ret = nullptr;

				//TODO: take care of multi-channel frames
				if( sf->depth() != 1 )
				{
					safeDelete( sf );
					throw std::runtime_error( "OnOff: multichannel frames not yet supported" );
				}

				if( _hysteresis )
				{
					if( _tempFrame && !dimensionsCompatible( _tempFrame, sf ) )
						clear();

					//ret->threshold( this->threshold );
					//ret->threshold
					if( _tempFrame )
					{
						ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );

						const float *src = sf->values();
						const float *prev = _tempFrame->values();
						float *dst = ret->values();

						size_t size = sf->width() * sf->height();
						for( int i = 0; i < size; i++ )
						{
							if( src[i] > _threshold )
								dst[i] = 1.0f;
							else if( src[i] < _lowerThreshold )
								dst[i] = 0.0f;
							else
								dst[i] = prev[i];

							if( !_continuous )
								sendFrame = ( dst[i] != prev[i] );
						}
					}
					else
					{
						_tempFrame = new SampleFrame( *sf );

						sendFrame = true;
						ret = new SampleFrame( *sf );
						ret->threshold( _threshold );
					}

					_tempFrame->set( ret );
				}
				else
				{
					ret = new SampleFrame( *sf );
					ret->threshold( _threshold );

					if( _tempFrame && !dimensionsCompatible( _tempFrame, ret ) )
						clear();

					if( !_continuous )
					{
						if( _tempFrame )
						{
							const float *ptr0 = _tempFrame->values();
							const float *ptr1 = ret->values();

							size_t size = sf->width() * sf->height();
							for( int i = 0; i < size; i++ )
								if( *( ptr0++ ) != *( ptr1++ ) )
								{
									sendFrame = true;
									break;
								}

							_tempFrame->set( ret );
						}
						else
						{
							sendFrame = true;
							_tempFrame = new SampleFrame( *ret );
						}
					}
				}

				safeDelete( sf );

				if( sendFrame )
				{
					drawFrame( ret );
					pushOutput( "out", ret );
				}
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool OnOff::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "hysteresis", _hysteresis );
			load<float>( j, "threshold", _threshold );
			load<float>( j, "lowerThreshold", _lowerThreshold );
			load<bool>( j, "continuous", _continuous );

			return ret;
		}

		bool OnOff::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "hysteresis", _hysteresis );
			save( j, "threshold", _threshold );
			save( j, "lowerThreshold", _lowerThreshold );
			save( j, "continuous", _continuous );

			return ret;
		}



		FlipFlop::FlipFlop( float threshold ) :
			Op(),
			_threshold( threshold ),
			_risingEdge( true ),
			_tempFrame( nullptr ),
			_prevFrame( nullptr )
		{}

		FlipFlop::~FlipFlop()
		{
			safeDelete( _tempFrame );
			safeDelete( _prevFrame );
		}

#ifdef __SUPPORT_GUI
		bool FlipFlop::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "on rise", &_risingEdge );
			ImGui::SliderFloat( "threshold", &_threshold, 0.0f, 1.0f );

			return true;
		}
#endif

		bool FlipFlop::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				//TODO: take care of multi-channel frames
				if( sf->depth() != 1 )
				{
					safeDelete( sf );
					throw std::runtime_error( "OnOff: multichannel frames not yet supported" );
				}

				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );

				if( _tempFrame && !dimensionsCompatible( _tempFrame, ret ) )
					safeDelete( _tempFrame );

				if( _tempFrame && _prevFrame )
				{
					const float *src0 = _tempFrame->values();
					const float *src1 = sf->values();
					const float *src2 = _prevFrame->values();

					float *dst = ret->values();

					size_t size = sf->width() * sf->height();
					if( _risingEdge )
					{
						for( int i = 0; i < size; i++ )
							if( src0[i] < _threshold && src1[i] >= _threshold )
								dst[i] = 1.0f - src2[i];
							else
								dst[i] = src2[i];
					}
					else
					{
						for( int i = 0; i < size; i++ )
							if( src0[i] > _threshold && src1[i] <= _threshold )
								dst[i] = 1.0f - src2[i];
							else
								dst[i] = src2[i];
					}
				}

				if( !_tempFrame )
					_tempFrame = new SampleFrame( *sf );
				else
					_tempFrame->set( sf );

				if( !_prevFrame )
					_prevFrame = new SampleFrame( *ret );
				else
					_prevFrame->set( ret );

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool FlipFlop::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "threshold", _threshold );

			return ret;
		}

		bool FlipFlop::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "threshold", _threshold );

			return ret;
		}






		const char *SampleAndHold::modeToString( SampleAndHold::Mode mode )
		{
			switch( mode )
			{
			case M_SYS_TIME:
				return "sysTime";
			case M_FRAME_TIME:
				return "frameTime";
			case M_COUNTDOWN:
				return "countdown";
			case M_TRIGGER:
				return "trigger";
			}
			return "UNKNOWN";
		}

		SampleAndHold::Mode SampleAndHold::modeFromString( const char *s )
		{
			if( !s )
				return SampleAndHold::M_COUNT;

			for( int i = 0; i < SampleAndHold::M_COUNT; i++ )
				if( !_stricmp( s, modeToString( ( SampleAndHold::Mode )i ) ) )
					return ( SampleAndHold::Mode )i;

			return SampleAndHold::M_COUNT;
		}

		SampleAndHold::Mode SampleAndHold::modeFromString( const std::string &s )
		{
			return modeFromString( s.c_str() );
		}

		SampleAndHold::SampleAndHold( Mode mode ) :
			Op(),
			_mode( mode ),
			_frames( 33 ),
			_period( 1.0f ),
			_cntr( 0 ),
			_t0( 0 ),
			_continuous( false ),
			_updateOut( false ),
			_lastInput( nullptr ),
			_snapshot( nullptr )
		{}

		SampleAndHold::~SampleAndHold()
		{
			safeDelete( _lastInput );
			safeDelete( _snapshot );
		}

#ifdef __SUPPORT_GUI
		bool SampleAndHold::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int m = _mode;
			for( int i = 0; i < M_COUNT; i++ )
				ImGui::RadioButton( modeToString( (Mode) i ), &m, i );
			if( _mode != m )
			{
				_mode = (Mode) m;

				if( _mode == M_SYS_TIME || _mode == M_FRAME_TIME || _mode == M_COUNTDOWN )
					takeSnapshot();
			}

			ImGui::Checkbox( "continuous", &_continuous );

			switch( _mode )
			{
			case M_SYS_TIME:
			case M_FRAME_TIME:
				ImGui::SliderFloat( "period", &_period, 0.0f, 10.0f );
				break;
			case M_COUNTDOWN:
				ImGui::SliderInt( "frames", &_frames, 1, 100 );
				break;
			case M_TRIGGER:
				if( ImGui::Button( "snapshot" ) )
					takeSnapshot();
				break;
			}

			return true;
		}
#endif

		bool SampleAndHold::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			safeDelete( _lastInput );
			_lastInput = sf;

			switch( _mode )
			{
			case M_SYS_TIME:
				if( getAppTime() - _t0 > _period )
					takeSnapshot();
				break;
			case M_FRAME_TIME:
				if( sf && sf->timeStamp() * 0.001f - _t0 > _period )
					takeSnapshot();
				break;
			case M_COUNTDOWN:
				if( ++_cntr >= _frames )
					takeSnapshot();
				break;
			case M_TRIGGER:
				break;
			}

			if( _snapshot )
			{
				if( _continuous || _updateOut )
				{
					SampleFrame *cpy = new SampleFrame( _snapshot->mat(), getAppTime() * 1000 );

					drawFrame( cpy );
					pushOutput( "out", cpy );

					safeDelete( cpy );

					_updateOut = false;
				}
			}

			return inputPending( "in" );
		}

		void SampleAndHold::takeSnapshot()
		{
			switch( _mode )
			{
			case M_SYS_TIME:
				_t0 = getAppTime();
				break;
			case M_FRAME_TIME:
				if( _lastInput )
					_t0 = _lastInput->timeStamp() * 0.001f;
				else
					_t0 = 0.0f;
				break;
			case M_COUNTDOWN:
				_cntr = 0;
				break;
			case M_TRIGGER:
				break;
			}

			if( !_lastInput )	//no input since (or yet)
				return;

			safeDelete( _snapshot );
			_snapshot = _lastInput;
			_lastInput = nullptr;
			_updateOut = true;
		}

		bool SampleAndHold::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<int>( j, "frames", _frames );
			load<float>( j, "period", _period );
			std::string str;
			if( load<std::string>( j, "mode", str ) )
				_mode = modeFromString( str );
			load<bool>( j, "continuous", _continuous );
			load<SampleFrame*>( j, "snapshot", _snapshot );

			if( _mode != M_TRIGGER )
				takeSnapshot();

			if( _snapshot )
				_updateOut = true;

			return ret;
		}

		bool SampleAndHold::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "frames", _frames );
			save( j, "period", _period );
			save( j, "mode", modeToString( _mode ) );
			save( j, "continuous", _continuous );
			save( j, "snapshot", *_snapshot );

			return ret;
		}





#ifdef __COMPRESSION_SUPPORT
		namespace Internal
		{
			std::vector<const char*> createCompressionAlgorithmsComboItems()
			{
				std::vector<const char*> items;
				for( int i = 0; i < CA_COUNT; i++ )
					items.push_back( compressionAlgorithmToString( (CompressionAlgorithm) i ) );
				return items;
			}

			const std::vector<const char*> &getCompressionAlgorithmsComboItems()
			{
				static std::vector<const char*> comboItems = createCompressionAlgorithmsComboItems();
				return comboItems;
			}
		}




		Compress::Compress() :
			Op(),
			_compressor( nullptr ),
			_algorithm( CA_NULL )
		{}

		Compress::~Compress()
		{
			safeDelete( _compressor );
		}

		void Compress::update()
		{
			if( !_compressor || _compressor->getAlgorithm() != _algorithm )
				safeDelete( _compressor );

			if( !_compressor )
			{
				switch( _algorithm )
				{
				case CA_NULL:
					_compressor = new CompressorDummy();
					break;
				case CA_RLE:
					_compressor = new CompressorRLE();
					break;
#ifdef __COMPRESSION_SUPPORT_LZO
				case CA_LZO:
					_compressor = new CompressorLZO();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_QLZ
				case CA_QLZ:
					_compressor = new CompressorQLZ();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_BZ2
				case CA_BZ2:
					_compressor = new CompressorBZ2();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_ZSTD
				case CA_ZSTD:
					_compressor = new CompressorZStd();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_ZLIB
				case CA_ZLIB:
					_compressor = new CompressorZLib();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_LZ4
				case CA_LZ4:
					_compressor = new CompressorLZ4();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_JPEG
				case CA_JPEG:
					//TODO: interface not compatible, have to use ImageCompressor base class
					break;
#endif
				}

				if( !_compressor )
					std::cerr << "<error> compressor type " << compressionAlgorithmToString( _algorithm ) << " not available" << std::endl;
				else
					std::cout << "created " << compressionAlgorithmToString( _algorithm ) << " compressor" << std::endl;
			}
		}

		bool Compress::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				drawFrame( sf );
				
				if( _compressor )
				{
					size_t inSize = sf->size() * sizeof( float );
					size_t compSize = 0;

					CompressedSampleFrame *csf = nullptr;
					if( inSize )
					{
						const unsigned char *inData = reinterpret_cast<const unsigned char*>( sf->values() );

						size_t safeSize = _compressor->toSafeSize( inSize );
						if( _buffer.size() < safeSize ) //assume worst case
							_buffer.resize( safeSize );
						compSize = _compressor->compress( inData, inSize, &_buffer[0], _buffer.size() );

						if( !compSize )
						{
							std::cerr << "<error> compressing frame failed" << std::endl;
							setEnabled( false );
						}
						else if( compSize < inSize )
							csf = new CompressedSampleFrame( &_buffer[0], compSize, _algorithm, sf->width(), sf->height(), sf->depth(), sf->timeStamp() );
						else if( compSize == inSize )
						{
							//	cbf = new CompressedBlobFrame( inData, CA_NULL, inSize, sf->timeStamp() );
						}
						else
						{
							std::cerr << "<warning> compressing with " << compressionAlgorithmToString( _algorithm ) << " would result in extra data (" << compSize << " from " << inSize << ")" << std::endl;
							//	cbf = new CompressedBlobFrame( inData, CA_NULL, inSize, sf->timeStamp() );
						}
					}

					if( csf )
						pushOutput( "out", csf );
					safeDelete( csf );
				}
			}

			safeDelete( sf );

			return inputPending( "in" );
		}

		void Compress::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
			addOutlet( new OutletPin( new DataContainer<CompressedSampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Compress::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			auto items = Internal::getCompressionAlgorithmsComboItems();
			int index = (int)_algorithm;

			const char* currentItem = items[index];
			if( ImGui::BeginCombo( "##combo", currentItem, ImGuiComboFlags_None ) )
			{
				for( int i = 0; i < items.size(); i++ )
				{
					bool isSelected = ( currentItem == items[i] );
					if( ImGui::Selectable( items[i], isSelected ) )
					{
						currentItem = items[i];
						_algorithm = (CompressionAlgorithm) i;

						update();
					}
					if( isSelected )
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			return true;
		}
#endif

		bool Compress::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string str;
			if( load<std::string>( j, "algorithm", str ) )
				_algorithm = compressionAlgorithmFromString( str );

			update();

			return ret;
		}

		bool Compress::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "algorithm", compressionAlgorithmToString( _algorithm ) );

			return ret;
		}




		Decompress::Decompress() :
			Op(),
			_decompressor( nullptr ),
			_algorithm( CA_NULL )
		{}

		Decompress::~Decompress()
		{
			safeDelete( _decompressor );
		}

		void Decompress::update()
		{
			if( !_decompressor || _decompressor->getAlgorithm() != _algorithm )
				safeDelete( _decompressor );

			if( !_decompressor )
			{
				switch( _algorithm )
				{
				case CA_NULL:
					_decompressor = new DecompressorDummy();
					break;
				case CA_RLE:
					_decompressor = new DecompressorRLE();
					break;
#ifdef __COMPRESSION_SUPPORT_LZO
				case CA_LZO:
					_decompressor = new DecompressorLZO();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_QLZ
				case CA_QLZ:
					_decompressor = new DecompressorQLZ();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_BZ2
				case CA_BZ2:
					_decompressor = new DecompressorBZ2();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_ZSTD
				case CA_ZSTD:
					_decompressor = new DecompressorZStd();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_ZLIB
				case CA_ZLIB:
					_decompressor = new DecompressorZLib();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_LZ4
				case CA_LZ4:
					_decompressor = new DecompressorLZ4();
					break;
#endif
#ifdef __COMPRESSION_SUPPORT_JPEG
				case CA_JPEG:
					//TODO: interface not compatible, have to use ImageDecompressor base class
					break;
#endif
				}

				if( !_decompressor )
					std::cerr << "<error> decompressor type " << compressionAlgorithmToString( _algorithm ) << " not available" << std::endl;
				else
					std::cout << "created " << compressionAlgorithmToString( _algorithm ) << " decompressor" << std::endl;
			}
		}

		bool Decompress::process()
		{
			CompressedSampleFrame *csf = fetchInput<CompressedSampleFrame>( "in" );

			if( csf )
			{
				//TODO: for decompression and reconstruction of a SF, we actually need to send a header (width x height x depth, maybe consider also sending ts)
				SampleFrame *sf = nullptr;

				if( _decompressor )
				{
					size_t expectedSize = csf->frameSize() * sizeof( float );
					size_t outSize = 0;

					if( _buffer.size() < expectedSize )
						_buffer.resize( expectedSize );

					if( csf->data() )
					{
						if( csf->algorithm() == _decompressor->getAlgorithm() )
							outSize = _decompressor->decompress( csf->data(), csf->bytes(), &_buffer[0], _buffer.size() );
						else if( csf->algorithm() == CA_NULL )
						{
							//TODO: non-compressed block, just copy
							//outSize = cbf->size();
						}
						else
							std::cerr << "<warning> blob frame compression algorithm does not match compressor (" << compressionAlgorithmToString( csf->algorithm() ) << " vs. " << compressionAlgorithmToString( _decompressor->getAlgorithm() ) << "), skipping frame..." << std::endl;

						if( outSize != expectedSize )
							std::cerr << "<warning> got " << outSize << " bytes, expected " << expectedSize << ", skipping frame..." << std::endl;
						else
							sf = new SampleFrame( csf->width(), csf->height(), reinterpret_cast<const float*>( &_buffer[0] ), csf->timeStamp(), csf->depth() );
					}
				}

				if( sf )
				{
					drawFrame( sf );
					pushOutput( "out", sf );

					safeDelete( sf );
				}
			}

			safeDelete( csf );

			return inputPending( "in" );
		}

		void Decompress::createPins()
		{
			addInlet( new InletPin( new DataContainer<CompressedSampleFrame>(), "in", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Decompress::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			auto items = Internal::getCompressionAlgorithmsComboItems();
			int index = (int) _algorithm;

			const char* currentItem = items[index];
			if( ImGui::BeginCombo( "##combo", currentItem, ImGuiComboFlags_None ) )
			{
				for( int i = 0; i < items.size(); i++ )
				{
					bool isSelected = ( currentItem == items[i] );
					if( ImGui::Selectable( items[i], isSelected ) )
					{
						currentItem = items[i];
						_algorithm = (CompressionAlgorithm) i;

						update();
					}
					if( isSelected )
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			return true;
		}
#endif

		bool Decompress::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string str;
			if( load<std::string>( j, "algorithm", str ) )
				_algorithm = compressionAlgorithmFromString( str );

			update();

			return ret;
		}

		bool Decompress::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "algorithm", compressionAlgorithmToString( _algorithm ) );

			return ret;
		}
#endif
	}
}