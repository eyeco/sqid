/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "math.h"

#include "../sceneGraph.h"

#include <processing/pin.h>
#include <fileIO/json.h>

#include <commonImGui.h>

#include <opencv2/imgproc.hpp>
//#include <opencv2/imgproc/types_c.h>

namespace sqid
{
	namespace Math
	{
		DEFINE_OP_DESC( AddConst, "addConst", "/math",
			"30802530-FFBF-4D31-AA3C-E23FCCE29407" );
		DEFINE_OP_DESC( MulConst, "mulConst", "/math",
			"A820115B-4E97-488B-9A0F-EEF71C3E0E95" );
		DEFINE_OP_DESC( Add, "add", "/math",
			"139329CA-6889-463E-914A-9048B8B5E155" );
		DEFINE_OP_DESC( Sub, "sub", "/math",
			"B8C16836-B9AD-46E4-86FF-D73358CE9772" );
		DEFINE_OP_DESC( Mul, "mul", "/math",
			"AFC90960-EEC2-416F-AD82-B1A742A42462" );
		DEFINE_OP_DESC( LinEqConst, "linEqConst", "/math",
			"EB5F7DAF-D9DE-4DC6-BB2D-D3AC94CD8FBC" );
		DEFINE_OP_DESC( LinEq, "linEq", "/math",
			"F28A91E1-2D26-4034-B7B2-F731D7F16FBB" );
		DEFINE_OP_DESC( MixConst, "mixConst", "/math",
			"A71F0D82-CEA3-467B-B28C-8056766B96DC" );
		DEFINE_OP_DESC( Mix, "mix", "/math",
			"CC84D1BB-14EC-4E0C-B18C-CBB29E175CB0" );
		DEFINE_OP_DESC( Abs, "abs", "/math",
			"B138EDCC-9669-489C-8C43-4B7B16E0E500" );
		DEFINE_OP_DESC( PowConst, "powConst", "/math",
			"33580153-0E15-4B91-B0D7-B6FDDCDED1BD" );
		DEFINE_OP_DESC( Sqrt, "sqrt", "/math",
			"B17BE2F7-5A57-44BF-A6DC-93396546EB7F" );
		DEFINE_OP_DESC( Log, "log", "/math",
			"FD74E284-A78F-444B-8312-9954DEA1318A" );
		DEFINE_OP_DESC( Exp, "exp", "/math",
			"59D1125C-15E8-4B98-9EAA-542EB0BE0A19" );
		DEFINE_OP_DESC( ExpConst, "expConst", "/math",
			"9315063E-AF3B-43EB-B690-7AF11BC4BC85" );
		DEFINE_OP_DESC( Sigmoid, "sigmoid", "/math",
			"0C47700F-7E59-469E-8437-F7684FEE6DAE" );
		DEFINE_OP_DESC( Sum, "sum", "/math",
			"F79A8221-2671-4E1D-A611-FFC2D51F96A4" );
		DEFINE_OP_DESC( Product, "product", "/math",
			"27B68E1B-3B0C-4A6E-B8FE-045B280617EC" );
		DEFINE_OP_DESC( Invert, "invert", "/math",
			"432A3429-A321-4F68-88A0-A193EB489C94" );
		DEFINE_OP_DESC( Threshold, "threshold", "/math",
			"F50BAE17-5C98-446E-A957-C79D7C75BF1C" );
		DEFINE_OP_DESC( Remap, "remap", "/math/transform",
			"5CE8FD66-DF75-4D00-839C-86BCC64C6A0E" );
		DEFINE_OP_DESC( Normalize, "normalize", "/math/transform",
			"464343E4-E68E-4809-AC50-BD0205316C3A" );
		DEFINE_OP_DESC( AutoNormalize, "autoNormalize", "/math/transform",
			"171C1E25-1520-4B27-BA91-4DCE2026B020" );
		DEFINE_OP_DESC( MinConst, "minConst", "/math",
			"E3FDF7FD-41EF-4FBC-8538-0568ED265CEC" );
		DEFINE_OP_DESC( Min, "min", "/math",
			"8FE02678-886E-4D9E-A137-18AF816B430D" );
		DEFINE_OP_DESC( MaxConst, "maxConst", "/math",
			"5E3C8213-C120-45E9-9D10-35B80602DE07" );
		DEFINE_OP_DESC( Max, "max", "/math",
			"0E3F53F9-542F-4983-9600-E0D7D7EC38AB" );
		DEFINE_OP_DESC( Clamp, "clamp", "/math",
			"62D938F5-5D5A-457C-87A4-9BCCA1D62C55" );
		DEFINE_OP_DESC( ClampConst, "clampConst", "/math",
			"56800462-67AA-43FD-BEF9-20DA6632530F" );
		DEFINE_OP_DESC( InRange, "inRange", "/math",
			"CEEEEC75-9D3F-4825-9630-352FB261754C" );
		DEFINE_OP_DESC( Approx, "approx", "/math",
			"2ED2ED03-8133-47AD-B34C-6378F28482E8" );
		DEFINE_OP_DESC( And, "and", "/math/boolean",
			"25FA618D-B5C2-4125-8C05-8C7861CA36F2" );
		DEFINE_OP_DESC( Or, "or", "/math/boolean",
			"4CD29FBE-4B1A-495C-BDB6-FB5C24F4C617" );
		DEFINE_OP_DESC( XOr, "xor", "/math/boolean",
			"AD0DD16B-00C7-4E2A-831B-49C4CBC4A51C" );
		DEFINE_OP_DESC( Determinant, "determinant", "/math/matrix",
			"8A8F0479-EED2-4F9D-A217-19621DA73B81" );
		DEFINE_OP_DESC( LinearFunction, "linear", "/math",
			"4BB0DF34-3BEB-4554-9AE3-F0B8D2F3353D" );
		DEFINE_OP_DESC( MultiLinearFunction, "multiLinear", "/math",
			"E29A4659-7CF3-4EFB-92A0-7EDB56257D25" );
		DEFINE_OP_DESC( FFT2D, "fft (2D)", "/math/spectral",
			"8C855143-9B1C-4CE4-BE10-3BCB9D277561" );
#ifdef __FFTW_SUPPORT
		DEFINE_OP_DESC( FFT1D, "fft (1D)", "/math/spectral",
			"9E65BC29-94DD-4B17-915B-DB1AF7D5893C" );
#endif
		DEFINE_OP_DESC( ToPolar, "toPolar", "/math/transform",
			"8A0B34D4-EB5F-4D2D-9440-1B8716FEE032" );


		namespace Internal
		{
			enum ReductionMethod
			{
				RM_SUM,
				RM_AVRG,
				RM_PRODUCT
			};

			SampleFrame *reduceX( const SampleFrame *sf, ReductionMethod rm )
			{
				if( !sf->size() )
					return nullptr;

				if( sf->width() == 1 )
					return new SampleFrame( *sf );

				int stride = sf->width() * sf->depth();
				SampleFrame *ret = new SampleFrame( 1, sf->height(), sf->timeStamp(), sf->depth() );
				for( int j = 0; j < sf->height(); j++ )
					for( int k = 0; k < sf->depth(); k++ )
					{
						float sum = ( rm == RM_PRODUCT ? 1 : 0 );
						const float *ptr = sf->values() + k + j * stride;

						if( rm == RM_PRODUCT )
						{
							for( int i = 0; i < sf->width(); i++, ptr += sf->depth() )
								sum *= *ptr;
						}
						else
						{
							for( int i = 0; i < sf->width(); i++, ptr += sf->depth() )
								sum += *ptr;
						}

						switch( rm )
						{
						case RM_SUM:
						case RM_PRODUCT:
							*( ret->values() + k + j * sf->depth() ) = sum;
							break;
						case RM_AVRG:
							*( ret->values() + k + j * sf->depth() ) = sum / sf->width();
							break;
						default:
							std::cerr << "<error> unknown reduction method" << std::endl;
						}
					}

				return ret;
			}

