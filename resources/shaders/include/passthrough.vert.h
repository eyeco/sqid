R"( 
/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the YPX Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/
#version 330 core
in vec3 inPosition;
in vec4 inColor;
in vec2 inUV;
out vec4 vColor;
out vec2 vUV;
void main()
{
	gl_Position = vec4( inPosition, 1 );
	vColor = inColor;
	vUV = inUV;
}
)" 
