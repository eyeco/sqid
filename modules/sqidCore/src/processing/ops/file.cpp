/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "file.h"

#include "../sceneGraph.h"
#include "../../fileIO/binWriter.h"
#include "../../fileIO/csvWriter.h"

#include <app.h>
#include <commonImGui.h>
#include <fileIO/json.h>
#include <processing/pin.h>

#include <opencv2/imgproc.hpp>
//#include <opencv2/imgproc/types_c.h>


//#include <filesystem>
//#include <experimental/filesystem>

#ifdef _WIN32
    #include <filesystem>
#else
    #include <experimental/filesystem>
#endif

namespace sqid
{
	namespace File
	{
		namespace Internal
		{
			class IReader
			{
			public:
				virtual ~IReader() {}

				virtual bool eof() = 0;
				virtual bool open( const std::string &file ) = 0;
				virtual SampleFrame *readFrame() = 0;
			};

			class CSVReader : public IReader
			{
			private:
				char _delimiter;
				std::fstream _file;

			public:
				CSVReader() :
					IReader(),
					_delimiter( ',' )
				{}

				virtual ~CSVReader()
				{
					_file.close();
				}

				virtual bool open( const std::string &file )
				{
					_file.close();

					_file.open( file, std::ios::in );
					if( _file.is_open() )
						return true;
					return false;
				}

				virtual bool eof()
				{
					return( _file.is_open() && _file.eof() );
				}

				virtual SampleFrame *readFrame()
				{
					if( !_file.is_open() )
						return nullptr;

					std::string line;
					std::getline( _file, line );

					trimInPlace( line );
					if( !line.size() )
					{
						std::cerr << "<error> empty line" << std::endl;
						return nullptr;
					}

					if( line[line.size() - 1] == _delimiter )
						line.pop_back();

					auto subs = split( line, _delimiter );

					if( subs.size() < 4 )
					{
						std::cerr << "<error> unexpected end of line" << std::endl;
						return nullptr;
					}

					size_t width = 0;
					size_t height = 0;
					size_t depth = 0;
					uint32_t ts = 0;

					errno = 0;
					width = strtoul( subs[0].c_str(), nullptr, 0 );
					height = strtoul( subs[1].c_str(), nullptr, 0 );
					depth = strtoul( subs[2].c_str(), nullptr, 0 );
					ts = strtoul( subs[3].c_str(), nullptr, 0 );
					if( errno == ERANGE )
					{
						std::cerr << "<error> format error" << std::endl;
						return nullptr;
					}

					size_t size = width * height * depth;
					if( size <= 0 )
					{
						std::cerr << "<error> read invalid header from file" << std::endl;
						return nullptr;
					}

					if( size != subs.size() - 4 )
					{
						std::cerr << "<error> invalid number of columns" << std::endl;
						return nullptr;
					}

					SampleFrame *sf = new SampleFrame( width, height, ts, depth );

					float *vals = sf->values();
					for( int i = 0; i < size; i++ )
						*( vals++ ) = strtof( subs[4 + i].c_str(), nullptr );

					if( errno == ERANGE )
					{
						std::cerr << "<error> format error" << std::endl;
						return nullptr;
					}

					return sf;
				}
			};

			class BinReader : public IReader
			{
			private:
				FILE *fp;

				template<typename T>
				bool read( FILE *file, T &t )
				{
					if( fread( &t, sizeof( T ), 1, file ) != 1 )
					{
						if( !feof( file ) )
							std::cerr << "<error> failed to read header from file" << std::endl;
						return false;
					}
					return true;
				}

			public:
				BinReader() :
					IReader(),
					fp( nullptr )
				{}

				virtual ~BinReader()
				{
					if( fp )
					{
						fclose( fp );
						fp = nullptr;
					}
				}

				virtual bool open( const std::string &file )
				{
					if( fp )
					{
						fclose( fp );
						fp = nullptr;
					}

					this->fp = fopen( file.c_str(), "rb" );
					return( fp != nullptr );
				}

				virtual bool eof()
				{
					if( !fp )
						return false;
					return feof( fp );
				}