			SampleFrame *reduceY( const SampleFrame *sf, ReductionMethod rm )
			{
				if( !sf->size() )
					return nullptr;

				if( sf->height() == 1 )
					return new SampleFrame( *sf );

				int stride = sf->width() * sf->depth();
				SampleFrame *ret = new SampleFrame( sf->width(), 1, sf->timeStamp(), sf->depth() );
				for( int i = 0; i < sf->width(); i++ )
					for( int k = 0; k < sf->depth(); k++ )
					{
						float sum = ( rm == RM_PRODUCT ? 1 : 0 );
						const float *ptr = sf->values() + k + i * sf->depth();

						if( rm == RM_PRODUCT )
						{
							for( int j = 0; j < sf->height(); j++, ptr += stride )
								sum *= *ptr;
						}
						else
						{
							for( int j = 0; j < sf->height(); j++, ptr += stride )
								sum += *ptr;
						}

						switch( rm )
						{
						case RM_SUM:
						case RM_PRODUCT:
							*( ret->values() + k + i * sf->depth() ) = sum;
							break;
						case RM_AVRG:
							*( ret->values() + k + i * sf->depth() ) = sum / sf->height();
							break;
						default:
							std::cerr << "<error> unknown reduction method" << std::endl;
						}
					}

				return ret;
			}

			SampleFrame *reduceZ( const SampleFrame *sf, ReductionMethod rm )
			{
				if( !sf->size() )
					return nullptr;

				if( sf->depth() == 1 )
					return new SampleFrame( *sf );

				int stride = sf->width() * sf->depth();
				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), 1 );
				for( int i = 0; i < sf->width(); i++ )
					for( int j = 0; j < sf->height(); j++ )
					{
						float sum = ( rm == RM_PRODUCT ? 1 : 0 );
						const float *ptr = sf->values() + i * sf->depth() + j * stride;

						if( rm == RM_PRODUCT )
						{
							for( int k = 0; k < sf->depth(); k++, ptr++ )
								sum *= *ptr;
						}
						else
						{
							for( int k = 0; k < sf->depth(); k++, ptr++ )
								sum += *ptr;
						}

						switch( rm )
						{
						case RM_SUM:
						case RM_PRODUCT:
							*( ret->values() + i + j * sf->width() ) = sum;
							break;
						case RM_AVRG:
							*( ret->values() + i + j * sf->width() ) = sum / sf->depth();
							break;
						default:
							std::cerr << "<error> unknown reduction method" << std::endl;
						}
					}

				return ret;
			}
		}


		AddConst::AddConst() :
			Op(),
			_value( 0.0f )
		{}

		AddConst::~AddConst()
		{}

#ifdef __SUPPORT_GUI
		bool AddConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			float v = _value;
			if( ImGui::SliderFloat( "b", &v, -1, 1, "%.2f" ) )
				_value = v;

			return true;
		}
#endif

		bool AddConst::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				//sf->add( _value );
				sf->add( _value );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}
		
			return inputPending( "in" );
		}

		bool AddConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "b", _value );

			return ret;
		}

		bool AddConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "b", _value );

			return ret;
		}




		MulConst::MulConst() :
			Op(),
			_value( 1.0f )
		{}

		MulConst::~MulConst()
		{}

#ifdef __SUPPORT_GUI
		bool MulConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			float v = _value;
			if( ImGui::SliderFloat( "b", &v, -10, 10, "%.2f" ) )
				_value = v;

			return true;
		}
#endif

		bool MulConst::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->mul( _value );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool MulConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "b", _value );

			return ret;
		}

		bool MulConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "b", _value );

			return ret;
		}





		Add::Add() :
			Op()
		{}

		Add::~Add()
		{}

		bool Add::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				SampleFrame *ret = nullptr;

				if( a )
				{
					if( b )
					{
						if( !dimensionsCompatible( a, b ) )
						{
							safeDelete( a );
							safeDelete( b );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( a->mat(), max( a->timeStamp(), b->timeStamp() ) );
						ret->add( b );
					}
					else
						ret = a;
				}
				else
					ret = b;

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != a && ret != b )
					safeDelete( ret );
				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		void Add::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}





		Sub::Sub() :
			Op()
		{}

		Sub::~Sub()
		{}

		bool Sub::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				SampleFrame *ret = nullptr;

				if( a )
				{
					if( b )
					{
						if( !dimensionsCompatible( a, b ) )
						{
							safeDelete( a );
							safeDelete( b );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( a->mat(), max( a->timeStamp(), b->timeStamp() ) );
						ret->sub( b );
					}
					else
						ret = a;
				}
				else
				{
					b->mul( -1.0f );
					ret = b;
				}

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != a && ret != b )
					safeDelete( ret );
				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		void Sub::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}





		Mul::Mul() :
			Op()
		{}

		Mul::~Mul()
		{}

		bool Mul::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				SampleFrame *ret = nullptr;

				if( a )
				{
					if( b )
					{
						if( !dimensionsCompatible( a, b ) )
						{
							safeDelete( a );
							safeDelete( b );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( a->mat(), max( a->timeStamp(), b->timeStamp() ) );
						ret->mul( b );
					}
					else
					{
						a->set( 0.0f );
						ret = a;
					}
				}
				else
				{
					b->set( 0.0f );
					ret = b;
				}

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != a && ret != b )
					safeDelete( ret );
				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		void Mul::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}






		LinEqConst::LinEqConst() :
			Op(),
			_k( 1.0f ),
			_d( 0.0f )
		{}

		LinEqConst::~LinEqConst()
		{}

#ifdef __SUPPORT_GUI
		bool LinEqConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "k", &_k, -5, 5, "%.2f" );
			ImGui::SliderFloat( "d", &_d, -5, 5, "%.2f" );

			return true;
		}
#endif

		bool LinEqConst::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->mul( _k );
				sf->add( _d );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool LinEqConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "k", _k );
			load<float>( j, "d", _d );

			return ret;
		}

		bool LinEqConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "k", _k );
			save( j, "d", _d );

			return ret;
		}






		LinEq::LinEq() :
			Op()
		{}

		LinEq::~LinEq()
		{}

		bool LinEq::process()
		{
			SampleFrame *x = fetchInput<SampleFrame>( "x" );
			SampleFrame *k = fetchInput<SampleFrame>( "k" );
			SampleFrame *d = fetchInput<SampleFrame>( "d" );

			if( x || k || d )
			{
				if( x )
				{
					if( k )
					{
						if( !dimensionsCompatible( x, k ) )
						{
							safeDelete( x );
							safeDelete( k );
							safeDelete( d );
							throw std::runtime_error( "inputs differ in size" );
						}

						x->mul( k );
					}
					else
						x->set( 0.0f );

					if( d )
					{
						if( !dimensionsCompatible( x, d ) )
						{
							safeDelete( x );
							safeDelete( k );
							safeDelete( d );
							throw std::runtime_error( "inputs differ in size" );
						}

						x->add( d );
					}

					drawFrame( x );
					pushOutput( "out", x );
				}

				safeDelete( x );
				safeDelete( k );
				safeDelete( d );
			}

			return inputPending( "x" ) || inputPending( "k" ) || inputPending( "d" );
		}

		void LinEq::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "x", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "k", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "d", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}






		MixConst::MixConst() :
			Op(),
			_t( 0.5f )
		{}

		MixConst::~MixConst()
		{}

		void MixConst::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool MixConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "t", &_t, 0, 1 );

			return true;
		}
