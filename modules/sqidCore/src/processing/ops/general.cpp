/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#include "general.h"

#include <fileIO/json.h>

#include <app.h>
#include <processing/pin.h>

#include "../../perlin.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <commonImGui.h>

//#include <drawing/frameDrawer.h>
#include "../../drawing/frameDrawerGeneral.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>


namespace sqid
{
	namespace General
	{
		DEFINE_OP_DESC( NOP, "nop", "",
			"4FD4F399-4542-4B80-8A21-B31C49DA3F09" );
		DEFINE_OP_DESC( Const, "const", "/gnr8",
			"96C3549D-69BE-4F8B-B83A-A2923DB8D09F" );
		DEFINE_OP_DESC( Noise, "noise", "/gnr8",
			"97413F38-28AC-4630-8E97-58B313B045DD" );
		DEFINE_OP_DESC( Signal, "signal", "/gnr8",
			"36B9684D-839A-4C5C-980A-8DCB78870423" );
		DEFINE_OP_DESC( ContourDetector, "contourDetector", "/imaging",
			"F35A45D0-184E-47D2-BEEC-87BD4E00ACA4" );



		NOP::NOP() :
			Op()
		{}

		NOP::~NOP()
		{}

		bool NOP::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				pushOutput( "out", sf );

				drawFrame( sf );

				safeDelete( sf );
			}

			return inputPending( "in" );
		}








		const char *Const::constTypeToString( Const::ConstType type )
		{
			switch( type )
			{
			case CT_IDENTITY:
				return "identity";
			case CT_FILLED:
				return "filled";
			}
			return "UNKNOWN";
		}

		Const::ConstType Const::constTypeFromString( const char *s )
		{
			if( !s )
				return CT_COUNT;

			for( int i = 0; i < CT_COUNT; i++ )
				if( !_stricmp( s, constTypeToString( (ConstType) i ) ) )
					return (ConstType) i;

			return CT_COUNT;
		}

		Const::ConstType Const::constTypeFromString( const std::string &s )
		{
			return constTypeFromString( s.c_str() );
		}

		Const::Const( ConstType type ) :
			Op(),
			_width( 1 ),
			_height( 1 ),
			_depth( 1 ),
			_uniform( true ),
			_value( 0.0f ),
			_type( type )
		{
		}

		Const::~Const()
		{
		}

		void Const::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Const::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int t = 0;

			t = _width;
			if( ImGui::InputInt( "width", &t ) )
				_width = clamp<int>( t, 1, 0x01 << 13 );
			t = _height;
			if( ImGui::InputInt( "height", &t ) )
				_height = clamp<int>( t, 1, 0x01 << 13 );
			t = _depth;
			if( ImGui::InputInt( "depth", &t ) )
				_depth = clamp<int>( t, 1, 4 );

			t = _type;
			for( int i = 0; i < CT_COUNT; i++ )
				ImGui::RadioButton( constTypeToString( (ConstType) i ), &t, i );
			_type = (ConstType) t;

			ImGui::Checkbox( "uniform", &_uniform );

			{
				ScopedImGuiDisable disable( false );

				static const char *labels[4] = { "x", "y", "z", "w" };
				for( int i = 0; i < _depth; i++ )
				{
					if( i == 1 )
					{
						if( _uniform )
							disable.disable();
					}

					float v = ( _uniform ? _value[0] : _value[i] );
					if( ImGui::SliderFloat( labels[i], &v, -1.0f, 1.0f ) )
						_value[i] = v;
				}
			}

			return true;
		}
