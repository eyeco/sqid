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
#ifdef __COMPRESSION_SUPPORT
	class Compressor;
	class Decompressor;
#endif

	namespace Util
	{
		class Time : public Op
		{
		public:
			enum Mode
			{
				M_RELATIVE,
				M_APPLICATION,
				M_UNIX_EPOCH,
				M_BOOT,

				M_COUNT
			};

			static const char* modeToString( Mode mode );
			static Mode modeFromString( const char* s );
			static Mode modeFromString( const std::string& s );

		private:
			Mode _mode;
			bool _utc;
			double _refTime;

		protected:
			virtual bool process();

		public:
			Time();
			virtual ~Time();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class TimeStamp : public Op
		{
		protected:
			virtual bool process();

		public:
			TimeStamp();
			virtual ~TimeStamp();

			DECLARE_OP_DESC;
		};

		class Sync : public Op
		{
		public:
			//TODO: provide several options what to do / how to sync
			enum Mode
			{
				M_OR,
				M_AND,

				M_COUNT
			};

			static const char *modeToString( Mode mode );
			static Mode modeFromString( const char *s );
			static Mode modeFromString( const std::string &s );

		private:
			SampleFrame *_lastA;
			SampleFrame *_lastB;

			Mode _mode;

			bool _useLatest;

		protected:
			virtual bool process();

		public:
			Sync();
			virtual ~Sync();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Sampler : public Op
		{
		private:
			//TODO: implement interpolation methods (NN, linear, cubic)
			//TODO: implement border handling (repeat, mirror, repeat)

			float _nx;
			float _ny;
			float _nz;

			bool _useZ;
			bool _normalized;

			float _x;
			float _y;
			float _z;

		protected:
			virtual bool process();

		public:
			Sampler();
			virtual ~Sampler();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Buffer : public Op
		{
		private:
			size_t _size;

			SampleFrame *_buffer;

		protected:
			void clear();
			void checkBuffer( const SampleFrame *sf );

			virtual bool process();

		public:
			Buffer();
			virtual ~Buffer();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Split : public Op
		{
		protected:
			virtual bool process();

		public:
			Split();
			virtual ~Split();

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Merge : public Op
		{
		protected:
			virtual bool process();

		public:
			Merge();
			virtual ~Merge();

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Crop : public Op
		{
		private:
			float _left;
			float _right;
			float _bottom;
			float _top;

			bool _normalized;

			unsigned int _refWidth;
			unsigned int _refHeight;

		protected:
			virtual bool process();

		public:
			Crop();
			virtual ~Crop();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Resize : public Op
		{
		private:
			unsigned int _width;
			unsigned int _height;

			bool _keepAspect;

			InterpolationMethod _method;

			bool _warp;

		protected:
			virtual bool process();

		public:
			Resize();
			virtual ~Resize();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};


		class MaxPooling : public Op
		{
		private:
			unsigned int _width;
			unsigned int _height;

			InterpolationMethod _method;

		protected:
			virtual bool process();

		public:
			MaxPooling();
			virtual ~MaxPooling();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON(const nlohmann::json& j);
			virtual bool saveToJSON(nlohmann::json& j) const;

			DECLARE_OP_DESC;
		};


		class Join : public Op
		{
		private: 
			bool _ver;

		protected:
			virtual bool process();

		public:
			Join();
			virtual ~Join();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Transpose : public Op
		{
		protected:
			virtual bool process();

		public:
			Transpose();
			virtual ~Transpose();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Flatten : public Op
		{
		protected:
			virtual bool process();

		public:
			Flatten();
			virtual ~Flatten();

			DECLARE_OP_DESC;
		};

		class Reshape : public Op
		{
		public:
			enum Mode
			{
				M_STRICT,
				M_AUTO_X,
				M_AUTO_Y,
				M_AUTO_Z,

				M_COUNT
			};

			static const char *modeToString( Mode mode );
			static Mode modeFromString( const char *s );
			static Mode modeFromString( const std::string &s );

		private:
			uint32_t _x;
			uint32_t _y;
			uint32_t _z;

			uint32_t _autoX;
			uint32_t _autoY;
			uint32_t _autoZ;

			Mode _autoMode;

		protected:
			virtual bool process();

		public:
			Reshape();
			virtual ~Reshape();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Flip : public Op
		{
		private:
			bool _horizontally;
			bool _vertically;

		protected:
			virtual bool process();

		public:
			Flip();
			virtual ~Flip();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Edge : public Op
		{
		public:
			enum Mode
			{
				M_BOTH,
				M_RISE,
				M_FALL,

				M_COUNT
			};

			static const char* modeToString( Mode mode );
			static Mode modeFromString( const char* s );
			static Mode modeFromString( const std::string& s );

		private:
			Mode _mode;

			bool _continuous;
			float _threshold;

			SampleFrame *_prevFrame;

		protected:
			virtual bool process();

		public:
			Edge();
			virtual ~Edge();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class OnOff : public Op
		{
		private:
			bool _hysteresis;
			float _threshold;
			float _lowerThreshold;

			bool _continuous;

			SampleFrame *_tempFrame;

			void clear();

		protected:
			virtual bool process();

		public:
			explicit OnOff( float threshold = 0.5f );
			virtual ~OnOff();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class FlipFlop : public Op
		{
		private:
			float _threshold;

			bool _risingEdge;

			SampleFrame *_tempFrame;
			SampleFrame *_prevFrame;

		protected:
			virtual bool process();

		public:
			explicit FlipFlop( float threshold = 0.5f );
			virtual ~FlipFlop();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class SampleAndHold : public Op
		{
		public:
			enum Mode
			{
				M_SYS_TIME,
				M_FRAME_TIME,
				M_COUNTDOWN,
				M_TRIGGER,

				M_COUNT
			};

			static const char *modeToString( Mode mode );
			static Mode modeFromString( const char *s );
			static Mode modeFromString( const std::string &s );

		private:
			Mode _mode;

			int _frames;
			float _period;

			unsigned int _cntr;
			float _t0;

			bool _continuous;
			bool _updateOut;

			SampleFrame *_lastInput;
			SampleFrame *_snapshot;

		protected:
			virtual bool process();

			void takeSnapshot();

		public:
			SampleAndHold( Mode mode = M_TRIGGER );
			virtual ~SampleAndHold();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

#ifdef __COMPRESSION_SUPPORT
		class Compress : public Op
		{
		private:
			std::vector<unsigned char> _buffer;

			Compressor *_compressor;
			CompressionAlgorithm _algorithm;

		protected:
			void update();
			virtual bool process();

		public:
			Compress();
			virtual ~Compress();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class Decompress : public Op
		{
		private:
			std::vector<unsigned char> _buffer;

			Decompressor *_decompressor;
			CompressionAlgorithm _algorithm;

		protected:
			void update();
			virtual bool process();

		public:
			Decompress();
			virtual ~Decompress();

			virtual void createPins();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
#endif
	}
}