#endif

		bool MixConst::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				if( !dimensionsCompatible( a, b ) )
				{
					safeDelete( a );
					safeDelete( b );
					throw std::runtime_error( "inputs differ in size" );
				}

				uint32_t ts = max( a ? a->timeStamp() : 0, b ? b->timeStamp() : 0 );

				SampleFrame *ret = ( a ?
					new SampleFrame( a->width(), a->height(), ts, a->depth() ) :
					new SampleFrame( b->width(), b->height(), ts, b->depth() ) );

				if( a )
					ret->add( a, 1 - _t );
				if( b )
					ret->add( b, _t );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );

				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		bool MixConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "t", _t );

			return ret;
		}

		bool MixConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "t", _t );

			return ret;
		}






		Mix::Mix() :
			Op(),
			_clamp( true )
		{}

		Mix::~Mix()
		{}

		void Mix::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "t", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Mix::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "clamp", &_clamp );

			return true;
		}
#endif

		bool Mix::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );
			SampleFrame *t = fetchInput<SampleFrame>( "t" );

			if( a || b || t )
			{
				if( !dimensionsCompatible( a, b ) )
				{
					safeDelete( a );
					safeDelete( b );
					safeDelete( t );
					throw std::runtime_error( "inputs differ in size" );
				}

				if( !dimensionsCompatible( a, t ) )
				{
					safeDelete( a );
					safeDelete( b );
					safeDelete( t );
					throw std::runtime_error( "inputs differ in size" );
				}

				if( t )
				{
					uint32_t ts = max( t->timeStamp(), max( a ? a->timeStamp() : 0, b ? b->timeStamp() : 0 ) );

					SampleFrame *ret = new SampleFrame( t->width(), t->height(), ts, t->depth() );

					if( _clamp )
						t->clamp01();

					if( b )
					{
						b->mul( t );
						ret->add( b );
					}
					if( a )
					{
						a->mul( t->mul( -1 )->add( 1 ) );
						ret->add( a );
					}

					drawFrame( ret );

					pushOutput( "out", ret );
					safeDelete( ret );
				}

				safeDelete( a );
				safeDelete( b );
				safeDelete( t );
			}

			return inputPending( "a" ) || inputPending( "b" ) || inputPending( "t" );
		}

		bool Mix::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "clamp", _clamp );

			return ret;
		}

		bool Mix::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "clamp", _clamp );

			return ret;
		}





		Abs::Abs() :
			Op()
		{}

		Abs::~Abs()
		{}

		bool Abs::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->abs();

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}





		PowConst::PowConst() :
			Op(),
			_value( 2.0f )
		{}

		PowConst::~PowConst()
		{}

#ifdef __SUPPORT_GUI
		bool PowConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			float v = _value;
			if( ImGui::SliderFloat( "p", &v, 0, 10, "%.2f" ) )
				_value = v;

			return true;
		}
#endif

		bool PowConst::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->pow( _value );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool PowConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "p", _value );

			return ret;
		}

		bool PowConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "p", _value );

			return ret;
		}





		Sqrt::Sqrt() :
			Op()
		{}

		Sqrt::~Sqrt()
		{}

		bool Sqrt::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->sqrt();

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}




		Log::Log() :
			Op()
		{}

		Log::~Log()
		{}

		bool Log::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->log();

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}





		Exp::Exp() :
			Op()
		{}

		Exp::~Exp()
		{}

		bool Exp::process()
		{
			SampleFrame *in = fetchInput<SampleFrame>( "in" );
			SampleFrame *base = fetchInput<SampleFrame>( "base" );

			if( in || base )
			{
				SampleFrame *ret = nullptr;

				if( in )
				{
					if( base )
					{
						if( !dimensionsCompatible( in, base ) )
						{
							safeDelete( in );
							safeDelete( base );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( in->mat(), max( in->timeStamp(), base->timeStamp() ) );
						ret->exp( base );
					}
					else
						ret = in;

					drawFrame( ret );
					pushOutput( "out", ret );

					if( ret != in )
						safeDelete( ret );
				}
				else
					ret = nullptr;

				safeDelete( in );
				safeDelete( base );
			}

			return inputPending( "in" ) || inputPending( "base" );
		}

		void Exp::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "base", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}





		ExpConst::ExpConst() :
			Op(),
			_euler( true ),
			_base( 2.0f )
		{}

		ExpConst::~ExpConst()
		{}

		bool ExpConst::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( _euler )
					sf->exp();
				else
					sf->exp( _base );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool ExpConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "euler", &_euler );

			{
				ScopedImGuiDisable disable( _euler );

				ImGui::SliderFloat( "base", &_base, 0, 10 );
			}

			return true;
		}
#endif

		bool ExpConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "euler", _euler );
			load<float>( j, "base", _base );

			return ret;
		}

		bool ExpConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "euler", _euler );
			save( j, "base", _base );

			return ret;
		}



		namespace Internal
		{
			float sigmoid( float x )
			{
				return 1.0 / ( 1.0 + exp( -x ) );
			}
		}

		Sigmoid::Sigmoid() :
			Op()
		{}

		Sigmoid::~Sigmoid()
		{}

		bool Sigmoid::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->transform( Internal::sigmoid );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}





		Sum::Sum() :
			Op(),
			_dimsMask( 0x07 )
		{}

		Sum::~Sum()
		{}

#ifdef __SUPPORT_GUI
		bool Sum::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			const char *labels[] = { "x", "y", "z" };
			for( int i = 0; i < 3; i++ )
			{
				bool b = ( _dimsMask & ( 1 << i ) );
				if( ImGui::Checkbox( labels[i], &b ) )
				{
					if( b )
						_dimsMask |= ( 1 << i );
					else
						_dimsMask &= ~( 1 << i );
				}
			}

			return true;
		}
#endif

		bool Sum::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame *ret = nullptr;// new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );

				//NOTE: this does exactly what you think it does (consider dim=0 -> y and dim=1 -> x ), but DOES NOT CONSIDER LAYERS!!
				// which means you have to split. also, reducing does not work along z, you would have to program this on your own
				//cv::reduce( sf->mat(), ret->mat(), 

				//TODO: this can be optimized

				if( _dimsMask & ( 1 << 0 ) && sf->width() > 1 )
					ret = Internal::reduceX( sf, Internal::RM_SUM );

				if( _dimsMask & ( 1 << 1 ) && sf->height() > 1 )
				{
					SampleFrame *in = ( ret ? ret : sf );
					ret = Internal::reduceY( in, Internal::RM_SUM );

					if( in != sf )
						safeDelete( in );
				}
				if( _dimsMask & ( 1 << 2 ) && sf->depth() > 1 )
				{
					SampleFrame *in = ( ret ? ret : sf );
					ret = Internal::reduceZ( in, Internal::RM_SUM );

					if( in != sf )
						safeDelete( in );
				}

				if( !ret )
					ret = new SampleFrame( *sf );

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Sum::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned char>( j, "dimsMask", _dimsMask );

			return ret;
		}

		bool Sum::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "dimsMask", (int)_dimsMask );

			return ret;
		}




		Product::Product() :
			Op(),
			_dimsMask( 0 )
		{}

		Product::~Product()
		{}

#ifdef __SUPPORT_GUI
		bool Product::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			const char *labels[] = { "x", "y", "z" };
			for( int i = 0; i < 3; i++ )
			{
				bool b = ( _dimsMask & ( 1 << i ) );
				if( ImGui::Checkbox( labels[i], &b ) )
				{
					if( b )
						_dimsMask |= ( 1 << i );
					else
						_dimsMask &= ~( 1 << i );
				}
			}

			return true;
		}