#endif

		bool Const::process()
		{
			if( !_width || !_height || !_depth )
				return false;

			float gt = getAppTime();
			SampleFrame *sf = new SampleFrame( _width, _height, gt * 1000.0f, _depth );
			float *ptr = sf->values();

			cv::Scalar s = _uniform ?
				cv::Scalar( _value.x, _value.x, _value.x, _value.x ) :
				cv::Scalar( _value.x, _value.y, _value.z, _value.w );

			//TODO: optimize: only update when values changed
			switch( _type )
			{
			case CT_IDENTITY:
			{
				cv::setIdentity( sf->mat(), s );
				break;
			}
			case CT_FILLED:
			{
				sf->mat().setTo( s );
				break;
			}
			default:
				std::cerr << "<error> unknown const type" << std::endl;
			}

			drawFrame( sf );

			bool ret = pushOutput( "out", sf );
			safeDelete( sf );

			return false;
		}

		bool Const::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "width", _width );
			load<unsigned int>( j, "height", _height );
			load<unsigned int>( j, "depth", _depth );
			load<bool>( j, "uniform", _uniform );
			load<float>( j, "x", _value.x );
			load<float>( j, "y", _value.y );
			load<float>( j, "z", _value.z );
			load<float>( j, "w", _value.w );
			std::string str;
			if( load<std::string>( j, "constType", str ) )
				_type = constTypeFromString( str );

			return ret;
		}

		bool Const::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "width", _width );
			save( j, "height", _height );
			save( j, "depth", _depth );
			save( j, "uniform", _uniform );
			save( j, "x", _value.x );
			save( j, "y", _value.y );
			save( j, "z", _value.z );
			save( j, "w", _value.w );
			save( j, "constType", constTypeToString( _type ) );

			return ret;
		}








		const char *Noise::noiseTypeToString( Noise::NoiseType type )
		{
			switch( type )
			{
			case NT_WHITE:
				return "white";
				//case NT_PINK:
				//	return "pink";
			case NT_PERLIN:
				return "perlin";
			}
			return "UNKNOWN";
		}

		Noise::NoiseType Noise::noiseTypeFromString( const char *s )
		{
			if( !s )
				return NT_COUNT;

			for( int i = 0; i < NT_COUNT; i++ )
				if( !_stricmp( s, noiseTypeToString( (NoiseType) i ) ) )
					return (NoiseType) i;

			return NT_COUNT;
		}

		Noise::NoiseType Noise::noiseTypeFromString( const std::string &s )
		{
			return noiseTypeFromString( s.c_str() );
		}

		Noise::Noise( NoiseType type ) :
			Op(),
			_width( 1 ),
			_height( 1 ),
			_depth( 1 ),
			_seed( 237 ),
			_speed( 1.0f ),
			_type( type ),
			_ken( nullptr )
		{
		}

		Noise::~Noise()
		{
			safeDelete( _ken );
		}

		void Noise::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Noise::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int i = 0;

			i = _width;
			if( ImGui::InputInt( "width", &i ) )
				_width = clamp<int>( i, 1, 0x01 << 13 );
			i = _height;
			if( ImGui::InputInt( "height", &i ) )
				_height = clamp<int>( i, 1, 0x01 << 13 );
			i = _depth;
			if( ImGui::InputInt( "depth", &i ) )
				_depth = clamp<int>( i, 1, 4 );

			i = _seed;
			if( ImGui::InputInt( "seed", &i ) )
			{
				_seed = i;
				safeDelete( _ken );	//delete so it will be re-created with new seed
			}

			int t = _type;
			for( int i = 0; i < NT_COUNT; i++ )
				ImGui::RadioButton( noiseTypeToString( (NoiseType) i ), &t, i );
			_type = (NoiseType) t;

			if( _type == NT_PERLIN )
				ImGui::InputFloat( "speed", &_speed );

			return true;
		}
#endif

		bool Noise::process()
		{
			if( !_width || !_height || !_depth )
				return false;

			float gt = getAppTime();
			SampleFrame *sf = new SampleFrame( _width, _height, gt * 1000.0f, _depth );
			float *ptr = sf->values();

			switch( _type )
			{
			case NT_WHITE:
			{
				for( int j = 0; j < _height; j++ )
					for( int i = 0; i < _width; i++ )
						for( int k = 0; k < _depth; k++ )
							*( ptr++ ) = rand() / (float) RAND_MAX;

				break;
			}
			//case NT_PINK:	//TODO
			//	break;
			case NT_PERLIN:
			{
				if( !_ken )
					_ken = new PerlinNoise( _seed );

				glm::vec2 center = glm::vec2( _width, _height ) * 0.5f;
				for( int j = 0; j < _height; j++ )
					for( int i = 0; i < _width; i++ )
						for( int k = 0; k < _depth; k++ )
							*( ptr++ ) = _ken->noise( (float) i / _width, (float) j / _height, (float) ( k + 1 ) / _depth * gt * _speed );

				break;
			}
			default:
				std::cerr << "<error> unknown noise type" << std::endl;
			}

			drawFrame( sf );

			pushOutput( "out", sf );
			safeDelete( sf );

			return false;
		}

		bool Noise::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "width", _width );
			load<unsigned int>( j, "height", _height );
			load<unsigned int>( j, "depth", _depth );
			load<unsigned int>( j, "seed", _seed );
			load<float>( j, "speed", _speed );
			std::string str;
			if( load<std::string>( j, "noiseType", str ) )
				_type = noiseTypeFromString( str );

			return ret;
		}

		bool Noise::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "width", _width );
			save( j, "height", _height );
			save( j, "depth", _depth );
			save( j, "seed", _seed );
			save( j, "speed", _speed );
			save( j, "noiseType", noiseTypeToString( _type ) );

			return ret;
		}






		const char *Signal::signalTypeToString( Signal::SignalType type )
		{
			switch( type )
			{
			case ST_SINE:
				return "sine";
			case ST_SQUARE:
				return "square";
			case ST_SAWTOOTH:
				return "saw";
			case ST_TRIANGLE:
				return "tri";
			}
			return "UNKNOWN";
		}

		Signal::SignalType Signal::signalTypeFromString( const char *s )
		{
			if( !s )
				return ST_COUNT;

			for( int i = 0; i < ST_COUNT; i++ )
				if( !_stricmp( s, signalTypeToString( (SignalType) i ) ) )
					return (SignalType) i;

			return ST_COUNT;
		}

		Signal::SignalType Signal::signalTypeFromString( const std::string &s )
		{
			return signalTypeFromString( s.c_str() );
		}

		Signal::Signal( SignalType type ) :
			Op(),
			_width( 1 ),
			_height( 1 ),
			_speed( 1.0f ),
			_period( 1.0f ),
			_angle( 0.0f ),
			_type( type )
		{
		}

		Signal::~Signal()
		{
		}

		void Signal::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Signal::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int i = 0;

			i = _width;
			if( ImGui::InputInt( "width", &i ) )
				_width = clamp<int>( i, 1, 0x01 << 13 );
			i = _height;
			if( ImGui::InputInt( "height", &i ) )
				_height = clamp<int>( i, 1, 0x01 << 13 );

			ImGui::SliderFloat( "speed", &_speed, 0.0f, 10.0f );
			ImGui::SliderFloat( "period", &_period, 0.0f, 10.0f );
			ImGui::SliderFloat( "angle", &_angle, 0.0f, 1.0f );

			int t = _type;
			for( int i = 0; i < ST_COUNT; i++ )
				ImGui::RadioButton( signalTypeToString( (SignalType) i ), &t, i );
			_type = (SignalType) t;

			return true;
		}
