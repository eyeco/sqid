/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "sceneGraphDrawer.h"

#ifdef __SUPPORT_GUI

#include <app.h>

#include <fileIO/json.h>

#include "../../processing/ops/sink.h"
#include "../../processing/ops/sensor.h"
#include <interfaces/dataInterface.h>

#include <processing/pin.h>
#include <processing/connector.h>
#include <processing/opFactory.h>
#include "../../processing/sceneGraph.h"

#include <drawing/frameBuffer.h>
#include <drawing/frameDrawer.h>
#include "opDrawer.h"
#include "interfaceDrawer.h"
#include "uiStyle.h"

#include <GLFW/glfw3.h>

#include <commonImGui.h>

#include <fstream>

namespace sqid
{
	namespace Internal
	{

		//taken from:
		// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C++
		template<typename T>
		typename T::size_type levensteinDistance( const T &source, const T &target )
		{
			if( source.size() > target.size() )
				return levensteinDistance( target, source );

			using TSizeType = typename T::size_type;
			const TSizeType min_size = source.size(), max_size = target.size();
			std::vector<TSizeType> lev_dist( min_size + 1 );

			for( TSizeType i = 0; i <= min_size; ++i )
				lev_dist[i] = i;

			for( TSizeType j = 1; j <= max_size; ++j )
			{
				TSizeType previous_diagonal = lev_dist[0], previous_diagonal_save;
				++lev_dist[0];

				for( TSizeType i = 1; i <= min_size; ++i )
				{
					previous_diagonal_save = lev_dist[i];
					if( source[i - 1] == target[j - 1] )
						lev_dist[i] = previous_diagonal;
					else
						lev_dist[i] = std::min( std::min( lev_dist[i - 1], lev_dist[i] ), previous_diagonal ) + 1;
					previous_diagonal = previous_diagonal_save;
				}
			}

			return lev_dist[min_size];
		}

		const std::map<DataInterfaceType, const char*> createInterfaceMap()
		{
			std::map<DataInterfaceType, const char*> im;
			for( int i = 0; i < DIT_COUNT; i++ )
				im.insert( std::make_pair( (DataInterfaceType) i, interfaceToString( (DataInterfaceType) i ) ) );
			return im;
		}

		const std::map<DataInterfaceType, const char*> &getInterfaceMap()
		{
			static std::map<DataInterfaceType, const char*> im = createInterfaceMap();
			return im;
		}

		class Node
		{
		public:
			std::string name;
			std::string path;

			std::string desc;

			GUID guid;

			bool isOp;
			DataInterfaceType dataInterface;

			Node( const std::string &n, const std::string &p, const std::string &d, const GUID &g ) :
				name( n ),
				path( p ),
				desc( d ),
				guid( g ),
				isOp( true ),
				dataInterface( DIT_COUNT )
			{}

			Node( const std::string &n, DataInterfaceType dit ) :
				name( n ),
				desc( "(interface)" ),
				isOp( false ),
				dataInterface( dit )
			{}
		};

		struct FilteredNodeSort
		{
			inline bool operator() ( const std::pair<int, Node> &lhs, const std::pair<int, Node> &rhs )
			{
				return ( lhs.first < rhs.first );
			}
		};

		class TreeNode
		{
		public:
			std::string name;
			std::list<TreeNode> children;
			GUID guid;

			explicit TreeNode( const std::string &n ) :
				name( n )
			{}

			TreeNode( const std::string &n, const GUID &g ) :
				name( n ),
				guid( g )
			{}
		};

		void printNode( const TreeNode &n, size_t l )
		{
			for( int i = 0; i < l; i++ )
				std::cout << "  ";
			std::cout << n.name << ( n.children.size() ? "/" : "" ) << std::endl;
			for( auto &it : n.children )
				printNode( it, l + 1 );
		}

		bool compareNodes( const Node &n1, const Node &n2 )
		{
			return n1.name.compare( n2.name ) < 0;
		}

		bool compareTreeNodes( const TreeNode &n1, const TreeNode &n2 )
		{
			return n1.name.compare( n2.name ) < 0;
		}

		void sortNodes( std::list<TreeNode> &n )
		{
			n.sort( compareTreeNodes );
			for( auto &it : n )
				sortNodes( it.children );
		}

		std::list<TreeNode> createSupportedTree()
		{
			std::list<TreeNode> nodes;
			for( auto &it : opFactory().getSupported() )
			{
				auto subs = split( it.path(), '/' );
				std::list<TreeNode> *level = &nodes;
				for( auto &s : subs )
				{
					if( !s.size() )
						continue;

					bool found = false;
					for( auto &l : *level )
					{
						if( !l.name.compare( s ) )
						{
							level = &l.children;
							found = true;
							break;
						}
					}
					if( !found )
					{
						TreeNode n( s );

						level->push_back( n );
						level = &level->back().children;
					}
				}
				TreeNode n( it.name(), it.guid() );
				level->push_back( n );
			}

			sortNodes( nodes );

			return nodes;
		}

		const std::list<TreeNode> &getSupportedTree()
		{
			static std::list<TreeNode> st = createSupportedTree();
			return st;
		}

		void printSupportedTree()
		{
			auto nodes = getSupportedTree();

			std::cout << "--- supported Nodes ---" << std::endl;
			for( auto &it : nodes )
				printNode( it, 0 );
			std::cout << "-----------------------" << std::endl;
		}

		const TreeNode *addNodeItem( const TreeNode &node )
		{
			const TreeNode *ret = nullptr;

			if( node.children.size() )
			{
				//TODO: warn if GUID is set

				if( ImGui::BeginMenu( node.name.c_str() ) )
				{
					for( auto &it : node.children )
					{
						const TreeNode *n = addNodeItem( it );
						if( n )
							ret = n;
					}

					ImGui::EndMenu();
				}
			}
			else
			{
				if( ImGui::MenuItem( node.name.c_str() ) )
				{
					return &node;
				}
			}

			return ret;
		}

		std::list<Node> createSupportedList()
		{
			std::list<Node> nodes;

			for( int i = 0; i < DIT_COUNT; i++ )
				nodes.push_back( Node( interfaceToString( (DataInterfaceType) i ), (DataInterfaceType) i ) );

			for( auto &it : opFactory().getSupported() )
				nodes.push_back( Node( it.name(), it.path(), "", it.guid() ) );

			nodes.sort( compareNodes );

			return nodes;
		}

		const std::list<Node> &getSupportedList()
		{
			static std::list<Node> st = createSupportedList();
			return st;
		}

		std::vector<std::pair<int, Node>> getFilteredSupported( const std::string &filter, size_t maxEntries = 0 )
		{
			std::vector<std::pair<int, Node>> list;

			if( filter.size() > 1 )
			{
				std::string f( toLower( filter ) );

				auto sl = getSupportedList();

				for( auto it : sl )
				{
					int dist = 0;
					std::string n( toLower( it.name ) );

					if( !n.compare( f ) )
						dist = -1000;
					else if( startsWith( n, f ) )
						dist = -1000 + ( n.length() - f.length() );
					else
						dist = levensteinDistance( f, n );
					list.push_back( std::make_pair( dist, it ) );
				}
			}

			std::sort( list.begin(), list.end(), FilteredNodeSort() );
			if( maxEntries && list.size() > maxEntries )
				list.erase( list.begin() + maxEntries, list.end() );

			return list;
		}
	}

	
	bool ElementSort( const GUI::Element *lhs, const GUI::Element *rhs )
	{
		return ( lhs->getZ() < rhs->getZ() );
	}

