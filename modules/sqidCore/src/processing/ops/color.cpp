/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "color.h"

#include "../sceneGraph.h"

#include <commonImGui.h>
#include <fileIO/json.h>
#include <processing/pin.h>

#include <opencv2/imgproc.hpp>

namespace sqid
{
	namespace Color
	{
		DEFINE_OP_DESC( ToGrayscale, "toGrayscale", "/color",
			"B2695D66-B1C8-40D1-B532-33C537D254DD" );
		DEFINE_OP_DESC( Convert, "convert", "/color",
			"8D7687E5-8AA7-4E40-B694-D28FC200BDF8" );
		DEFINE_OP_DESC( HSVShift, "HSVshift", "/color",
			"16C972D5-BBCE-472B-9857-D582C041655A" );



		const float ToGrayscale::uniform[3] = { 1.0f, 1.0f, 1.0f };
		const float ToGrayscale::rec601[3] = { 0.299f, 0.587f, 0.114f };
		const float ToGrayscale::bt709[3] = { 0.2126f, 0.7152f, 0.0722f };
		const float ToGrayscale::bt2100[3] = { 0.2627f, 0.6780f, 0.0593f };

		ToGrayscale::ToGrayscale() :
			Op(),
			_normalize( true )
		{
			_weights[0] = uniform[0];
			_weights[1] = uniform[1];
			_weights[2] = uniform[2];
		}

		ToGrayscale::~ToGrayscale()
		{}

#ifdef __SUPPORT_GUI
		bool ToGrayscale::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			const char *rgb[] = { "r", "g", "b" };

			for( int i = 0; i < 3; i++ )
			{
				float f = _weights[i];
				if( ImGui::SliderFloat( rgb[i], &f, 0, 1 ) )
					_weights[i] = f;
			}

			ImGui::Checkbox( "normalize", &_normalize );

			ImGui::Text( "presets:" );
			if( ImGui::Button( "uniform" ) )
			{
				_weights[0] = uniform[0];
				_weights[1] = uniform[1];
				_weights[2] = uniform[2];
			}
			if( ImGui::Button( "rec601" ) )
			{
				_weights[0] = rec601[0];
				_weights[1] = rec601[1];
				_weights[2] = rec601[2];
			}
			if( ImGui::Button( "ITU-R BT.709" ) )
			{
				_weights[0] = bt709[0];
				_weights[1] = bt709[1];
				_weights[2] = bt709[2];
			}
			if( ImGui::Button( "ITU-R BT.2100" ) )
			{
				_weights[0] = bt2100[0];
				_weights[1] = bt2100[1];
				_weights[2] = bt2100[2];
			}

			return true;
		}
#endif

