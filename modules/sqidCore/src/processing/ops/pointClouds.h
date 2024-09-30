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

#include <processing/op.h>

namespace sqid
{
#ifdef __SUPPORT_GUI
	class FrameDrawer3D;
#endif

	namespace PointClouds
	{
		class ProjectTo3D : public Op
		{
		private:
			bool _switchHandedness;
			float _f;

		protected:
			virtual bool process();

		public:
			ProjectTo3D();
			virtual ~ProjectTo3D();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();

			virtual std::vector<FrameDrawer*> createDrawers();
#endif

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		//TODO:
		// (also think of 2D point clouds)
		//class CenterOfMass3D : public Op
		//{};

		//TODO:
		// (also think of 2D point clouds)
		//class PCA3D : public Op
		//{};

		//TODO: 
		//https://docs.opencv.org/2.4/modules/core/doc/operations_on_arrays.html#perspectivetransform
	}
}