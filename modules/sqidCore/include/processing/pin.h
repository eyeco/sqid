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

#include <guid.h>
#include <common.h>

#include <blobFrame.h>
#include <sampleFrame.h>
#include <genericFrame.h>

#include "connector.h"

#define INSTANTIATE_CONTAINER( _DATA_TYPE ) \
	EXPIMP_TEMPLATE template class SQID_API DataContainer<_DATA_TYPE>;

namespace sqid
{
	class Op;
	class Connector;

	class SQID_API DataContainerBase
	{
	public:
		virtual ~DataContainerBase() {}

		virtual const GUID &getTypeGUID() const = 0;
		virtual const std::string &getTypeName() const = 0;
		virtual const std::string &getTypeShortName() const = 0;

		virtual unsigned int getPending() const = 0;
		virtual void clear() = 0;
	};

	template<typename T>
	class DataContainer : public DataContainerBase
	{
		typedef T _ValueType;
		typedef DataContainer<T> _MyType;

	private:
		std::list<T*> _ptrs;

		bool _bufferData;
		unsigned int _bufferSize;

	public:
		static const GUID typeGUID;
		static const std::string typeName;
		static const std::string typeShortName;

#ifdef __BUFFER_INPUT_PINS
		DataContainer( bool bufferData = true, unsigned int bufferSize = 64 ) :
#else
		DataContainer( bool bufferData = false, unsigned int bufferSize = 1 ) :
#endif
			_bufferData( bufferData ),
			_bufferSize( bufferSize )
		{}

		virtual ~DataContainer()
		{
			for( auto it : _ptrs )
				safeDelete( it );
			_ptrs.clear();
		}

		virtual unsigned int getPending() const { return _ptrs.size(); }

		//NOTE: container takes ownership
		void enqueue( T *ptr ) 
		{ 
			if( _bufferData )
			{
				_ptrs.push_back( ptr );
				while( _ptrs.size() > _bufferSize )
				{
					safeDelete( _ptrs.front() );
					_ptrs.pop_front();

					std::cerr << "<warning> pin buffer overrun (discarding oldest frame)" << std::endl;
				}
			}
			else
			{
				clear();
				_ptrs.push_back( ptr );
			}
		}

		//NOTE: container hands over ownership
		T *dequeue()
		{
			if( _ptrs.size() )
			{
				T *ret = _ptrs.front();
				_ptrs.pop_front();

				return ret;
			}
			return nullptr;
		}

		virtual const GUID &getTypeGUID() const { return _MyType::typeGUID; }
		virtual const std::string &getTypeName() const { return _MyType::typeName; }
		virtual const std::string &getTypeShortName() const { return _MyType::typeShortName; }

		virtual void clear()
		{
			for( auto it : _ptrs )
				safeDelete( it );
			_ptrs.clear();
		}
	};

	class SQID_API Pin
	{
	private:
		std::string _name;

	protected:
		std::string _contentDesc;
		Op *_op;

		bool _activity;
		unsigned int _activityCntr;

		//TODO: refactor: container only required for inlet
		DataContainerBase *_cont;

	public:
		Pin( DataContainerBase *cont, const std::string &name, Op *op ) :
			_name( name ),
			_contentDesc( "<not set>" ),
			_op( op ),
			_activity( false ),
			_activityCntr( 0 ),
			_cont( cont )
		{}

		virtual ~Pin()
		{
			safeDelete( _cont );
		}

		void clear()
		{
			_cont->clear();
		}

		bool isCompatible( const Pin *pin ) const
		{
			if( !pin )
				return false;

			return ( _cont->getTypeGUID() == pin->getDataGUID() );
		}

		bool activity() const { return _activity; }
		unsigned int getDataCntr() const { return _activityCntr; }

		unsigned int getPending() const { return _cont->getPending(); }

		void resetActivity() { _activity = false; _activityCntr = 0; }

		const std::string getTypeName() const { return _cont->getTypeName(); }
		const std::string getTypeShorteName() const { return _cont->getTypeShortName(); }
		const GUID &getDataGUID() const { return _cont->getTypeGUID(); }

		std::string getContentDesc() const { return _contentDesc; }

		const std::string &getName() const { return _name; }

