/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "sceneGraph.h"

#include <app.h>

#include <processing/op.h>
#include <processing/pin.h>
#include <processing/connector.h>

#include <interfaces/dataInterface.h>
#include <sampleFrame.h>

#include "ops/sink.h"
#include "ops/sensor.h"
#include <processing/opFactory.h>

#include <fstream>

namespace sqid
{
	namespace Internal
	{
		class GraphNode
		{
		public:
			enum State
			{
				S_UNUSED,
				S_HELD,
				S_ADDED
			};

			State state;
			const Op *op;

			std::vector<GraphNode*> prev;
			std::vector<GraphNode*> next;
			
			GraphNode( const Op *op ) :
				state( S_UNUSED ),
				op( op )
			{}
		};


		void insertNode( std::map<const Op*, GraphNode*> &map, const Op *op )
		{
			if( map.find( op ) == map.end() )
			{
				GraphNode *n = new GraphNode( op );
				map.insert( std::make_pair( op, n ) );

				for( auto &i : op->getInlets() )
				{
					auto conn = i.second->getConnector();
					if( conn )
					{
						auto opSrc = conn->getSrcPin()->getOp();
						auto opSrcNode = map.find( opSrc );
						if( opSrcNode == map.end() )
						{
							insertNode( map, opSrc );
							opSrcNode = map.find( opSrc );
						}

						n->prev.push_back( opSrcNode->second );
					}
				}

				for( auto &o : op->getOutlets() )
				{
					for( auto &conn : o.second->getConnectors() )
					{
						auto opDst = conn->getDstPin()->getOp();
						auto opDstNode = map.find( opDst );
						if( opDstNode == map.end() )
						{
							insertNode( map, opDst );
							opDstNode = map.find( opDst );
						}

						n->next.push_back( opDstNode->second );
					}
				}
			}
		}

		//NOTE: there's probably a more efficient way to do this
		void resolve( GraphNode *node, std::list<GraphNode*> &queue, std::list<GraphNode*> &list, std::list<GraphNode*> &remaining )
		{
			//std::cout << "checking " << node->op->getDebugID() << std::endl;

			if( node->state != GraphNode::S_UNUSED )
			{
				if( node->state == GraphNode::S_HELD )
				{
					//circular dependency!!
					std::cerr << "<warning> circular dependency!" << std::endl;
				}

				//std::cout << node->op->getDebugID() << " in use -- returning" << std::endl;

				return;
			}

			node->state = GraphNode::S_HELD;

			//std::cout << node->op->getDebugID() << ": resolving dependencies" << std::endl;

			//resolve all dependencies
			for( auto in : node->prev )
				resolve( in, queue, list, remaining );

			//std::cout << node->op->getDebugID() << ": adding to list" << std::endl;

			//add to list, mark as used, remove from remaining
			list.push_back( node );
			node->state = GraphNode::S_ADDED;
			remaining.remove( node );

			//std::cout << node->op->getDebugID() << ": queueing children" << std::endl;

			//push all children to queue
			for( auto out : node->next )
			{
				if( out->state == GraphNode::S_UNUSED )
				{
					//std::cout << node->op->getDebugID() << ": queueing " << out->op->getDebugID() << std::endl;
					queue.push_back( out );
				}
				else
				{
					//std::cout << node->op->getDebugID() << ": skipping " << out->op->getDebugID() << ", already taken care of" << std::endl;
				}
			}

			//std::cout << node->op->getDebugID() << ": returning" << std::endl;
		}
	}



	SceneGraph::SceneGraph()
	{
	}

	SceneGraph::~SceneGraph()
	{
		close();
	}

	void SceneGraph::clear()
	{
		for( auto &i : _interfaces )
		{
			//i->close();
			safeDelete( i );
		}
		_interfaces.clear();

		_sourceFeeds.clear();
		_sinkFeeds.clear();

		for( auto &it : _connectors )
			safeDelete( it );
		_connectors.clear();

		for( auto &op : _ops )
			safeDelete( op );
		_ops.clear();
		_ordered.clear();
		_guidMap.clear();
	}

