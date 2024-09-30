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
	namespace Color
	{
		class ToGrayscale : public Op
		{
		private:
			static const float uniform[3];
			static const float rec601[3];
			static const float bt709[3];
			static const float bt2100[3];

			float _weights[3];

			bool _normalize;

		protected:
			virtual bool process();

		public:
			ToGrayscale();
			virtual ~ToGrayscale();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		//TODO: use ColorSpace enum from common.h
		// augment it with 
		enum ColorSpace
		{
			CS_RGB,
			CS_HSV,
			CS_YUV,
			CS_CMY,
			CS_CMYK,
			CS_CIE,

			CS_COUNT
		};

		const char *colorSpaceToString( sqid::Color::ColorSpace cs );
		sqid::Color::ColorSpace colorSpaceFromString( const char *str );
		sqid::Color::ColorSpace colorSpaceFromString( const std::string &str );

		size_t colorSpaceChannels( sqid::Color::ColorSpace cs );

		class Convert : public Op
		{
		private:
			sqid::Color::ColorSpace _from;
			sqid::Color::ColorSpace _to;

		protected:
			virtual bool process();

		public:
			Convert();
			virtual ~Convert();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class HSVShift : public Op
		{
		private:
			float _hue;
			float _saturation;
			float _value;

		protected:
			virtual bool process();

		public:
			HSVShift();
			virtual ~HSVShift();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
	}
}
