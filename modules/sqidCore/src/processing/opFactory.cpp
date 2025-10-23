/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <processing/opFactory.h>

namespace sqid
{
	SQID_API OpFactory& SQID_API_CALL opFactory()
	{
		static OpFactory opf;
		return opf;
	}

	OpFactory::OpFactory()
	{}

	OpFactory::~OpFactory()
	{}

	Op *OpFactory::create( const GUID &classID )
	{
		static size_t dbgCntr = 0;

		auto it = _creators.find( classID );
		if( it != _creators.end() )
		{
			Op *op = it->second->create();
			if( op )
				op->setDebugID( dbgCntr++ );

			op->createPins();

			return op;
		}

		return nullptr;
	}

	void OpFactory::shutdown()
	{
		for( auto& it : _creators )
			safeDelete( it.second );
		_creators.clear();
	}

	void OpFactory::printSupported()
	{
		for( auto &it : _supported )
			std::cout << "  " << it.name() << std::endl;
	}

	const std::vector<OpDesc> &OpFactory::getSupported()
	{
		return _supported;
	}

	void OpFactory::registerType( ProcessorCreatorBase *creator, const std::string &name, const std::string &path, const GUID &classID )
	{
		if( !creator )
		{
			std::stringstream sstr;
			sstr << "creator for type " << name << " not specified!";
			throw std::runtime_error( sstr.str() );
		}

		if( classID == GUID() )
		{
			std::stringstream sstr;
			sstr << "type " << name << " has invalid class GUID: " << guidToString( classID );
			throw std::runtime_error( sstr.str() );
		}

		auto it = _creators.find( classID );
		if( it != _creators.end() )
		{
			safeDelete( creator );

			std::stringstream sstr;
			sstr << "creator for type " << name << " already present! keeping existing one.";
			throw std::runtime_error( sstr.str() );
		}
		_creators.insert( std::make_pair( classID, creator ) );

		_supported.push_back( OpDesc( name, path, classID ) );
	}
}