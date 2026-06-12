/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "temporal.h"

//#include <processing/pin.h>

#include <app.h>
#include <fileIO/json.h>

#include <commonImGui.h>

#include <opencv2/imgproc.hpp>
//#include <opencv2/imgproc/types_c.h>


namespace sqid
{
	namespace Temporal
	{
		DEFINE_OP_DESC( RunningAverage, "runningAvrg", "/math/temporal",
			"AC3532DA-C2EF-4D2D-B1F4-F413C9FA43EC" );
		DEFINE_OP_DESC( Slope, "slope", "/math/temporal",
			"EA1FC780-B2A9-4BEE-8575-AE5A33EDDFC0" );
		DEFINE_OP_DESC( Integral, "integral", "/math/temporal",
			"238F1226-473D-4B4F-B6A0-77FD56E68ACB" );
		DEFINE_OP_DESC( PID, "PID", "/math/temporal",
			"9B0C9707-2EB0-4551-99D8-9863E1580F06" );
		DEFINE_OP_DESC( Drag, "drag", "/math/temporal",
			"DCB23B86-8234-42C3-B7D7-06919AE420D2" );
		DEFINE_OP_DESC( BoxFilter, "box", "/math/temporal",
			"0C1DCBB7-9CFB-4716-9FB3-94C733A0538F" );
		DEFINE_OP_DESC( Median, "median", "/math/temporal",
			"3A86C458-B322-46F8-A64E-D89CF2F43D75" );
		DEFINE_OP_DESC( Mean, "mean", "/math/temporal",
			"8374EED4-C732-471A-BB3E-F94E491242C7" );
		DEFINE_OP_DESC( BGSubtraction, "bgSubtraction", "/math/temporal",
			"A80C0D9A-0C8D-4EF0-92A3-48AC7A7339F7" );
		DEFINE_OP_DESC( OpticalFlow, "opticalFlow", "/imaging",
			"8C29551C-E8CF-4724-8CD2-3D04F87F645F" );
		DEFINE_OP_DESC( Kalman, "kalman", "/math/temporal",
			"CBFE4C50-A94F-4363-8C95-B48F8F88D69B" );
		DEFINE_OP_DESC( Resample, "resample", "/math/temporal",
			"BF8D9CD1-7783-4460-9609-7F950422474E" );


		RunningAverage::RunningAverage( float drag ) :
			Op(),
			_runningAverage( nullptr ),
			_runningAverageDrag( drag )
		{}

		RunningAverage::~RunningAverage()
		{
			clear();
		}

#ifdef __SUPPORT_GUI
		bool RunningAverage::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "drag", &_runningAverageDrag, 0.001f, 0.999f );
			if( ImGui::Button( "reset" ) )
				clear();

			return true;
		}
#endif

		bool RunningAverage::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( !_runningAverage )
					_runningAverage = new SampleFrame( *sf );
				else
				{
					if( dimensionsCompatible( _runningAverage, sf ) )
						_runningAverage->blend( sf, _runningAverageDrag );
					else
						clear();
				}

				if( _runningAverage )
				{
					sf->set( _runningAverage );

					drawFrame( sf );

					pushOutput( "out", sf );
				}

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool RunningAverage::clear()
		{
			safeDelete( _runningAverage );

			return true;
		}

		bool RunningAverage::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "drag", _runningAverageDrag );

			return ret;
		}

		bool RunningAverage::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "drag", _runningAverageDrag );

			return ret;
		}






		Slope::Slope() :
			Op(),
			_lastValue( nullptr )
		{}

		Slope::~Slope()
		{
			this->clear();
		}

		bool Slope::process()
		{
			SampleFrame* sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame* ret = nullptr;

				if( _lastValue )
				{
					float dt = sf->timeStamp() - _lastValue->timeStamp();
					if( dt > 0.00001f )
					{
						ret = new SampleFrame( *sf );
						ret->sub( _lastValue )->mul( 1.0f / ( ( sf->timeStamp() - _lastValue->timeStamp() ) * 0.001f ) );

						safeDelete( _lastValue );
						_lastValue = sf;
					}
				}
				else
				{
					ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );
					_lastValue = sf;
				}

				if( ret )
				{
					drawFrame( ret );

					pushOutput( "out", ret );
					safeDelete( ret );
				}
			}

			return inputPending( "in" );
		}

		bool Slope::clear()
		{
			safeDelete( _lastValue );

			return true;
		}









		Integral::Integral() :
			Op(),
			_prevTS( 0 ),
			_integral( nullptr )
		{}

		Integral::~Integral()
		{
			clear();
		}

