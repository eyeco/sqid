/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <interfaces/dataInterface.h>

#include <config.h>
#include "comInterface.h"
#include "btsInterface.h"
#include "oscInterface.h"

#include <fileIO/json.h>


namespace sqid
{
	DataInterface::DataInterface() :
		_objectID( randomGUID() )
	{}

	DataInterface::~DataInterface()
	{
	}

	bool DataInterface::loadFromJSON( const nlohmann::json &j )
	{
		load<GUID>( j, "objectID", _objectID );

		return true;
	}

	void DataInterface::saveToJSON( nlohmann::json &j ) const
	{
		save( j, "objectID", _objectID );
	}

	DataInterface *DataInterface::create( DataInterfaceType dit )
	{
		DataInterface *di = nullptr;

		switch( dit )
		{
		case DIT_COM:
		{
			if( !COMInterface::isInitialized() )
			{
				if( COMInterface::init() )
					COMInterface::enumerate();
			}

			std::cout << "creating COM interface" << std::endl;
			di = new COMInterface();

			break;
		}
		case DIT_RFCOMM:
		{
#ifdef __RFCOMM_SUPPORT
			if( !BTSInterface::isInitialized() )
			{
				if( BTSInterface::init() )
					BTSInterface::enumerate();
			}

			std::cout << "creating BTS interface" << std::endl;
			di = new BTSInterface();
#else
			std::cerr << "<error> compiled without RFCOMM support" << std::endl;
#endif

			break;
		}
		case DIT_OSC:
		{
			//if( !OSCInterface::isInitialized() )
			//{
			//	if( OSCInterface::init() )
			//		OSCInterface::enumerate();
			//}

			std::cout << "creating OSC interface" << std::endl;
			di = new OSCInterface();

			break;
		}
		default:
		{
			std::cout << "<warning> unhandled interface type \"" << di << "\"" << std::endl;
		}
		}

		return di;
	}
}