				virtual SampleFrame *readFrame()
				{
					size_t width = 0;
					size_t height = 0;
					size_t depth = 0;
					uint32_t ts = 0;

					if( !read<size_t>( fp, width ) )
						return nullptr;

					if( !read<size_t>( fp, height ) )
					{
						std::cerr << "<error> unexpected end of file" << std::endl;
						return nullptr;
					}
					if( !read<size_t>( fp, depth ) )
					{
						std::cerr << "<error> unexpected end of file" << std::endl;
						return nullptr;
					}
					if( !read<uint32_t>( fp, ts ) )
					{
						std::cerr << "<error> unexpected end of file" << std::endl;
						return nullptr;
					}

					if( width * height * depth <= 0 )
					{
						std::cerr << "<error> read invalid header from file" << std::endl;
						return nullptr;
					}

					SampleFrame *sf = new SampleFrame( width, height, ts, depth );

					if( fread( sf->values(), sizeof( float ), sf->size(), fp ) != sf->size() )
					{
						std::cerr << "<error> failed to read data from file" << std::endl;
						safeDelete( sf );

						return nullptr;
					}

					return sf;
				}
			};

			class FileSourceImpl
			{
			private:
				std::string path;

				IReader *_reader;

				bool realtime;
				bool loopPlayback;

				bool reachedEnd;

				double appRefTime;
				double fileRefTime;

				SampleFrame *bufferedFrame;

				SampleFrame *readFrameCSV()
				{

				}

				bool readNextFrame()
				{
					if( !_reader )
						return false;

					safeDelete( bufferedFrame );

					if( _reader->eof() )
					{
						reachedEnd = true;
						return false;
					}

					bufferedFrame = _reader->readFrame();

					if( _reader->eof() )
						reachedEnd = true;

					return ( bufferedFrame != nullptr );
				}

			public:
				FileSourceImpl( const std::string &path, bool loopPlayback, bool realtime ) :
					path( trim( toLower( path ) ) ),
					_reader( nullptr ),
					realtime( realtime ),
					loopPlayback( loopPlayback ),
					reachedEnd( false ),
					appRefTime( 0.0 ),
					fileRefTime( 0.0 ),
					bufferedFrame( nullptr )
				{}

				~FileSourceImpl()
				{
					this->close();

					safeDelete( bufferedFrame );
				}

				bool run()
				{
					this->close();

					reachedEnd = false;

					if( endsWith( this->path, ".csv" ) )
						_reader = new CSVReader();
					else if( endsWith( this->path, ".bin" ) )
						_reader = new BinReader();
					else
					{
						std::cerr << "<error> only csv and bin files supported so far" << std::endl;
						return false;
					}

					if( !_reader->open( this->path ) )
					{
						std::cerr << "<error> unable to open file " << this->path << " for playback" << std::endl;
						return false;
					}

					std::cout << "starting playback from file " << this->path << std::endl;

					if( this->readNextFrame() )
					{
						this->fileRefTime = this->bufferedFrame->timeStamp() * 0.001;
						this->appRefTime = getAppTime();
					}
					else
					{
						std::cerr << "<error> stopped playing back from file " << this->path << std::endl;

						safeDelete( _reader );

						return false;
					}

					return true;
				}

				void close()
				{
					safeDelete( _reader );
					safeDelete( bufferedFrame );
				}

				bool fetchFrames( std::vector<SampleFrame*> &frames )
				{
					if( frames.size() )
						std::cerr << "<error> expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory!" << std::endl;

					if( !_reader )
						return false;

					if( !bufferedFrame )
					{
						if( !readNextFrame() )
							return false;
					}

					while( !realtime || bufferedFrame && getAppTime() - appRefTime >= bufferedFrame->timeStamp() * 0.001 - fileRefTime )
					{
						//time to process buffered frame
						frames.push_back( bufferedFrame );
						bufferedFrame = nullptr;

						//read next frame from file
						if( !readNextFrame() )
						{
							if( _reader->eof() )
							{
								reachedEnd = true;
								std::cout << "reached end of playback file" << std::endl;

								if( this->loopPlayback )
								{
									std::cout << "rewinding file..." << std::endl;

									this->run();
									break;
								}
							}
							else
								std::cerr << "<error> failed reading playback file" << std::endl;

							this->close();

							break;
						}

						if( !realtime ) //so far, only push one frame at a time
							break;
					}

					return true;
				}

				bool getLoopPlayback() const { return loopPlayback; }
				void setLoopPlayback( bool loop ) { loopPlayback = loop; }

				bool getReachedEnd() const { return reachedEnd; }
			};
		}


		DEFINE_OP_DESC( FileIn, "fileIn", "/fileIO",
			"F8C21817-0926-401F-A733-3E5A4CEDB474" );
		DEFINE_OP_DESC( FileOut, "fileOut", "/fileIO",
			"0C74DDA6-3B5B-4DD2-9B5D-4340B9094E73" );