#ifdef __SUPPORT_GUI
		bool Integral::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( "reset" ) )
				if( _integral )
					_integral->set( 0.0f );

			return true;
		}
#endif

		bool Integral::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( !_integral )
					_integral = new SampleFrame( sf->width(), sf->height(), 0, sf->depth() );
				else
					_integral->add( sf, ( sf->timeStamp() - _prevTS ) * 0.001f );

				_prevTS = sf->timeStamp();

				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), &_integral->values()[0], sf->timeStamp(), sf->depth() );
				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Integral::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			return ret;
		}

		bool Integral::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			return ret;
		}

		bool Integral::clear()
		{
			safeDelete( _integral );

			return true;
		}






		const char *PID::antiWindupMethodToString( PID::AntiWindupMethod awm )
		{
			switch( awm )
			{
			case AWM_NONE:
				return "NONE";
			case AWM_LIMIT_INTEGRAL:
				return "LIMIT_INTEGRAL";
			case AWM_LIMIT_OUTPUT:
				return "LIMIT_OUTPUT";
			}

			return "UNKNOWN";
		}

		PID::AntiWindupMethod PID::antiWindupMethodFromString( const char *str )
		{
			if( !str )
				return AWM_COUNT;

			for( int i = 0; i < AWM_COUNT; i++ )
				if( !_stricmp( str, antiWindupMethodToString( (PID::AntiWindupMethod) i ) ) )
					return (PID::AntiWindupMethod) i;

			return AWM_COUNT;
		}

		PID::AntiWindupMethod PID::antiWindupMethodFromString( const std::string& str )
		{
			return antiWindupMethodFromString( str.c_str() );
		}

		PID::PID() :
			Op(),
			_useSystemTime( false ),
			_prevSysTime( 0.0f ),
			_kp( 0.0f ), _ki( 0.0f ), _kd( 0.0f ),
			_useKickAvoidance( true ),
			_useReversalResetsIntegral( true ),
			_reversalSlopeThreshold( 1e-6f ),
			_useDeadband( true ),
			_deadbandThresh( 0.001f ),
			_antiWindupMethod( AWM_NONE ),
			_integralCap( 1.0f ),
			_outputCap( 1.0f ),
			_applyCapToOutput( true ),
			_prevValue( nullptr ),
			_prevTarget( nullptr ),
			_prevTargetDir( nullptr ),
			_prevError( nullptr ),
			_integral( nullptr )
		{}

		PID::~PID()
		{
			clear();
		}

		void PID::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "value", this ) );
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "target", this ) );

			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool PID::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "kp", &_kp, 0, 1 );
			ImGui::SliderFloat( "ki", &_ki, 0, 1 );
			ImGui::SliderFloat( "kd", &_kd, 0, 1 );

			ImGui::Checkbox( "deadband", &_useDeadband );
			if( _useDeadband )
			{
				if( ImGui::InputFloat( "threshold", &_deadbandThresh, 0.0f, 0.0f, "%.6f" ) )
					_deadbandThresh = max( _deadbandThresh, 0.0f );
			}

			ImGui::Text( "anti-windup method" );
			int e = (int)_antiWindupMethod;
			for( int i = 0; i < AntiWindupMethod::AWM_COUNT; i++ )
				ImGui::RadioButton( antiWindupMethodToString( (AntiWindupMethod) i ), &e, i );
			_antiWindupMethod = (AntiWindupMethod)e;
			if( _antiWindupMethod == AWM_LIMIT_INTEGRAL )
				ImGui::InputFloat( "integral cap", &_integralCap );
			else if( _antiWindupMethod == AWM_LIMIT_OUTPUT )
			{
				ImGui::Checkbox( "apply to output", &_applyCapToOutput );
				ImGui::InputFloat( "output cap", &_outputCap );
			}

			ImGui::Checkbox( "kick avoidance", &_useKickAvoidance );

			ImGui::Checkbox( "reversal resets integral", &_useReversalResetsIntegral );
			if( _useReversalResetsIntegral )
			{
				ImGui::InputFloat( "slope threshold", &_reversalSlopeThreshold, 0.0f, 0.0f, "%.6f" );
				if( _reversalSlopeThreshold < 0.0f )
					_reversalSlopeThreshold = 0.0f;
				else if( _reversalSlopeThreshold > 1.0f )
					_reversalSlopeThreshold = 1.0f;
			}

			ImGui::Checkbox( "sys time", &_useSystemTime );

			if( ImGui::Button( "reset" ) )
				clear();

			return true;
		}
