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
#ifdef __SUPPORT_GUI

#include <drawing/frameDrawer.h>

namespace sqid
{
	namespace Plugins
	{
		class TUIO;
	}

	class FrameDrawerTUIO2 : public FrameDrawer
	{
	private:
		const Plugins::TUIO *_tuio;

	protected:
		virtual void lateDraw( const SampleFrame *sf );

	public:
		explicit FrameDrawerTUIO2( const Plugins::TUIO *tuio );
		virtual ~FrameDrawerTUIO2();

		virtual bool update( const SampleFrame *sf );

		virtual void draw( FrameBuffer *fb, size_t width, size_t height );

		DECLARE_FRAMEDRAWER_DESC
	};
}
#endif