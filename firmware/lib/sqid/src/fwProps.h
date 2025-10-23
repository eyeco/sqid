/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#ifndef _SQID_FW_PROPS
#define _SQID_FW_PROPS

#include <stdio.h>
#include <string.h>

#include <list>
#include <vector>
#include <string>
#include <sstream>

namespace sqid
{
    class Sender;

    /**
    protocol:
    COMMAND [ARGS];
      command must be terminated with semicolon!
    ------------
    desc
      requests description
    status
      requests status
    set propname:value
      sets property with name "propname" to "value". value string must be parsable to the respective data type
      will be answered with ACK package    
    ------------
     */

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

        typedef void(*ValueChangedCallback)( const char * );

    private:
        class PropContainerBase
        {
        public:
            virtual ~PropContainerBase() {}
            
            virtual std::string status() const = 0;
            virtual std::string desc() const = 0;

            virtual bool parse( const char *str ) = 0;
            virtual void reset() = 0;
        };

        struct Prop
        {
            DataType type;

            std::string name;
            PropContainerBase *ptr;
        };

    public:
        FWProps( Sender *sender );
        ~FWProps();

        bool isValidName( const char *name );

        template<typename T>
        bool registerProp( const char *name, T defaultValue, T minValue, T maxValue )
        {
            if( !isValidName( name ) )
                return false;
            
            size_t len = strlen( name );
            if( !len )
                return false;

            if( PropTraits<T>::type == DT_UNKNOWN )
                return false;

            if( contains( name ) )
                return false;

            Prop p;
            p.type = PropTraits<T>::type;
            p.name = std::string( name );
            p.ptr = createPropContainer<T>( defaultValue, minValue, maxValue, defaultValue );

            props.push_back( p );

            return true;
        }

        bool unregisterProp( const char *name );

        bool contains( const char *name );
        DataType getType( const char *name );

        void reset();

        bool sendDesc();
        bool sendStatus();
        bool sendAck( const char *status );

        void onReceive( const uint8_t *bytes, size_t size );
        bool onCommand( const uint8_t *bytes, size_t size );

        ValueChangedCallback setChangedCallback( ValueChangedCallback cb );

        template<typename T>
        bool get( const char *name, T &t )
        {
            if( !name )
                return false;

            for( auto it = props.begin(); it != props.end(); ++it )
                if( !it->name.compare( name ) )
                {
                    //NOTE: 'dynamic_cast' not permitted with -fno-rtti, so do it the hard way
                    PropContainer<T> *pc = reinterpret_cast<PropContainer<T>*>( it->ptr );
                    if( !pc )
                        return false;

                    t = pc->value;
                    return true;
                }

            return false;
        }

    private:

        std::string getStatus();
        std::string getDescription();

        bool set( const char *key, const char *value );
    
        template<typename T>
        struct PropTraits
        {
            static const T minValue;
            static const T maxValue;

            static const T defValue;

            static const DataType type = DT_UNKNOWN;
            static const size_t size;

            typedef T _MyType;
            typedef T _ReadableType;
        };

        template<typename T>
        class PropContainer : public PropContainerBase
        {
        public:
            T value;

            T minValue;
            T maxValue;

            T defValue;

            virtual std::string status() const
            {
                std::stringstream sstr;

                sstr 
                    << ":" << (typename FWProps::PropTraits<T>::_ReadableType)value;

                return sstr.str();
            }

            virtual std::string desc() const
            {
                std::stringstream sstr;

                sstr
                    << ":" << (typename FWProps::PropTraits<T>::_ReadableType)value
                    << "<" << (typename FWProps::PropTraits<T>::_ReadableType)defValue << ">"
                    << "[" << (typename FWProps::PropTraits<T>::_ReadableType)minValue << " " << (typename FWProps::PropTraits<T>::_ReadableType)maxValue << "]";

                return sstr.str();
            }

            virtual bool parse( const char *str )
            {
                uint32_t i = 0;
                if( sscanf( str, "%u", &i ) != 1 )
                    return false;

                value = clamp( (T)i, minValue, maxValue );

                return true;
            }

            virtual void reset()
            {
                value = defValue;
            }
        };

        Sender *sender;
        std::list<Prop> props;

        size_t inputBytes;
        std::vector<uint8_t> inputBuffer;

        ValueChangedCallback changedCallback;

    	template<typename T>
        PropContainer<T> *createPropContainer( T value, T minValue, T maxValue, T defValue )
        {
           PropContainer<T> *ret = new PropContainer<T>();
           ret->value = clamp( value, minValue, maxValue );
           ret->minValue = minValue;
           ret->maxValue = maxValue;
           ret->defValue = clamp( defValue, minValue, maxValue );

           return ret;
        }
    };

    template<> struct FWProps::PropTraits<uint8_t>
    {
        static const uint8_t minValue = 0x00;
        static const uint8_t maxValue = 0xff;

        static const uint8_t defValue = 0x00;

        static const FWProps::DataType type = DT_BYTE;
        static const size_t size = 1;

        typedef uint8_t _MyType;
        typedef uint16_t _ReadableType;
    };

    template<> struct FWProps::PropTraits<uint16_t>
    {
        static const uint16_t minValue = 0x0000;
        static const uint16_t maxValue = 0xffff;

        static const uint16_t defValue = 0x0000;

        static const FWProps::DataType type = DT_SHORT;
        static const size_t size = 2;

        typedef uint16_t _MyType;
        typedef uint16_t _ReadableType;
    };

    template<> struct FWProps::PropTraits<uint32_t>
    {
        static const uint32_t minValue = 0x00000000;
        static const uint32_t maxValue = 0xffffffff;

        static const uint32_t defValue = 0x00000000;

        static const FWProps::DataType type = DT_LONG;
        static const size_t size = 4;

        typedef uint32_t _MyType;
        typedef uint32_t _ReadableType;
    };

    template<> struct FWProps::PropTraits<float>
    {
        static const float minValue;
        static const float maxValue;

        static const float defValue;

        static const FWProps::DataType type = DT_FLOAT;
        static const size_t size = 4;

        typedef float _MyType;
        typedef float _ReadableType;
    };
}

#endif