#endif

		SampleFrame* PID::update( const SampleFrame* value, const SampleFrame* target, float dt )
		{
			dt = max( 0.0f, dt );

			SampleFrame *error = new SampleFrame( *target );
			error->sub( value );

			if( _useDeadband )
			{
				//set all values within deadband to 0, keep the rest
				SampleFrame* mask = new SampleFrame( *error );
				mask->inRange( -_deadbandThresh, _deadbandThresh );
				mask->logNot();
				error->mul( mask );
				safeDelete( mask );
			}

			//initialize with 0
			SampleFrame* ret = new SampleFrame( value->width(), value->height(), value->timeStamp(), value->depth() );

			//calculate p-term (error * kp) and add to output
			ret->add( error, _kp );

			if( _useKickAvoidance )
			{
				if( _prevValue && dt > 0.000001f )
				{
					//NOTE: clarify why this is not (value - prevValue) / dt !!!
					//calculate derivative (prevValue - value) / dt
					SampleFrame *derivative = new SampleFrame( *_prevValue );
					derivative->sub( value );
					derivative->div( dt );
					//calculate d-term (derivative * kd) and add to output
					ret->add( derivative, _kd );
					safeDelete( derivative );
				}
			}
			else
			{
				if( _prevError && dt > 0.000001f )
				{
					//calculate derivative (error - prevError) / dt
					SampleFrame *derivative = new SampleFrame( *error );
					derivative->sub( _prevError );
					derivative->div( dt );
					//calculate d-term (derivative * kd) and add to output
					ret->add( derivative, _kd );
					safeDelete( derivative );
				}
			}

			//initialize integral
			if( !_integral )
				_integral = new SampleFrame( value->width(), value->height(), value->timeStamp(), value->depth() );

			if( _useReversalResetsIntegral && dt > 0.000001f )
			{
				//calculate target slope
				SampleFrame *slope = new SampleFrame( *target );
				slope->sub( _prevTarget );
				slope->div( dt );

				if( !dimensionsCompatible( slope, _prevTargetDir ) )
					safeDelete( _prevTargetDir );

				if( !_prevTargetDir )
					_prevTargetDir = new SampleFrame( slope->width(), slope->height(), slope->timeStamp(), slope->depth() );

				const float *ptr = slope->values();

				float *prevPtr = _prevTargetDir->values();
				float *integralPtr = _integral->values();

				size_t size = slope->size();
				for( int i = 0; i < size; i++ )
				{
					if( abs( ptr[i] ) > _reversalSlopeThreshold )
					{
						int dir = sgn( ptr[i] );
						// if previous is nonzero and directions differ, then reset integral
						if( abs( prevPtr[i] ) > 0.000001f && abs( prevPtr[i] - dir ) > 0.000001f )
							integralPtr[i] = 0.0f;
						// set new direction only if there is a non-zero slope
						if( dir )
							prevPtr[i] = dir;
					}
				}
				safeDelete( slope );
			}
			else
				safeDelete( _prevTargetDir );
			
			if( _antiWindupMethod == AWM_LIMIT_INTEGRAL )
				_integral->clamp( -_integralCap, _integralCap );
			else if( _antiWindupMethod == AWM_LIMIT_OUTPUT )
			{
				const float *errPtr = error->values();
				const float *retPtr = ret->values();

				float* integralPtr = _integral->values();

				size_t size = _integral->size();
				for( int i = 0; i < size; i++ )
				{
					float iCandidate = integralPtr[i] + errPtr[i] * dt;
					float uCandidate = retPtr[i] + iCandidate * _ki;
					if( !( ( uCandidate > _outputCap && errPtr[i] > 0 ) ||
						( uCandidate < -_outputCap && errPtr[i] < 0 ) ) )
						integralPtr[i] = iCandidate;
				}
			}
			else
				_integral->add( error, dt );

			//calculate i-term (integral * ki) and add to output
			ret->add( _integral, _ki );

			if( _antiWindupMethod == AWM_LIMIT_OUTPUT && _applyCapToOutput )
				ret->clamp( -_outputCap, _outputCap );

			safeDelete( _prevError );
			_prevError = error;

			return ret;
		}

		bool PID::process()
		{
			float sysTime = getAppTime();

			SampleFrame *v = fetchInput<SampleFrame>( "value" );
			SampleFrame *t = fetchInput<SampleFrame>( "target" );
			//NOTE: currently assuming that value and target are in sync (e.g. from same source)
			// and have same timestamp and arrive at same frequency.
			//TODO: sync value and target based on their timestamp if out of sync

			//can't compute error term if I only get updated target -> so, only updating when value changed
			if( v )
			{
				SampleFrame *refT = t ? t : _prevTarget;

				if( refT )
				{
					if( !dimensionsCompatible( v, refT ) )
					{
						safeDelete( v );
						safeDelete( t );
						throw std::runtime_error( "inputs differ in size" );
					}
					if( _prevError && !dimensionsCompatible( _prevError, v ) )
					{
						safeDelete( v );
						safeDelete( t );
						
						throw std::runtime_error( "input size changed" );
					}
					if( _integral && !dimensionsCompatible( _integral, v ) )
					{
						safeDelete( v );
						safeDelete( t );
						
						throw std::runtime_error( "input size changed" );
					}

					if( _prevValue )
					{
						if( v->timeStamp() < _prevValue->timeStamp() )
						{
							safeDelete( t );
							safeDelete( v );

							throw std::runtime_error( "input out of order" );
						}

						if( v->timeStamp() == _prevValue->timeStamp() )
						{
							std::cerr << "<warning> received value with same timestamp as previous value - skipping update" << std::endl;
						}
						else
						{
							if( _integral && !dimensionsCompatible( _integral, refT ) )
							{
								safeDelete( t );
								safeDelete( v );

								throw std::runtime_error( "input size changed" );
							}

							float dt = 
								_useSystemTime ? 
								( sysTime - _prevSysTime ) :
								( v->timeStamp() - _prevValue->timeStamp() ) * 0.001f;

							SampleFrame *ret = update( v, refT, dt );

							drawFrame( ret );
							pushOutput( "out", ret );

							safeDelete( ret );
						}
					}
				}
			}

			if( v )
			{
				safeDelete( _prevValue );
				_prevValue = v;
			}

			if( t )
			{
				safeDelete( _prevTarget );
				_prevTarget = t;
			}

			_prevSysTime = sysTime;

			return inputPending( "value" ) || inputPending( "target" );
		}

		bool PID::loadFromJSON( const nlohmann::json& j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "useSystemTime", _useSystemTime );

			load<float>( j, "kp", _kp );
			load<float>( j, "ki", _ki );
			load<float>( j, "kd", _kd );

			load<bool>( j, "kickAvoidance", _useKickAvoidance );

			load<bool>( j, "reversalResetsIntegral", _useReversalResetsIntegral );
			load<float>( j, "reversalSlopeThreshold", _reversalSlopeThreshold );

			load<bool>( j, "deadband", _useDeadband );
			load<float>( j, "deadbandThresh", _deadbandThresh );

			std::string s;
			if( load<std::string>( j, "antiwindup", s ) )
			{
				_antiWindupMethod = antiWindupMethodFromString( s );
				if( _antiWindupMethod == AWM_COUNT )
					_antiWindupMethod = AWM_NONE;
			}
			else
				_antiWindupMethod = AWM_NONE;
			load<float>( j, "integralCap", _integralCap );
			load<float>( j, "outputCap", _outputCap );
			load<bool>( j, "applyCapToOutput", _applyCapToOutput );

			return ret;
		}

		bool PID::saveToJSON( nlohmann::json& j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "useSystemTime", _useSystemTime );

			save( j, "kp", _kp );
			save( j, "ki", _ki );
			save( j, "kd", _kd );

			save( j, "kickAvoidance", _useKickAvoidance );

			save( j, "reversalResetsIntegral", _useReversalResetsIntegral );
			save( j, "reversalSlopeThreshold", _reversalSlopeThreshold );

			save( j, "deadband", _useDeadband );
			save( j, "deadbandThresh", _deadbandThresh );

			save( j, "antiwindup", antiWindupMethodToString( _antiWindupMethod ) );
			save( j, "integralCap", _integralCap );
			save( j, "outputCap", _outputCap );
			save( j, "applyCapToOutput", _applyCapToOutput );

			return ret;
		}

		bool PID::clear()
		{
			_prevSysTime = 0.0f;

			safeDelete( _prevValue );
			safeDelete( _prevTarget );
			safeDelete( _prevTargetDir );

			safeDelete( _prevError );
			safeDelete( _integral );

			return true;
		}





		Drag::Drag( float drag, float targetValue ) :
			Op(),
			_drag( drag ),
			_targetValue( targetValue )
		{}

		Drag::~Drag()
		{}

