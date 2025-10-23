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

#include <config.h>
#ifdef __SUPPORT_GUI

#include <drawing/frameDrawer.h>

namespace sqid
{
	namespace Statistics
	{
		class Histogram;
	}

	class FrameDrawerHistogram : public FrameDrawer2D
	{
	private:
		const Statistics::Histogram *_histogram;

	protected:
		virtual void lateDraw( const SampleFrame *sf );

	public:
		explicit FrameDrawerHistogram( const Statistics::Histogram *histogram );
		virtual ~FrameDrawerHistogram();

		DECLARE_FRAMEDRAWER_DESC
	};
}
#endif