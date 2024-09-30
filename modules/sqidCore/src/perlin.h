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

//taken from 
// https://solarianprogrammer.com/2012/07/18/perlin-noise-cpp-11/
// https://github.com/sol-prog/Perlin_Noise

#include <config.h>

#include <vector>

namespace sqid
{
	class PerlinNoise
	{
		// The permutation vector
		std::vector<int> p;
	public:
		// Initialize with the reference values for the permutation vector
		PerlinNoise();
		// Generate a new permutation vector based on the value of seed
		PerlinNoise( unsigned int seed );
		// Get a noise value, for 2D images z can have any value
		double noise( double x, double y, double z );
	private:
		double fade( double t );
		double lerp( double t, double a, double b );
		double grad( int hash, double x, double y, double z );
	};
}