#endif

		bool Product::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame *ret = nullptr;// new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );

				//NOTE: this does exactly what you think it does (consider dim=0 -> y and dim=1 -> x ), but DOES NOT CONSIDER LAYERS!!
				// which means you have to split. also, reducing does not work along z, you would have to program this on your own
				//cv::reduce( sf->mat(), ret->mat(), 

				//TODO: this can be optimized

				if( _dimsMask & ( 1 << 0 ) && sf->width() > 1 )
					ret = Internal::reduceX( sf, Internal::RM_PRODUCT );

				if( _dimsMask & ( 1 << 1 ) && sf->height() > 1 )
				{
					SampleFrame *in = ( ret ? ret : sf );
					ret = Internal::reduceY( in, Internal::RM_PRODUCT );

					if( in != sf )
						safeDelete( in );
				}
				if( _dimsMask & ( 1 << 2 ) && sf->depth() > 1 )
				{
					SampleFrame *in = ( ret ? ret : sf );
					ret = Internal::reduceZ( in, Internal::RM_PRODUCT );

					if( in != sf )
						safeDelete( in );
				}

				if( !ret )
					ret = new SampleFrame( *sf );

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Product::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned char>( j, "dimsMask", _dimsMask );

			return ret;
		}

		bool Product::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "dimsMask", (int) _dimsMask );

			return ret;
		}






		Invert::Invert() :
			Op()
		{}

		Invert::~Invert()
		{}

		bool Invert::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );
				ret->set( 1.0f )->sub( sf );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool Invert::drawUI()
		{
			return Op::drawUI();
		}
#endif

		bool Invert::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			return ret;
		}

		bool Invert::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			return ret;
		}





		Threshold::Threshold( float threshold ) :
			Op(),
			_binary( true ),
			_threshold( threshold )
		{}

		Threshold::~Threshold()
		{}

#ifdef __SUPPORT_GUI
		bool Threshold::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "threshold", &_threshold, 0.0f, 1.0f );
			ImGui::Checkbox( "binary", &_binary );

			return true;
		}
#endif

		bool Threshold::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->threshold( _threshold, _binary );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Threshold::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "binary", _binary );
			load<float>( j, "threshold", _threshold );

			return ret;
		}

		bool Threshold::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "binary", _binary );
			save( j, "threshold", _threshold );

			return ret;
		}





		Remap::Remap( float minValue, float maxValue, bool clamp ) :
			Op(),
			_minValue( minValue ),
			_maxValue( maxValue ),
			_clamp( clamp )
		{}

		Remap::~Remap()
		{}

#ifdef __SUPPORT_GUI
		bool Remap::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "min", &_minValue, -10.0f, 10.0f );
			ImGui::SliderFloat( "max", &_maxValue, -10.0f, 10.0f );

			ImGui::Checkbox( "clamp", &_clamp );

			_maxValue = std::max( _maxValue, _minValue );

			return true;
		}
#endif

		bool Remap::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->sub( _minValue );
				sf->mul( 1.0f / ( _maxValue - _minValue ) );

				if( _clamp )
					sf->clamp01();

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Remap::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "min", _minValue );
			load<float>( j, "max", _maxValue );
			load<bool>( j, "clamp", _clamp );

			return ret;
		}

		bool Remap::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "min", _minValue );
			save( j, "max", _maxValue );
			save( j, "clamp", _clamp );

			return ret;
		}






		Normalize::Normalize() :
			Op(),
			_minValue( 0.0f ),
			_maxValue( 1.0f )
		{}

		Normalize::~Normalize()
		{}

#ifdef __SUPPORT_GUI
		bool Normalize::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "min", &_minValue, -5, 5, "%.2f" );
			ImGui::SliderFloat( "max", &_maxValue, -5, 5, "%.2f" );

			if( _maxValue - _minValue <= 0.0001f )
				_maxValue = _minValue + 0.0001f;

			return true;
		}
#endif

		bool Normalize::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->sub( _minValue )->mul( 1.0f / ( _maxValue - _minValue ) )->clamp01();

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Normalize::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "min", _minValue );
			load<float>( j, "max", _maxValue );

			return ret;
		}

		bool Normalize::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "min", _minValue );
			save( j, "max", _maxValue );

			return ret;
		}



		AutoNormalize::AutoNormalize( Mode mode ) :
			Op(),
			_mode( mode ),
			_isCalibrationInProgress( false ),
			_globalMin( std::numeric_limits<float>().max() ),
			_globalMax( std::numeric_limits<float>().lowest() ),
			_globalScale( 0.0f ),
			_individualMin( nullptr ),
			_individualMax( nullptr ),
			_individualScale( nullptr )
		{}

		AutoNormalize::~AutoNormalize()
		{
			this->clear();
		}

		bool AutoNormalize::reset()
		{
			_globalMin = std::numeric_limits<float>().max();
			_globalMax = std::numeric_limits<float>().lowest();
			_globalScale = 0.0f;

			this->clear();

			return true;
		}

		bool AutoNormalize::clear()
		{
			safeDelete( _individualMin );
			safeDelete( _individualMax );
			safeDelete( _individualScale );

			return true;
		}

#ifdef __SUPPORT_GUI
		bool AutoNormalize::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "learn", &_isCalibrationInProgress );
			if( ImGui::Button( "reset" ) )
				this->reset();

			int m = _mode;
			for( int i = 0; i < M_COUNT; i++ )
				ImGui::RadioButton( modeToString( (Mode) i ), &m, i );
			_mode = (Mode) m;

			return true;
		}