	void SceneGraph::close()
	{
		clear();
	}

	void SceneGraph::run()
	{}

	bool SceneGraph::load( const std::string &scene )
	{
		_sceneName = scene;
		std::string filename = scene + ".json";
		std::ifstream i( filename );
		if( !i.is_open() )
		{
			std::cerr << "<error> failed to open file " << filename << " for loading scene" << std::endl;
			return false;
		}

		using json = nlohmann::json;

		json root;
		i >> root;

		clear();

		bool err = false;

		json ops = root["ops"];
		if( !ops.is_null() )
		{
			for( auto it = ops.begin(); it != ops.end(); ++it )
			{
				json p = it.value();

				std::string name = p["name"];
				GUID guid = guidFromString( p["classID"] );

				//std::cout << "creating " << desc->name() << " (" << desc->path() << ")" << std::endl;
				std::cout << "creating " << guidToString( guid ) << std::endl;

				Op *op = opFactory().create( guid );
				if( !op )
				{
					std::cerr << "<error> failed creating Op with \"" << name << "\"" << std::endl;
					err = true;

					continue;
				}

				if( op->loadFromJSON( p ) )
				{
					if( isValid( op ) )
					{
						_ops.push_back( op );
						_ordered.push_back( op );	//not connected so far, but we want it to be updated
						_guidMap.insert( std::make_pair( op->getObjectID(), op ) );
					}
					else
					{
						std::cerr << "<error> loaded Op " << name << " is invalid" << std::endl;
						safeDelete( op );
						err = true;

						continue;
					}
				}
				else
				{
					std::cerr << "<error> failed to initialize Op " << name << " from JSON file" << std::endl;
					safeDelete( op );
					err = true;

					continue;
				}
			}
		}

		json connectors = root["connectors"];
		for( auto it = connectors.begin(); it != connectors.end(); ++it )
		{
			json src = it.value()["src"];
			json dst = it.value()["dst"];

			std::string srcName = src["name"].get<std::string>();
			std::string srcGuidStr = src["obj"].get<std::string>();
			GUID srcGuid = guidFromString( srcGuidStr );

			std::string dstName = dst["name"].get<std::string>();
			std::string dstGuidStr = dst["obj"].get<std::string>();
			GUID dstGuid = guidFromString( dstGuidStr );

			Op *opSrc = getOp( srcGuid );
			Op *opDst = getOp( dstGuid );

			if( !opSrc )
			{
				std::cerr << "<error> object \"" << srcName << "\" with GUID " << srcGuidStr << " not found" << std::endl;
				err = true;
				continue;
			}

			if( !opDst )
			{
				std::cerr << "<error> object \"" << dstName << "\" with GUID " << dstGuidStr << " not found" << std::endl;
				err = true;
				continue;
			}

			if( !connect( opSrc->getOutlet( srcName ), opDst->getInlet( dstName ), true ) )
			{
				std::cerr << "<error> unable to connect " << srcName << " to " << dstName << std::endl;
				continue;
			}
		}

		reorder();

		json interfaces = root["interfaces"];
		if( !interfaces.is_null() )
		{
			for( auto it = interfaces.begin(); it != interfaces.end(); ++it )
			{
				json s = it.value();

				std::string ditString( s["interface"].get<std::string>() );
				DataInterfaceType dit = interfaceFromString( ditString );

				json props = s["props"];

				DataInterface *di = createInterface( dit );
				if( !di )
				{
					std::cerr << "<error> creating interface of type " << ditString << " failed" << std::endl;
					err = true;
					continue;
				}

				if( !di->loadFromJSON( props ) )
				{
					std::cerr << "<error> reading props for interfaces of type " << ditString << " failed" << std::endl;
					err = true;

					_interfaces.remove( di );

					safeDelete( di );
					continue;
				}
			}
		}

		return err;
	}