#ifdef __SUPPORT_GUI
		bool Drag::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "drag", &_drag, 0.0f, 1.0f );
			ImGui::SliderFloat( "targetValue", &_targetValue, -1.0f, 1.0f );

			return true;
		}
#endif

		bool Drag::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				sf->blend( _targetValue, _drag );

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Drag::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "drag", _drag );
			load<float>( j, "targetValue", _targetValue );

			return ret;
		}

		bool Drag::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "drag", _drag );
			save( j, "targetValue", _targetValue );

			return ret;
		}





		BoxFilter::BoxFilter( unsigned int boxSize ) :
			Op(),
			_boxSize( boxSize )
		{}

		BoxFilter::~BoxFilter()
		{
			clear();
		}

		bool BoxFilter::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				_boxSamples.push_back( sf );
				while( _boxSamples.size() > _boxSize )
				{
					delete _boxSamples.front();
					_boxSamples.pop_front();
				}

				SampleFrame *ret = nullptr;
				if( _boxSamples.size() )
				{
					ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );
					int cntr = 0;
					for( auto it = _boxSamples.begin(); it != _boxSamples.end(); ++it, cntr++ )
						ret->add( *it );
					ret->mul( 1.0f / cntr );
				}
				else
					ret = new SampleFrame( *sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool BoxFilter::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "size", _boxSize );

			return ret;
		}

		bool BoxFilter::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "size", _boxSize );

			return ret;
		}

		bool BoxFilter::clear()
		{
			for( auto it = _boxSamples.begin(); it != _boxSamples.end(); ++it )
				safeDelete( *it );
			_boxSamples.clear();

			return true;
		}





		Median::Median( unsigned int windowSize ) :
			Op(),
			_windowSize( windowSize )
		{}

		Median::~Median()
		{
			clear();
		}