#endif

		bool AutoNormalize::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( _isCalibrationInProgress && sf->size() )
				{
					switch( _mode )
					{
					case M_GLOBAL:
					{
						//find new global minimum in received frame
						float localMin = sf->minValue();
						if( _globalMin > localMin )
							_globalMin = localMin;

						//find new global maximum in received frame
						float localMax = sf->maxValue();
						if( _globalMax < localMax )
							_globalMax = localMax;

						float div = _globalMax - _globalMin;

						if( std::abs( div ) > std::numeric_limits<float>::epsilon() )
							_globalScale = 1.0f / div;
						else
						{
							std::cerr << "<warning> range too low" << std::endl;
							_globalScale = 1.0f;
						}

						break;
					}
					case M_INDIVIDUALLY:
					{
						if( !_individualMin )
						{
							_individualMin = new SampleFrame( sf->width(), sf->height(), 0, sf->depth() );
							_individualMin->set( std::numeric_limits<float>().max() );
						}
						if( !_individualMax )
						{
							_individualMax = new SampleFrame( sf->width(), sf->height(), 0, sf->depth() );
							_individualMax->set( std::numeric_limits<float>().lowest() );
						}
						//if( !_individualScale )
						//{
						//	_individualScale = new SampleFrame( sf->width(), sf->height(), 0, sf->depth() );
						//	_individualScale->set( 0.0f );
						//}

						for( int i = 0; i < sf->size(); i++ )
						{
							if( _individualMin->values()[i] > sf->values()[i] )
								_individualMin->values()[i] = sf->values()[i];
							if( _individualMax->values()[i] < sf->values()[i] )
								_individualMax->values()[i] = sf->values()[i];
						}

						rebuildIndividualScale();
						break;
					}
					default:
						throw std::runtime_error( "unknown mode" );
					}
				}

				if( sf->size() )
				{
					switch( _mode )
					{
					case M_GLOBAL:
					{
						if( std::abs( _globalScale ) > std::numeric_limits<float>::epsilon() )
							sf->sub( _globalMin )->mul( _globalScale );
					}
					break;
					case M_INDIVIDUALLY:
					{
						if( _individualMin && _individualScale && _individualMin->size() && _individualScale->size() )
							sf->sub( _individualMin )->mul( _individualScale );
					}
					break;
					}
				}

				drawFrame( sf );

				pushOutput( "out", sf );
			}

			safeDelete( sf );

			return inputPending( "in" );
		}

		bool AutoNormalize::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string str;
			if( load<std::string>( j, "mode", str ) )
				_mode = modeFromString( str );

			load<float>( j, "globalMin", _globalMin );
			load<float>( j, "globalMax", _globalMax );
			load<float>( j, "globalScale", _globalScale );

			safeDelete( _individualMin );
			safeDelete( _individualMax );

			load<SampleFrame*>( j, "individualMin", _individualMin );
			load<SampleFrame*>( j, "individualMax", _individualMax );

			rebuildIndividualScale();

			return ret;
		}

		bool AutoNormalize::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "mode", modeToString( _mode ) );

			save( j, "globalMin", _globalMin );
			save( j, "globalMax", _globalMax );
			save( j, "globalScale", _globalScale );

			save( j, "individualMin", *_individualMin );
			save( j, "individualMax", *_individualMax );

			return ret;
		}

		const char *AutoNormalize::modeToString( AutoNormalize::Mode mode )
		{
			switch( mode )
			{
			case M_GLOBAL:
				return "global";
			case M_INDIVIDUALLY:
				return "individually";
			}
			return "UNKNOWN";
		}

		AutoNormalize::Mode AutoNormalize::modeFromString( const char *s )
		{
			if( !s )
				return AutoNormalize::M_COUNT;

			for( int i = 0; i < AutoNormalize::M_COUNT; i++ )
				if( !_stricmp( s, modeToString( ( AutoNormalize::Mode )i ) ) )
					return ( AutoNormalize::Mode )i;

			return AutoNormalize::M_COUNT;
		}

		AutoNormalize::Mode AutoNormalize::modeFromString( const std::string &s )
		{
			return modeFromString( s.c_str() );
		}

		void AutoNormalize::rebuildIndividualScale()
		{
			if( !_individualMin || !_individualMax )
			{
				std::cerr << "<error> cannot calc scale, min or max are null" << std::endl;
				return;
			}

			if( _individualScale && 
				( _individualScale->width() != _individualMin->width() || _individualScale->height() != _individualMin->height() || _individualScale->depth() != _individualMin->depth() ) )
				safeDelete( _individualScale );

			if( !_individualScale )
				_individualScale = new SampleFrame( _individualMin->width(), _individualMin->height(), 0, _individualMin->depth() );

			for( int i = 0; i < _individualScale->size(); i++ )
			{
				float div = _individualMax->values()[i] - _individualMin->values()[i];

				if( std::abs( div ) < 0.000001f )
					_individualScale->values()[i] = 1.0f;
				else
					_individualScale->values()[i] = 1.0f / div;
			}
		}





		MinConst::MinConst() :
			Op(),
			_val( 0.0f )
		{}

		MinConst::~MinConst()
		{}

#ifdef __SUPPORT_GUI
		bool MinConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "v", &_val, -1, 1, "%.2f" );

			return true;
		}
#endif

		bool MinConst::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->mat() = cv::min( sf->mat(), _val );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool MinConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "val", _val );

			return ret;
		}

		bool MinConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "val", _val );

			return ret;
		}







		Min::Min() :
			Op()
		{}

		Min::~Min()
		{}

		bool Min::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				SampleFrame *ret = nullptr;

				if( a )
				{
					if( b )
					{
						if( !dimensionsCompatible( a, b ) )
						{
							safeDelete( a );
							safeDelete( b );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( a->mat(), max( a->timeStamp(), b->timeStamp() ) );
						cv::min( a->mat(), b->mat(), ret->mat() );
					}
					else
						ret = a;
				}
				else
					ret = b;

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != a && ret != b )
					safeDelete( ret );
				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		void Min::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}




		MaxConst::MaxConst() :
			Op(),
			_val( 1.0f )
		{}

		MaxConst::~MaxConst()
		{}

#ifdef __SUPPORT_GUI
		bool MaxConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "v", &_val, -1, 1, "%.2f" );

			return true;
		}
#endif

		bool MaxConst::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->mat() = cv::max( sf->mat(), _val );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool MaxConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "val", _val );

			return ret;
		}

		bool MaxConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "val", _val );

			return ret;
		}





		Max::Max() :
			Op()
		{}

		Max::~Max()
		{}

		bool Max::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				SampleFrame *ret = nullptr;

				if( a )
				{
					if( b )
					{
						if( !dimensionsCompatible( a, b ) )
						{
							safeDelete( a );
							safeDelete( b );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( a->mat(), max( a->timeStamp(), b->timeStamp() ) );
						cv::max( a->mat(), b->mat(), ret->mat() );
					}
					else
						ret = a;
				}
				else
					ret = b;

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != a && ret != b )
					safeDelete( ret );
				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		void Max::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}






		ClampConst::ClampConst() :
			Op(),
			_min( 0.0f ),
			_max( 1.0f )
		{}

		ClampConst::~ClampConst()
		{}

#ifdef __SUPPORT_GUI
		bool ClampConst::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "min", &_min, -1, 1, "%.2f" );
			ImGui::SliderFloat( "max", &_max, -1, 1, "%.2f" );

			return true;
		}
#endif

		bool ClampConst::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->clamp( _min, _max );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool ClampConst::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "min", _min );
			load<float>( j, "max", _max );

			return ret;
		}

		bool ClampConst::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "min", _min );
			save( j, "max", _max );

			return ret;
		}





		Clamp::Clamp() :
			Op()
		{}

		Clamp::~Clamp()
		{}

		bool Clamp::process()
		{
			SampleFrame *in = fetchInput<SampleFrame>( "in" );
			SampleFrame *mn = fetchInput<SampleFrame>( "min" );
			SampleFrame *mx = fetchInput<SampleFrame>( "max" );

			if( in || mn || mx )
			{
				SampleFrame *ret = nullptr;

				if( in )
				{
					if( mn && mx )
					{
						if( !dimensionsCompatible( in, mn ) )
						{
							safeDelete( in );
							safeDelete( mn );
							safeDelete( mx );
							throw std::runtime_error( "inputs differ in size" );
						}
						if( !dimensionsCompatible( in, mx ) )
						{
							safeDelete( in );
							safeDelete( mn );
							safeDelete( mx );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( in->mat(), max( in->timeStamp(), max( mn->timeStamp(), mx->timeStamp() ) ) );
						cv::max( in->mat(), mn->mat(), in->mat() );
						cv::min( in->mat(), mx->mat(), ret->mat() );
					}
					else
						ret = in;
				}
				else
					ret = in;

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != in )
					safeDelete( in );

				safeDelete( in );
				safeDelete( mn );
				safeDelete( mx );
			}

			return inputPending( "in" ) || inputPending( "min" ) || inputPending( "max" );
		}

		void Clamp::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "min", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "max", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}





		InRange::InRange( float minValue, float maxValue ) :
			Op(),
			_minValue( minValue ),
			_maxValue( maxValue )
		{}

		InRange::~InRange()
		{}

#ifdef __SUPPORT_GUI
		bool InRange::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "min", &_minValue, -10.0f, 10.0f );
			ImGui::SliderFloat( "max", &_maxValue, -10.0f, 10.0f );

			_maxValue = std::max( _maxValue, _minValue );

			return true;
		}
