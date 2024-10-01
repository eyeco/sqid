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

#include <singleton.h>

namespace sqid
{
	namespace Plugins
	{
		class ZMQContext
		{
		private:
			void* _ctx;

		public:
			ZMQContext();
			~ZMQContext();

			void* ctx() const { return _ctx; }
		};
	}
}

DECLARE_SINGLETON( sqid::Plugins::ZMQContext, ZMQSingleton )