	SceneGraphDrawer::SceneGraphDrawer( SceneGraph *sg, const UIStyle *uis ) :
		GUI::Drawer( uis ),
		_drawDebug( false ),
		_contextMenuOpen( false ),
		_exitContextMenu( false ),
		_suppressContextMenu( false ),
		_openFinder( false ),
		_closeFinder( false ),
		_finderBuffer( 256 ),
		_finderSelected( 0 ),
		_maxZ( 0 ),
		_sg( sg ),
		_sticky( nullptr ),
		_maximizedDrawer( nullptr ),
		_fpMaximized( new FrameBuffer( "maximized" ) ),
		_maximizedWidth( 2048 ),
		_maximizedHeight( 2048 ),
		_mouseDown( 5 ),
		_selectionWindow( false ),
		_selectionWindowAdditive( false )
	{
		_fpMaximized->build( _maximizedWidth, _maximizedHeight );

		_mapVerts.resize( 4 );
		_mapVerts[0] = glm::vec2( -1, 1 );
		_mapVerts[1] = glm::vec2( -1, 1 - 2 );
		_mapVerts[2] = glm::vec2( -1 + 2, 1 - 2 );
		_mapVerts[3] = glm::vec2( -1 + 2, 1 );

		_mapUVs.resize( 4 );
		_mapUVs[0] = glm::vec2( 0, 1 );
		_mapUVs[1] = glm::vec2( 0, 0 );
		_mapUVs[2] = glm::vec2( 1, 0 );
		_mapUVs[3] = glm::vec2( 1, 1 );

		_mapCols.resize( 4 );
		_mapCols[0] = glm::vec3( 1 );
		_mapCols[1] = glm::vec3( 1 );
		_mapCols[2] = glm::vec3( 1 );
		_mapCols[3] = glm::vec3( 1 );
	}

	SceneGraphDrawer::~SceneGraphDrawer()
	{
		_sticky = nullptr;
		_selectedDrawers.clear();
		_maximizedDrawer = nullptr;

		for( auto &it : _drawers )
			safeDelete( it.second );
		_drawers.clear();

		safeDelete( _fpMaximized );
	}

	void SceneGraphDrawer::build()
	{
		int cntr = 0;
		for( auto &it : _sg->getOps() )
			createOpDrawer( it, glm::vec2( 300, 150 + cntr++ * 200 ) );

		for( auto &it : _sg->getInterfaces() )
			createInterfaceDrawer( it, glm::vec2( 300, 150 + cntr++ * 200 ) );

		load( _sg->getSceneName() );
		resortZ();
	}

	OpDrawer *SceneGraphDrawer::createOpDrawer( Op *op, const glm::vec2 &pos )
	{
		OpDrawer *nd = new OpDrawer( this, op, pos, 
			[this] ( PinDrawer *pd, PinDrawer::Event e )
			{
				this->inletCallback( pd, e );
			},
			[this] ( PinDrawer *pd, PinDrawer::Event e )
			{
				this->outletCallback( pd, e );
			}
			);
		if( !_drawers.insert( std::make_pair( op->getObjectID(), nd ) ).second )
		{
			std::cerr << "<error> drawer for op with ID " << guidToString( op->getObjectID() ) << " already present!" << std::endl;
			safeDelete( nd );
			nd = nullptr;
		}
		else
		{
			nd->build();
			moveToFront( nd );

			resortZ();
		}

		return nd;
	}

	InterfaceDrawer *SceneGraphDrawer::createInterfaceDrawer( DataInterface *di, const glm::vec2 &pos )
	{
		InterfaceDrawer *id = new InterfaceDrawer( this, di, pos );
		if( !_drawers.insert( std::make_pair( di->getObjectID(), id ) ).second )
		{
			std::cerr << "<error> drawer for interface with ID " << guidToString( di->getObjectID() ) << " already present!" << std::endl;
			safeDelete( id );
			id = nullptr;
		}
		else
		{
			id->build();
			moveToFront( id );

			resortZ();
		}

		return id;
	}

	void SceneGraphDrawer::select( NodeDrawer *nd, bool additive )
	{
		std::list<NodeDrawer*> nds;
		if( nd )
			nds.push_back( nd );

		select( nds, additive );
	}

	void SceneGraphDrawer::select( std::list<NodeDrawer*> &nds, bool additive )
	{
		if( !additive )
		{
			for( auto it : _selectedDrawers )
				it->setSelected( false );
			_selectedDrawers.clear();
		}

		for( auto it : nds )
		{
			if( _selectedDrawers.find( it ) == _selectedDrawers.end() )
				_selectedDrawers.insert( it );
			it->setSelected( true );
		}

		resortZ();
	}

	void SceneGraphDrawer::toggleSelection( NodeDrawer *nd, bool additive )
	{
		std::list<NodeDrawer*> nds;
		if( nd )
			nds.push_back( nd );

		toggleSelection( nds, additive );
	}

	void SceneGraphDrawer::toggleSelection( std::list<NodeDrawer*> &nds, bool additive )
	{
		if( !additive )
		{
			for( auto it : _selectedDrawers )
				it->setSelected( false );
			_selectedDrawers.clear();
		}

		for( auto it : nds )
		{
			if( _selectedDrawers.find( it ) == _selectedDrawers.end() )
			{
				_selectedDrawers.insert( it );
				it->setSelected( true );
			}
			else
			{
				_selectedDrawers.erase( it );
				it->setSelected( false );
			}
		}

		resortZ();
	}



	void SceneGraphDrawer::deselect( NodeDrawer *nd )
	{
		std::list<NodeDrawer*> nds;
		if( nd )
			nds.push_back( nd );

		deselect( nds );
	}

	void SceneGraphDrawer::deselect( std::list<NodeDrawer*> &nds )
	{
		for( auto it : nds )
		{
			if( _selectedDrawers.find( it ) == _selectedDrawers.end() )
				continue;

			_selectedDrawers.erase( it );
			it->setSelected( false );
		}

		resortZ();
	}

	Rect SceneGraphDrawer::getSelectionRect() const
	{
		return Rect(
			glm::vec2( min( _selectionWindowP0.x, _mousePos.x ), min( _selectionWindowP0.y, _mousePos.y ) ),
			glm::vec2( max( _selectionWindowP0.x, _mousePos.x ), max( _selectionWindowP0.y, _mousePos.y ) )
		);
	}

	void SceneGraphDrawer::startDrag()
	{
		for( auto it : _selectedDrawers )
			it->setDragging( true );
	}

	void SceneGraphDrawer::moveToFront( NodeDrawer *nd )
	{
		_maxZ = nd->setZ( _maxZ + 1.0f );
	}

	void SceneGraphDrawer::resortZ()
	{
		_elements.sort( ElementSort );

		_maxZ = 0;
		for( auto it : _elements )
			if( dynamic_cast<NodeDrawer*>( it ) )
				_maxZ = it->setZ( _maxZ + 1.0f );
	}

	const OpDrawer *SceneGraphDrawer::getOpDrawer( const GUID &objectID ) const
	{
		auto it = _drawers.find( objectID );
		if( it == _drawers.end() )
			return nullptr;
		return it->second->asOpDrawer();
	}

	const InterfaceDrawer *SceneGraphDrawer::getInterfaceDrawer( const GUID &objectID ) const
	{
		auto it = _drawers.find( objectID );
		if( it == _drawers.end() )
			return nullptr;
		return it->second->asInterfaceDrawer();
	}