#endif

		bool InRange::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->inRange( _minValue, _maxValue );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool InRange::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "min", _minValue );
			load<float>( j, "max", _maxValue );

			return ret;
		}

		bool InRange::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "min", _minValue );
			save( j, "max", _maxValue );

			return ret;
		}





		const char *Approx::modeToString( Approx::Mode mode )
		{
			switch( mode )
			{
			case Approx::M_ROUND:
				return "round";
			case Approx::M_FLOOR:
				return "floor";
			case Approx::M_CEIL:
				return "ceil";
			}
			return "UNKNOWN";
		}

		Approx::Mode Approx::modeFromString( const char *s )
		{
			if( !s )
				return Approx::M_COUNT;

			for( int i = 0; i < Approx::M_COUNT; i++ )
				if( !_stricmp( s, Approx::modeToString( ( Approx::Mode )i ) ) )
					return ( Approx::Mode )i;

			return Approx::M_COUNT;
		}

		Approx::Mode Approx::modeFromString( const std::string &s )
		{
			return Approx::modeFromString( s.c_str() );
		}


		Approx::Approx() :
			Op(),
			_mode( Approx::M_ROUND )
		{}

		Approx::~Approx()
		{}

#ifdef __SUPPORT_GUI
		bool Approx::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int m = _mode;
			for( int i = 0; i < M_COUNT; i++ )
				ImGui::RadioButton( modeToString( (Mode) i ), &m, i );
			_mode = (Mode) m;

			return true;
		}
#endif

		bool Approx::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				switch( _mode )
				{
				case M_ROUND:
					sf->transform( roundf );
					break;
				case M_FLOOR:
					sf->transform( floorf );
					break;
				case M_CEIL:
					sf->transform( ceilf );
					break;
				}

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Approx::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			std::string s;
			if( load<std::string>( j, "mode", s ) )
				_mode = modeFromString( s );

			return ret;
		}

		bool Approx::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "mode", modeToString( _mode ) );

			return ret;
		}





		And::And() :
			Op()
		{}

		And::~And()
		{}

		bool And::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				SampleFrame *ret = nullptr;

				if( a )
				{
					if( b )
					{
						if( !dimensionsCompatible( a, b ) )
						{
							safeDelete( a );
							safeDelete( b );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( a->mat(), max( a->timeStamp(), b->timeStamp() ) );
						ret->logAnd( b );
					}
					else
					{
						ret = a;
						ret->set( 0.0f );
					}
				}
				else
				{
					ret = b;
					ret->set( 0.0f );
				}

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != a && ret != b )
					safeDelete( ret );
				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		void And::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}






		Or::Or() :
			Op()
		{}

		Or::~Or()
		{}

		bool Or::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				SampleFrame *ret = nullptr;

				if( a )
				{
					if( b )
					{
						if( !dimensionsCompatible( a, b ) )
						{
							safeDelete( a );
							safeDelete( b );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( a->mat(), max( a->timeStamp(), b->timeStamp() ) );
						ret->logOr( b );
					}
					else
						ret = a;
				}
				else
					ret = b;

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != a && ret != b )
					safeDelete( ret );
				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		void Or::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}





		XOr::XOr() :
			Op()
		{}

		XOr::~XOr()
		{}

		bool XOr::process()
		{
			SampleFrame *a = fetchInput<SampleFrame>( "a" );
			SampleFrame *b = fetchInput<SampleFrame>( "b" );

			if( a || b )
			{
				SampleFrame *ret = nullptr;

				if( a )
				{
					if( b )
					{
						if( !dimensionsCompatible( a, b ) )
						{
							safeDelete( a );
							safeDelete( b );
							throw std::runtime_error( "inputs differ in size" );
						}

						ret = new SampleFrame( a->mat(), max( a->timeStamp(), b->timeStamp() ) );
						ret->logXOr( b );
					}
					else
						ret = a;
				}
				else
					ret = b;

				drawFrame( ret );
				pushOutput( "out", ret );

				if( ret != a && ret != b )
					safeDelete( ret );
				safeDelete( a );
				safeDelete( b );
			}

			return inputPending( "a" ) || inputPending( "b" );
		}

		void XOr::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "a", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "b", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}







		Determinant::Determinant() :
			Op()
		{}

		Determinant::~Determinant()
		{}

		bool Determinant::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( sf->width() != sf->height() )
				{
					safeDelete( sf );
					throw std::runtime_error( "input frame must be square" );
				}

				SampleFrame *ret = new SampleFrame( 1, 1, sf->timeStamp(), sf->depth() );

				float *f = ret->values();
			
				if( sf->depth() > 1 )
				{
					std::vector<cv::Mat> layers;
					cv::split( sf->mat(), layers );
					for( int i = 0; i < sf->depth(); i++ )
						f[i] = cv::determinant( layers[i] );
				}
				else
					f[0] = cv::determinant( sf->mat() );

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );

				safeDelete( ret );
			}

			return inputPending( "in" );
		}






		LinearFunction::LinearFunction() :
			Op(),
			_p0( 0, 0 ),
			_p1( 1, 1 )/*,
			_current( 0 )*/
		{}

		LinearFunction::~LinearFunction()
		{}

#ifdef __SUPPORT_GUI
		bool LinearFunction::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::InputFloat2( "p0", glm::value_ptr( _p0 ), "%.03f" );
			ImGui::InputFloat2( "p1", glm::value_ptr( _p1 ), "%.03f" );

			/*
			//TODO: setting from current value(s) via UI is broken since non-single-value frames are supported
			// fix/complete this at some point
			ImGui::SameLine();

			sprintf( tempStr, "set##%d", i );
			if( ImGui::Button( tempStr ) )
			{
				_points[i].first = _current;

				reorder();
			}
			*/

			return true;
		}
#endif

		bool LinearFunction::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				float dx = _p1.x - _p0.x;

				float *v = sf->values();

				if( abs( dx ) < 0.0001 )
				{
					//if gradient is infinite, make hard edge/rect:
					// set to y0 for values below
					// set to y1 for values above
					for( int i = 0; i < sf->size(); i++ )
						v[i] = v[i] < _p0.x ? _p0.y : _p1.y;
				}
				else
				{
					float s = ( _p1.y - _p0.y ) / dx;

					for( int i = 0; i < sf->size(); i++ )
						v[i] = ( v[i] - _p0.x ) * s;
				}

				drawFrame( sf );

				pushOutput( "out", sf );

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool LinearFunction::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<glm::vec2>( j, "p0", _p0 );
			load<glm::vec2>( j, "p1", _p1 );

			return ret;
		}

		bool LinearFunction::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "p0", _p0 );
			save( j, "p1", _p1 );

			return ret;
		}






		MultiLinearFunction::MultiLinearFunction() :
			Op(),
			_numPoints( 0 )/*,
			_current( 0.0f )*/
		{}

		MultiLinearFunction::~MultiLinearFunction()
		{}

