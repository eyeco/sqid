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

#include <guid.h>
#include <common.h>
#include <sampleFrame.h>

#include <string>
#include <vector>

#define SOURCE_MAX_QUEUE_SIZE	128

namespace sqid
{
	class SQID_API DataInterface
	{
	protected:
		GUID _objectID;

	public:
		explicit DataInterface();
		virtual ~DataInterface();

		const GUID &getObjectID() const { return _objectID; }

		virtual std::string getDesc() const = 0;

		virtual void fetchFrames( std::vector<SampleFrameContainer> &frames ) = 0;

		virtual unsigned short getDevicePort() const = 0;
		virtual DataInterfaceType getDataInterfaceType() const = 0;

#ifdef __SUPPORT_GUI
		virtual bool drawUI() = 0;
#endif

		virtual bool loadFromJSON( const nlohmann::json &j );
		virtual void saveToJSON( nlohmann::json &j ) const;

		static DataInterface *create( DataInterfaceType dit );
	};
}
