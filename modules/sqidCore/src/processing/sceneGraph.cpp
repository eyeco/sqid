/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "sceneGraph.h"

#include <app.h>

#include <processing/op.h>
#include <processing/pin.h>
#include <processing/connector.h>

#include <sources/dataSource.h>
#include <sampleFrame.h>

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
		for( auto &src : _sources )
		{
			//src->close();
			safeDelete( src );
		}
		_sources.clear();

		_feeds.clear();

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
		if( App().getAutoSave() )
			save( _sceneName );
		clear();
	}

	void SceneGraph::run()
	{
		//for( auto &it : _sources )
		//{
		//	if( !it->run() )
		//		std::cerr << "<error> unable to start source " << it->getDesc() << std::endl;
		//}
	}

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

		json sources = root["sources"];
		if( !sources.is_null() )
		{
			for( auto it = sources.begin(); it != sources.end(); ++it )
			{
				json s = it.value();

				std::string diString( s["interface"].get<std::string>() );
				DeviceInterface di = interfaceFromString( diString );

				json props = s["props"];

				DataSource *src = createSource( di );
				if( !src )
				{
					std::cerr << "<error> creating source of type " << diString << " failed" << std::endl;
					err = true;
					continue;
				}

				if( !src->loadFromJSON( props ) )
				{
					std::cerr << "<error> reading props for source of type " << diString << " failed" << std::endl;
					err = true;

					_sources.remove( src );

					safeDelete( src );
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

		json sources = json::array();

		cntr = 0;
		for( auto &s : _sources )
		{
			json source;
			json props;

			source["interface"] = interfaceToString( s->getDeviceInterface() );
			
			s->saveToJSON( props );
			if( !props.is_null() )
				source["props"] = props;

			sources[cntr++] = source;
		}

		root["sources"] = sources;

		o << std::setw( 2 ) << root;

		return true;
	}

	void SceneGraph::onSourceData( DeviceInterface di, unsigned short portNr, const SampleFrameContainer *sfc, const std::string &desc )
	{
		bool inserted = false;
		Sensor *sensor = nullptr;
		for( auto &it : _ops )
		{
			sensor = dynamic_cast<Sensor*>( it );
			if( sensor && sensor->getInterface() == di && sensor->getPort() == portNr && sensor->doesWant( sfc, desc ) )
			{
				//TODO: actually, the source may receive multiple frames until a graph traverse is done, however we cannot always traverse the graph once a source
				// was updated, as there may be multiple sources and we have to wait for all. a solution would be to buffer frames at Source's inlet buffer and at 
				// outlet pins and process multiple during each traverse, however, this may cause timing issues. we would actually have to consider frame timestamps 
				// and sync Ops accordingly. summarizing, this all gets quite complicted, quickly. for the time being, we don't deal with this and assume fast-
				// enough processing, which obviously may drop frames.
				//TODO: for dropped frames, implement a warning in sources so the user is at least aware of the fact. also check for mem-leaks caused by frames not 
				// collected for processing.
				sensor->setSourceDesc( desc );

				//TODO: merge this somehow in a reasonable way
				if( sensor->feed( sfc->frame ) )
					inserted = true;
				else
					std::cerr << "<error> failed to insert frame to sensor" << std::endl;
			}
		}
	}

	bool SceneGraph::traverse()
	{
		for( auto &it : _ordered )
			if( !it->update() )
			{
			}

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
		_feeds.clear();
		const Sensor *sensor = nullptr;
		for( auto &it : _ops )
		{
			sensor = dynamic_cast<const Sensor*>( it );

			for( auto &src : _sources )
				if( sensor && sensor->getInterface() == src->getDeviceInterface() && sensor->getPort() == src->getDevicePort() )
					_feeds.push_back( SourceFeed( src, sensor ) );
		}

		for( auto &src : _sources )
		{
			std::vector<SampleFrameContainer> frames;
			src->fetchFrames( frames );

			if( frames.size() )
			{
				for( auto &sfp : frames )
				{
					onSourceData( src->getDeviceInterface(), src->getDevicePort(), &sfp, src->getDesc() );
					safeDelete( sfp.frame );
				}
				frames.clear();
			}
		};

		for( auto &it : _ops )
		{
			Sensor *s = dynamic_cast<Sensor*>( it );
			if( s )
				s->updateStats( dt );
		}

		/*
		Sensor *sensor = nullptr;
		for( auto &it : _ops )
		{
			sensor = dynamic_cast<Sensor*>( it );
			if( sensor )
			{
				if( sensor->update() )
					Internal::traverse( sensor );
			}
		}
		*/
		traverse();
	}

#ifdef __SUPPORT_GUI
#endif

	DataSource *SceneGraph::createSource( DeviceInterface di )
	{
		std::cout << "creating " << interfaceToString( di ) << std::endl;

		DataSource *source = nullptr;
		int retryCountdown = 0;
		while( true )
		{
			source = DataSource::create( di );
			if( source )
			{
				if( isValid( source ) )
				{
					_sources.push_back( source );
					break;
				}
				else
				{
					std::cerr << "<error> unable to insert source " << interfaceToString( di ) << " into scenegraph" << std::endl;
					safeDelete( source );

					if( retryCountdown < 100 )
					{
						std::cerr << "retrying to instantiate source " << interfaceToString( di ) << std::endl;
						retryCountdown++;
					}
					else
					{
						std::cerr << "giving up trying to instantiate source " << interfaceToString( di ) << std::endl;
						break;
					}
				}
			}
			else
			{
				std::cerr << "<error> creating source failed" << std::endl;
				break;
			}
		}

		return source;
	}

	bool SceneGraph::destroySource( const DataSource *source )
	{
		if( source )
		{
			for( auto &it : _sources )
				if( it == source )
				{
					std::cout << "deleting " << interfaceToString( source->getDeviceInterface() ) << " source " << guidToString( source->getObjectID() ) << std::endl;

					_sources.remove( it );
					safeDelete( source );

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
		for( auto &it : _sources )
			if( it->getObjectID() == op->getObjectID() )
			{
				std::cerr << "<error> Op GUID already in use for source -- what are the odds!!" << std::endl;
				//NOTE: the odds are actually quite high when you don't randomize the randseed.

				return false;
			}
		
		return true;
	}

	bool SceneGraph::isValid( const DataSource *source )
	{
		for( auto &it : _ops )
			if( it->getObjectID() == source->getObjectID() )
			{
				std::cerr << "<error> source GUID already in use for Op -- what are the odds!!" << std::endl;
				//NOTE: the odds are actually quite high when you don't randomize the randseed.

				return false;
			}
		for( auto &it : _sources )
			if( it->getObjectID() == source->getObjectID() )
			{
				std::cerr << "<error> source GUID already in use for source -- what are the odds!!" << std::endl;
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

	DataSource *SceneGraph::getSource( const GUID &objectID ) const
	{
		for( auto it : _sources )
			if( it->getObjectID() == objectID )
				return it;
		return nullptr;
	}
}