	bool SceneGraph::save( const std::string &scene )
	{
		if( !scene.size() )
			return false;

		std::cout << "saving scene to " << scene << "..." << std::endl;

		_sceneName = scene;
		std::string filename = _sceneName + ".json";
		std::ofstream o( filename );
		if( !o.is_open() )
		{
			std::cerr << "<error> failed to open file " << filename << " for saving scene" << std::endl;
			return false;
		}

		using json = nlohmann::json;

		json root;

		json ops = json::array();

		int cntr = 0;
		for( auto &it : _ops )
		{
			nlohmann::json p;

			if( it->saveToJSON( p ) )
				ops[cntr++] = p;
			else
				std::cerr << "<warning> failed to save op " << it->getPath() << "/" << it->getName() << " to JSON" << std::endl;
		}

		root["ops"] = ops;

		json connectors = json::array();

		cntr = 0;
		for( auto &it : _connectors )
		{
			json connector;

			json src;
			json dst;

			if( it->getSrcPin() )
			{
				src["name"] = it->getSrcPin()->getName();
				src["obj"] = guidToString( it->getSrcPin()->getOp()->getObjectID() );
			}

			if( it->getDstPin() )
			{
				dst["name"] = it->getDstPin()->getName();
				dst["obj"] = guidToString( it->getDstPin()->getOp()->getObjectID() );
			}

			connector["src"] = src;
			connector["dst"] = dst;

			connectors[cntr++] = connector;
		}

		root["connectors"] = connectors;

		json interfaces = json::array();

		cntr = 0;
		for( auto &di : _interfaces )
		{
			json i;
			json props;

			i["interface"] = interfaceToString( di->getDataInterfaceType() );
			
			di->saveToJSON( props );
			if( !props.is_null() )
				i["props"] = props;

			interfaces[cntr++] = i;
		}

		root["interfaces"] = interfaces;

		o << std::setw( 2 ) << root;

		return true;
	}

	void SceneGraph::onSourceData( DataInterfaceType dit, unsigned short portNr, const SampleFrameContainer* sfc, const std::string &desc )
	{
		Sensor *sensor = nullptr;
		for( auto &it : _ops )
		{
			sensor = dynamic_cast<Sensor*>( it );
			if( sensor && sensor->getInterface() == dit && sensor->getPort() == portNr && sensor->doesWant( sfc, desc ) )
			{
				//TODO: actually, the source may receive multiple frames until a graph traverse is done, however we cannot always traverse the graph once a source
				// was updated, as there may be multiple sources and we have to wait for all. a solution would be to buffer frames at Source's inlet buffer and at 
				// outlet pins and process multiple during each traverse, however, this may cause timing issues. we would actually have to consider frame timestamps 
				// and sync Ops accordingly. this all gets quite complicted, quickly. for the time being, we don't deal with this and assume fast-
				// enough processing, which obviously may drop frames.
				//TODO: for dropped frames, implement a warning in sources so the user is at least aware of the fact. also check for mem-leaks caused by frames not 
				// collected for processing.
				sensor->setSourceDesc( desc );

				//TODO: merge this somehow in a reasonable way
				if( !sensor->feed( sfc->frame ) )
					std::cerr << "<error> failed to insert frame to sensor" << std::endl;
			}
		}
	}

	void SceneGraph::onSinkData( DataInterfaceType dit, unsigned short portNr, const SampleFrameContainer* sfc )
	{
		Sensor* sensor = nullptr;
		for( auto& it : _interfaces )
		{
			if( it->getDataInterfaceType() == dit && it->getDevicePort() == portNr && it->doesWant( sfc ) )
			{
				//TODO: for dropped frames, implement a warning in sources so the user is at least aware of the fact. also check for mem-leaks caused by frames not 
				// collected for processing.

				if( !it->queueFrame( *sfc ) )
					std::cerr << "<error> failed to insert frame from sink" << std::endl;
			}
		}
	}