		bool ToGrayscale::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), 1 );

				int channels = min<int>( sf->depth(), 3 );
				std::vector<float> weights( channels );

				for( int i = 0; i < channels; i++ )
					weights[i] = _weights[2 - i];	//OpenCV is BGR

				if( _normalize )
				{
					float sum = 0;
					for( int i = 0; i < channels; i++ )
						sum += weights[i];
					if( abs( sum ) > std::numeric_limits<float>::epsilon() )
						for( int i = 0; i < channels; i++ )
							weights[i] /= sum;
				}

				std::vector<cv::Mat> inLayers;
				cv::split( sf->mat(), inLayers );
				cv::Mat &mOut = ret->mat();

				for( int i = 0; i < channels; i++ )
					cv::scaleAdd( inLayers[i], weights[i], mOut, mOut );

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool ToGrayscale::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "weightR", _weights[0] );
			load<float>( j, "weightG", _weights[1] );
			load<float>( j, "weightB", _weights[2] );

			load<bool>( j, "normalize", _normalize );

			return ret;
		}

		bool ToGrayscale::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "weightR", _weights[0] );
			save( j, "weightG", _weights[1] );
			save( j, "weightB", _weights[2] );

			save( j, "normalize", _normalize );

			return ret;
		}





		const char *colorSpaceToString( sqid::Color::ColorSpace cs )
		{
			switch( cs )
			{
			case sqid::Color::CS_RGB:
				return "RGB";
			case sqid::Color::CS_HSV:
				return "HSV";
			case sqid::Color::CS_YUV:
				return "YUV";
			case sqid::Color::CS_CMY:
				return "CMY";
			case sqid::Color::CS_CMYK:
				return "CMYK";
			case sqid::Color::CS_CIE:
				return "CIE";
			}

			return "UNKNOWN";
		}

		sqid::Color::ColorSpace colorSpaceFromString( const char *str )
		{
			if( !str )
				return sqid::Color::CS_COUNT;

			for( int i = 0; i < sqid::Color::CS_COUNT; i++ )
				if( !_stricmp( str, colorSpaceToString( ( sqid::Color::ColorSpace) i ) ) )
					return ( sqid::Color::ColorSpace) i;

			return sqid::Color::CS_COUNT;
		}

		sqid::Color::ColorSpace colorSpaceFromString( const std::string &str )
		{
			return colorSpaceFromString( str.c_str() );
		}

		size_t colorSpaceChannels( sqid::Color::ColorSpace cs )
		{
			switch( cs )
			{
			case sqid::Color::CS_RGB:
			case sqid::Color::CS_HSV:
			case sqid::Color::CS_YUV:
			case sqid::Color::CS_CMY:
			case sqid::Color::CS_CIE:
				return 3;
			case sqid::Color::CS_CMYK:
				return 4;
			}

			return 0;
		}

		inline glm::vec3 hsv2yuv( const glm::vec3 c ) { return rgb2yuv( hsv2rgb( c ) ); }
		inline glm::vec3 hsv2cmy( const glm::vec3 c ) { return rgb2cmy( hsv2rgb( c ) ); }
		inline glm::vec4 hsv2cmyk( const glm::vec3 c ) { return rgb2cmyk( hsv2rgb( c ) ); }
		inline glm::vec3 hsv2cie( const glm::vec3 c ) { return rgb2cie( hsv2rgb( c ) ); }

		inline glm::vec3 yuv2hsv( const glm::vec3 c ) { return rgb2hsv( yuv2rgb( c ) ); }
		inline glm::vec3 yuv2cmy( const glm::vec3 c ) { return rgb2cmy( yuv2rgb( c ) ); }
		inline glm::vec4 yuv2cmyk( const glm::vec3 c ) { return rgb2cmyk( yuv2rgb( c ) ); }
		inline glm::vec3 yuv2cie( const glm::vec3 c ) { return rgb2cie( yuv2rgb( c ) ); }

		inline glm::vec3 cmy2hsv( const glm::vec3 c ) { return rgb2hsv( cmy2rgb( c ) ); }
		inline glm::vec3 cmy2yuv( const glm::vec3 c ) { return rgb2yuv( cmy2rgb( c ) ); }
		inline glm::vec3 cmy2cie( const glm::vec3 c ) { return rgb2cie( cmy2rgb( c ) ); }

		inline glm::vec3 cmyk2hsv( const glm::vec4 c ) { return rgb2hsv( cmyk2rgb( c ) ); }
		inline glm::vec3 cmyk2yuv( const glm::vec4 c ) { return rgb2yuv( cmyk2rgb( c ) ); }
		inline glm::vec3 cmyk2cie( const glm::vec4 c ) { return rgb2cie( cmyk2rgb( c ) ); }

		inline glm::vec3 cie2hsv( const glm::vec3 c ) { return rgb2hsv( cie2rgb( c ) ); }
		inline glm::vec3 cie2yuv( const glm::vec3 c ) { return rgb2yuv( cie2rgb( c ) ); }
		inline glm::vec3 cie2cmy( const glm::vec3 c ) { return rgb2cmy( cie2rgb( c ) ); }
		inline glm::vec4 cie2cmyk( const glm::vec3 c ) { return rgb2cmyk( cie2rgb( c ) ); }


		template<typename srcVec, typename dstVec, typename F>
		bool convert( const SampleFrame *src, SampleFrame *dst, F &functor )
		{
			size_t elements = src->width() * src->height();

			const srcVec *s = reinterpret_cast<const srcVec*>( src->values() );
			dstVec *d = reinterpret_cast<dstVec*>( dst->values() );

			if( s->length() > src->depth() )
			{
				std::cerr << "<warning> source frame has insufficient nr of channels" << std::endl;
				return false;
			}
			else if( s->length() != src->depth() )
				std::cerr << "<warning> source frame has unexpected depth" << std::endl;

			if( d->length() > dst->depth() )
			{
				std::cerr << "<warning> dest frame has insufficient nr of channels" << std::endl;
				return false;
			}
			else if( d->length() != dst->depth() )
				std::cerr << "<warning> dest frame has unexpected depth" << std::endl;

#pragma omp parallel for
			for( int i = 0; i < elements; i++ )
				d[i] = functor( s[i] );

			return true;
		}



		Convert::Convert() :
			Op(),
			_from( sqid::Color::ColorSpace::CS_RGB ),
			_to( sqid::Color::ColorSpace::CS_RGB )
		{}

		Convert::~Convert()
		{}