		FileIn::FileIn() :
			Op(),
			_loop( false ),
			_realtime( true ),
			_impl( nullptr ),
			_inputBufferPath( 256 )
		{
			updateBuffers();
		}

		FileIn::~FileIn()
		{
			safeDelete( _impl );
		}

		void FileIn::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool FileIn::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _impl ? ( _impl->getReachedEnd() ? "restart" : "stop" ) : "play" ) )
			{
				if( _impl )
				{
					if( _impl->getReachedEnd() )
						_impl->run();
					else
						stop();
				}
				else
					start();
			}

			if( ImGui::Checkbox( "loop", &_loop ) )
				if( _impl )
					_impl->setLoopPlayback( _loop );

			{
				ScopedImGuiDisable disable( _impl != nullptr );

				if( ImGui::InputText( "path", &_inputBufferPath[0], _inputBufferPath.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
				{
					std::string str = trim( &_inputBufferPath[0] );

					if( str.size() )
					{
						//TODO: validate file path, check if file exists
						_path = str;
					}

					updateBuffers();
				}

				ImGui::Checkbox( "realtime", &_realtime );
			}

			return true;
		}
#endif

		//TODO: when dealing with multiple inputs, this does not really make sense
		// think of a better way to do this
		bool FileIn::process()
		{
			if( _impl )
			{
				std::vector<SampleFrame*> frames;

				_impl->fetchFrames( frames );

				if( frames.size() )
				{
					//TODO: buffer -- for now, we just use the most recent one
					SampleFrame *ret = frames.back();

					if( ret )
					{
						drawFrame( ret );

						pushOutput( "out", ret );
						ret = nullptr;
					}

					for( int i = 0; i < frames.size(); i++ )
						safeDelete( frames[i] );
					frames.clear();
				}
			}

			return false;
		}

		bool FileIn::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			load<bool>( j, "loop", _loop );
			load<bool>( j, "realtime", _realtime );
			load<std::string>( j, "filePath", _path );

			bool playing = false;
			load<bool>( j, "playing", playing );

			updateBuffers();

			if( playing )
				start();

			return ret;
		}

		bool FileIn::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "loop", _loop );
			save( j, "realtime", _realtime );
			save( j, "filePath", _path );

			save( j, "playing", ( _impl ? true : false ) );

			return ret;
		}

		void FileIn::updateBuffers()
		{
			strncpy( &_inputBufferPath[0], _path.c_str(), _path.size() + 1 );
		}

		void FileIn::start()
		{
			stop();

			std::cerr << "running file " << _path << std::endl;

			_impl = new Internal::FileSourceImpl( _path, _loop, _realtime );
			if( !_impl->run() )
			{
				std::cerr << "<error> failed to run file " << _path << std::endl;
				stop();
			}
		}

		void FileIn::stop()
		{
			safeDelete( _impl );
		}





		FileOut::FileOut( const std::string &path ) :
			Op(),
			_writing( false ),
			_append( true ),
			_path( path ),
			_pathValid( false ),
			_format( FF_BIN ),
			_writer( nullptr ),
			_inputBufferPath( 1024 )
		{
			validateExtension();
			checkPathString();
			updateBuffers();
		}

		FileOut::~FileOut()
		{
			closeWriter();
		}

		bool FileOut::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				if( _writing )
				{
					if( _writer )
					{
						if( !_writer->writeFrame( sf ) )
						{
							std::cerr << "<error> failed writing to file" << std::endl;
							setEnabled( false );
						}
					}
					else
					{
						std::cerr << "<error> writer not created" << std::endl;
						setEnabled( false );
					}
				}

				drawFrame( sf );

				safeDelete( sf );
			}

			return inputPending( "in" );
		}

#ifdef __SUPPORT_GUI
		bool FileOut::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _writing ? "stop" : "start" ) )
			{
				_writing = !_writing;

				if( _writing )
					createWriter();
				else
					closeWriter();
			}

			{
				ScopedImGuiDisable disable( _writing );

				int e = (int) _format;
				for( int i = 0; i < FF_COUNT; i++ )
					ImGui::RadioButton( fileFormatToString( (FileFormat) i ), &e, i );
				if( _format != (FileFormat) e )
				{
					_format = (FileFormat) e;
					validateExtension();
					updateBuffers();
				}

				{
					ScopedImGuiStyleColor style;
					if( !_pathValid )
						style.set( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );

					if( ImGui::InputText( "path", &_inputBufferPath[0], _inputBufferPath.size(), ImGuiInputTextFlags_EnterReturnsTrue ) )
					{
						std::string str = trim( &_inputBufferPath[0] );

						if( str.size() )
						{
							_path = str;
							validateExtension();
							checkPathString();
						}

						updateBuffers();
					}
				}

				ImGui::Checkbox( "append", &_append );
			}

			return true;
		}