#ifdef __SUPPORT_GUI
		bool Median::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int i = _windowSize;
			if( ImGui::SliderInt( "size", &i, 1, 10 ) )
				_windowSize = i;

			return true;
		}
#endif

		bool Median::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				_medianSamples.push_back( sf );
				while( _medianSamples.size() > _windowSize )
				{
					delete _medianSamples.front();
					_medianSamples.pop_front();
				}

				SampleFrame *ret = nullptr;
				if( _medianSamples.size() )
				{
					ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );

					//TODO: clean this up and make more efficient -- i had to do this fast, sorry....
					std::vector<float> sorted( _medianSamples.size() );
					for( int j = 0; j < sf->size(); j++ )
					{
						int i = 0;
						for( auto it = _medianSamples.begin(); it != _medianSamples.end(); ++it, i++ )
							sorted[i] = ( *it )->values()[j];

						std::sort( sorted.begin(), sorted.end() );
						ret->values()[j] = sorted[sorted.size() / 2];
					}
				}
				else
					ret = new SampleFrame( *sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Median::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "size", _windowSize );

			return ret;
		}

		bool Median::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "size", _windowSize );

			return ret;
		}

		bool Median::clear()
		{
			for( auto it = _medianSamples.begin(); it != _medianSamples.end(); ++it )
				safeDelete( *it );
			_medianSamples.clear();

			return true;
		}





		Mean::Mean( unsigned int windowSize ) :
			Op(),
			_windowSize( windowSize )
		{}

		Mean::~Mean()
		{
			clear();
		}

		void Mean::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );

			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "m", this ) );
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "sd", this ) );
		}

#ifdef __SUPPORT_GUI
		bool Mean::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			int i = _windowSize;
			if( ImGui::SliderInt( "size", &i, 1, 10 ) )
				_windowSize = i;

			return true;
		}
#endif

		bool Mean::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				_meanSamples.push_back( sf );
				while( _meanSamples.size() > _windowSize )
				{
					delete _meanSamples.front();
					_meanSamples.pop_front();
				}

				SampleFrame *m = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );
				SampleFrame *sd = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );

				if( _meanSamples.size() )
				{
					for( auto s : _meanSamples )
						m->add( s );
					m->mul( 1.0f / _meanSamples.size() );

					SampleFrame diff( sf->width(), sf->height(), sf->timeStamp(), sf->depth() );
					for( auto s : _meanSamples )
					{
						diff.set( s );
						diff.sub( m );
						diff.pow( 2.0f );

						sd->add( &diff );
					}
					sd->mul( 1.0f / _meanSamples.size() );
					sd->sqrt();
				}


				drawFrame( m );

				pushOutput( "m", m );
				pushOutput( "sd", sd );

				safeDelete( m );
				safeDelete( sd );
			}

			return inputPending( "in" );
		}

		bool Mean::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<unsigned int>( j, "size", _windowSize );

			return ret;
		}

		bool Mean::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "size", _windowSize );

			return ret;
		}

		bool Mean::clear()
		{
			for( auto it = _meanSamples.begin(); it != _meanSamples.end(); ++it )
				safeDelete( *it );
			_meanSamples.clear();

			return true;
		}





		BGSubtraction::BGSubtraction( bool adaptive, float adaptiveDrag ) :
			Op(),
			_adaptive( adaptive ),
			_adaptiveDrag( adaptiveDrag ),
			_lastInput( nullptr ),
			_background( nullptr )
		{
		}

		BGSubtraction::~BGSubtraction()
		{
			clear();
		}

