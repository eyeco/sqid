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

#include <processing/op.h>
#include <guid.h>
#include <common.h>

#include <list>

namespace sqid
{
	//class Op;
	class Sink;
	class Sensor;
	class DataInterface;
	//class SampleFrame;


	//NOTE: as of now, feeds are only required for drawing connecting lines in UI
	class SourceFeed
	{
	private:
		const DataInterface* _dataInterface;
		const Sensor* _sensor;

	public:
		SourceFeed( const DataInterface *di, const Sensor* sensor ) :
			_dataInterface( di ),
			_sensor( sensor )
		{}

		const DataInterface* getDataInterface() const { return _dataInterface; }
		const Sensor* getOp() const { return _sensor; }
	};

	//NOTE: as of now, feeds are only required for drawing connecting lines in UI
	class SinkFeed
	{
	private:
		const DataInterface* _dataInterface;
		const Sink* _sink;

	public:
		SinkFeed( const DataInterface *di, const Sink *sink ) :
			_dataInterface( di ),
			_sink( sink )
		{}

		const DataInterface* getDataInterface() const { return _dataInterface; }
		const Sink* getOp() const { return _sink; }
	};

	//TODO: reduce number of copies of a sampleframe, when handed from one node to the other
	class SQID_API SceneGraph
	{
	private:
		std::string _sceneName;

		std::list<Op*> _ops;
		std::map<GUID, Op*, CompareGUID> _guidMap;

		std::list<Connector*> _connectors;
		std::list<DataInterface*> _interfaces;

		//NOTE: as of now, feeds are only required for drawing connecting lines in UI
		//TODO: use them to have sources/sinks directly interact with interfaces
		std::list<SourceFeed> _sourceFeeds;
		std::list<SinkFeed> _sinkFeeds;

		std::list<Op*> _ordered;

		void clear();

		bool reorder();
		bool traverse();

		bool isValid( const Op *op );
		bool isValid( const DataInterface *di );

		void onSourceData( DataInterfaceType dit, unsigned short portNr, const SampleFrameContainer *sfc, const std::string &desc );
		void onSinkData( DataInterfaceType dit, unsigned short portNr, const SampleFrameContainer *sfc );

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
		void update( float dt );
		//---------------------------------------

		bool connect( OutletPin *src, InletPin *dst, bool skipReorder = false );
		bool disconnect( OutletPin *src, bool skipReorder = false );
		bool disconnect( InletPin *dst, bool skipReorder = false );
		bool disconnect( Op *op, bool skipReorder = false );

		DataInterface *createInterface( DataInterfaceType dit );
		bool destroyInterface( const DataInterface *di );

		std::list<DataInterface*> &getInterfaces() { return _interfaces; }
		const std::list<DataInterface*> &getInterfaces() const { return _interfaces; }

		Op *instantiate( const GUID &classID );
		bool destroy( const Op *op, bool skipReorder = false );
		bool destroy( const GUID &objectID, bool skipReorder = false );

		Op *getOp( const GUID &objectID ) const;
		DataInterface *getInterface( const GUID &objectID ) const;

		std::list<Op*> &getOps() { return _ops; }
		const std::list<Op*> &getOps() const { return _ops; }

		const std::list<Connector*> &getConnectors() const { return _connectors; }

		const std::list<SourceFeed> &getSourceFeeds() const { return _sourceFeeds; }
		const std::list<SinkFeed> &getSinkFeeds() const { return _sinkFeeds; }

		bool objectIDInUse( const GUID &objectID ) const { return ( getOp( objectID ) || getInterface( objectID ) ); }
	};
}