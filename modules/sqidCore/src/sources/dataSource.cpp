/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <sources/dataSource.h>

#include <config.h>
#include "comSource.h"
#include "btsSource.h"
#include "oscSource.h"

#include <fileIO/json.h>


namespace sqid
{
	DataSource::DataSource() :
		_objectID( randomGUID() )
	{}

	DataSource::~DataSource()
	{
	}

	bool DataSource::loadFromJSON( const nlohmann::json &j )
	{
		load<GUID>( j, "objectID", _objectID );

		return true;
	}

	void DataSource::saveToJSON( nlohmann::json &j ) const
	{
		save( j, "objectID", _objectID );
	}

	DataSource *DataSource::create( DeviceInterface di )
	{
		DataSource *source = nullptr;

		switch( di )
		{
		case DI_COM:
		{
			if( !COMSource::isInitialized() )
			{
				if( COMSource::init() )
					COMSource::enumerate();
			}

			std::cout << "creating COM source" << std::endl;
			source = new COMSource();

			break;
		}
		case DI_RFCOMM:
		{
#ifdef __RFCOMM_SUPPORT
			if( !BTSSource::isInitialized() )
			{
				if( BTSSource::init() )
					BTSSource::enumerate();
			}

			std::cout << "creating BTS source" << std::endl;
			source = new BTSSource();
#else
			std::cerr << "<error> compiled without RFCOMM support" << std::endl;
#endif

			break;
		}
		case DI_OSC:
		{
			//if( !OSCSource::isInitialized() )
			//{
			//	if( OSCSource::init() )
			//		OSCSource::enumerate();
			//}

			std::cout << "creating OSC source" << std::endl;
			source = new OSCSource();

			break;
		}
		default:
		{
			std::cout << "<warning> unhandled source type \"" << di << "\"" << std::endl;
		}
		}

		return source;
	}
}