	bool SceneGraph::traverse()
	{
		for( auto &it : _ordered )
			if( !it->update() )
			{}

		return true;
	}

	bool SceneGraph::reorder()
	{
		_ordered.clear();

		if( _ops.size() )
		{
			using namespace sqid::Internal;

			std::map<const Op*, GraphNode*> nodeMap;
			for( auto &it : _ops )
				insertNode( nodeMap, it );

			std::list<GraphNode*> remaining;
			for( auto &it : nodeMap )
				remaining.push_back( it.second );
			std::list<GraphNode*> queue;
			std::list<GraphNode*> list;

			while( remaining.size() )
			{
				//std::cout << "adding to queue: " << remaining.front()->op->getDebugID() << std::endl;
				queue.push_back( remaining.front() );

				//resolve all in queue
				while( queue.size() )
				{
					GraphNode *next = queue.front();
					queue.pop_front();

					//std::cout << "getting next from queue: " << next->op->getDebugID() << std::endl;
					resolve( next, queue, list, remaining );
				}
			}

			for( auto &it : list )
			{
				//std::cout << "# " << it->op->getDebugID() << std::endl;
				_ordered.push_back( const_cast<Op*>( it->op ) );
			}

			for( auto &it : nodeMap )
				safeDelete( it.second );
			nodeMap.clear();


			printOrder();
		}

		return true;
	}

	void SceneGraph::printOrder()
	{
		char oldFill = std::cout.fill( '0' );
		auto oldW = std::cout.width( 2 );

		std::cout << "--- the new order ---" << std::endl;
		for( auto &it : _ordered )
			std::cout << "#" << it->getDebugID() << ": " << it->getName() << " " << guidToString( it->getObjectID() ) << std::endl;
		std::cout << "---------------------" << std::endl;

		std::cout.fill( oldFill );
		std::cout.width( oldW );
	}

	void SceneGraph::update( float dt )
	{
		//NOTE: as of now, feeds are only required for drawing connecting lines in UI
		//TODO: use them to have sources/sinks directly interact with interfaces (and do not rebuild them every frame, fcs!)
		_sourceFeeds.clear();
		_sinkFeeds.clear();
		for( auto &it : _ops )
		{
			const Sensor *sensor = dynamic_cast<const Sensor*>( it );
			const Sink *sink = dynamic_cast<const Sink*>( it );

			if( sensor )
				for( auto &i : _interfaces )
					if( sensor->getInterface() == i->getDataInterfaceType() && sensor->getPort() == i->getDevicePort() )
						_sourceFeeds.push_back( SourceFeed( i, sensor ) );
			if( sink )
				for( auto &i : _interfaces )
					if( sink->getInterface() == i->getDataInterfaceType() && sink->getPort() == i->getDevicePort() )
						_sinkFeeds.push_back( SinkFeed( i, sink ) );
		}

		for( auto &i : _interfaces )
		{
			std::vector<SampleFrameContainer> frames;
			i->fetchFrames( frames );

			if( frames.size() )
			{
				for( auto &sfc : frames )
				{
					onSourceData( i->getDataInterfaceType(), i->getDevicePort(), &sfc, i->getDesc() );
					safeDelete( sfc.frame );
				}
				frames.clear();
			}
		};

		for( auto &it : _ops )
		{
			Sensor *sensor = dynamic_cast<Sensor*>( it );
			if( sensor )
				sensor->updateStats( dt );

			Sink* sink = dynamic_cast<Sink*>( it );
			if( sink )
				sink->updateStats( dt );
		}

		traverse();

		for( auto& it : _ops )
		{
			Sink *sink = dynamic_cast<Sink*>( it );
			if( sink )
			{
				std::vector<SampleFrameContainer> frames;
				sink->fetchFrames( frames );

				if( frames.size() )
				{
					for( auto& sfc : frames )
					{
						onSinkData( sink->getInterface(), sink->getPort(), &sfc );
						safeDelete( sfc.frame );
					}
					frames.clear();
				}
			}
		}
	}

#ifdef __SUPPORT_GUI
#endif