	void SceneGraphDrawer::preDraw()
	{
		for( auto it : _elements )
			it->preDraw();

		if( _maximizedDrawer && _maximizedDrawer->getCurrentDrawer() )
			_maximizedDrawer->getCurrentDrawer()->draw( _fpMaximized, _maximizedWidth, _maximizedHeight );
	}

	void SceneGraphDrawer::draw()
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glDisable( GL_DEPTH_TEST );
			glDisable( GL_TEXTURE_2D );
			glDisable( GL_LIGHTING );
			glEnable( GL_COLOR_MATERIAL );

			glEnableClientState( GL_VERTEX_ARRAY );
			glEnableClientState( GL_COLOR_ARRAY );

			if( _maximizedDrawer && _maximizedDrawer->getCurrentDrawer() )
			{
				glPushAttrib( GL_ALL_ATTRIB_BITS );
				{
					glMatrixMode( GL_PROJECTION );
					glPushMatrix();
					glLoadIdentity();
					glMatrixMode( GL_MODELVIEW );
					glPushMatrix();
					glLoadIdentity();

					glVertexPointer( 2, GL_FLOAT, 0, &_mapVerts[0] );
					glColorPointer( 3, GL_FLOAT, 0, &_mapCols[0] );

					glEnableClientState( GL_TEXTURE_COORD_ARRAY );

					glTexCoordPointer( 2, GL_FLOAT, 0, &_mapUVs[0] );

					glEnable( GL_TEXTURE_2D );
					glBindTexture( GL_TEXTURE_2D, _fpMaximized->getRenderTextureName() );

					glDrawArrays( GL_QUADS, 0, _mapVerts.size() );

					glDisableClientState( GL_TEXTURE_COORD_ARRAY );

					glPopMatrix();
					glMatrixMode( GL_PROJECTION );
					glPopMatrix();
					glMatrixMode( GL_MODELVIEW );
				}
				glPopAttrib();
			}

