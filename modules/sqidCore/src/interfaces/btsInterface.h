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

#ifdef __RFCOMM_SUPPORT
#include <interfaces/dataInterface.h>

namespace sqid
{
	namespace Internal
	{
		class BTSInterfaceImpl;
	}

	class SQID_API BTSInterface : public DataInterface
	{
	private:
		static unsigned int idCntr;

		//TODO: this running ID should be changed to something meaningful, actually identifying the device, e.g. address
		unsigned int _id;

		std::string _name;
		std::string _address;

		unsigned int _dropdownSelected;

		Internal::BTSInterfaceImpl *_impl;

	public:
		BTSInterface();
		virtual ~BTSInterface();

		bool run( const std::string &name, const std::string &address );
		void close();

		virtual std::string getDesc() const;
		virtual void fetchFrames( std::vector<SampleFrameContainer> &frames );

		virtual unsigned short getDevicePort() const { return _id; }
		virtual DataInterfaceType getDataInterfaceType() const { return DIT_RFCOMM; }

		const std::string &getName() const { return _name; }
		const std::string &getAddress() const { return _address; }

#ifdef __SUPPORT_GUI
		virtual bool drawUI();
#endif
		/*
		bool write( const unsigned char *data, size_t size );
		bool write( const std::string &str );
		bool write( const std::vector<unsigned char> &data );*/

		static bool init();
		static bool isInitialized();

		static void rescan();
		static void enumerate();

		bool loadFromJSON( const nlohmann::json &j );
		void saveToJSON( nlohmann::json &j ) const;
	};
}
#endif // __RFCOMM_SUPPORT