#ifdef __SUPPORT_GUI
		bool Convert::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Text( "from colorspace" );
			int e = (int) _from;
			for( int i = 0; i < sqid::Color::CS_COUNT; i++ )
				ImGui::RadioButton( std::string( colorSpaceToString( (sqid::Color::ColorSpace) i ) ).append( "##from" ).c_str(), &e, i );
			if( _from != (sqid::Color::ColorSpace) e )
				_from = (sqid::Color::ColorSpace) e;

			ImGui::Text( "to colorspace" );
			e = (int) _to;
			for( int i = 0; i < sqid::Color::CS_COUNT; i++ )
				ImGui::RadioButton( std::string( colorSpaceToString( ( sqid::Color::ColorSpace ) i ) ).append( "##to" ).c_str(), &e, i );
			if( _to != ( sqid::Color::ColorSpace ) e )
				_to = ( sqid::Color::ColorSpace ) e;

			return true;
		}
#endif

		bool Convert::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				size_t ccFrom = colorSpaceChannels( _from );
				size_t ccTo = colorSpaceChannels( _to );
				//TODO: warn if too many channels, consider throwing error when not enough channels
				if( sf->depth() >= ccFrom )
				{
					SampleFrame *ret = nullptr;
					
					if( _from == _to )
						ret = sf;
					else
					{
						if( ccFrom == ccTo )
							ret = sf; //channel count is identical -- reuse sf
						else
							ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), ccTo );

						//TODO: performance-wise this may not be ideal... 
						switch( _from )
						{
						case sqid::Color::CS_RGB:
							switch( _to )
							{
							case sqid::Color::CS_HSV:
								convert<glm::vec3, glm::vec3>( sf, ret, rgb2hsv );
								break;
							case sqid::Color::CS_YUV:
								convert<glm::vec3, glm::vec3>( sf, ret, rgb2yuv );
								break;
							case sqid::Color::CS_CMY:
								convert<glm::vec3, glm::vec3>( sf, ret, rgb2cmy );
								break;
							case sqid::Color::CS_CMYK:
								convert<glm::vec3, glm::vec4>( sf, ret, rgb2cmyk );
								break;
							case sqid::Color::CS_CIE:
								convert<glm::vec3, glm::vec3>( sf, ret, rgb2cie );
								break;
							default:
								std::cerr << "<warning> unknown 'to' colorspace" << std::endl;
							}
							break;
						case sqid::Color::CS_HSV:
							switch( _to )
							{
							case sqid::Color::CS_RGB:
								convert<glm::vec3, glm::vec3>( sf, ret, hsv2rgb );
								break;
							case sqid::Color::CS_YUV:
								convert<glm::vec3, glm::vec3>( sf, ret, hsv2yuv );
								break;
							case sqid::Color::CS_CMY:
								convert<glm::vec3, glm::vec3>( sf, ret, hsv2cmy );
								break;
							case sqid::Color::CS_CMYK:
								convert<glm::vec3, glm::vec4>( sf, ret, hsv2cmyk );
								break;
							case sqid::Color::CS_CIE:
								convert<glm::vec3, glm::vec3>( sf, ret, hsv2cie );
								break;
							default:
								std::cerr << "<warning> unknown 'to' colorspace" << std::endl;
							}
							break;
						case sqid::Color::CS_YUV:
							switch( _to )
							{
							case sqid::Color::CS_RGB:
								convert<glm::vec3, glm::vec3>( sf, ret, yuv2rgb );
								break;
							case sqid::Color::CS_HSV:
								convert<glm::vec3, glm::vec3>( sf, ret, yuv2hsv );
								break;
							case sqid::Color::CS_CMY:
								convert<glm::vec3, glm::vec3>( sf, ret, yuv2cmy );
								break;
							case sqid::Color::CS_CMYK:
								convert<glm::vec3, glm::vec4>( sf, ret, yuv2cmyk );
								break;
							case sqid::Color::CS_CIE:
								convert<glm::vec3, glm::vec3>( sf, ret, yuv2cie );
								break;
							default:
								std::cerr << "<warning> unknown 'to' colorspace" << std::endl;
							}
							break;
						case sqid::Color::CS_CMY:
							switch( _to )
							{
							case sqid::Color::CS_RGB:
								convert<glm::vec3, glm::vec3>( sf, ret, cmy2rgb );
								break;
							case sqid::Color::CS_HSV:
								convert<glm::vec3, glm::vec3>( sf, ret, cmy2hsv );
								break;
							case sqid::Color::CS_YUV:
								convert<glm::vec3, glm::vec3>( sf, ret, cmy2yuv );
								break;
							case sqid::Color::CS_CMYK:
								convert<glm::vec3, glm::vec4>( sf, ret, cmy2cmyk );
								break;
							case sqid::Color::CS_CIE:
								convert<glm::vec3, glm::vec3>( sf, ret, cmy2cie );
								break;
							default:
								std::cerr << "<warning> unknown 'to' colorspace" << std::endl;
							}
							break;
						case sqid::Color::CS_CMYK:
							switch( _to )
							{
							case sqid::Color::CS_RGB:
								convert<glm::vec4, glm::vec3>( sf, ret, cmyk2rgb );
								break;
							case sqid::Color::CS_HSV:
								convert<glm::vec4, glm::vec3>( sf, ret, cmyk2hsv );
								break;
							case sqid::Color::CS_YUV:
								convert<glm::vec4, glm::vec3>( sf, ret, cmyk2yuv );
								break;
							case sqid::Color::CS_CMY:
								convert<glm::vec4, glm::vec3>( sf, ret, cmyk2cmy );
								break;
							case sqid::Color::CS_CIE:
								convert<glm::vec4, glm::vec3>( sf, ret, cmyk2cie );
								break;
							default:
								std::cerr << "<warning> unknown 'to' colorspace" << std::endl;
							}
							break;
						case sqid::Color::CS_CIE:
							switch( _to )
							{
							case sqid::Color::CS_RGB:
								convert<glm::vec3, glm::vec3>( sf, ret, cie2rgb );
								break;
							case sqid::Color::CS_HSV:
								convert<glm::vec3, glm::vec3>( sf, ret, cie2hsv );
								break;
							case sqid::Color::CS_YUV:
								convert<glm::vec3, glm::vec3>( sf, ret, cie2yuv );
								break;
							case sqid::Color::CS_CMY:
								convert<glm::vec3, glm::vec3>( sf, ret, cie2cmy );
								break;
							case sqid::Color::CS_CMYK:
								convert<glm::vec3, glm::vec3>( sf, ret, cie2cmyk );
								break;
							default:
								std::cerr << "<warning> unknown 'to' colorspace" << std::endl;
							}
							break;
						default:
							std::cerr << "<warning> unknown 'from' colorspace" << std::endl;
						}
					}

					drawFrame( ret );

					pushOutput( "out", ret );
					if( ret != sf )
						safeDelete( ret );
				}

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Convert::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string s;
			if( load<std::string>( j, "from", s ) )
				_from = colorSpaceFromString( s );
			if( load<std::string>( j, "to", s ) )
				_to = colorSpaceFromString( s );

			return ret;
		}

		bool Convert::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "from", colorSpaceToString( _from ) );
			save( j, "to", colorSpaceToString( _to ) );

			return ret;
		}




		HSVShift::HSVShift() :
			Op(),
			_hue( 0 ),
			_saturation( 0 ),
			_value( 0 )
		{}

		HSVShift::~HSVShift()
		{}

