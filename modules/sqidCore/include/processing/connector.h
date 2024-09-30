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

#include <config.h>

namespace sqid
{
	class InletPin;
	class OutletPin;
	class SceneGraph;

	class SQID_API Connector
	{
	private:
		OutletPin *_src;
		InletPin *_dst;

		SceneGraph *_sg;

	public:
		explicit Connector( SceneGraph *sg );
		~Connector();

		OutletPin *getSrcPin() const { return _src; }
		InletPin *getDstPin() const { return _dst; }

		SceneGraph *getSceneGraph() const { return _sg; }

		bool connect( OutletPin *src, InletPin *dst );
	};
}