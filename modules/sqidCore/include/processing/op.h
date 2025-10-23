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

#include <list>

#include <processing/pin.h>
#include <guid.h>
#include <common.h>
#include <sampleFrame.h>

#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>

#include <nlohmann/json.hpp>

#define DECLARE_OP_DESC \
	virtual const GUID &getClassID() const; \
	virtual const char *getName() const; \
	virtual const char *getPath() const; \
	static const GUID &ClassID(); \
	static const char *Name(); \
	static const char *Path();

#define DEFINE_OP_DESC( _TYPE, _NAME, _PATH, _GUID ) \
	const char *_TYPE::Name() { return _NAME; } \
	const char *_TYPE::getName() const { return _NAME; } \
	const char *_TYPE::Path() { return _PATH; } \
	const char *_TYPE::getPath() const { return _PATH; } \
	const GUID &_TYPE::ClassID() { static const GUID clsID = guidFromString( _GUID ); return clsID; } \
	const GUID &_TYPE::getClassID() const { return _TYPE::ClassID(); }

namespace sqid
{
#ifdef __SUPPORT_GUI
	class OpDrawer;
	class FrameDrawer;
#endif
	class InletPin;
	class OutletPin;

	//TODO: reimplement BTS sending (via node)
	class SQID_API Op
	{
		friend class Sensor;

	private:
		GUID _objectID;

		std::string _customName;

		unsigned int _processingTime;

		size_t _dbgID;

#ifdef __SUPPORT_GUI
		const OpDrawer *_drawer;
#endif

	protected:
		bool _isEnabled;

		std::map<std::string, InletPin*> _inlets;
		std::map<std::string, OutletPin*> _outlets;

		//NOTE: hands over ownership
		template<typename T>
		T *fetchInput( const std::string &inletName )
		{
			auto it = _inlets.find( inletName );
			if( it == _inlets.end() )
			{
				std::cerr << "<warning> tried to fetch unknown input \"" << inletName << "\"" << std::endl;
				return nullptr;
			}

			InletPin *inlet = it->second;
			T *ret = nullptr;

			if( inlet && inlet->activity() )
				ret = inlet->get<T>();
	
			return ret;
		}

		//NOTE: does not take ownership. copies will be made from passed argument
		template<typename T>
		bool pushOutput( const std::string &outletName, const T *t )
		{
			auto it = _outlets.find( outletName );
			if( it == _outlets.end() )
			{
				std::cerr << "<error> failed pushing output to \"" << outletName << "\": outlet not present" << std::endl;
				return false;
			}

			OutletPin *outlet = it->second;

			if( outlet )
				outlet->distribute<T>( t );

			return ( outlet != nullptr );
		}

		bool inputPending( const std::string &outletName ) const;

		//TODO: use smartptrs for sampleframe --- thrown OpenCV exceptoions will
		// cause memleaks in most implementations of Op::process
		//NOTE: return value tells if process needs to be called again (e.g. still input pending)
		virtual bool process() = 0;

		InletPin *addInlet( InletPin *inlet );
		OutletPin *addOutlet( OutletPin *outlet );

	public:
		Op();
		virtual ~Op();

#ifdef __SUPPORT_GUI
		bool drawFrame( const SampleFrame *sf );

		virtual bool drawUI();
#else
		bool drawFrame( const SampleFrame* ) { return true; }
#endif

		bool update();

		InletPin *getInlet( const std::string &name ) const;
		OutletPin *getOutlet( const std::string &name ) const;

		const std::map<std::string, InletPin*> &getInlets() const { return _inlets; }
		const std::map<std::string, OutletPin*> &getOutlets() const { return _outlets; }

#ifdef __SUPPORT_GUI
		void setDrawer( const OpDrawer *drawer ) { _drawer = drawer; }

		//NOTE: hands over ownership of created drawers to callee
		virtual std::vector<FrameDrawer*> createDrawers();
#endif

		//TODO: call from factory function and make these two private
		virtual void createPins();
		virtual void deletePins();
		//---------

		void setDebugID( size_t dbgID ) { _dbgID = dbgID; }
		size_t getDebugID() const { return _dbgID; }

		const GUID &getObjectID() const { return _objectID; }

		virtual const GUID &getClassID() const = 0;
		virtual const char *getName() const = 0;
		virtual const char *getPath() const = 0;

		const std::string &getCustomName() const { return _customName; }

		bool getEnabled() const { return _isEnabled; }
		void setEnabled( bool enabled ) { _isEnabled = enabled; }

		virtual bool loadFromJSON( const nlohmann::json &j );
		virtual bool saveToJSON( nlohmann::json &j ) const;
	};
}