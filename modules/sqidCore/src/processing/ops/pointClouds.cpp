/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "pointClouds.h"

#include <drawing/frameDrawer.h>

#include <fileIO/json.h>

#include <imgui/imgui.h>

#include <glm/ext.hpp>

namespace sqid
{
	namespace PointClouds
	{
		namespace Internal
		{
			void project2DTo3D( size_t size, const glm::vec3 *in, glm::vec3 *out, unsigned int cx, unsigned int cy, float f, bool switchHandedness, float unit = 0.001f )
			{
				float s = unit / f;
				float a = -( cx * s );
				float b = -( cy * s );
				float zs = unit * ( switchHandedness ? -1.0f : 1.0f );

				while( size-- )
				{
					out->x = -( in->x * s + a ) * in->z;
					out->y = ( in->y * s + b ) * in->z;
					out->z = in->z * zs;

					in++;
					out++;
				}
			}

			void project3DTo2D( size_t size, const glm::vec3 *in, glm::vec3 *out, unsigned int cx, unsigned int cy, float f, bool switchHandedness, float unit = 0.001f )
			{
				float a = cx;
				float b = cy;
				float zs = ( switchHandedness ? -1.0f : 1.0f ) / unit;

				while( size-- )
				{
					out->z = in->z * zs;

					float s = f / ( out->z * unit );

					out->x = ( -in->x * s + a );
					out->y = in->y * s + b;

					in++;
					out++;
				}
			}
		}

		DEFINE_OP_DESC( ProjectTo3D, "projectTo3D", "/pointClouds",
			"AF39629F-FE2A-4C1C-9903-3D5B1293AB02" );



		ProjectTo3D::ProjectTo3D() :
			Op(),
			_f( 1.0 ),
			_switchHandedness( false )
		{
		}

		ProjectTo3D::~ProjectTo3D()
		{}

#ifdef __SUPPORT_GUI
		bool ProjectTo3D::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			ImGui::Checkbox( "switch handedness", &_switchHandedness );
			ImGui::SliderFloat( "focal length", &_f, 0, 5 );

			return true;
		}

		std::vector<FrameDrawer*> ProjectTo3D::createDrawers()
		{
			std::vector<FrameDrawer*> drawers;

			drawers.push_back( new PointCloudDrawer() );

			return drawers;
		}
#endif

		bool ProjectTo3D::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( sf->depth() != 1 )
				{
					safeDelete( sf );
					throw std::runtime_error( "input depth must be 1" );
				}

				SampleFrame *ret = new SampleFrame( sf->width(), sf->height(), sf->timeStamp(), 3 );

				float cx = ret->width() * 0.5f;
				float cy = ret->height() * 0.5f;

				float *inZ = sf->values();
				glm::vec3 *pos = reinterpret_cast<glm::vec3*>( ret->values() );

#pragma omp parallel for
				for( int j = 0; j < ret->height(); j++ )
					for( int i = 0; i < ret->width(); i++ )
					{
						int idX = j * ret->width() + i;
						pos[idX] = glm::vec3( i - cx, cy - j, inZ[idX] );
					}

				Internal::project2DTo3D( ret->width() * ret->height(), pos, pos, 0, 0, _f * ret->height(), _switchHandedness, 1.0f );

				safeDelete( sf );

				drawFrame( ret );

				pushOutput( "out", ret );
				safeDelete( ret );
			}

			return inputPending( "in" );
		}

		bool ProjectTo3D::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<float>( j, "f", _f );
			load<bool>( j, "switchHandedness", _switchHandedness );

			return ret;
		}

		bool ProjectTo3D::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "f", _f );
			save( j, "switchHandedness", _switchHandedness );

			return ret;
		}
	}
}