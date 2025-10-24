/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "imaging.h"

#include "../sceneGraph.h"

#include <fileIO/json.h>
#include <processing/pin.h>

#include <commonImGui.h>

#include <opencv2/imgproc.hpp>

namespace sqid
{
	namespace Imaging
	{
		DEFINE_OP_DESC( LowPass, "lowPass", "/imaging",
			"4E4300B9-4D5F-4641-A006-22B87779E489" );
		DEFINE_OP_DESC( HighPass, "highPass", "/imaging",
			"CFA73516-80A0-4515-B80D-F471BC33F432" );
		DEFINE_OP_DESC( Blur, "blur", "/imaging",
			"2439A1BF-2FD4-4F01-8577-F34E2E800FBD" );
		DEFINE_OP_DESC( Morph, "morph", "/imaging",
			"05DE1846-1F52-48CB-831A-0B3A673A51EA" );
		//DEFINE_OP_DESC( CameraIntrinsics, "intrinsics", "/imaging",
		//	"4C238FB9-0F29-4378-901C-7E36C5AC775D" );
		//DEFINE_OP_DESC( CameraExtrinsics, "extrinsics", "/imaging",
		//	"F9B3D611-3F2A-4DAC-BC81-CA93993A72D7" );





		const char *LowPass::kernelToString( LowPass::Kernel k )
		{
			switch( k )
			{
			case K_GAUSS:
				return "gauss";
			}
			return "UNKNOWN";
		}

		LowPass::Kernel LowPass::kernelFromString( const char *s )
		{
			if( !s )
				return K_COUNT;

			for( int i = 0; i < K_COUNT; i++ )
				if( !_stricmp( s, kernelToString( (Kernel) i ) ) )
					return (Kernel) i;

			return K_COUNT;
		}

		LowPass::Kernel LowPass::kernelFromString( const std::string &s )
		{
			return kernelFromString( s.c_str() );
		}

		LowPass::LowPass( Kernel kernel ) :
			Op(),
			_size( 5 ),
			_sigma( 0.3*( ( _size - 1 ) * 0.5f - 1 ) + 0.8f ),
			_autoSigma( true ),
			_kernel( kernel )
		{
			updateKernel();
		}

		LowPass::~LowPass()
		{}

#ifdef __SUPPORT_GUI
		bool LowPass::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::InputInt( "size", &_size ) )
				updateKernel();

			if( ImGui::Checkbox( "auto", &_autoSigma ) )
				updateKernel();

			{
				ScopedImGuiDisable disable( _autoSigma );

				if( ImGui::SliderFloat( "sigma", &_sigma, 0.0f, 5.0f ) )
					updateKernel();
			}

			return true;
		}
#endif

		bool LowPass::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				cv::Mat dst;
				cv::sepFilter2D( sf->mat(), dst, -1, _k, _k, cv::Point( -1, -1 ), 0.0, cv::BORDER_REFLECT101 ); //TODO: implement more border types

				SampleFrame *ret = new SampleFrame( dst, sf->timeStamp() );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool LowPass::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			//load( j, "b", _value );

			load<int>( j, "size", _size );
			load<float>( j, "sigma", _sigma );
			load<bool>( j, "autoSigma", _autoSigma );
			std::string str;
			if( load<std::string>( j, "kernel", str ) )
				_kernel = kernelFromString( str );

			updateKernel();

			return ret;
		}

		bool LowPass::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "size", _size );
			save( j, "sigma", _sigma );
			save( j, "autoSigma", _autoSigma );
			save( j, "kernel", kernelToString( _kernel ) );

			return ret;
		}

		void LowPass::updateKernel()
		{
			_size = max( _size, 1 );

			switch( _kernel )
			{
			case K_GAUSS:
				_k = cv::getGaussianKernel( _size, _autoSigma ? -1.0f : _sigma );
				break;
			default:
				_k = cv::Mat();
			}
		}








		const char *HighPass::kernelToString( HighPass::Kernel k )
		{
			switch( k )
			{
			case K_SOBEL:
				return "sobel";
			case K_SCHARR:
				return "scharr";
			}
			return "UNKNOWN";
		}

		HighPass::Kernel HighPass::kernelFromString( const char *s )
		{
			if( !s )
				return K_COUNT;

			for( int i = 0; i < K_COUNT; i++ )
				if( !_stricmp( s, kernelToString( (Kernel) i ) ) )
					return (Kernel) i;

			return K_COUNT;
		}

		HighPass::Kernel HighPass::kernelFromString( const std::string &s )
		{
			return kernelFromString( s.c_str() );
		}

		HighPass::HighPass( Kernel kernel ) :
			Op(),
			_horizontally( true ),
			_deriv( 1 ),
			_aperture( 3 ),
			_normalize( true ),
			_kernel( kernel )
		{
			updateKernel();
		}

		HighPass::~HighPass()
		{}