#ifdef __SUPPORT_GUI
		bool BGSubtraction::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( _adaptive )
			{
				ImGui::SliderFloat( "drag", &_adaptiveDrag, 0.001f, 0.999f );
				if( ImGui::Button( "reset" ) )
					setBackground( nullptr );
			}
			else
			{
				if( ImGui::Button( "snapshot" ) )
					backgroundFromSnapshot();
				ImGui::SameLine();
				if( ImGui::Button( "reset" ) )
					setBackground( nullptr );
			}

			return true;
		}
#endif

		bool BGSubtraction::backgroundFromSnapshot()
		{
			if( !_lastInput )
				return false;

			setBackground( _lastInput );
			return true;
		}

		void BGSubtraction::setBackground( const SampleFrame *f )
		{
			if( f )
			{
				if( !_background )
					_background = new SampleFrame( *f );
				else
					_background->set( f );
			}
			else
				safeDelete( _background );
		}

		bool BGSubtraction::clear()
		{
			safeDelete( _background );
			safeDelete( _lastInput );

			return true;
		}

		bool BGSubtraction::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				safeDelete( _lastInput );
				_lastInput = new SampleFrame( *sf );

				if( _adaptive )
				{
					if( !_background )
						_background = new SampleFrame( *sf );
					else
						_background->blend( sf, _adaptiveDrag );
				}

				if( _background )
					sf->sub( _background )->clamp01();

				drawFrame( sf );

				pushOutput( "out", sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool BGSubtraction::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "adaptive", _adaptive );
			load<float>( j, "adaptiveDrag", _adaptiveDrag );

			SampleFrame *sf = nullptr;
			load<SampleFrame*>( j, "background", sf );
			setBackground( sf );
			safeDelete( sf );

			return ret;
		}

		bool BGSubtraction::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "adaptive", _adaptive );
			save( j, "adaptiveDrag", _adaptiveDrag );
			save( j, "background", *_background );

			return ret;
		}






		OpticalFlow::OpticalFlow() :
			Op(),
			_lastValue( nullptr ),
			_pyrScale( 0.5f ),
			_levels( 3 ),
			_winSize( 15 ),
			_iterations( 3 ),
			_polyN( 5 ),
			_polySigma( 1.2f )
		{
			//reinitialize();
		}

		OpticalFlow::~OpticalFlow()
		{
			clear();
		}

#ifdef __SUPPORT_GUI
		bool OpticalFlow::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::SliderFloat( "pyr scale", &_pyrScale, 0.01f, 1.0f );
			ImGui::SliderInt( "levels", &_levels, 1, 10 );
			ImGui::SliderInt( "winSize", &_winSize, 1, 50 );
			ImGui::SliderInt( "iterations", &_iterations, 1, 10 );
			ImGui::SliderInt( "poly N", &_polyN, 1, 10 );
			ImGui::SliderFloat( "poly Sigma", &_polySigma, 1.0f, 3.0f );

			return true;
		}

		//FrameDrawer *OpticalFlow::createDrawer()
		//{
		//	return new OpticalFlowDrawer( this );
		//}
