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

in vec4 vColor;
in vec2 vUV;

out vec4 col;

uniform sampler2D tex;

void main()
{
	float alpha = texture( tex, vec2( vUV.x, vUV.y ) ).r;

	col = vec4( vColor.rgb, vColor.a * alpha );
}