	DataInterface *SceneGraph::createInterface( DataInterfaceType dit )
	{
		std::cout << "creating " << interfaceToString( dit ) << std::endl;

		DataInterface *di = nullptr;
		int retryCountdown = 0;
		while( true )
		{
			di = DataInterface::create( dit );
			if( di )
			{
				if( isValid( di ) )
				{
					_interfaces.push_back( di );
					break;
				}
				else
				{
					std::cerr << "<error> unable to insert interface " << interfaceToString( dit ) << " into scenegraph" << std::endl;
					safeDelete( di );

					if( retryCountdown < 100 )
					{
						std::cerr << "retrying to instantiate interface " << interfaceToString( dit ) << std::endl;
						retryCountdown++;
					}
					else
					{
						std::cerr << "giving up trying to instantiate interface " << interfaceToString( dit ) << std::endl;
						break;
					}
				}
			}
			else
			{
				std::cerr << "<error> creating interface failed" << std::endl;
				break;
			}
		}

		return di;
	}

	bool SceneGraph::destroyInterface( const DataInterface *di )
	{
		if( di )
		{
			for( auto &it : _interfaces )
				if( it == di )
				{
					std::cout << "deleting " << interfaceToString( di->getDataInterfaceType() ) << " interface " << guidToString( di->getObjectID() ) << std::endl;

					_interfaces.remove( it );
					safeDelete( di );

					return true;
				}
		}

		return false;
	}

	Op *SceneGraph::instantiate( const GUID &classID )
	{
		//std::cout << "creating " << desc->name() << " (" << desc->path() << ")" << std::endl;
		std::cout << "creating " << guidToString( classID ) << std::endl;

		Op *op = nullptr;
		int retryCountdown = 0;
		while( true )
		{
			op = opFactory().create( classID );
			if( op )
			{
				if( isValid( op ) )
				{
					_ops.push_back( op );
					_ordered.push_back( op );	//not connected so far, but we want it to be updated
					_guidMap.insert( std::make_pair( op->getObjectID(), op ) );

					break;
				}
				else
				{
					std::cerr << "<error> unable to insert Op " << guidToString( classID ) << " into scenegraph" << std::endl;
					safeDelete( op );

					if( retryCountdown < 100 )
					{
						std::cerr << "retrying to instantiate Op " << guidToString( classID ) << std::endl;
						retryCountdown++;
					}
					else
					{
						std::cerr << "giving up trying to instantiate Op " << guidToString( classID ) << std::endl;
						break;
					}
				}
			}
			else
			{
				//std::cerr << "<error> failed creating Op " << desc->name() << " (" << desc->path() << ")" << std::endl;
				std::cerr << "<error> failed creating Op " << guidToString( classID ) << std::endl;
				break;
			}
		}

		return op;
	}

	bool SceneGraph::destroy( const Op *op, bool skipReorder )
	{
		if( op )
		{
			for( auto &it : _ops )
				if( it == op )
				{
					std::cout << "deleting " << guidToString( op->getObjectID() ) << std::endl;

					if( !_guidMap.erase( it->getObjectID() ) )
					{
						std::cerr << "<error> could not erase object " << guidToString( it->getObjectID() ) << " from map" << std::endl;

						std::cerr << "currently known ops in map are: " << std::endl;
						for( auto it : _guidMap )
							std::cerr << "    " << guidToString( it.first ) << " (" << it.second->getObjectID() << ")" << std::endl;

						return false;
					}

					if( !disconnect( it, true ) )
					{
						std::cerr << "<error> could not disconnect at least one pin of object " << guidToString( it->getObjectID() ) << std::endl;
						return false;
					}

					_ops.remove( it );
					_ordered.remove( it );
					safeDelete( op );

					if( !skipReorder )
						reorder();

					return true;
				}
		}

		return false;
	}