			if( App().getUIActive() )
			{
				for( auto &it : _drawers )
				{
					OpDrawer *od = it.second->asOpDrawer();
					if( od )
						for( auto &id : od->getInlets() )
							id->updateCompatibility( _sticky );
				}

				if( _sg->getSourceFeeds().size() )
				{
					glLineWidth( getUIStyle()->ConnectorLineWidth );

					std::vector<glm::vec2> lineVerts( _sg->getSourceFeeds().size() * 2 );
					std::vector<glm::vec3> lineCols( _sg->getSourceFeeds().size() * 2 );

					std::vector<glm::vec2> triVerts( _sg->getSourceFeeds().size() * 3 );
					std::vector<glm::vec3> triCols( _sg->getSourceFeeds().size() * 3 );

					int cntr = 0;
					for( auto &it : _sg->getSourceFeeds() )
					{
						const InterfaceDrawer *src = getInterfaceDrawer( it.getDataInterface()->getObjectID() );
						const OpDrawer *dst = getOpDrawer( it.getOp()->getObjectID() );

						if( !src )
						{
							std::cerr << "InterfaceDrawer not found" << std::endl;
							continue;
						}
						if( !dst )
						{
							std::cerr << "OpDrawer not found" << std::endl;
							continue;
						}

						glm::vec2 srcPos( src->getPos() + src->getSize() * 0.5f );
						glm::vec2 dstPos( dst->getPos() + dst->getSize() * 0.5f );

						lineVerts[cntr * 2 + 0] = srcPos;
						lineVerts[cntr * 2 + 1] = dstPos;

						lineCols[cntr * 2 + 0] = getUIStyle()->SourceFeedLineColor;
						lineCols[cntr * 2 + 1] = getUIStyle()->SourceFeedLineColor;

						glm::vec2 fwd( dstPos - srcPos );
						float l = glm::length( fwd );
						if( l > std::numeric_limits<float>::epsilon() )
							fwd *= 1.0f / l;
						float t = getAppTime() * 0.5f;
						glm::vec2 side( -fwd.y, fwd.x );
						glm::vec2 center( ( srcPos + dstPos ) * 0.5f + fwd * ( t - (int) t - 0.65f ) * l * 0.75f );
						float size = 10;
						triVerts[cntr * 3 + 0] = center + fwd * size;
						triVerts[cntr * 3 + 1] = center - ( fwd * 0.5f + side ) * size;
						triVerts[cntr * 3 + 2] = center - ( fwd * 0.5f - side ) * size;

						triCols[cntr * 3 + 0] = getUIStyle()->SourceFeedLineColor;
						triCols[cntr * 3 + 1] = getUIStyle()->SourceFeedLineColor;
						triCols[cntr * 3 + 2] = getUIStyle()->SourceFeedLineColor;

						cntr++;
					}

					glVertexPointer( 2, GL_FLOAT, 0, &lineVerts[0] );
					glColorPointer( 3, GL_FLOAT, 0, &lineCols[0] );

					glDrawArrays( GL_LINES, 0, lineVerts.size() );

					glVertexPointer( 2, GL_FLOAT, 0, &triVerts[0] );
					glColorPointer( 3, GL_FLOAT, 0, &triCols[0] );

					glDrawArrays( GL_TRIANGLES, 0, triVerts.size() );
				}

				if( _sg->getSinkFeeds().size() )
				{
					glLineWidth( getUIStyle()->ConnectorLineWidth );

					std::vector<glm::vec2> lineVerts( _sg->getSinkFeeds().size() * 2 );
					std::vector<glm::vec3> lineCols( _sg->getSinkFeeds().size() * 2 );

					std::vector<glm::vec2> triVerts( _sg->getSinkFeeds().size() * 3 );
					std::vector<glm::vec3> triCols( _sg->getSinkFeeds().size() * 3 );

					int cntr = 0;
					for( auto& it : _sg->getSinkFeeds() )
					{
						const OpDrawer* src = getOpDrawer( it.getOp()->getObjectID() );
						const InterfaceDrawer* dst = getInterfaceDrawer( it.getDataInterface()->getObjectID() );

						if( !src )
						{
							std::cerr << "OpDrawer not found" << std::endl;
							continue;
						}
						if( !dst )
						{
							std::cerr << "InterfaceDrawer not found" << std::endl;
							continue;
						}

						glm::vec2 srcPos( src->getPos() + src->getSize() * 0.5f );
						glm::vec2 dstPos( dst->getPos() + dst->getSize() * 0.5f );

						lineVerts[cntr * 2 + 0] = srcPos;
						lineVerts[cntr * 2 + 1] = dstPos;

						lineCols[cntr * 2 + 0] = getUIStyle()->SinkFeedLineColor;
						lineCols[cntr * 2 + 1] = getUIStyle()->SinkFeedLineColor;

						glm::vec2 fwd( dstPos - srcPos );
						float l = glm::length( fwd );
						if( l > std::numeric_limits<float>::epsilon() )
							fwd *= 1.0f / l;
						float t = getAppTime() * 0.5f;
						glm::vec2 side( -fwd.y, fwd.x );
						glm::vec2 center( ( srcPos + dstPos ) * 0.5f + fwd * ( t - (int) t - 0.5f ) * l * 0.9f );
						float size = 10;
						triVerts[cntr * 3 + 0] = center + fwd * size;
						triVerts[cntr * 3 + 1] = center - ( fwd * 0.5f + side ) * size;
						triVerts[cntr * 3 + 2] = center - ( fwd * 0.5f - side ) * size;

						triCols[cntr * 3 + 0] = getUIStyle()->SinkFeedLineColor;
						triCols[cntr * 3 + 1] = getUIStyle()->SinkFeedLineColor;
						triCols[cntr * 3 + 2] = getUIStyle()->SinkFeedLineColor;

						cntr++;
					}

					glVertexPointer( 2, GL_FLOAT, 0, &lineVerts[0] );
					glColorPointer( 3, GL_FLOAT, 0, &lineCols[0] );

					glDrawArrays( GL_LINES, 0, lineVerts.size() );

					glVertexPointer( 2, GL_FLOAT, 0, &triVerts[0] );
					glColorPointer( 3, GL_FLOAT, 0, &triCols[0] );

					glDrawArrays( GL_TRIANGLES, 0, triVerts.size() );
				}


				if( _sg->getConnectors().size() )
				{
					glLineWidth( getUIStyle()->ConnectorLineWidth );

#ifdef __DRAW_SPLINE_CONNECTORS
					int resolution = 50;
					std::vector<glm::vec2> verts( resolution );
					std::vector<glm::vec3> cols( resolution );

					for( int i = 0; i < resolution; i++ )
						cols[i] = getUIStyle()->ConnectorLineColor;

					glColorPointer( 3, GL_FLOAT, 0, &cols[0] );

					for( auto &it : _sg->getConnectors() )
					{
						const OpDrawer *srcNode = getOpDrawer( it->getSrcPin()->getOp()->getObjectID() );
						const OpDrawer *dstNode = getOpDrawer( it->getDstPin()->getOp()->getObjectID() );

						const PinDrawer *srcPin = srcNode->getOutlet( it->getSrcPin()->getName() );
						const PinDrawer *dstPin = dstNode->getInlet( it->getDstPin()->getName() );

						glm::vec2 p1( srcPin->getWorldPos() );
						glm::vec2 p2( dstPin->getWorldPos() );

						float dist = glm::distance( p1, p2 ) * 2;

						glm::vec2 p0( p1 - glm::vec2( 0, dist ) );
						glm::vec2 p3( p2 + glm::vec2( 0, dist ) );

						for( int i = 0; i < resolution; i++ )
							verts[i] = interpolateCatRomSpline( p0, p1, p2, p3, i / ( resolution - 1.0f ) );

						glVertexPointer( 2, GL_FLOAT, 0, &verts[0] );

						glDrawArrays( GL_LINE_STRIP, 0, verts.size() );
					}
#else
					std::vector<glm::vec2> verts( _sg->getConnectors().size() * 2 );
					std::vector<glm::vec3> cols( _sg->getConnectors().size() * 2 );

					for( int i = 0; i < cols.size(); i++ )
						cols[i] = getUIStyle()->ConnectorLineColor;

					int cntr = 0;

					for( auto &it : _sg->getConnectors() )
					{
						const OpDrawer *srcNode = getOpDrawer( it->getSrcPin()->getOp()->getObjectID() );
						const OpDrawer *dstNode = getOpDrawer( it->getDstPin()->getOp()->getObjectID() );

						const PinDrawer *srcPin = srcNode->getOutlet( it->getSrcPin()->getName() );
						const PinDrawer *dstPin = dstNode->getInlet( it->getDstPin()->getName() );

						verts[cntr * 2 + 0] = srcPin->getWorldPos();
						verts[cntr * 2 + 1] = dstPin->getWorldPos();

						cntr++;
					}

					glVertexPointer( 2, GL_FLOAT, 0, &verts[0] );
					glColorPointer( 3, GL_FLOAT, 0, &cols[0] );

					glDrawArrays( GL_LINE_STRIP, 0, verts.size() );
#endif
				}

				if( _sticky )
				{
					std::vector<glm::vec2> verts( 2 );
					std::vector<glm::vec3> cols( 2 );

					verts[0] = _mousePos;
					verts[1] = _sticky->getWorldPos();

					cols[0] = getUIStyle()->RubberColor;
					cols[1] = getUIStyle()->RubberColor;

					glLineWidth( getUIStyle()->RubberLineWidth );

					glVertexPointer( 2, GL_FLOAT, 0, &verts[0] );
					glColorPointer( 3, GL_FLOAT, 0, &cols[0] );
					glDrawArrays( GL_LINES, 0, verts.size() );
				}

				for( auto it : _elements )
					it->draw();

				if( _selectionWindow )
				{
					std::vector<glm::vec2> verts( 4 );
					std::vector<glm::vec3> cols( 4, getUIStyle()->SelectionLineColor );

					Rect r = getSelectionRect();
					verts[0] = r.P0();
					verts[2] = r.P1();
					verts[1] = glm::vec2( verts[0].x, verts[2].y );
					verts[3] = glm::vec2( verts[2].x, verts[0].y );

					glVertexPointer( 2, GL_FLOAT, 0, &verts[0] );
					glColorPointer( 3, GL_FLOAT, 0, &cols[0] );
					glDrawArrays( GL_LINE_LOOP, 0, verts.size() );

					glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT );
					{
						glEnable( GL_BLEND );
						glBlendFunc( GL_ONE, GL_ONE );

						cols.assign( 4, getUIStyle()->SelectionFillColor );

						glColorPointer( 3, GL_FLOAT, 0, &cols[0] );
						glDrawArrays( GL_QUADS, 0, verts.size() );
					}
					glPopAttrib();
				}
			}

			glDisableClientState( GL_VERTEX_ARRAY );
			glDisableClientState( GL_COLOR_ARRAY );
		}
		glPopAttrib();
	}

	void SceneGraphDrawer::drawUI()
	{
		if( _openFinder )
		{
			ImGui::SetNextWindowPos( ImVec2( ( App().getWindowSize().x - getUIStyle()->FinderWidth ) / 2.0f, App().getWindowSize().y / 2.0f ) );
			ImGui::SetNextWindowSize( ImVec2( getUIStyle()->FinderWidth, 0 ) );
			ImGui::OpenPopup( "Finder" );
		}

		//TODO: figure out a future-proof way of doing this -- ImGuiWindowFlags_Modal is internal and not supposed to be used
		// we should use BeginPopupModal instead, but this does not behave as required, it'd contain the Selectable items within the
		// window which we don't want.
		if( ImGui::BeginPopup( "Finder", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_Modal ) )
		{
			if( ImGui::IsKeyPressed( (ImGuiKey)ImGui::GetKeyIndex( ImGuiKey_Escape ), false ) )
				_closeFinder = true;

			if( _openFinder )
				ImGui::SetKeyboardFocusHere( 0 );

			if( _closeFinder )
				ImGui::CloseCurrentPopup();
			else
			{
				if( ImGui::IsKeyPressed( (ImGuiKey)ImGui::GetKeyIndex( ImGuiKey_UpArrow ), false ) )
					_finderSelected--;
				else if( ImGui::IsKeyPressed( (ImGuiKey)ImGui::GetKeyIndex( ImGuiKey_DownArrow ), false ) )
					_finderSelected++;

				auto filtered = Internal::getFilteredSupported( std::string( &_finderBuffer[0] ), 10 );
				if( filtered.size() )
					_finderSelected = clamp<int>( _finderSelected, 0, filtered.size() - 1 );
				else
					_finderSelected = 0;

				if( ImGui::InputText( "##Find", &_finderBuffer[0], _finderBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					if( filtered.size() )
					{
						if( filtered[_finderSelected].second.isOp )
							select( instantiateOp( filtered[_finderSelected].second.guid, glm::vec2( App().getWindowSize() ) * 0.5f ) );
						else
							select( instantiateInterface( filtered[_finderSelected].second.dataInterface, glm::vec2( App().getWindowSize() ) * 0.5f ) );
					}

					_finderBuffer[0] = 0;
					_finderSelected = 0;

					ImGui::CloseCurrentPopup();
				}

				for( int i = 0; i < filtered.size(); i++ )
				{
					bool selected = ( i == _finderSelected );

					std::stringstream sstr;
					if( filtered[i].second.isOp )
						sstr << filtered[i].second.name << " (" << filtered[i].second.path << ")";// [" << filtered[i].desc << "]";
					else
						sstr << filtered[i].second.name << " [" << filtered[i].second.desc << "]";

					if( ImGui::Selectable( sstr.str().c_str(), &selected ) )
					{
						_finderSelected = i;

						if( filtered[_finderSelected].second.isOp )
							select( instantiateOp( filtered[_finderSelected].second.guid, glm::vec2( App().getWindowSize() ) * 0.5f ) );
						else
							select( instantiateInterface( filtered[_finderSelected].second.dataInterface, glm::vec2( App().getWindowSize() ) * 0.5f ) );
					}
				}
			}

			ImGui::EndPopup();
		}

		_openFinder = false;
		_closeFinder = false;

		if( ImGui::Begin( "Inspector" ) )
		{
			for( auto it : _elements )
				if( it->drawUI() )
					ImGui::Separator();
		}
		ImGui::End();

		if( !_suppressContextMenu && !_exitContextMenu &&
			ImGui::BeginPopupContextVoid( "scene context menu", 1 ) )
		{
			DataInterfaceType selectedInterface = DIT_COUNT;

			_contextMenuOpen = true;

			if( ImGui::BeginMenu( "interfaces" ) )
			{
				for( auto &it : Internal::getInterfaceMap() )
				{
					if( ImGui::MenuItem( it.second ) )
						selectedInterface = it.first;
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			const Internal::TreeNode *selectedNode = nullptr;
			for( auto &it : Internal::getSupportedTree() )
			{
				const Internal::TreeNode *node = Internal::addNodeItem( it );
				if( node )
					selectedNode = node;
			}
			ImGui::EndPopup();

			if( selectedInterface != DIT_COUNT )
				select( instantiateInterface( selectedInterface, _creationPos ) );
			else if( selectedNode )
				select( instantiateOp( selectedNode->guid, _creationPos ) );
		}
		else
			_contextMenuOpen = false;
	}

	bool SceneGraphDrawer::mouseDown( int button, int mods, bool imGuiHandled )
	{
		bool ret = false;

		if( App().getUIActive() )
		{
			if( button < _mouseDown.size() )
				_mouseDown[button] = true;

			//std::cout << "imGuiHandled: " << imGuiHandled << std::endl;

			bool hit = false;
			for( auto it : _elements )
				if( it->hitTest( _mousePos ) )
					hit = true;

			if( button == 1 )
			{
				if( _contextMenuOpen )
				{
					_exitContextMenu = true;
					std::cout << "exit context menu" << std::endl;
				}
				else
					_exitContextMenu = false;

				if( hit )
					_suppressContextMenu = true;
				else
					_suppressContextMenu = false;
			}
			else if( _contextMenuOpen && !imGuiHandled )	//only close when not handled by ImGui, otherwise we close the menu right at the moment the user clicked an item in the context menu
			{
				_exitContextMenu = true;
			}

			bool drawerHit = false;
			bool drawerChildHit = false;
			for( auto &it : _drawers )
			{
				if( it.second->mouseDown( button, mods, imGuiHandled ) )
				{
					if( !it.second->childHit( _mousePos ) )
					{
						moveToFront( it.second );
						if( !it.second->getSelected() )		//if already selected, don't do anything, otherwise we will potentially cause a deselect of multiple selecte nodes
						{
							if( mods & ( GLFW_MOD_CONTROL | GLFW_MOD_SHIFT ) )	//if ctrl-/shift-selected, add to current selection
								select( it.second, true );
							else
								select( it.second );
						}
						else if( mods & ( GLFW_MOD_CONTROL | GLFW_MOD_SHIFT ) )	//if ctrl-/shift-clicked, remove from current selection
							deselect( it.second );
					}
					else
						drawerChildHit = true;

					ret = true;
					drawerHit = true;
				}
			}
			if( button == 0 && !imGuiHandled )
			{
				if( drawerHit )
				{
					if( mods == 0 && !drawerChildHit )
						startDrag();
				}
				else
				{
					_selectionWindow = true;
					_selectionWindowAdditive = ( mods & GLFW_MOD_SHIFT );
					_selectionWindowP0 = _mousePos;
				}
			}
		}
		else if( _maximizedDrawer && _maximizedDrawer->getCurrentDrawer() )
			ret |= _maximizedDrawer->getCurrentDrawer()->mouseDown( button, mods, imGuiHandled );

		return ret;
	}

	bool SceneGraphDrawer::mouseUp( int button, int mods, bool imGuiHandled )
	{
		bool ret = false;

		if( App().getUIActive() )
		{
			if( button < _mouseDown.size() )
				_mouseDown[button] = false;

			for( auto &it : _drawers )
			{
				it.second->mouseUp( button, mods, imGuiHandled );
				//if( it.second->mouseUp( button, mods, imGuiHandled ) )
				//	select( it.second );

				//ret = true;
			}

			bool hit = false;
			for( auto it : _elements )
				if( it->hitTest( _mousePos ) )
					hit = true; //TODO: suppress context menu when object was clicked

			if( button == 1 )
				_creationPos = im2glm( ImGui::GetMousePos() );
		}
		else if( _maximizedDrawer && _maximizedDrawer->getCurrentDrawer() )
			ret |= _maximizedDrawer->getCurrentDrawer()->mouseUp( button, mods, imGuiHandled );

		if( button == 0 )
		{
			_sticky = nullptr;

			if( _selectionWindow )
			{
				_selectionWindow = false;

				Rect sr( getSelectionRect() );
				std::list<NodeDrawer*> overlapped;
				for( auto &it : _drawers )
				{
					if( it.second->overlap( sr ) )
						overlapped.push_back( it.second );
				}
				toggleSelection( overlapped, _selectionWindowAdditive );
			}
		}

		return ret;
	}

	void SceneGraphDrawer::mouseMotion( const glm::vec2 &pos )
	{
		if( App().getUIActive() )
		{
			glm::vec2 oldMousePos( _mousePos );
			_mousePos = ( App().getMVCanvasInv() * glm::vec4( pos.x, pos.y, 0, 1 ) ).xy;
			glm::vec2 mouseMotion( _mousePos - oldMousePos );

			if( _mouseDown[1] )
				_suppressContextMenu = true;
			else
				_suppressContextMenu = false;

			GUI::Element *hovered = nullptr;
			for( auto it : _elements )
			{
				it->mouseMotion( _mousePos );

				GUI::Draggable *d = dynamic_cast<GUI::Draggable*>( it );
				if( d && d->getDragging() )
					it->setPos( it->getPos() + mouseMotion );

				GUI::Hoverable *h = dynamic_cast<GUI::Hoverable*>( it );
				if( h )
				{
					h->setHovered( false );
					if( it->hitTest( _mousePos ) )
						if( !hovered || it->getZ() > hovered->getZ() )
							hovered = it;
				}
			}
			if( hovered )
				dynamic_cast<GUI::Hoverable*>( hovered )->setHovered( true );
		}
		else if( _maximizedDrawer && _maximizedDrawer->getCurrentDrawer() )
			_maximizedDrawer->getCurrentDrawer()->mouseMotion( pos );
	}

	bool SceneGraphDrawer::scroll( const glm::vec2 &offset, bool imGuiHandled )
	{
		bool ret = false;
		if( App().getUIActive() )
		{
			for( auto it : _elements )
				ret |= it->scroll( offset, imGuiHandled );
		}
		else if( _maximizedDrawer && _maximizedDrawer->getCurrentDrawer() )
			ret |= _maximizedDrawer->getCurrentDrawer()->scroll( offset, imGuiHandled );

		return ret;
	}

	bool SceneGraphDrawer::keyDown( int key, int scanCode, int action, int mods, bool imGuiHandled )
	{
		bool ret = false;

		if( App().getUIActive() )
		{
			if( !imGuiHandled )
			{
				switch( key )
				{
				case GLFW_KEY_F1:
				{
					if( action == GLFW_PRESS )
					{
						_openFinder = true;
						_finderBuffer[0] = 0;
						_finderSelected = 0;
					}
					break;
				}
				case GLFW_KEY_DELETE:
				{
					if( action == GLFW_RELEASE )
					{
						auto it = _drawers.begin();
						while( it != _drawers.end() )
						{
							if( it->second->getSelected() )
							{
								if( _maximizedDrawer == it->second )
									setMaximized( nullptr );

								OpDrawer *od = it->second->asOpDrawer();
								InterfaceDrawer *id = it->second->asInterfaceDrawer();

								if( od )
								{
									if( _sg->destroy( od->getOp() ) )
									{
										delete( it->second );
										it = _drawers.erase( it );

										ret = true;

										continue;
									}
									else
										std::cerr << "<error> failed to delete Op (mem-leak?)" << std::endl;
								}
								else if( id )
								{
									if( _sg->destroyInterface( id->getInterface() ) )
									{
										delete( it->second );
										it = _drawers.erase( it );

										ret = true;

										continue;
									}
									else
										std::cerr << "<error> failed to delete source (mem-leak?)" << std::endl;
								}
							}

							it++;
						}
					}

					break;
				}
				case GLFW_KEY_C:
				{
					if( action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL )
						copyToClipboard();
					break;
				}
				case GLFW_KEY_V:
				{
					if( action == GLFW_PRESS && mods == GLFW_MOD_CONTROL )
						pasteFromClipboard();
					break;
				}
				}
			}
		}
		else if( _maximizedDrawer && _maximizedDrawer->getCurrentDrawer() )
			ret |= _maximizedDrawer->getCurrentDrawer()->keyDown( key, scanCode, action, mods, imGuiHandled );

		return ret;
	}

	bool SceneGraphDrawer::charDown( unsigned char c, bool imGuiHandled )
	{
		bool ret = false;
		if( App().getUIActive() )
		{
		}
		else if( _maximizedDrawer && _maximizedDrawer->getCurrentDrawer() )
			ret |= _maximizedDrawer->getCurrentDrawer()->charDown( c, imGuiHandled );

		return ret;
	}

	OpDrawer *SceneGraphDrawer::instantiateOp( const GUID &classID, const glm::vec2 &atScreenPos )
	{
		Op *op = _sg->instantiate( classID );
		if( !op )
			return nullptr;

		OpDrawer *od = createOpDrawer( op, glm::vec2() );
		if( !od )
		{
			_sg->destroy( op );
			op = nullptr;
		}

		od->setPos( ( App().getMVCanvasInv() * glm::vec4(
			atScreenPos.x - od->getSize().x * 0.5f,
			atScreenPos.y - od->getSize().y * 0.5f,
			0, 1 ) ).xy );

		return od;
	}

	InterfaceDrawer *SceneGraphDrawer::instantiateInterface( DataInterfaceType dit, const glm::vec2 &atScreenPos )
	{
		DataInterface *di = _sg->createInterface( dit );
		if( !di )
			return nullptr;

		InterfaceDrawer *sd = createInterfaceDrawer( di, glm::vec2() );
		if( !sd )
		{
			_sg->destroyInterface( di );
			di = nullptr;
		}

		sd->setPos( ( App().getMVCanvasInv() * glm::vec4(
			atScreenPos.x - sd->getSize().x * 0.5f,
			atScreenPos.y - sd->getSize().y * 0.5f,
			0, 1 ) ).xy );

		return sd;
	}

	bool SceneGraphDrawer::getBB( Rect &bb ) const
	{
		bool first = true;
		glm::vec2 minPos;
		glm::vec2 maxPos;

		for( auto &it : _drawers )
		{
			if( first )
			{
				minPos = it.second->getPos();
				maxPos = it.second->getPos() + it.second->getSize();
				first = false;
			}
			else
			{
				minPos = glm::min( minPos, it.second->getPos() );
				maxPos = glm::max( maxPos, it.second->getPos() + it.second->getSize() );
			}
		}

		if( first )
			return false;

		bb = Rect( minPos, maxPos );
		return true;
	}

	bool SceneGraphDrawer::getBBSelected( Rect &bb ) const
	{
		bool first = true;
		glm::vec2 minPos;
		glm::vec2 maxPos;

		for( auto &it : _drawers )
			if( it.second->getSelected() )
			{
				if( first )
				{
					minPos = it.second->getPos();
					maxPos = it.second->getPos() + it.second->getSize();
					first = false;
				}
				else
				{
					minPos = glm::min( minPos, it.second->getPos() );
					maxPos = glm::max( maxPos, it.second->getPos() + it.second->getSize() );
				}
			}

		if( first )
			return false;

		bb = Rect( minPos, maxPos );
		return true;
	}
	
	void SceneGraphDrawer::copyToClipboard()
	{
		if( _selectedDrawers.size() )
		{
			using json = nlohmann::json;

			json ui;
			json sg;

			json ops = json::array();
			json uiOps = json::array();

			int cntr = 0;
			for( auto &it : _selectedDrawers )
			{
				OpDrawer *od = it->asOpDrawer();
				if( od )
				{
					json p;
					json uip;

					od->getOp()->saveToJSON( p );
					od->saveToJSON( uip );

					ops[cntr] = p;
					uiOps[cntr] = uip;

					cntr++;
				}
			}
			sg["ops"] = ops;
			ui["ops"] = uiOps;

			json interfaces = json::array();
			json uiInterfaces = json::array();

			cntr = 0;
			for( auto &it : _selectedDrawers )
			{
				InterfaceDrawer *id = it->asInterfaceDrawer();
				if( id )
				{
					json p;
					json uip;

					p["interface"] = interfaceToString( id->getInterface()->getDataInterfaceType() );

					id->getInterface()->saveToJSON( p );
					id->saveToJSON( uip );

					interfaces[cntr] = p;
					uiInterfaces[cntr] = uip;

					cntr++;
				}
			}
			sg["interfaces"] = interfaces;
			ui["interfaces"] = uiInterfaces;

			json connectors = json::array();
			cntr = 0;
			for( auto &it : _selectedDrawers )
			{
				OpDrawer *od = it->asOpDrawer();
				if( !od )
					continue;
				
				Op *opOut = od->getOp();
				auto outlets = opOut->getOutlets();

				//iterate Op's outlets
				for( auto o : outlets )
				{
					OutletPin *outlet = o.second;
					auto ocs = outlet->getConnectors();

					//iterate outlet's connectors
					for( auto c : ocs )
					{
						//get connected inlet (if any)
						InletPin *inlet = c->getDstPin();
						if( !inlet )
							continue;

						Op *opIn = inlet->getOp();
						if( !opIn )
							throw std::runtime_error( "inlet not associted with any Op?" );

						//check if Op is part of selection
						bool found = false;
						for( auto it2 : _selectedDrawers )
						{
							OpDrawer *od2 = it2->asOpDrawer();
							if( !od2 )
								continue;

							if( opIn == od2->getOp() )
								found = true;
						}
						//if not part of selection, then don't include connector in JSON
						if( !found )
							continue;

						json connector;

						json src;
						json dst;

						src["name"] = outlet->getName();
						src["obj"] = guidToString( opOut->getObjectID() );

						dst["name"] = inlet->getName();
						dst["obj"] = guidToString( opIn->getObjectID() );

						connector["src"] = src;
						connector["dst"] = dst;

						connectors[cntr++] = connector;
					}
				}
			}
			sg["connectors"] = connectors;

			if( _maximizedDrawer )
				ui["maximized"] = guidToString( _maximizedDrawer->getOp()->getObjectID() );

			json root;
			root["sg"] = sg;
			root["ui"] = ui;

			std::stringstream sstr;
			sstr << std::setw( 2 ) << root;

			App().writeToClipboard( sstr.str() );

			std::cout << "copied selection to clipboard" << std::endl;
		}
		else
		{
			App().writeToClipboard( "" );

			std::cout << "nothing selected to copy to clipboard" << std::endl;
		}
	}

	void SceneGraphDrawer::pasteFromClipboard()
	{
		std::string str( App().readFromClipboard() );

		if( !str.size() )
		{
			std::cout << "clipboard is empty, nothing to paste" << std::endl;
			return;
		}

		std::stringstream sstr;
		sstr << str;

		try
		{
			using json = nlohmann::json;

			std::map<GUID, GUID, CompareGUID> opObjIdMapping;
			std::map<GUID, GUID, CompareGUID> interfaceObjIdMapping;

			std::map<GUID, OpDrawer*, CompareGUID> newOpDrawers;
			std::map<GUID, InterfaceDrawer*, CompareGUID> newInterfaceDrawers;

			Rect bb;
			bool bbFirst = true;

			json root;
			sstr >> root;

			json sg = root["sg"];

			json ops = sg["ops"];
			for( auto it = ops.begin(); it != ops.end(); ++it )
			{
				json p = it.value();

				GUID clsID = guidFromString( p["classID"] );
				GUID objID = guidFromString( p["objectID"] );

				std::cout << "op copied from " << guidToString( objID );

				OpDrawer *od = instantiateOp( clsID, glm::vec2() );
				if( !od )
				{
					std::cerr << "<error> could not instantiate OpDrawer from clipboard" << std::endl;
					continue;
				}
				GUID newObjID = od->getOp()->getObjectID();

				std::cout << " got new ID " << guidToString( newObjID );

				//overwrite with new GUID
				sqid::save<GUID>( p, "objectID", newObjID );
				if( !od->getOp()->loadFromJSON( p ) )
				{
					std::cerr << "<error> failed to initialize OpDrawer from JSON" << std::endl;
					continue;
				}

				std::cout << " now has ID " << guidToString( od->getOp()->getObjectID() ) << std::endl;

				opObjIdMapping.insert( std::make_pair( objID, newObjID ) );
				newOpDrawers.insert( std::make_pair( newObjID, od ) );
			}

			json interfaces = sg["interfaces"];
			for( auto it = interfaces.begin(); it != interfaces.end(); ++it )
			{
				json p = it.value();

				GUID objID = guidFromString( p["objectID"] );

				std::string diString;
				sqid::load<std::string>( p, "interface", diString );
				DataInterfaceType dit = interfaceFromString( diString );
				if( dit == DIT_COUNT )
					throw std::runtime_error( "unable to parse interface type" );

				InterfaceDrawer *id = instantiateInterface( dit, glm::vec2() );
				if( !id )
				{
					std::cerr << "<error> could not instantiate InterfaceDrawer from clipboard" << std::endl;
					continue;
				}
				GUID newObjID = id->getInterface()->getObjectID();

				//overwrite with new GUID
				sqid::save<GUID>( p, "objectID", newObjID );
				if( !id->getInterface()->loadFromJSON( p ) )
				{
					std::cerr << "<error> failed to initialize InterfaceDrawer from JSON" << std::endl;
					continue;
				}

				interfaceObjIdMapping.insert( std::make_pair( objID, newObjID ) );
				newInterfaceDrawers.insert( std::make_pair( newObjID, id ) );
			}

			json connections = sg["connectors"];
			for( auto it = connections.begin(); it != connections.end(); ++it )
			{
				json src = it.value()["src"];
				json dst = it.value()["dst"];

				std::string srcName = src["name"].get<std::string>();
				std::string srcGuidStr = src["obj"].get<std::string>();
				GUID srcID = guidFromString( srcGuidStr );

				std::string dstName = dst["name"].get<std::string>();
				std::string dstGuidStr = dst["obj"].get<std::string>();
				GUID dstID = guidFromString( dstGuidStr );

				auto newSrcID = opObjIdMapping.find( srcID );
				auto newDstID = opObjIdMapping.find( dstID );

				if( newSrcID == opObjIdMapping.end() )
				{
					std::cerr << "<error> connected source Op not found in GUID map" << std::endl;
					continue;
				}
				if( newDstID == opObjIdMapping.end() )
				{
					std::cerr << "<error> connected dest Op not found in GUID map" << std::endl;
					continue;
				}

				Op *opSrc = _sg->getOp( newSrcID->second );
				Op *opDst = _sg->getOp( newDstID->second );

				if( !opSrc )
				{
					std::cerr << "<error> connected source Op not found in scenegraph" << std::endl;
					continue;
				}
				if( !opDst )
				{
					std::cerr << "<error> connected dest Op not found in scenegraph" << std::endl;
					continue;
				}

				if( !_sg->connect( opSrc->getOutlet( srcName ), opDst->getInlet( dstName ), true ) )
				{
					std::cerr << "<error> unable to connect " << srcName << " to " << dstName << std::endl;
					continue;
				}
			}

			json ui = root["ui"];

			json uiOps = ui["ops"];
			for( auto it = uiOps.begin(); it != uiOps.end(); ++it )
			{
				json p = it.value();

				GUID clsID = guidFromString( p["classID"] );
				GUID objID = guidFromString( p["objectID"] );

				GUID newObjID = opObjIdMapping[objID];

				auto it2 = newOpDrawers.find( newObjID );
				if( it2 == newOpDrawers.end() )
				{
					std::cerr << "<error> opDrawer not found" << std::endl;
					continue;
				}
				OpDrawer *od = it2->second;

				//overwrite with new GUID
				sqid::save<GUID>( p, "objectID", newObjID );

				float z = od->getZ();
				if( !od->loadFromJSON( p ) )
				{
					std::cerr << "<error> failed to initialize opDrawer from JSON" << std::endl;
					continue;
				}
				od->setZ( z );

				if( bbFirst )
				{
					bb.P0() = od->getPos();
					bb.P1() = od->getPos();
					bbFirst = false;
				}
				else
				{
					bb.P0() = glm::min( bb.P0(), od->getPos() );
					bb.P1() = glm::max( bb.P1(), od->getPos() );
				}
			}

			json uiInterfaces = ui["interfaces"];
			for( auto it = uiInterfaces.begin(); it != uiInterfaces.end(); ++it )
			{
				json p = it.value();

				GUID objID = guidFromString( p["objectID"] );
				GUID newIdID = interfaceObjIdMapping[objID];

				auto it2 = newInterfaceDrawers.find( newIdID );
				if( it2 == newInterfaceDrawers.end() )
				{
					std::cerr << "<error> interfaceDrawer not found" << std::endl;
					continue;
				}
				InterfaceDrawer *id = it2->second;

				//overwrite with new GUID
				sqid::save<GUID>( p, "objectID", newIdID );

				float z = id->getZ();
				if( !id->loadFromJSON( p ) )
				{
					std::cerr << "<error> failed to initialize InterfaceDrawer from JSON" << std::endl;
					continue;
				}
				id->setZ( z );

				if( bbFirst )
				{
					bb.P0() = id->getPos();
					bb.P1() = id->getPos();
					bbFirst = false;
				}
				else
				{
					bb.P0() = glm::min( bb.P0(), id->getPos() );
					bb.P1() = glm::max( bb.P1(), id->getPos() );
				}
			}

			std::list<NodeDrawer*> newDrawers;
			glm::vec2 windowCenter = glm::vec2( App().getWindowSize() ) * 0.5f;
			glm::vec2 worldPosWindowCenter = ( App().getMVCanvasInv() * glm::vec4(
				windowCenter.x,
				windowCenter.y,
				0, 1 ) ).xy;
			glm::vec2 offset = worldPosWindowCenter - bb.Center();

			for( auto it : newOpDrawers )
			{
				newDrawers.push_back( it.second );
				it.second->setPos( it.second->getPos() + offset );
			}
			for( auto it : newInterfaceDrawers )
			{
				newDrawers.push_back( it.second );
				it.second->setPos( it.second->getPos() + offset );
			}

			select( newDrawers );
		}
		catch( std::exception &e )
		{
			std::cerr << "<warning> failed to parse JSON from clipboard: " << e.what() << std::endl;
		}
	}

	bool SceneGraphDrawer::load( const std::string &scene )
	{
		std::string filename = scene + ".ui.json";
		std::ifstream i( filename );
		if( !i.is_open() )
		{
			std::cerr << "<error> failed to open file " << filename << " for loading scene" << std::endl;
			return false;
		}

		using json = nlohmann::json;

		json root;
		i >> root;

		_maxZ = 0;

		json ops = root["ops"];
		if( !ops.is_null() )
		{
			for( auto it = ops.begin(); it != ops.end(); ++it )
			{
				json p = it.value();

				GUID clsID = guidFromString( p["classID"] );
				GUID objID = guidFromString( p["objectID"] );

				auto ndIt = _drawers.find( objID );
				if( ndIt == _drawers.end() )
					continue;

				OpDrawer *od = ndIt->second->asOpDrawer();
				if( !od )
				{
					std::cerr << "<error> drawer with objectID " << guidToString( objID ) << ") is not of type OpDrawer" << std::endl;
					continue;
				}

				if( clsID != od->getOp()->getClassID() )
				{
					std::cerr << "<warning> classID does not match (" << guidToString( objID ) << ") in " << filename << std::endl;
					continue;
				}

				ndIt->second->loadFromJSON( p );

				_maxZ = std::max<float>( _maxZ, ndIt->second->getZ() );
			}
		}

		json interfaces = root["interfaces"];
		if( !interfaces.is_null() )
		{
			for( auto it = interfaces.begin(); it != interfaces.end(); ++it )
			{
				json s = it.value();

				GUID objID = guidFromString( s["objectID"] );

				auto iIt = _drawers.find( objID );
				if( iIt == _drawers.end() )
					continue;

				InterfaceDrawer *id = iIt->second->asInterfaceDrawer();
				if( !id )
				{
					std::cerr << "<error> drawer with objectID " << guidToString( objID ) << ") is not of type InterfaceDrawer" << std::endl;
					continue;
				}

				iIt->second->loadFromJSON( s );

				_maxZ = std::max<float>( _maxZ, iIt->second->getZ() );
			}
		}

		resortZ();

		json maximized = root["maximized"];
		if( !maximized.is_null() )
		{
			GUID objID = guidFromString( maximized );
			auto ndIt = _drawers.find( objID );
			if( ndIt == _drawers.end() )
				std::cerr << "<warning> previously maximized drawer not found" << std::endl;
			else
			{
				OpDrawer *od = ndIt->second->asOpDrawer();
				if( !od )
				{
					std::cerr << "<warning> drawer with objectID " << guidToString( objID ) << ") is not of type OpDrawer, unable to maximize" << std::endl;
				}
				else
				{
					setMaximized( od );
				}
			}
		}

		return true;
	}

	bool SceneGraphDrawer::save( const std::string &scene )
	{
		std::string filename = scene + ".ui.json";

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
		for( auto &it : _drawers )
		{
			if( !it.second->asOpDrawer() )
				continue;

			nlohmann::json p;

			if( it.second )
			{
				json p;

				it.second->saveToJSON( p );

				ops[cntr++] = p;
			}
			else
				std::cerr << "<warning> failed to save nodeDrawer " << guidToString( it.first ) << " to JSON" << std::endl;
		}
		root["ops"] = ops;

		json ids = json::array();

		cntr = 0;
		for( auto &it : _drawers )
		{
			if( !it.second->asInterfaceDrawer() )
				continue;

			nlohmann::json p;

			if( it.second )
			{
				json id;

				it.second->saveToJSON( id );

				ids[cntr++] = id;
			}
			else
				std::cerr << "<warning> failed to save nodeDrawer " << guidToString( it.first ) << " to JSON" << std::endl;
		}
		root["interfaces"] = ids;

		if( _maximizedDrawer )
			root["maximized"] = guidToString( _maximizedDrawer->getOp()->getObjectID() );

		o << std::setw( 2 ) << root;

		return true;
	}

	void SceneGraphDrawer::setMaximized( OpDrawer *drawer )
	{
		if( drawer == _maximizedDrawer )
			return;

		if( _maximizedDrawer )
			_maximizedDrawer->setMaximized( false );
		_maximizedDrawer = drawer;
		if( _maximizedDrawer )
			_maximizedDrawer->setMaximized( true );
	}



	void SceneGraphDrawer::inletCallback( PinDrawer *pinDrawer, PinDrawer::Event e )
	{
		switch( e )
		{
		case PinDrawer::E_PRESSED:
			break;
		case GUI::Button::E_RELEASED:
		{
			InletPinDrawer *ipd = dynamic_cast<InletPinDrawer*>( pinDrawer );
			if( ipd )
			{
				if( _sticky )
				{
					if( !_sg->connect( _sticky->getOutlet(), ipd->getInlet() ) )
						std::cerr << "<error> unable to connect" << std::endl;

					_sticky = nullptr;
				}
				else
				{
					const Connector *c = ipd->getInlet()->getConnector();
					if( c )
						_sg->disconnect( c->getDstPin() );
				}
			}
			else
				std::cerr << "<error> pindrawer is of unexpected type" << std::endl;
			break;
		}
		}
	}

	void SceneGraphDrawer::outletCallback( PinDrawer *pinDrawer, PinDrawer::Event e )
	{
		switch( e )
		{
		case PinDrawer::E_PRESSED:
		{
			_sticky = dynamic_cast<OutletPinDrawer*>( pinDrawer );
			if( !_sticky )
				std::cerr << "<error> pindrawer is of unexpected type" << std::endl;
			break;
		}
		case GUI::Button::E_RELEASED:
		{
			break;
		}
		}
	}
}
#endif