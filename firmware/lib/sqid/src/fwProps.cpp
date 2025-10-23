/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#include "fwProps.h"

#include "comMsg.h"
#include "sender.h"

#include "Arduino.h"

namespace sqid
{
    char dataTypeToChar( FWProps::DataType dt )
    {
        switch( dt )
        {
        case FWProps::DT_BYTE:
            return 'B';
        case FWProps::DT_SHORT:
            return 'S';
        case FWProps::DT_LONG:
            return 'L';
        case FWProps::DT_FLOAT:
            return 'F';
        default:
            break;
        }

        return '?';
    }

    bool isCommand( const char *str, const char *command, const char *&args )
    {
        if( !str || !command )
            return false;

        size_t cmdLen = strlen( command );
        if( !strncmp( str, command, cmdLen ) )
        {
            args = str + cmdLen + 1;
            return true;
        }

        return false;
    }


    const float FWProps::PropTraits<float>::minValue = 0.0f;
    const float FWProps::PropTraits<float>::maxValue = 1.0f;
    const float FWProps::PropTraits<float>::defValue = 0.0f;


    FWProps::FWProps( Sender *sender ) :
        sender( sender ),
        inputBytes( 0 ),
        inputBuffer( 1024 ),
        changedCallback( nullptr )
    {}

    FWProps::~FWProps()
    {
        for( auto &it : props )
            delete it.ptr;
        props.clear();
    }

    bool FWProps::isValidName( const char *name )
    {
        if( !name )
            return false;

        while( *name )
        {
            if( !isalnum( *name ) )
                return false;
            name++;
        }

        return true;
    }

    bool FWProps::unregisterProp( const char *name )
    {
        if( !name )
            return false;

        if( !strlen( name ) )
            return false;

        for( auto it = props.begin(); it != props.end(); ++it )
            if( !it->name.compare( name ) )
            {
                delete it->ptr;

                props.erase( it );

                return true;
            }

        return false;
    }

    std::string FWProps::getStatus()
    {
        std::stringstream sstr;

        for( auto it = props.begin(); it != props.end(); ++it )
            sstr << it->name << it->ptr->status() << ";";

        return sstr.str();
    }

    std::string FWProps::getDescription()
    {
        std::stringstream sstr;

        for( auto it = props.begin(); it != props.end(); ++it )
            sstr << dataTypeToChar( it->type ) << ":" << it->name << it->ptr->desc() << ";";

        return sstr.str();
    }

    bool FWProps::contains( const char *name )
    {
        if( !name )
            return false;

        for( auto it = props.begin(); it != props.end(); ++it )
            if( !it->name.compare( name ) )
                return true;

        return false;
    }

    FWProps::DataType FWProps::getType( const char *name )
    {
        if( !name )
            return DT_UNKNOWN;

        for( auto it = props.begin(); it != props.end(); ++it )
            if( !it->name.compare( name ) )
                return it->type;

        return DT_UNKNOWN;
    }

    void FWProps::reset()
    {
        for( auto it = props.begin(); it != props.end(); ++it )
        {
            it->ptr->reset();
            if( changedCallback )
                changedCallback( it->name.c_str() );
        }
    }

    bool FWProps::sendDesc()
    {
        if( !sender )
            return false;

        return sender->sendFWDesc( getDescription().c_str() );
    }

    bool FWProps::sendStatus()
    {
        if( !sender )
            return false;

        return sender->sendFWStatus( getStatus().c_str() );
    }

    bool FWProps::sendAck( const char *name )
    {
        if( !sender )
            return false;

        for( auto it = props.begin(); it != props.end(); ++it )
            if( !it->name.compare( name ) )
            {
                std::stringstream sstr;
                sstr << it->name << it->ptr->status();

                return sender->sendFWAck( sstr.str().c_str() );
            }
        
        return false;
    }

    void FWProps::onReceive( const uint8_t *bytes, size_t size )
    {
        size_t copyBytes = std::min( size, inputBuffer.size() );
        const uint8_t *copyStart = bytes + ( size - copyBytes );

        if( inputBytes + size > inputBuffer.size() )
        {
            size_t shift = inputBytes + copyBytes - inputBuffer.size();

            size_t remaining = inputBytes - shift;
            if( remaining )
                memcpy( &inputBuffer[0], &inputBuffer[shift], remaining );

            inputBytes -= shift;
        }

        if( !copyBytes )
            return; 

        uint8_t *copyDest = &inputBuffer[inputBytes];
        memcpy( copyDest, copyStart, copyBytes );
        inputBytes += copyBytes;

        bool found = false;
        const uint8_t *ptrBegin = &inputBuffer[0];
        const uint8_t *ptrCurr = copyDest;
        const uint8_t *ptrEnd = &inputBuffer[inputBytes];

        size_t read = 0;
        do
        {
            found = false;

            while( ptrCurr < ptrEnd )
                if( *( ptrCurr++ ) == ';' )
                {
                    found = true;

                    size_t length = ptrCurr - ptrBegin;
                    onCommand( ptrBegin, length );

                    ptrBegin = ptrCurr;
                    read += length;
                }
        }while( found );

        if( read )
        {
            size_t remaining = inputBytes - read;
            if( remaining )
                memcpy( &inputBuffer[0], &inputBuffer[read], remaining );

            inputBytes -= read;
        }
    }

    bool FWProps::onCommand( const uint8_t *bytes, size_t size )
    {
        const char *str = reinterpret_cast<const char*>( bytes );
        const char *args = nullptr;        

        if( isCommand( str, "desc", args ) )
            return sendDesc();
        else if( isCommand( str, "status", args ) )
            return sendStatus();
        else if( isCommand( str, "reset", args ) )
        {
            reset();
            return sendStatus();
        }
        else if( isCommand( str, "set", args ) )
        {
            if( !args )
                return false;

            //char tempStr[128];
            //sprintf( tempStr, "trying to set -- args is \"%s\"", args );
            //Serial.println( tempStr );
            //return true;

            const char *ptr = strchr( args, ':' );
            if( !ptr )
                return false;
            
            size_t len = ptr - args;
            std::vector<char> key( len + 1 );
            strncpy( &key[0], args, ptr - args );
            key[len] = 0;

            if( set( &key[0], ptr + 1 ) )
                return sendAck( &key[0] );
            return false;
        }

        return false;
    }

    FWProps::ValueChangedCallback FWProps::setChangedCallback( FWProps::ValueChangedCallback cb )
    {
        ValueChangedCallback prev = changedCallback;
        changedCallback = cb;
        return prev;
    }

    bool FWProps::set( const char *key, const char *value )
    {
        for( auto it = props.begin(); it != props.end(); ++it )
            if( !it->name.compare( key ) )
            {
                bool success = it->ptr->parse( value );

                if( success && changedCallback )
                    changedCallback( it->name.c_str() );

                return success;
            }
        
        return false;
    }

    template<> bool FWProps::PropContainer<float>::parse( const char *str )
    {
        float f = 0.0f;
        if( sscanf( str, "%f", &f ) != 1 )
            return false;

        value = clamp( f, minValue, maxValue );

        return true;
    }
}