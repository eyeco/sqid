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

#include "nodeDrawer.h"

namespace sqid
{
	class DataSource;

	class SourceDrawer : public NodeDrawer
	{
	private:
		DataSource *_source;

	public:
		SourceDrawer( GUI::Drawer *drawer, DataSource *source, const glm::vec2 &pos );
		virtual ~SourceDrawer();

		virtual void build();

		virtual bool draw();

		virtual bool drawUI();

		virtual bool saveToJSON( nlohmann::json &j ) const;
		virtual bool loadFromJSON( const nlohmann::json &j );

		DataSource *getSource() const { return _source; }

		virtual SourceDrawer *asSourceDrawer() { return this; }
	};
}
#endif