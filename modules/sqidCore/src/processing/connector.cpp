/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/



#include <processing/connector.h>

#include <processing/pin.h>


namespace sqid
{
	Connector::Connector( SceneGraph *sg ) :
		_src( nullptr ),
		_dst( nullptr ),
		_sg( sg )
	{}

	Connector::~Connector()
	{
		if( _src )
			_src->disconnect( this );
		if( _dst )
			_dst->disconnect();

		_src = nullptr;
		_dst = nullptr;

		_sg = nullptr;
	}

	bool Connector::connect( OutletPin *src, InletPin *dst )
	{
		if( !src || !dst || !src->isCompatible( dst ) )
			return false;

		if( _dst )
			_dst->disconnect();

		_src = src;
		_dst = dst;

		_src->connect( this );
		_dst->connect( this );

		return true;
	}
}