#endif

		void FileOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

		bool FileOut::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<bool>( j, "writing", _writing );
			load<bool>( j, "append", _append );
			load<std::string>( j, "filePath", _path );
			std::string str;
			if( load<std::string>( j, "format", str ) )
				_format = fileFormatFromString( str );

			validateExtension();
			checkPathString();
			updateBuffers();

			if( _writing )
				createWriter();
			else
				closeWriter();

			return ret;
		}

		bool FileOut::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "writing", _writing );
			save( j, "append", _append );
			save( j, "filePath", _path );
			save( j, "format", fileFormatToString( _format ) );

			return ret;
		}

		void FileOut::updateBuffers()
		{
			strncpy( &_inputBufferPath[0], _path.c_str(), _path.size() + 1 );
		}

		void FileOut::validateExtension()
		{
			std::string path( toLower( _path ) );
			std::replace( path.begin(), path.end(), '\\', '/' );

			std::string lower( path );
			std::string targetExt( toLower( fileFormatExtension( _format ) ) );

			if( !endsWith( lower, targetExt ) )
			{
				size_t extPos = lower.find_last_of( '.' );
				if( extPos != std::string::npos )
				{
					size_t slashPos = lower.find( '/' );
					if( slashPos > extPos )
						extPos = std::string::npos;
				}

				_path = path.substr( 0, extPos ) + targetExt;
			}
		}

		std::string FileOut::makePathString()
		{
			std::string modifiedPath = _path;

			std::string dt = getDateTimeString( true );
			modifiedPath = replace( modifiedPath, "<t>", dt, true );
			modifiedPath = replace( modifiedPath, "<s>", App().getSceneName(), true );

			using namespace std::filesystem;

			if( contains( modifiedPath, "<c>", true ) )
			{
				unsigned int cntr = 0;
				std::string temp = modifiedPath;
				do
				{
					char tempStr[16];
					sprintf( tempStr, "%03d", cntr++ );
					temp = replace( modifiedPath, "<c>", tempStr, true );

					if( !cntr ) //overflow
					{
						_pathValid = false;
						return "";
					}

				} while( exists( temp ) );
				modifiedPath = temp;
			}

			return modifiedPath;
		}

		bool FileOut::checkPathString()
		{
			if( !_path.size() )
				_pathValid = false;
			else
			{
				_pathValid = !containsAny( makePathString(), ":*?\"|><" );
			}

			return _pathValid;
		}

		bool FileOut::createWriter()
		{
			closeWriter();

			std::string p = makePathString();

			using namespace std::filesystem;

			path dir( path( p ).parent_path() );
			if( !exists( dir ) )
				create_directories( dir );

			switch( _format )
			{
				//case FF_MAT_ASCII:
				//{
				//	MatWriterAscii *mat = new MatWriterAscii( path, _append, columns, true );
				//	if( !mat->setup() )
				//	{
				//		std::cerr << "<error> setting up ASCII mat file " << filename << " failed" << std::endl;
				//		safeDelete( mat );
				//	}
				//	_writer = mat;
				//	break;
				//}
			case FF_BIN:
			{
				BinWriter *bin = new BinWriter( p, _append );
				if( !bin->open() )
				{
					std::cerr << "<error> setting up bin file " << p << " failed" << std::endl;
					safeDelete( bin );
				}
				_writer = bin;
				break;
			}
			case FF_CSV:
			{
				CSVWriter *csv = new CSVWriter( p, _append );
				if( !csv->open() )
				{
					std::cerr << "<error> setting up csv file " << p << " failed" << std::endl;
					safeDelete( csv );
				}
				_writer = csv;
				break;
			}
			default:
				std::cerr << "<error> unknown format: " << fileFormatToString( _format ) << std::endl;
				break;
			}

			if( _writer )
				std::cout << "set up file " << p << " for recording" << std::endl;

			return( _writer != nullptr );
		}

		bool FileOut::closeWriter()
		{
			std::cout << "closing file writer" << std::endl;
			safeDelete( _writer );

			return true;
		}
	}
}