#ifdef __SUPPORT_GUI
		bool MultiLinearFunction::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int n = _numPoints;
			if( ImGui::InputInt( "point count", &n ) )
			{
				n = max<int>( 0, n );

				std::vector<std::pair<float, float>> newPoints( n );
				for( int i = 0; i < n; i++ )
				{
					if( i < _numPoints )
						newPoints[i] = _points[i];
					else if( _numPoints > 0 )
						newPoints[i] = _points[_numPoints - 1];
					else
					{
						newPoints[i].first = 0.0f;
						newPoints[i].second = 0.0f;
					}
				}

				_points = newPoints;
				_numPoints = n;
			}

			if( ImGui::TreeNode( "points" ) )
			{
				for( int i = 0; i < _numPoints; i++ )
				{
					float f[2] =
					{
						_points[i].first,
						_points[i].second
					};

					char tempStr[64];
					sprintf( tempStr, "#%d", i );

					//TODO: color text red as long as input is not applied with Return (e.g., use ScopedImGuiStyleColor, see oscOut)
					if( ImGui::InputFloat2( tempStr, f, "%.03f", ImGuiInputTextFlags_EnterReturnsTrue ) )
					{
						_points[i].first = f[0];
						_points[i].second = f[1];

						reorder();
					}

					/*
					//TODO: setting from current value(s) via UI is broken since non-single-value frames are supported
					// fix/complete this at some point
					ImGui::SameLine();

					sprintf( tempStr, "set##%d", i );
					if( ImGui::Button( tempStr ) )
					{
						_points[i].first = _current;

						reorder();
					}
					*/
				}

				ImGui::TreePop();
			}

			return true;
		}
#endif

		bool MultiLinearFunction::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( _numPoints )
				{
					float *v = sf->values();
					for( int i = 0; i < sf->size(); i++ )
						v[i] = multiLinearInterpolation( _points, v[i] );

					drawFrame( sf );

					pushOutput( "out", sf );
				}

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool MultiLinearFunction::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "numPoints", _numPoints );
			_points.resize( _numPoints );

			char tempStr[64];
			for( int i = 0; i < _numPoints; i++ )
			{
				sprintf( tempStr, "x%03d", i );
				load<float>( j, tempStr, _points[i].first );

				sprintf( tempStr, "y%03d", i );
				load<float>( j, tempStr, _points[i].second );
			}
			reorder();

			return ret;
		}

		bool MultiLinearFunction::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "numPoints", _numPoints );

			char tempStr[64];
			for( int i = 0; i < _numPoints; i++ )
			{
				sprintf( tempStr, "x%03d", i );
				save( j, tempStr, _points[i].first );

				sprintf( tempStr, "y%03d", i );
				save( j, tempStr, _points[i].second );
			}

			return ret;
		}

		void MultiLinearFunction::reorder()
		{
			std::sort( _points.begin(), _points.end(), [] ( const std::pair<float, float> &lhs, const std::pair<float, float> &rhs )
				{
					return lhs.first < rhs.first;
				} );
		}






		FFT2D::FFT2D() :
			Op(),
			_useLog( true ),
			_rearrange( true ),
			_normalize( true )
		{}

		FFT2D::~FFT2D()
		{}

#ifdef __SUPPORT_GUI
		bool FFT2D::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "log scale", &_useLog );
			ImGui::Checkbox( "rearrange", &_rearrange );
			ImGui::Checkbox( "normalize", &_normalize );

			return true;
		}
#endif

		bool FFT2D::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				std::vector<cv::Mat> layers;

				cv::split( sf->mat(), layers );

				for( auto &it : layers )
				{
					//https://docs.opencv.org/2.4/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html
					cv::Mat padded;								//expand input image to optimal size
					int optCols = cv::getOptimalDFTSize( it.cols );
					int optRows = cv::getOptimalDFTSize( it.rows );
					// on the border add zero values
					cv::copyMakeBorder( it, padded, 0, optRows - it.rows, 0, optCols - it.cols, cv::BORDER_CONSTANT, cv::Scalar::all( 0 ) );

					cv::Mat planes[] = { cv::Mat_<float>( padded ), cv::Mat::zeros( padded.size(), CV_32F ) };
					cv::Mat complexI;
					cv::merge( planes, 2, complexI );			// Add to the expanded another plane with zeros

					cv::dft( complexI, complexI );				// this way the result may fit in the source matrix

					// compute the magnitude and switch to logarithmic scale
					// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
					split( complexI, planes );					// planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
					magnitude( planes[0], planes[1], planes[0] );// planes[0] = magnitude
					cv::Mat magI = planes[0];

					if( _useLog )
					{
						magI += cv::Scalar::all( 1 );				// switch to logarithmic scale
						cv::log( magI, magI );
					}

					if( _rearrange )
					{
						if( magI.cols > 1 && magI.rows > 1 )
						{
							// crop the spectrum, if it has an odd number of rows or columns
							magI = magI( cv::Rect( 0, 0, magI.cols & -2, magI.rows & -2 ) );

							// rearrange the quadrants of Fourier image  so that the origin is at the image center
							int cx = magI.cols / 2;
							int cy = magI.rows / 2;

							cv::Mat q0( magI, cv::Rect( 0, 0, cx, cy ) );		// Top-Left - Create a ROI per quadrant
							cv::Mat q1( magI, cv::Rect( cx, 0, cx, cy ) );		// Top-Right
							cv::Mat q2( magI, cv::Rect( 0, cy, cx, cy ) );		// Bottom-Left
							cv::Mat q3( magI, cv::Rect( cx, cy, cx, cy ) );		// Bottom-Right

							cv::Mat tmp;										// swap quadrants (Top-Left with Bottom-Right)
							q0.copyTo( tmp );
							q3.copyTo( q0 );
							tmp.copyTo( q3 );

							q1.copyTo( tmp );									// swap quadrant (Top-Right with Bottom-Left)
							q2.copyTo( q1 );
							tmp.copyTo( q2 );
						}
						else if( magI.cols > 1 )
						{
							// crop the spectrum, if it has an odd number of columns
							magI = magI( cv::Rect( 0, 0, magI.cols & -2, magI.rows ) );

							int cx = magI.cols / 2;

							cv::Mat h0( magI, cv::Rect( 0, 0, cx, 1 ) );		// Left - Create a ROI per half
							cv::Mat h1( magI, cv::Rect( cx, 0, cx, 1 ) );		// Right

							cv::Mat tmp;										// swap quadrants (Left with Right)
							h0.copyTo( tmp );
							h1.copyTo( h0 );
							tmp.copyTo( h1 );
						}
						else if( magI.rows > 1 )
						{
							// crop the spectrum, if it has an odd number of rows
							magI = magI( cv::Rect( 0, 0, magI.cols, magI.rows & -2 ) );

							int cy = magI.rows / 2;

							cv::Mat h0( magI, cv::Rect( 0, 0, 1, cy ) );		// Top - Create a ROI per half
							cv::Mat h1( magI, cv::Rect( 0, cy, 1, cy ) );		// Bottom

							cv::Mat tmp;										// swap quadrants (Top with Bottom)
							h0.copyTo( tmp );
							h1.copyTo( h0 );
							tmp.copyTo( h1 );
						}
					}

					if( _normalize )
					{
						cv::normalize( magI, magI, 0, 1, cv::NORM_MINMAX );	// Transform the matrix with float values into a
																			// viewable image form (float between values 0 and 1).
					}

					it = magI;
				}

				cv::Mat m;
				cv::merge( layers, m );

				SampleFrame *ret = new SampleFrame( m, sf->timeStamp() );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool FFT2D::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "useLog", _useLog );
			load<bool>( j, "rearrange", _rearrange );
			load<bool>( j, "normalize", _normalize );

			return ret;
		}

		bool FFT2D::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "useLog", _useLog );
			save( j, "rearrange", _rearrange );
			save( j, "normalize", _normalize );

			return ret;
		}




#ifdef __FFTW_SUPPORT
#pragma comment( lib, "fftw3.lib" )
#pragma comment( lib, "fftw3f.lib" )

		FFT1D::FFT1D() :
			Op(),
			_n( 0 ),
			_in( nullptr ),
			_out( nullptr ),
			_plan( nullptr )
		{}

		FFT1D::~FFT1D()
		{
			clear();
		}