#endif

		bool OpticalFlow::clear()
		{
			safeDelete( _lastValue );

			return true;
		}

		bool OpticalFlow::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), 3 );

				cv::Mat gray;
				if( sf->depth() == 1 )
					gray = sf->mat();
				else if( sf->depth() == 3 )
				{
					cv::Mat fgray;
					cvtColor( sf->mat(), fgray, cv::COLOR_BGR2GRAY );
					fgray.convertTo( gray, CV_8U, 255 );
				}
				else
				{
					safeDelete( ret );
					safeDelete( sf );

					//TODO:
					throw std::runtime_error( "not implemented" );
				}

				if( _lastValue )
				{
					cv::Mat flow;
					calcOpticalFlowFarneback( _lastValue->mat(), gray, flow, _pyrScale, _levels, _winSize, _iterations, _polyN, _polySigma, 0 );
					//calcOpticalFlowFarneback( _lastValue->mat(), gray, flow, 0.5, 3, 15, 3, 5, 1.2, 0 );

					cv::Mat cflow;
					cv::Mat flowTemp;
					cvtColor( _lastValue->mat(), cflow, cv::COLOR_GRAY2BGR );


					//visualization code taken from
					// https://docs.opencv.org/3.4/d4/dee/tutorial_optical_flow.html

					// visualization
					cv::Mat flow_parts[2];
					split( flow, flow_parts );
					cv::Mat magnitude, angle, magn_norm;
					cartToPolar( flow_parts[0], flow_parts[1], magnitude, angle, true );
					//normalize( magnitude, magn_norm, 0.0f, 1.0f, cv::NORM_MINMAX );
					angle *= ( ( 1.f / 360.f ) * ( 180.f / 255.f ) );

					//build hsv image
					cv::Mat _hsv[3], hsv, hsv8, bgr;
					_hsv[0] = angle;
					_hsv[1] = cv::Mat::ones( angle.size(), CV_32F );
					//_hsv[2] = magn_norm;
					_hsv[2] = magnitude;
					merge( _hsv, 3, hsv );
					hsv.convertTo( hsv8, CV_8U, 255.0 );
					cvtColor( hsv8, bgr, cv::COLOR_HSV2BGR );

					//NOTE: tried to avoid detour via 8bit, since result should be 
					// the same when directly doing HSV->BGR conversion with float matrices, 
					// but it isn't. couldn't figure out how cvtColor deals with float matrices
					// maybe deal with this when it's really necessary due to performance
					// bottlenecks -- it does its job for now.

					bgr.convertTo( ret->mat(), CV_32FC3, 1 / 255.0 );

					safeDelete( _lastValue );
				}

				_lastValue = new SampleFrame( gray, sf->timeStamp() );

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool OpticalFlow::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "pyrScale", _pyrScale );
			load<int>( j, "levels", _levels );
			load<int>( j, "winSize", _winSize );
			load<int>( j, "iterations", _iterations );
			load<int>( j, "polyN", _polyN );
			load<float>( j, "polySigma", _polySigma );

			return ret;
		}

		bool OpticalFlow::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "pyrScale", _pyrScale );
			save( j, "levels", _levels );
			save( j, "winSize", _winSize );
			save( j, "iterations", _iterations );
			save( j, "polyN", _polyN );
			save( j, "polySigma", _polySigma );

			return ret;
		}






		Kalman::Kalman() :
			Op(),
			_width( 0 ),
			_height( 0 ),
			_depth( 0 ),
			_initialized( false ),
			/*_filterGain( 0.1f ),
			_noiseVarianceEstimate( 0.5f ),
			_errorSeed( 0.0f )*/
			_processNoiseCov( 0.001f ),
			_measurementNoiseCov( 0.1f )
		{}

		Kalman::~Kalman()
		{
			clear();
		}

		//void Kalman::createPins()
		//{
		//	addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );

		//	addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "m", this ) );
		//	addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "sd", this ) );
		//}

#ifdef __SUPPORT_GUI
		bool Kalman::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			//int i = windowSize;
			//if( ImGui::SliderInt( "size", &i, 1, 10 ) )
			//	windowSize = i;

			ImGui::SliderFloat( "Q", &_processNoiseCov, 0, 1 );
			ImGui::SliderFloat( "R", &_measurementNoiseCov, 0, 1 );

			return true;
		}
