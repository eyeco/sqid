/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <common.h>

#define DECLARE_SINGLETON( type, name ) typedef sqid::Singleton<type> name;

#define DEFINE_SINGLETON( type ) \
	bool sqid::Singleton<type>::shutdown = false; \
	type *sqid::Singleton<type>::s = nullptr;

namespace sqid
{
	template <typename T, bool lazyInit = true, bool preventRecreate = true>
	class Singleton
	{
	private:
		static T* s;

		static bool shutdown;

	protected:
		Singleton()
		{}

	public:

		~Singleton()
		{
			if( !shutdown )
				uninit();
		}

		static void init()
		{
			if constexpr( !lazyInit )
				get();	//trigger creation
		}

		static void uninit()
		{
			if( s )
				shutdown = true;

			safeDelete( s );
		}

		static T* get()
		{
			if( !s )
			{
				if( shutdown )
				{
					if constexpr( !preventRecreate )	//prevent accidental re-recation if already shut down
						s = new T();
				}
				else
					s = new T();
			}

			return s;
		};
	};
}
