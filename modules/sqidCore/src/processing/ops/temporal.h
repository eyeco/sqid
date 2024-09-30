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

namespace sqid
{
	namespace Temporal
	{
		class RunningAverage : public Op
		{
		private:
			SampleFrame *_runningAverage;
			float _runningAverageDrag;

		protected:
			virtual bool process();

		public:
			explicit RunningAverage( float drag = 0.9f );
			virtual ~RunningAverage();

			bool clear();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Integral : public Op
		{
		private:
			uint32_t _prevTS;
			SampleFrame *_integral;

		protected:
			virtual bool process();

		public:
			Integral();
			virtual ~Integral();

			bool clear();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Drag : public Op
		{
		private:
			float _drag;
			float _targetValue;

		protected:
			virtual bool process();

		public:
			explicit Drag( float drag = 0.9f, float targetValue = 0.0f );
			virtual ~Drag();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class BoxFilter : public Op
		{
		private:
			unsigned int _boxSize;
			std::list<SampleFrame*> _boxSamples;

		protected:
			virtual bool process();

		public:
			explicit BoxFilter( unsigned int boxSize = 5 );
			virtual ~BoxFilter();

			bool clear();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Median : public Op
		{
		private:
			unsigned int _windowSize;
			std::list<SampleFrame*> _medianSamples;

		protected:
			virtual bool process();

		public:
			explicit Median( unsigned int windowSize = 5 );
			virtual ~Median();

			bool clear();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Mean : public Op
		{
		private:
			unsigned int _windowSize;
			std::list<SampleFrame*> _meanSamples;

		protected:
			virtual bool process();

		public:
			explicit Mean( unsigned int windowSize = 5 );
			virtual ~Mean();

			bool clear();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class BGSubtraction : public Op
		{
		private:
			bool _adaptive;
			float _adaptiveDrag;

			SampleFrame *_lastInput;
			SampleFrame *_background;

			void setBackground( const SampleFrame *f );

		protected:
			virtual bool process();

		public:
			explicit BGSubtraction( bool adaptive = true, float adaptiveDrag = 0.9f );
			virtual ~BGSubtraction();

			bool clear();

			bool backgroundFromSnapshot();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class OpticalFlow : public Op
		{
		private:
			SampleFrame *_lastValue;

			float _pyrScale;
			int _levels;
			int _winSize;
			int _iterations;
			int _polyN;
			float _polySigma;

		protected:
			virtual bool process();

			//void reinitialize();
			bool clear();

		public:
			OpticalFlow();
			virtual ~OpticalFlow();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Kalman : public Op
		{
		private:
			cv::KalmanFilter _kalman;

			unsigned int _width;
			unsigned int _height;
			unsigned int _depth;

			bool _initialized;

			float _processNoiseCov;
			float _measurementNoiseCov;

			//TODO:
			// implement options for using velocity and acceleration, adapt measurement and transition matrices accordingly, see as a reference:
			// https://stackoverflow.com/questions/17836267/kalmanfilter6-2-0-transition-matrix
			// https://github.com/opencv/opencv/blob/master/modules/video/src/kalman.cpp

		protected:
			virtual bool process();

		public:
			explicit Kalman();
			virtual ~Kalman();

			bool clear();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Resample : public Op
		{
		private:
			float _targetFPS;
			double _refTime;

			bool _upsample;
			bool _useSystemTime;

		protected:
			virtual bool process();

		public:
			explicit Resample();
			virtual ~Resample();

			bool clear();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
	}
}