#ifdef __SUPPORT_GUI
		bool FFT1D::drawUI()
		{
			bool ret = Op::drawUI();

			return ret;
		}
#endif

		void FFT1D::clear()
		{
			if( _plan )
				fftwf_destroy_plan( _plan );
			if( _in )
				fftwf_free( _in );
			if( _out )
				fftwf_free( _out );
		}

		void FFT1D::checkPlan( const SampleFrame *sf )
		{
			if( !sf )
				return;

			if( sf->width() != _n )
				clear();

			_n = sf->width();

			if( !_n )
				return;

			if( !_plan )
			{
				size_t size = sizeof( fftwf_complex ) * _n;

				_in = (fftwf_complex*) fftwf_malloc( size );
				_out = (fftwf_complex*) fftwf_malloc( size );

				memset( _in, 0, size );
				memset( _out, 0, size );

				_plan = fftwf_plan_dft_1d( _n, _in, _out, FFTW_FORWARD, FFTW_ESTIMATE );
			}
		}

		void FFT1D::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "mag", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "phase", this ) );
		}

		bool FFT1D::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				checkPlan( sf );

				if( _plan )
				{
					SampleFrame *mag = new SampleFrame( sf->width() / 2 + 1, sf->height(), sf->timeStamp(), sf->depth() );
					SampleFrame *phase = new SampleFrame( sf->width() / 2 + 1, sf->height(), sf->timeStamp(), sf->depth() );

					int strideIn = sf->width() * sf->depth();
					int strideOut = mag->width() * mag->depth();

					for( int j = 0; j < sf->height(); j++ )
					{
						for( int k = 0; k < sf->depth(); k++ )
						{
							const float *ptrIn = sf->values() + j * strideIn;

							float *ptrMag = mag->values() + j * strideOut;
							float *ptrPhase = phase->values() + j * strideOut;

							for( int i = 0; i < sf->width(); i++ )
								_in[i][0] = ptrIn[i * sf->depth() + k];

							fftwf_execute( _plan );

							for( int i = 0; i < mag->width(); i++ )
							{
								float a = _out[i][0];
								float b = _out[i][1];

								ptrMag[i * mag->depth() + k] = sqrt( a * a + b * b );
								if( abs( a ) > std::numeric_limits<float>::epsilon() )
									ptrPhase[i * phase->depth() + k] = atan( b / a );
								else
									ptrPhase[i * phase->depth() + k] = 0.0f;
							}
						}
					}

					drawFrame( mag );

					pushOutput( "mag", mag );
					pushOutput( "phase", phase );

					safeDelete( mag );
					safeDelete( phase );
				}

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool FFT1D::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			return ret;
		}

		bool FFT1D::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			return ret;
		}
#endif




		ToPolar::ToPolar() :
			Op(),
			_radius( 8.0 ),
			_mode( M_TARGET2SOURCE )
		{}

		ToPolar::~ToPolar()
		{}

#ifdef __SUPPORT_GUI
		bool ToPolar::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "radius", &_radius, 1.0f, 31.0f );

			int m = _mode;
			for( int i = 0; i < M_COUNT; i++ )
				ImGui::RadioButton( modeToString( (Mode) i ), &m, i );
			_mode = (Mode) m;

			return true;
		}
#endif

		bool ToPolar::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				unsigned int width = _radius * 2 + 1;
				unsigned int height = _radius * 2 + 1;

				float cx = width * 0.5f;
				float cy = height * 0.5f;

				cv::Mat m( cv::Size( width, height ), CV_MAKETYPE( CV_32F, sf->depth() ) );
				m.setTo( 0 );

				switch( _mode )
				{
				case M_SOURCE2TARGET:
				{
					for( int j = 0; j < sf->height(); j++ )
					{
						float r = ( sf->height() > 1 ? ( j / ( sf->height() - 1.0f ) ) : 1.0f ) * _radius;
						for( int i = 0; i < sf->width(); i++ )
						{
							float a = ( i - ( sf->width() * 0.5f ) + 0.5f ) / ( sf->width() * 0.5f ) * pi();

							int x = cx + sin( a ) * r;
							int y = cy - cos( a ) * r;

							if( x >= 0 && x < width && y >= 0 && y < height )
							{
								switch( sf->depth() )
								{
								case 1:
								{
									m.at<float>( y, x ) = sf->mat().at<float>( j, i );
									break;
								}
								case 2:
								{
									m.at<cv::Vec2f>( y, x ) = sf->mat().at<cv::Vec2f>( j, i );
									break;
								}
								case 3:
								{
									m.at<cv::Vec3f>( y, x ) = sf->mat().at<cv::Vec3f>( j, i );
									break;
								}
								case 4:
								{
									m.at<cv::Vec4f>( y, x ) = sf->mat().at<cv::Vec4f>( j, i );
									break;
								}
								}
							}
						}
					}
					break;
				}
				case M_TARGET2SOURCE:
				{
					//TODO: use remap function to gain speed (and even get proper interpolation along the way)
					// https://docs.opencv.org/master/da/d54/group__imgproc__transform.html#gab75ef31ce5cdfb5c44b6da5f3b908ea4
					float invPi2 = 1.0f / pi2();
					float invRadius = 1.0f / _radius;
					for( int y = 0; y < height; y++ )
					{
						for( int x = 0; x < width; x++ )
						{
							float dx = ( x + 0.5f ) - cx;
							float dy = ( y + 0.5f ) - cy;

							int j = std::sqrt( dx * dx + dy * dy ) * invRadius * ( sf->height() - 1.0f ) + 0.5f;
							//int i = (int)( ( std::atan2( dx, -dy ) * invPi2 ) * width - 0.5f ) % width;
							int i = (int) ( std::atan2( dx, -dy ) * sf->width() * invPi2 + ( sf->width() + 1 ) * 0.5 ) % sf->width();

							if( j >= 0 && j < sf->height() )
							{
								switch( sf->depth() )
								{
								case 1:
								{
									m.at<float>( y, x ) = sf->mat().at<float>( j, i );
									break;
								}
								case 2:
								{
									m.at<cv::Vec2f>( y, x ) = sf->mat().at<cv::Vec2f>( j, i );
									break;
								}
								case 3:
								{
									m.at<cv::Vec3f>( y, x ) = sf->mat().at<cv::Vec3f>( j, i );
									break;
								}
								case 4:
								{
									m.at<cv::Vec4f>( y, x ) = sf->mat().at<cv::Vec4f>( j, i );
									break;
								}
								}
							}
						}
					}
					break;
				}
				}

				SampleFrame *ret = new SampleFrame( m, sf->timeStamp() );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool ToPolar::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "radius", _radius );
			std::string str;
			if( load<std::string>( j, "mode", str ) )
				_mode = modeFromString( str );

			return ret;
		}

		bool ToPolar::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "radius", _radius );
			save( j, "mode", modeToString( _mode ) );

			return ret;
		}


		const char *ToPolar::modeToString( ToPolar::Mode mode )
		{
			switch( mode )
			{
			case M_SOURCE2TARGET:
				return "source to target";
			case M_TARGET2SOURCE:
				return "target to source";
			}
			return "UNKNOWN";
		}

		ToPolar::Mode ToPolar::modeFromString( const char *s )
		{
			if( !s )
				return ToPolar::M_COUNT;

			for( int i = 0; i < ToPolar::M_COUNT; i++ )
				if( !_stricmp( s, modeToString( ( ToPolar::Mode )i ) ) )
					return ( ToPolar::Mode )i;

			return ToPolar::M_COUNT;
		}

		ToPolar::Mode ToPolar::modeFromString( const std::string &s )
		{
			return modeFromString( s.c_str() );
		}
	}
}