	bool SceneGraph::destroy( const GUID &objectID, bool skipReorder )
	{
		auto it = _guidMap.find( objectID );
		if( it == _guidMap.end() )
		{
			std::cerr << "<error> could not find object " << guidToString( objectID ) << " for deletion" << std::endl;
			return false;
		}
		
		return destroy( it->second, skipReorder );
	}

	bool SceneGraph::isValid( const Op *op )
	{
		for( auto &it : _ops )
			if( it->getObjectID() == op->getObjectID() )
			{
				std::cerr << "<error> Op GUID already in use for Op -- what are the odds!!" << std::endl;
				//NOTE: the odds are actually quite high when you don't randomize the randseed.

				return false;
			}
		for( auto &it : _interfaces )
			if( it->getObjectID() == op->getObjectID() )
			{
				std::cerr << "<error> Op GUID already in use for interface -- what are the odds!!" << std::endl;
				//NOTE: the odds are actually quite high when you don't randomize the randseed.

				return false;
			}
		
		return true;
	}

	bool SceneGraph::isValid( const DataInterface *di )
	{
		for( auto &it : _ops )
			if( it->getObjectID() == di->getObjectID() )
			{
				std::cerr << "<error> interface GUID already in use for Op -- what are the odds!!" << std::endl;
				//NOTE: the odds are actually quite high when you don't randomize the randseed.

				return false;
			}
		for( auto &it : _interfaces )
			if( it->getObjectID() == di->getObjectID() )
			{
				std::cerr << "<error> interface GUID already in use for interface -- what are the odds!!" << std::endl;
				//NOTE: the odds are actually quite high when you don't randomize the randseed.

				return false;
			}

		return true;
	}

	bool SceneGraph::connect( OutletPin *src, InletPin *dst, bool skipReorder )
	{
		Connector *c = new Connector( this );

		if( dst && dst->getConnector() )
			disconnect( dst, true );

		if( !c->connect( src, dst ) )
		{
			safeDelete( c );
			return false;
		}

		_connectors.push_back( c );
		
		if( !skipReorder )
			reorder();

		return true;
	}

	bool SceneGraph::disconnect( OutletPin *src, bool skipReorder )
	{
		if( !src )
			return false;

		while( src->getConnectors().size() )
		{
			auto c = src->getConnectors().front();

			bool found = false;
			for( auto &it : _connectors )
				if( it == c )
				{
					_connectors.remove( it );
					safeDelete( c );

					found = true;

					break;
				}

			if( !found )
			{
				std::cerr << "<warning> unable to disconnect -- could not find connector in list" << std::endl;
				return false;
			}
		}

		if( !skipReorder )
			reorder();

		return true;
	}

	bool SceneGraph::disconnect( InletPin *dst, bool skipReorder )
	{
		if( !dst )
			return false;

		auto c = dst->getConnector();
		if( c )
		{
			for( auto &it : _connectors )
				if( it == c )
				{
					_connectors.remove( it );
					delete( c );

					if( !skipReorder )
						reorder();

					return true;
				}

			std::cerr << "<warning> unable to disconnect -- could not find connector in list" << std::endl;
			return false;
		}

		return true;
	}

	bool SceneGraph::disconnect( Op *op, bool skipReorder )
	{
		if( !op )
			return false;

		bool ret = true;

		for( auto inIt : op->getInlets() )
			if( inIt.second )
				ret &= disconnect( inIt.second, true );
		for( auto outIt : op->getOutlets() )
			if( outIt.second )
				ret &= disconnect( outIt.second, true );

		if( ret && !skipReorder )
			reorder();

		return ret;
	}

	Op *SceneGraph::getOp( const GUID &objectID ) const
	{
		auto it = _guidMap.find( objectID );
		if( it == _guidMap.end() )
			return nullptr;
		return it->second;
	}

	DataInterface *SceneGraph::getInterface( const GUID &objectID ) const
	{
		for( auto it : _interfaces )
			if( it->getObjectID() == objectID )
				return it;
		return nullptr;
	}
}