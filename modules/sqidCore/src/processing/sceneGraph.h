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

#include <processing/op.h>
#include <guid.h>
#include <common.h>

#include <list>

namespace sqid
{
	class Sensor;
	class DataSource;

	//NOTE: as of now, feeds are only required for drawing connecting lines in UI
	class SourceFeed
	{
	private:
		const DataSource* _dataSource;
		const Sensor* _sensor;

	public:
		SourceFeed( const DataSource* dataSource, const Sensor* sensor ) :
			_dataSource( dataSource ),
			_sensor( sensor )
		{}

		~SourceFeed()
		{}

		const DataSource* getDataSource() const { return _dataSource; }
		const Sensor* getOp() const { return _sensor; }
	};

	//TODO: reduce number of copies of a sampleframe, when handed from one node to the other
	class SQID_API SceneGraph
	{
	private:
		std::string _sceneName;

		std::list<Op*> _ops;
		std::map<GUID, Op*, CompareGUID> _guidMap;

		std::list<Connector*> _connectors;
		std::list<DataSource*> _sources;

		//NOTE: as of now, feeds are only required for drawing connecting lines in UI
		std::list<SourceFeed> _feeds;
		std::list<Op*> _ordered;

		void clear();

		bool reorder();
		bool traverse();

		bool isValid( const Op *op );
		bool isValid( const DataSource *source );

		void onSourceData( DeviceInterface di, unsigned short portNr, const SampleFrameContainer *sfc, const std::string &desc );

	public:
		SceneGraph();
		~SceneGraph();

		void close();

		void run();

		void printOrder();

		bool load( const std::string &scene );
		bool save( const std::string &scene );

		const std::string &getSceneName() const { return _sceneName; }

		//TODO: this is temporarily, until VP is finished
		//bool addPipe( DeviceInterface di, int portNr, const std::vector<std::string> &nodeNames );
		//bool addSource( DataSource *source );
		//void onSensorData( DeviceInterface di, unsigned short portNr, unsigned char deviceID, unsigned char sensorID, SampleFrame *sf, const std::string &desc );
		void update( float dt );
		//---------------------------------------

		bool connect( OutletPin *src, InletPin *dst, bool skipReorder = false );
		bool disconnect( OutletPin *src, bool skipReorder = false );
		bool disconnect( InletPin *dst, bool skipReorder = false );
		bool disconnect( Op *op, bool skipReorder = false );

		DataSource *createSource( DeviceInterface di );
		bool destroySource( const DataSource *source );

		std::list<DataSource*> &getSources() { return _sources; }
		const std::list<DataSource*> &getSources() const { return _sources; }

		Op *instantiate( const GUID &classID );
		bool destroy( const Op *op, bool skipReorder = false );
		bool destroy( const GUID &objectID, bool skipReorder = false );

		Op *getOp( const GUID &objectID ) const;
		DataSource *getSource( const GUID &objectID ) const;

		std::list<Op*> &getOps() { return _ops; }
		const std::list<Op*> &getOps() const { return _ops; }

		const std::list<Connector*> &getConnectors() const { return _connectors; }

		const std::list<SourceFeed> &getFeeds() const { return _feeds; }

		bool objectIDInUse( const GUID &objectID ) const { return ( getOp( objectID ) || getSource( objectID ) ); }
	};
}