#endif

		bool Signal::process()
		{
			if( !_width || !_height )
				return false;

			float gt = getAppTime();
			SampleFrame *sf = new SampleFrame( _width, _height, gt * 1000.0f, 1 );
			float *ptr = sf->values();

			float sX = 1.0f / ( _width > 1 ? _width - 1 : 1 );
			float sY = 1.0f / ( _height > 1 ? _height - 1 : 1 );
			float a = _angle * pi2();
			glm::vec2 dir( cos( a ), -sin( a ) );

			switch( _type )
			{
			case ST_SINE:
			{
				glm::vec2 p;
				for( int j = 0; j < _height; j++ )
				{
					p.y = j * sY;
					for( int i = 0; i < _width; i++ )
					{
						p.x = i * sX;
						*( ptr++ ) = sin( ( glm::dot( p, dir ) + gt * _speed ) * pi2() * _period );
					}
				}

				break;
			}
			case ST_SQUARE:
			{
				float dummy = 0.0f;
				glm::vec2 p;
				for( int j = 0; j < _height; j++ )
				{
					p.y = j * sY;
					for( int i = 0; i < _width; i++ )
					{
						p.x = i * sX;
						float d = ( glm::dot( p, dir ) + gt * _speed ) * _period;
						d -= min( 0.0f, floor( d ) );
						*( ptr++ ) = modf( d, &dummy ) >= 0.5f ? -1.0f : 1.0f;
					}
				}

				break;
			}
			case ST_SAWTOOTH:
			{
				float dummy = 0.0f;
				glm::vec2 p;
				for( int j = 0; j < _height; j++ )
				{
					p.y = j * sY;
					for( int i = 0; i < _width; i++ )
					{
						p.x = i * sX;
						float d = ( glm::dot( p, dir ) + gt * _speed ) * _period;
						d -= min( 0.0f, floor( d ) );
						*( ptr++ ) = modf( d, &dummy ) * 2.0f - 1.0f;
					}
				}

				break;
			}
			case ST_TRIANGLE:
			{
				float dummy = 0.0f;
				glm::vec2 p;
				for( int j = 0; j < _height; j++ )
				{
					p.y = j * sY;
					for( int i = 0; i < _width; i++ )
					{
						p.x = i * sX;
						float d = ( glm::dot( p, dir ) + gt * _speed ) * _period;
						d -= min( 0.0f, floor( d ) );
						*( ptr++ ) = 1 - abs( modf( d, &dummy ) * 2.0f - 1 ) * 2;
					}
				}

				break;
			}
			default:
				std::cerr << "<error> unknown signal type" << std::endl;
			}

			drawFrame( sf );

			pushOutput( "out", sf );
			safeDelete( sf );

			return false;
		}

		bool Signal::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "width", _width );
			load<unsigned int>( j, "height", _height );
			load<float>( j, "speed", _speed );
			load<float>( j, "period", _period );
			load<float>( j, "angle", _angle );
			std::string str;
			if( load<std::string>( j, "signalType", str ) )
				_type = signalTypeFromString( str );

			return ret;
		}

		bool Signal::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "width", _width );
			save( j, "height", _height );
			save( j, "speed", _speed );
			save( j, "period", _period );
			save( j, "angle", _angle );
			save( j, "signalType", signalTypeToString( _type ) );

			return ret;
		}






		ContourDetector::ContourDetector() :
			Op(),
			_approximate( true ),
			_approxEpsilon( 0.01f ),
			_areaFilterMin( 0.0f ),
			_areaFilterMax( 1.0f )
		{
		}

		ContourDetector::~ContourDetector()
		{}

		void ContourDetector::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "coms", this ) );
		}

		bool ContourDetector::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				std::vector<std::vector<cv::Point>> cvContours;

				int type = 0;
				int mode = CV_RETR_LIST;
				//NOTE: OpenCV limitation
				if( mode == CV_RETR_FLOODFILL )
					type = CV_32SC1;
				else
					type = CV_8UC1;

				cv::Mat m( sf->width(), sf->height(), type );
				if( sf->depth() > 1 )
				{
					cv::Mat gray;
					cvtColor( sf->mat(), gray, CV_BGR2GRAY );
					gray.convertTo( m, type );
				}
				else
					sf->mat().convertTo( m, type );

				size_t area = sf->width() * sf->height();

				cv::findContours( m, cvContours, mode, CV_CHAIN_APPROX_SIMPLE );

				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp() );
				safeDelete( sf );

				std::vector<std::vector<cv::Point>> cvFilteredContours;

				_contours.clear();
				for( auto &c : cvContours )
				{
					Contour contour;

					cv::Moments M = cv::moments( c );

					contour.centroid.x = M.m10 / M.m00;
					contour.centroid.y = M.m01 / M.m00;

					contour.area = cv::contourArea( c );

					if( contour.area < _areaFilterMin * area )
						continue;
					if( contour.area > _areaFilterMax * area )
						continue;

					std::vector<cv::Point> simplified;
					if( _approximate )
					{
						double epsilon = _approxEpsilon * cv::arcLength( c, true );
						cv::approxPolyDP( c, simplified, epsilon, true );
					}
					else
						simplified = c;

					cvFilteredContours.push_back( simplified );

					for( auto &p : simplified )
						contour.points.push_back( glm::vec2( p.x, p.y ) );

					_contours.push_back( contour );
				}

				cv::drawContours( ret->mat(), cvFilteredContours, -1, cv::Scalar( 1.0f, 1.0f, 1.0f, 1.0f ), cv::FILLED, 8 );

				drawFrame( ret );

				//test
				SampleFrame *coms = new SampleFrame( 2, _contours.size(), ret->timeStamp() );
				for( int i = 0; i < _contours.size(); i++ )
				{
					coms->values()[i * 2 + 0] = _contours[i].centroid.x / ( ret->width() - 1 );
					coms->values()[i * 2 + 1] = _contours[i].centroid.y / ( ret->height() - 1 );
				}
				pushOutput( "coms", coms );
				safeDelete( coms );
				//----

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool ContourDetector::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "approximate", &_approximate );

			{
				ScopedImGuiDisable disable( _approximate );

				ImGui::SliderFloat( "epsilon", &_approxEpsilon, 0.0f, 1.0f );
			}

			ImGui::SliderFloat( "min area", &_areaFilterMin, 0.0f, 1.0f );
			ImGui::SliderFloat( "max area", &_areaFilterMax, 0.0f, 1.0f );
			if( _areaFilterMax < _areaFilterMin )
				_areaFilterMax = _areaFilterMin;

			return true;
		}

		std::vector<FrameDrawer*> ContourDetector::createDrawers()
		{
			std::vector<FrameDrawer*> drawers;

			drawers.push_back( new ContourDrawer( this ) );

			return drawers;
		}
#endif

		bool ContourDetector::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "approximate", _approximate );
			load<float>( j, "approxEpsilon", _approxEpsilon );
			if( !load<float>( j, "areaFilter", _areaFilterMin ) ) //for backwards-compatibility
				load<float>( j, "areaFilterMin", _areaFilterMin );
			load<float>( j, "areaFilterMax", _areaFilterMax );

			return ret;
		}

		bool ContourDetector::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "approximate", _approximate );
			save( j, "approxEpsilon", _approxEpsilon );
			save( j, "areaFilterMin", _areaFilterMin );
			save( j, "areaFilterMax", _areaFilterMax );

			return ret;
		}
	}
}