#endif

		bool Kalman::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( _initialized && ( sf->width() != _width || sf->height() != _height || sf->depth() != _depth ) )
					_initialized = false;

				const cv::Mat &measurement = sf->mat();
				size_t size = sf->size();

				if( !_initialized )
				{
					//_kalman.correct( arg )		...		z_k				m
					//_kalman.errorCovPre			...		???				
					//_kalman.errorCovPost			...		P_k				n x n
					//_kalman.gain					...		K_k				
					//_kalman.measurementMatrix		...		H_k				m x n
					//_kalman.measurementNoiseCov	...		R_k				m x m
					//_kalman.processNoiseCov		...		Q_k				n x n
					//_kalman.statePre				...		???
					//_kalman.statePost				...		x_k ? X_k		n
					//_kalman.transitionMatrix		...		F_k				n x n

					_kalman.init( size, size, 0, CV_32F );

					cv::setIdentity( _kalman.transitionMatrix );
					cv::setIdentity( _kalman.measurementMatrix );
					cv::setIdentity( _kalman.errorCovPost, cv::Scalar::all( 1 ) );

					memcpy( _kalman.statePost.data, measurement.data, size * sizeof( float ) );

					_width = sf->width();
					_height = sf->height();
					_depth = sf->depth();

					_initialized = true;
				}

				if( abs( _kalman.processNoiseCov.at<float>( 0 ) - _processNoiseCov ) > std::numeric_limits<float>::epsilon() )
					cv::setIdentity( _kalman.processNoiseCov, cv::Scalar::all( _processNoiseCov ) );
				if( abs( _kalman.measurementNoiseCov.at<float>( 0 ) - _measurementNoiseCov ) > std::numeric_limits<float>::epsilon() )
					cv::setIdentity( _kalman.measurementNoiseCov, cv::Scalar::all( _measurementNoiseCov ) );

				cv::Mat prediction = _kalman.predict();
				_kalman.correct( measurement.reshape( 1, size ) );

				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), reinterpret_cast<float*>( prediction.data ), sf->timeStamp(), sf->depth() );

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool Kalman::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "Q", _processNoiseCov );
			load<float>( j, "R", _measurementNoiseCov );

			clear();

			return ret;
		}

		bool Kalman::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "Q", _processNoiseCov );
			save( j, "R", _measurementNoiseCov );

			return ret;
		}

		bool Kalman::clear()
		{
			//for( auto it = _meanSamples.begin(); it != _meanSamples.end(); ++it )
			//	safeDelete( *it );
			//_meanSamples.clear();

			_initialized = false;

			return true;
		}




		Resample::Resample() :
			Op(),
			_targetFPS( 10 ),
			_refTime( -1.0f ),
			_upsample( false ),
			_useSystemTime( false )
		{
			clear();
		}

		Resample::~Resample()
		{
			clear();
		}

#ifdef __SUPPORT_GUI
		bool Resample::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::InputFloat( "f [Hz]", &_targetFPS, 0.0f, 0.0f, "%.4f" ) )
			{
				_targetFPS = clamp<float>( _targetFPS, 0.0001f, 1000.0f );
				clear();
			}


			if( ImGui::Checkbox( "upsample", &_upsample ) )
				clear();

			if( ImGui::Checkbox( "use system time", &_useSystemTime ) )
				clear();

			{
				ScopedImGuiDisable disable( _useSystemTime );

				if( ImGui::Button( "reset timebase" ) )
					clear();
			}

			return true;
		}
#endif

		bool Resample::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( !_useSystemTime && _refTime < 0.0f )
					_refTime = sf->timeStamp() * 0.001f;

				if( _targetFPS > 0.00001f )
				{
					float tau = 1.0f / _targetFPS;

					int cntr = 0;
					bool triggered = false;
					double currentTime = ( _useSystemTime ? getAppTime() : sf->timeStamp() * 0.001f );
					while( currentTime >= _refTime + tau )
					{
						_refTime += tau;
						triggered = true;
							
						//TODO: implement upsampling and downsampling inter-/extrapolation methods
						// for the time being, this is enough
						pushOutput( "out", sf );

						if( !_upsample )
						{
							if( currentTime > _refTime + tau )
							{
								int n = ceil( ( currentTime - _refTime ) / tau );
								std::cout << "<warning> seems there was a lag -- setting refTime from " << _refTime << " to " << ( _refTime + tau * n ) << " to avoid inserting numerous frames" << std::endl;
								_refTime += tau * n;
							}
							break;
						}

						if( cntr++ > 100 )
						{
							std::cerr << "<warning> upsampling inserted 100 new frames, are you sure this is what you want? check your configurations (hint: did the timebase of your input change?); possible endless loop or memory cluttering, resetting reference time and performing panic break..." << std::endl;
							if( currentTime > _refTime + tau )
							{
								int n = ceil( ( currentTime - _refTime ) / tau );
								_refTime += tau * n;
							}
							break;
						}
					}

					if( triggered )
						drawFrame( sf );
				}

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool Resample::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "targetFPS", _targetFPS );
			load<bool>( j, "upsample", _upsample );
			load<bool>( j, "useSystemTime", _useSystemTime );

			clear();

			return ret;
		}

		bool Resample::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save<float>( j, "targetFPS", _targetFPS );
			save<bool>( j, "upsample", _upsample );
			save<bool>( j, "useSystemTime", _useSystemTime );

			return ret;
		}

		bool Resample::clear()
		{
			if( _useSystemTime )
				_refTime = getAppTime();
			else
				_refTime = -1.0f;

			return true;
		}
	}
}