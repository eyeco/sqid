/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/

#include "zeroMQContext.h"

#include <zmq.h>

DEFINE_SINGLETON( sqid::Plugins::ZMQContext )

namespace sqid
{
	namespace Plugins
	{
		ZMQContext::ZMQContext() :
			_ctx( nullptr )
		{
			int major = 0;
			int minor = 0;
			int patch = 0;

			zmq_version( &major, &minor, &patch );

			std::cout << "creating ZMQ context (version " << major << "." << minor << "." << patch << ")" << std::endl;
			_ctx = zmq_ctx_new();
			if( !_ctx )
				std::cerr << "<error> failed to create zmq context: " << zmq_strerror( zmq_errno() ) << std::endl;
		}

		ZMQContext::~ZMQContext()
		{
			if( _ctx )
			{
				std::cout << "destroying ZMQ context" << std::endl;
				zmq_ctx_destroy( _ctx );
				_ctx = nullptr;
			}
		}
	}
}