		Op *getOp() { return _op; }

		bool isOpEnabled() const;
	};



	class SQID_API InletPin : public Pin
	{
	private:
		const Connector *_connector;

	public:
		InletPin( DataContainerBase *cont, const std::string &name, Op *op ) :
			Pin( cont, name, op ),
			_connector( nullptr )
		{}

		virtual ~InletPin()
		{}

		void connect( const Connector *conn )
		{
			_connector = conn;
		}

		void disconnect()
		{
			_connector = nullptr;
		}

		const Connector *getConnector() const { return _connector; }

		template<typename T>
		bool insert( const T *ptr )
		{
			if( !ptr )
				return false;

			if( !isOpEnabled() )
				return false;

			DataContainer<T> *dc = dynamic_cast<DataContainer<T>*>( _cont );
			if( !dc )
			{
				std::cerr << "<error> tried to insert incompatible type" << std::endl;
				return false;
			}

			dc->enqueue( new T( *ptr ) );
			_activity = true;
			_activityCntr++;

			//TODO: find a good way to keep track of activity input/output data (this also includes activity counting in pins)
			// problem is that pins are cleared at time of drawing UI.
			// also remember is is only used for UI, so this can and should be omitted when running headless
			_contentDesc = toString<T>( *ptr );

			return true;
		}

		//TODO: use unique_ptrs for data handling. there are many cases with potential memleaks, e.g.
		// when an openCV exception is thrown in an overload of Op::process, it is very 
		// likely that data is not freed correctly
		template<typename T>
		T *get()
		{
			DataContainer<T> *dc = dynamic_cast<DataContainer<T>*>( _cont );
			if( !dc )
			{
				std::cerr << "<error> tried to get incompatible type" << std::endl;
				return nullptr;
			}

			return dc->dequeue();
		}
	};
	
	class SQID_API OutletPin : public Pin
	{
	private:
		std::list<const Connector*> _connectors;

	public:
		OutletPin( DataContainerBase *cont, const std::string &name, Op *op ) :
			Pin( cont, name, op )
		{}

		void connect( const Connector *conn )
		{
			for( auto &it : _connectors )
				if( conn == it )
					return;
			_connectors.push_back( conn );
		}

		void disconnect( const Connector *conn )
		{
			_connectors.remove( conn );
		}

		void disconnectAll()
		{
			_connectors.clear();
		}

		//NOTE: does NOT take ownership; inserts to all connected inlet pins, which take copies
		template<typename T>
		size_t distribute( const T *ptr )
		{
			size_t cntr = 0;
			for( auto &it : _connectors )
				if( it->getDstPin()->insert<T>( ptr ) )
					cntr++;

			_activity = true;
			_activityCntr++;

			//TODO: find a good way to keep track of activity input/output data (this also includes activity counting in pins)
			// problem is that pins are cleared at time of drawing UI.
			// also remember is is only used for UI, so this can and should be omitted when running headless
			if( ptr )
				_contentDesc = toString<T>( *ptr );
			else
				_contentDesc = "<empty>";

			return cntr;
		}

		const std::list<const Connector*> &getConnectors() const { return _connectors; }
	};

	class SQID_API PinConnecter
	{
	public:
		virtual ~PinConnecter() {}

		virtual bool connect( OutletPin *src, InletPin *dst ) = 0;
		virtual bool connectWith( InletPin *dst ) = 0;
		virtual bool disconnect( OutletPin *src, InletPin *dst ) = 0;
	};

	INSTANTIATE_CONTAINER( BoolFrame )
	INSTANTIATE_CONTAINER( Int32Frame )
	INSTANTIATE_CONTAINER( FloatFrame )
	INSTANTIATE_CONTAINER( Vec2Frame )
	INSTANTIATE_CONTAINER( Vec3Frame )
	INSTANTIATE_CONTAINER( Vec4Frame )
	INSTANTIATE_CONTAINER( QuatFrame )
	INSTANTIATE_CONTAINER( Mat2Frame )
	INSTANTIATE_CONTAINER( Mat3Frame )
	INSTANTIATE_CONTAINER( Mat4Frame )

	INSTANTIATE_CONTAINER( SampleFrame )
}