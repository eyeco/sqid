/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <processing/op.h>

#ifdef __FFTW_SUPPORT
#include <fftw3.h>
#endif

namespace sqid
{
	namespace Math
	{
		class AddConst : public Op
		{
		private:
			float _value;

		protected:
			virtual bool process();

		public:
			AddConst();
			virtual ~AddConst();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class MulConst : public Op
		{
		private:
			float _value;

		protected:
			virtual bool process();

		public:
			MulConst();
			virtual ~MulConst();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Add : public Op
		{
		protected:
			virtual bool process();

		public:
			Add();
			virtual ~Add();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class Sub : public Op
		{
		protected:
			virtual bool process();

		public:
			Sub();
			virtual ~Sub();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class Mul : public Op
		{
		protected:
			virtual bool process();

		public:
			Mul();
			virtual ~Mul();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class LinEqConst : public Op
		{
		private:
			float _k;
			float _d;

		protected:
			virtual bool process();

		public:
			LinEqConst();
			virtual ~LinEqConst();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class LinEq : public Op
		{
		protected:
			virtual bool process();

		public:
			LinEq();
			virtual ~LinEq();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class MixConst : public Op
		{
		private:
			float _t;

		protected:
			virtual bool process();

		public:
			MixConst();
			virtual ~MixConst();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Mix : public Op
		{
		private:
			bool _clamp;

		protected:
			virtual bool process();

		public:
			Mix();
			virtual ~Mix();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Abs : public Op
		{
		protected:
			virtual bool process();

		public:
			Abs();
			virtual ~Abs();

			DECLARE_OP_DESC;
		};

		class PowConst : public Op
		{
		private:
			float _value;

		protected:
			virtual bool process();

		public:
			PowConst();
			virtual ~PowConst();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Sqrt : public Op
		{
		protected:
			virtual bool process();

		public:
			Sqrt();
			virtual ~Sqrt();

			DECLARE_OP_DESC;
		};

		class Log : public Op
		{
		protected:
			virtual bool process();

		public:
			Log();
			virtual ~Log();

			DECLARE_OP_DESC;
		};

		class Exp : public Op
		{
		protected:
			virtual bool process();

		public:
			Exp();
			virtual ~Exp();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class ExpConst : public Op
		{
		private:
			bool _euler;
			float _base;

		protected:
			virtual bool process();

		public:
			ExpConst();
			virtual ~ExpConst();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};


		//TODO: implements Logistic Sigmoid function, 
		// maybe also provide others: https://en.wikipedia.org/wiki/Sigmoid_function
		class Sigmoid : public Op
		{
		protected:
			virtual bool process();

		public:
			Sigmoid();
			virtual ~Sigmoid();

			DECLARE_OP_DESC;
		};

		class Sum : public Op
		{
		private:
			unsigned char _dimsMask;

		protected:
			virtual bool process();

		public:
			Sum();
			virtual ~Sum();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Product : public Op
		{
		private:
			unsigned char _dimsMask;

		protected:
			virtual bool process();

		public:
			Product();
			virtual ~Product();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Invert : public Op
		{
		private:

		protected:
			virtual bool process();

		public:
			Invert();
			virtual ~Invert();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Slope : public Op
		{
		private:
			SampleFrame *_lastValue;

		protected:
			virtual bool process();

		public:
			Slope();
			virtual ~Slope();

			bool clear();

			DECLARE_OP_DESC;
		};

		class Gradient : public Op
		{
		private:
			float _scale;
			SampleFrame *_lastValue;

		protected:
			virtual bool process();

		public:
			Gradient();
			virtual ~Gradient();

			bool clear();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Threshold : public Op
		{
		private:
			bool _binary;
			float _threshold;

		protected:
			virtual bool process();

		public:
			explicit Threshold( float threshold = 0.5f );
			virtual ~Threshold();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Remap : public Op
		{
		private:
			float _minValue;
			float _maxValue;

			bool _clamp;

		protected:
			virtual bool process();

		public:
			explicit Remap( float minValue = 0.0f, float maxValue = 1.0f, bool clamp = false );
			virtual ~Remap();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Normalize : public Op
		{
		private:
			float _minValue;
			float _maxValue;

		protected:
			virtual bool process();

		public:
			Normalize();
			virtual ~Normalize();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class AutoNormalize : public Op
		{
		public:
			enum Mode
			{
				M_GLOBAL,
				M_INDIVIDUALLY,

				M_COUNT
			};

			static const char *modeToString( AutoNormalize::Mode mode );
			static AutoNormalize::Mode modeFromString( const char *s );
			static AutoNormalize::Mode modeFromString( const std::string &s );

		private:
			Mode _mode;
			bool _isCalibrationInProgress;

			float _globalMin;
			float _globalMax;
			float _globalScale;

			SampleFrame *_individualMin;
			SampleFrame *_individualMax;
			SampleFrame *_individualScale;

			void rebuildIndividualScale();

		protected:
			virtual bool process();

		public:
			explicit AutoNormalize( Mode mode = M_INDIVIDUALLY );
			virtual ~AutoNormalize();

			bool reset();
			bool clear();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class MinConst : public Op
		{
		private:
			float _val;

		protected:
			virtual bool process();

		public:
			MinConst();
			virtual ~MinConst();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Min : public Op
		{
		protected:
			virtual bool process();

		public:
			Min();
			virtual ~Min();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class MaxConst : public Op
		{
		private:
			float _val;

		protected:
			virtual bool process();

		public:
			MaxConst();
			virtual ~MaxConst();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Max : public Op
		{
		protected:
			virtual bool process();

		public:
			Max();
			virtual ~Max();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class ClampConst : public Op
		{
		private:
			float _min;
			float _max;

		protected:
			virtual bool process();

		public:
			ClampConst();
			virtual ~ClampConst();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Clamp : public Op
		{
		protected:
			virtual bool process();

		public:
			Clamp();
			virtual ~Clamp();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class InRange : public Op
		{
		private:
			float _minValue;
			float _maxValue;

		protected:
			virtual bool process();

		public:
			InRange( float minValue = 0.0f, float maxValue = 1.0f );
			virtual ~InRange();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Approx : public Op
		{
		private:
			enum Mode
			{
				M_ROUND,
				M_FLOOR,
				M_CEIL,

				M_COUNT
			};

			static const char *modeToString( Approx::Mode mode );
			static Approx::Mode modeFromString( const char *s );
			static Approx::Mode modeFromString( const std::string &s );

			Mode _mode;

		protected:
			virtual bool process();

		public:
			Approx();
			virtual ~Approx();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class And : public Op
		{
		protected:
			virtual bool process();

		public:
			And();
			virtual ~And();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class Or : public Op
		{
		protected:
			virtual bool process();

		public:
			Or();
			virtual ~Or();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		class XOr : public Op
		{
		protected:
			virtual bool process();

		public:
			XOr();
			virtual ~XOr();

			virtual void createPins();

			DECLARE_OP_DESC;
		};

		//TODO:
		// trace
		// https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#trace

		class Determinant : public Op
		{
		protected:
			virtual bool process();

		public:
			Determinant();
			virtual ~Determinant();

			DECLARE_OP_DESC;
		};

		class LinearFunction : public Op
		{
		private:
			glm::vec2 _p0;
			glm::vec2 _p1;

			//float _current;

		protected:
			virtual bool process();

		public:
			LinearFunction();
			virtual ~LinearFunction();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class MultiLinearFunction : public Op
		{
		private:
			unsigned int _numPoints;
			std::vector<std::pair<float, float>> _points;

			//float _current;

			void reorder();

		protected:
			virtual bool process();

		public:
			MultiLinearFunction();
			virtual ~MultiLinearFunction();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		//TODO:
		// eigen
		// https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#eigen

		//TODO: 
		// have a look here:
		// https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#mulspectrums
		// https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#dct
		class FFT2D : public Op
		{
		private:
			bool _useLog;
			bool _rearrange;
			bool _normalize;

		protected:
			virtual bool process();

		public:
			FFT2D();
			virtual ~FFT2D();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

#ifdef __FFTW_SUPPORT
		class FFT1D : public Op
		{
		private:
			int _n;

			fftwf_complex *_in;
			fftwf_complex *_out;

			fftwf_plan _plan;

		protected:
			void checkPlan( const SampleFrame *sf );
			void clear();

			virtual bool process();

		public:
			FFT1D();
			virtual ~FFT1D();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
#endif

		class ToPolar : public Op
		{
		public:
			enum Mode
			{
				M_SOURCE2TARGET,
				M_TARGET2SOURCE,

				M_COUNT
			};

			static const char *modeToString( ToPolar::Mode mode );
			static ToPolar::Mode modeFromString( const char *s );
			static ToPolar::Mode modeFromString( const std::string &s );

		private:
			float _radius;
			Mode _mode;

		protected:
			virtual bool process();

		public:
			ToPolar();
			virtual ~ToPolar();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
	}
}