#ifdef __SUPPORT_GUI
		bool HSVShift::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "hue", &_hue, 0, 1 );
			ImGui::SliderFloat( "saturation", &_saturation, -1, 1 );
			ImGui::SliderFloat( "value", &_value, -1, 1 );

			return true;
		}
#endif

		bool HSVShift::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( sf->depth() != 3 )
					std::cerr << "<error> require RGB frame as input" << std::endl;
				else
				{
					size_t elements = sf->width() * sf->height();
					glm::vec3 *ptr = reinterpret_cast<glm::vec3*>( sf->values() );

					float dummy = 0;

#pragma omp parallel for
					for( int i = 0; i < elements; i++ )
					{
						glm::vec3 v = rgb2hsv( ptr[i] );

						v.x = modf( v.x + _hue, &dummy );
						v.y = clamp01( v.y + _saturation );
						v.z = clamp01( v.z + _value );

						ptr[i] = hsv2rgb( v );
					}
					
					drawFrame( sf );

					pushOutput( "out", sf );

				}
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool HSVShift::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "hue", _hue );
			load<float>( j, "saturation", _saturation );
			load<float>( j, "value", _value );

			return ret;
		}

		bool HSVShift::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "hue", _hue );
			save( j, "saturation", _saturation );
			save( j, "value", _value );

			return ret;
		}
	}
}