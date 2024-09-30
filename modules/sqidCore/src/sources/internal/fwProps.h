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

#include <map>
#include <string>

#include <common.h>

#include "abstractWriter.h"

namespace sqid
{
	namespace Internal
	{
		class FWProps
		{
		public:
			enum DataType
			{
				DT_BYTE,
				DT_SHORT,
				DT_LONG,
				DT_FLOAT,

				//DT_DOUBLE,

				// DT_BYTE_MAP,
				// DT_SHORT_MAP,
				// DT_LONG_MAP,
				// DT_FLOAT_MAP,

				DT_UNKNOWN
			};

		private:
			class PropContainerBase
			{
			public:
#ifdef __SUPPORT_GUI
				virtual void drawUI( const std::string &name ) = 0;
#endif
				virtual bool parse( const std::string &s ) = 0;

				virtual std::string toString() = 0;

				virtual bool isDirty() const = 0;
				virtual void clearDirtyFlag() = 0;
			};

			template<typename T>
			class PropContainer : public PropContainerBase
			{
			public:
				T value;

				T minValue;
				T maxValue;

				T defValue;

				bool dirty;

				PropContainer() :
					value( 0 ),
					minValue( 0 ),
					maxValue( 0 ),
					defValue( 0 ),
					dirty( false )
				{}

#ifdef __SUPPORT_GUI
				virtual void drawUI( const std::string &name )
				{
					bool highlight = dirty;

					{
						ScopedImGuiStyleColor style;
						if( highlight )
							style.set( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );

						int v = value;
						if( ImGui::SliderInt( name.c_str(), &v, minValue, maxValue ) )
						{
							value = (T) v;
							dirty = true;
						}
					}
				}
#endif

				virtual bool parse( const std::string &s )
				{
					int v = 0;
					if( sscanf( s.c_str(), "%u", &v ) != 1 )
						return false;

					value = (T) v;

					return true;
				}

				virtual std::string toString()
				{
					std::stringstream sstr;
					sstr << value;

					return sstr.str();
				}

				virtual bool isDirty() const { return dirty; }
				virtual void clearDirtyFlag() { dirty = false; }
			};

			struct Prop
			{
				DataType type;

				std::string name;
				PropContainerBase *ptr;

#ifdef __SUPPORT_GUI
				void drawUI()
				{
					if( ptr )
					{
						ptr->drawUI( name );
					}
				}
#endif
			};

		public:
			explicit FWProps( IAbstractWriter *writer );
			~FWProps();

			bool requestDesc();
			bool requestStatus();
			bool set( const std::string &name );
			bool reset();

#ifdef __SUPPORT_GUI
			void drawUI();
#endif

			bool onDesc( const unsigned char *data, size_t size );
			bool onStatus( const unsigned char *data, size_t size );
			bool onAck( const unsigned char *data, size_t size );


		private:
			bool autoSet;
			IAbstractWriter *writer;
			std::map<std::string, Prop> props;

			template<typename T>
			bool parseDesc( const std::string &desc, T &value, T &defValue, T &minValue, T &maxValue )
			{
				int v = 0;
				int d = 0;
				int mn = 0;
				int mx = 0;

				if( sscanf( desc.c_str(), "%u<%u>[%u %u]", &v, &d, &mn, &mx ) != 4 )
					return false;

				value = (T) v;
				defValue = (T) d;
				minValue = (T) mn;
				maxValue = (T) mx;

				return true;
			}

			template<typename T>
			PropContainer<T> *createPropContainer( const std::string &desc )
			{
				T value = 0;
				T defValue = 0;
				T minValue = 0;
				T maxValue = 0;

				if( !parseDesc( desc, value, defValue, minValue, maxValue ) )
					return nullptr;

				PropContainer<T> *ret = new PropContainer<T>();
				ret->value = value;
				ret->defValue = defValue;
				ret->minValue = minValue;
				ret->maxValue = maxValue;

				return ret;
			}

			void clear();
		};
	}
}
