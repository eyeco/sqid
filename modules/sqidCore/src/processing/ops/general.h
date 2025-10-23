/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <processing/op.h>

namespace sqid
{
	class PerlinNoise;

	namespace General
	{
		class NOP : public Op
		{
		private:
			virtual bool process();

		public:
			NOP();
			virtual ~NOP();

			DECLARE_OP_DESC;
		};

		class Const : public Op
		{
		public:
			enum ConstType
			{
				CT_IDENTITY,
				CT_FILLED,

				CT_COUNT
			};

			static const char *constTypeToString( ConstType type );
			static ConstType constTypeFromString( const char *s );
			static ConstType constTypeFromString( const std::string &s );

		private:
			unsigned int _width;
			unsigned int _height;
			unsigned int _depth;

			bool _uniform;
			glm::vec4 _value;

			ConstType _type;

		protected:
			virtual bool process();

		public:
			explicit Const( ConstType type = CT_FILLED );
			virtual ~Const();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		//TODO: have a look at OpenCV randomizer
		// https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#rng
		class Noise : public Op
		{
		public:
			enum NoiseType
			{
				NT_WHITE,
				//NT_PINK, //TODO
				NT_PERLIN,

				NT_COUNT
			};

			static const char *noiseTypeToString( NoiseType type );
			static NoiseType noiseTypeFromString( const char *s );
			static NoiseType noiseTypeFromString( const std::string &s );

		private:
			unsigned int _width;
			unsigned int _height;
			unsigned int _depth;

			unsigned int _seed;

			float _speed;

			NoiseType _type;
			PerlinNoise *_ken;

		protected:
			virtual bool process();

		public:
			explicit Noise( NoiseType type = NT_PERLIN );
			virtual ~Noise();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Signal : public Op
		{
		public:
			enum SignalType
			{
				ST_SINE,
				ST_SQUARE,
				ST_SAWTOOTH,
				ST_TRIANGLE,

				ST_COUNT
			};

			static const char *signalTypeToString( SignalType type );
			static SignalType signalTypeFromString( const char *s );
			static SignalType signalTypeFromString( const std::string &s );

		private:
			unsigned int _width;
			unsigned int _height;

			float _speed;
			float _period;
			float _angle;

			SignalType _type;

		protected:
			virtual bool process();

		public:
			explicit Signal( SignalType type = ST_SINE );
			virtual ~Signal();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class ContourDetector : public Op
		{
			struct Contour
			{
				float area;
				glm::vec2 centroid;
				std::vector<glm::vec2> points;
			};

		private:
			bool _approximate;
			float _approxEpsilon;

			float _areaFilterMin;
			float _areaFilterMax;

			std::vector<Contour> _contours;

		protected:
			virtual bool process();

		public:
			ContourDetector();
			virtual ~ContourDetector();

			const std::vector<Contour> &getContours() const { return _contours; }

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();

			virtual std::vector<FrameDrawer*> createDrawers();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
	}
}