#ifdef __SUPPORT_GUI
		bool HighPass::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int k = _kernel;
			for( int i = 0; i < K_COUNT; i++ )
				ImGui::RadioButton( kernelToString( (Kernel) i ), &k, i );
			if( k != _kernel )
			{
				_kernel = (Kernel) k;
				updateKernel();
			}

			ImGui::Separator();

			int h = ( _horizontally ? 0 : 1 );
			ImGui::RadioButton( "horizontally", &h, 0 );
			ImGui::RadioButton( "vertically", &h, 1 );
			switch( h )
			{
			case 0:
				if( !_horizontally )
				{
					_horizontally = true;
					updateKernel();
				}
				break;
			case 1:
				if( _horizontally )
				{
					_horizontally = false;
					updateKernel();
				}
				break;
			}

			{
				ScopedImGuiDisable disable( _kernel == K_SCHARR );

				if( ImGui::InputInt( "order", &_deriv ) )
					updateKernel();
			}

			if( ImGui::Checkbox( "normalize", &_normalize ) )
				updateKernel();

			if( _kernel != K_SCHARR )
			{
				if( ImGui::InputInt( "aperture", &_aperture, 2 ) )
					updateKernel();
			}

			return true;
		}
#endif

		bool HighPass::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				cv::Mat dst;
				cv::sepFilter2D( sf->mat(), dst, -1, _kHor, _kVer, cv::Point( -1, -1 ), 0.0, cv::BORDER_REFLECT101 ); //TODO: implement more border types

				SampleFrame *ret = new SampleFrame( dst, sf->timeStamp() );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool HighPass::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			//load( j, "b", _value );

			load<int>( j, "deriv", _deriv );
			load<int>( j, "aperture", _aperture );
			load<bool>( j, "normalize", _normalize );
			std::string str;
			if( load<std::string>( j, "kernel", str ) )
				_kernel = kernelFromString( str );
			load<bool>( j, "horizontally", _horizontally );

			updateKernel();

			return ret;
		}

		bool HighPass::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "deriv", _deriv );
			save( j, "aperture", _aperture );
			save( j, "normalize", _normalize );
			save( j, "kernel", kernelToString( _kernel ) );
			save( j, "horizontally", _horizontally );

			return ret;
		}

		void HighPass::updateKernel()
		{
			_aperture = clamp( _aperture, 1, 7 );
			if( !( _aperture % 2 ) )
				_aperture--;

			if( _aperture > 1 )
				_deriv = clamp( _deriv, 1, _aperture - 1 );
			else
				_deriv = max( _deriv, 1 );

			switch( _kernel )
			{
			case K_SOBEL:
				if( _horizontally )
					cv::getDerivKernels( _kHor, _kVer, _deriv, 0, _aperture, _normalize );
				else
					cv::getDerivKernels( _kHor, _kVer, 0, _deriv, _aperture, _normalize );
				break;
			case K_SCHARR:
				if( _horizontally )
					cv::getDerivKernels( _kHor, _kVer, 1, 0, cv::FILTER_SCHARR, _normalize );
				else
					cv::getDerivKernels( _kHor, _kVer, 0, 1, cv::FILTER_SCHARR, _normalize );
				break;
			default:
				_kHor = cv::Mat();
				_kVer = cv::Mat();
			}
		}







		Blur::Blur() :
			Op(),
			_method( M_GAUSS ),
			_size( 5 ),
			_sigma( 0 )
		{}

		Blur::~Blur()
		{}

#ifdef __SUPPORT_GUI
		bool Blur::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int m = this->_method;
			for( int i = 0; i < M_COUNT; i++ )
				ImGui::RadioButton( methodToString( (Method) i ), &m, i );
			this->_method = (Method) m;

			int v = _size;
			if( ImGui::SliderInt( "size", &v, 1, 7 ) )
				_size = v;

			{
				ScopedImGuiDisable disable( _method != M_GAUSS );

				ImGui::SliderFloat( "sigma", &_sigma, 0.0f, 10.0f );
			}

			return true;
		}
#endif

		bool Blur::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );

				//must be odd value
				_size = ( _size / 2 ) * 2 + 1;

				switch( _method )
				{
				case M_BOX:
					cv::blur( sf->mat(), ret->mat(), cv::Size( _size, _size ), cv::Point( -1, -1 ), cv::BORDER_REPLICATE );
					break;
				case M_GAUSS:
					cv::GaussianBlur( sf->mat(), ret->mat(), cv::Size( _size, _size ), _sigma, 0.0, cv::BORDER_REPLICATE );
					break;
				case M_MEDIAN:
					cv::medianBlur( sf->mat(), ret->mat(), _size );
					break;
				default:
					safeDelete( sf );
					safeDelete( ret );
					throw std::runtime_error( "invalid blur method" );
				}

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Blur::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string str;
			if( load<std::string>( j, "method", str ) )
				_method = methodFromString( str );
			load<unsigned int>( j, "size", _size );
			load<float>( j, "sigma", _sigma );

			return ret;
		}

		bool Blur::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "method", methodToString( this->_method ) );
			save( j, "size", _size );
			save( j, "sigma", _sigma );

			return ret;
		}

		const char *Blur::methodToString( Blur::Method method )
		{
			switch( method )
			{
			case M_BOX:
				return "box";
			case M_GAUSS:
				return "gauss";
			case M_MEDIAN:
				return "median";
			}
			return "UNKNOWN";
		}

		Blur::Method Blur::methodFromString( const char *s )
		{
			if( !s )
				return M_COUNT;

			for( int i = 0; i < Blur::Method::M_COUNT; i++ )
				if( !_stricmp( s, methodToString( ( Blur::Method )i ) ) )
					return ( Blur::Method )i;

			return M_COUNT;
		}

		Blur::Method Blur::methodFromString( const std::string &s )
		{
			return methodFromString( s.c_str() );
		}





		const char *Morph::typeToString( Morph::Type t )
		{
			switch( t )
			{
			case T_ERODE:
				return "erode";
			case T_DILATE:
				return "dilate";
			case T_OPEN:
				return "open";
			case T_CLOSE:
				return "close";
			}
			return "UNKNOWN";
		}

		Morph::Type Morph::typeFromString( const char *s )
		{
			if( !s )
				return T_COUNT;

			for( int i = 0; i < T_COUNT; i++ )
				if( !_stricmp( s, typeToString( (Type) i ) ) )
					return (Type) i;

			return T_COUNT;
		}

		Morph::Type Morph::typeFromString( const std::string &s )
		{
			return typeFromString( s.c_str() );
		}

		Morph::Morph( Type type ) :
			Op(),
			_type( type ),
			_iterations( 1 )
		{
			updateKernel();
		}

		Morph::~Morph()
		{}

#ifdef __SUPPORT_GUI
		bool Morph::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int t = _type;
			for( int i = 0; i < T_COUNT; i++ )
				ImGui::RadioButton( typeToString( (Type) i ), &t, i );
			if( t != _type )
			{
				_type = (Type) t;
				updateKernel();
			}

			if( ImGui::InputInt( "iterations", &_iterations ) )
				updateKernel();

			return true;
		}
#endif

		bool Morph::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				//sf->add( _value );

				cv::Mat dst;

				switch( _type )
				{
				case T_ERODE:
					cv::erode( sf->mat(), dst, _kernel, cv::Point( -1, -1 ), _iterations );
					break;
				case T_DILATE:
					cv::dilate( sf->mat(), dst, _kernel, cv::Point( -1, -1 ), _iterations );
					break;
				case T_OPEN:
					cv::morphologyEx( sf->mat(), dst, cv::MORPH_OPEN, _kernel, cv::Point( -1, -1 ), _iterations );
					break;
				case T_CLOSE:
					cv::morphologyEx( sf->mat(), dst, cv::MORPH_CLOSE, _kernel, cv::Point( -1, -1 ), _iterations );
					break;
				default:
					safeDelete( sf );
					throw std::runtime_error( "unknown type" );
				}

				SampleFrame *ret = new SampleFrame( dst, sf->timeStamp() );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Morph::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string str;
			if( load<std::string>( j, "type", str ) )
				_type = typeFromString( str );
			load<int>( j, "iterations", _iterations );

			updateKernel();

			return ret;
		}

		bool Morph::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "type", typeToString( _type ) );
			save( j, "iterations", _iterations );

			return ret;
		}

		void Morph::updateKernel()
		{
			_iterations = max( _iterations, 1 );
			_kernel = cv::Mat();
		}












		/*
		CameraIntrinsics::CameraIntrinsics() :
			Op()
		{}

		CameraIntrinsics::~CameraIntrinsics()
		{}

		bool CameraIntrinsics::drawUI()
		{
			if( !Op::drawUI() )
				return false;


			return true;
		}

		bool CameraIntrinsics::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				//sf->add( _value );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool CameraIntrinsics::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			//load( j, "b", _value );

			return ret;
		}

		bool CameraIntrinsics::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			//save( j, "b", _value );

			return ret;
		}







		CameraExtrinsics::CameraExtrinsics() :
			Op()
		{}

		CameraExtrinsics::~CameraExtrinsics()
		{}

		bool CameraExtrinsics::drawUI()
		{
			if( !Op::drawUI() )
				return false;


			return true;
		}

		bool CameraExtrinsics::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				//sf->add( _value );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool CameraExtrinsics::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			//load( j, "b", _value );

			return ret;
		}

		bool CameraExtrinsics::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "b", _value );

			return ret;
		}
		*/
	}
}