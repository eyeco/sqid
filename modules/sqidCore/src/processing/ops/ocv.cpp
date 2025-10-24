/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include "ocv.h"

#include <app.h>
#include "../sceneGraph.h"

#include <fileIO/json.h>

#include <commonImGui.h>

#include <opencv2/imgproc.hpp>

namespace sqid
{
	namespace OCV
	{
		namespace Internal
		{
			class OCVSourceImpl
			{
			private:
				bool _isStarted;

				bool _loop;
				bool _atEnd;

				bool _isFile;

				unsigned int _camID;
				std::string _file;

				cv::VideoCapture _capture;

				bool tryOpen()
				{
					close();

					if( _isFile )
					{
						if( _file.size() )
						{
							std::cout << "opening OpenCV capture with file " << _file << std::endl;

							if( !_capture.open( _file, cv::CAP_ANY ) )
							{
								std::cerr << "<error> failed to open OpenCV capture with file " << _file << " (file in use?)" << std::endl;
								return false;
							}
							std::cout << "successfully opened capture with backend \"" << _capture.getBackendName() << "\"" << std::endl;
						}
						else
						{
							std::cerr << "<error> no file name specified" << std::endl;
							return false;
						}
					}
					else
					{
						std::cout << "opening OpenCV Camera #" << _camID << std::endl;

						if( !_capture.open( _camID, cv::CAP_ANY ) )
						{
							std::cerr << "<error> failed to open camera #" << _camID << std::endl;
							return false;
						}
						std::cout << "successfully opened camera with backend \"" << _capture.getBackendName() << "\"" << std::endl;
					}

					return true;
				}

			public:
				OCVSourceImpl( unsigned int camID ) :
					_isStarted( false ),
					_loop( false ),
					_atEnd( false ),
					_isFile( false ),
					_camID( camID )
				{}

				OCVSourceImpl( const std::string &file ) :
					_isStarted( false ),
					_loop( false ),
					_atEnd( false ),
					_isFile( true ),
					_camID( ~0x00 ),
					_file( file )
				{}

				~OCVSourceImpl()
				{
					close();
				}

				bool run()
				{
					if( _isStarted )
						return false;
					_isStarted = tryOpen();

					return _isStarted;
				}

				void close()
				{
					_capture.release();
				}

				void rewind()
				{
					if( _capture.isOpened() )
					{
						_capture.set( cv::CAP_PROP_POS_FRAMES, 0 );
						_atEnd = false;
					}
				}

				void setLoopPlayback( bool loop ) { _loop = loop; }
				bool getLoopPlayback() const { return _loop; }

				bool getReachedEnd() const { return _atEnd; }

				bool fetchFrames( std::vector<SampleFrame*> &frames )
				{
					if( frames.size() )
						std::cerr << "expecting empty vector here... class user is responsible for deletion of frames, make sure you're not leaking memory!" << std::endl;

					//TODO: read asynchronously and put into ring buffer
					if( _capture.isOpened() )
					{
						if( _atEnd )
							return false;

						double time = getAppTime();
						uint32_t ts = time * 1000;

						cv::Mat srcM;

						_capture >> srcM;

						//TODO: try to avoid having to convert here....
						SampleFrame *sf = new SampleFrame( srcM.size().width, srcM.size().height, ts, srcM.channels() );
						srcM.convertTo( sf->mat(), CV_MAKETYPE( CV_32F, srcM.channels() ), 1.0f / 255 );

						frames.push_back( sf );

						if( _isFile )
						{
							int currentPos = _capture.get( cv::CAP_PROP_POS_FRAMES );
							int totalFrames = _capture.get( cv::CAP_PROP_FRAME_COUNT );

							if( currentPos >= totalFrames )
							{
								if( _loop )
									rewind();
								else
									_atEnd = true;
							}
						}

						return true;
					}

					return false;
				}
			};


			class OCVSinkImpl
			{
			private:
				bool _isStarted;

				float _fps;
				unsigned int _width;
				unsigned int _height;
				bool _isColor;

				std::string _file;

				cv::VideoWriter _writer;

				bool tryOpen()
				{
					close();

					if( _file.size() )
					{
						std::cout << "opening OpenCV writer with file " << _file << std::endl;

						if( !_writer.open( _file, cv::CAP_ANY, cv::VideoWriter::fourcc( 'X', '2', '6', '4' ), _fps, cv::Size( _width, _height ), _isColor ) )
						{
							std::cerr << "<error> failed to open OpenCV writer with file " << _file << std::endl;
							return false;
						}
						std::cout << "successfully opened writer with backend \"" << _writer.getBackendName() << "\"" << std::endl;
					}
					else
					{
						std::cerr << "<error> no file name specified" << std::endl;
						return false;
					}

					return true;
				}

			public:
				OCVSinkImpl( const std::string &file, float fps, unsigned int width, unsigned int height, bool color ) :
					_isStarted( false ),
					_fps( fps ),
					_width( width ),
					_height( height ),
					_isColor( color ),
					_file( file )
				{}

				~OCVSinkImpl()
				{
					close();
				}

				bool run()
				{
					if( _isStarted )
						return false;
					_isStarted = tryOpen();

					return _isStarted;
				}

				void close()
				{
					_writer.release();
				}

				bool write( const SampleFrame* sf )
				{
					if( !sf )
						return false;

					//TODO: read asynchronously and put into ring buffer
					if( _writer.isOpened() )
					{
						if( sf->width() != _width || sf->height() != _height )
						{
							std::cout << "<error> invalid frame size" << std::endl;
							return false;
						}

						if( ( _isColor && sf->depth() != 3 ) || ( !_isColor && sf->depth() != 1 ) )
						{
							std::cout << "<error> invalid frame depth" << std::endl;
							return false;
						}

						cv::Mat dstMat;
						sf->mat().convertTo( dstMat, CV_MAKE_TYPE( CV_8U, sf->depth() ), 255.0f );

						//TODO: conside frame timing
						_writer << dstMat;

						return true;
					}

					return false;
				}
			};
		}



		DEFINE_OP_DESC( OCVCam, "capture", "/devices",
			"4B03646B-80E9-4146-A7A6-01AD66DAF9BA" );
		DEFINE_OP_DESC( OCVVideoIn, "videoIn", "/fileIO",
			"D7B4CD7F-6310-44AE-93EB-ED814928C58B" );
		DEFINE_OP_DESC( OCVVideoOut, "videoOut", "/fileIO",
			"21A863D8-E849-430E-A40E-962DB69DDE36" );
		DEFINE_OP_DESC( OCVImageIn, "imageIn", "/fileIO",
			"1990E762-546E-41AD-BDAE-9A96582000B8" );
		DEFINE_OP_DESC( OCVImageOut, "imageOut", "/fileIO",
			"48C78969-B68B-4714-9B0D-5EE32AA18FE9" );




		OCVCam::OCVCam() :
			Op(),
			_camID( 0 ),
			_running( false ),
			_impl( nullptr )
		{}

		OCVCam::~OCVCam()
		{
			stop();

			safeDelete( _impl );
		}

		void OCVCam::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool OCVCam::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			if( ImGui::Button( _running ? "stop" : "start" ) )
			{
				if( _running )
					stop();
				else
					start();
			}

			{
				ScopedImGuiDisable disable( _impl != nullptr );

				int i = _camID;
				if( ImGui::InputInt( "camID", &i ) )
					_camID = i; //TODO: clamp
			}

			return true;
		}
#endif

		bool OCVCam::process()
		{
			if( _impl )
			{
				std::vector<SampleFrame*> frames;
				_impl->fetchFrames( frames );

				if( frames.size() )
				{
					//TODO: buffer -- for now, we just use the most recent one
					SampleFrame *ret = frames.back();

					drawFrame( ret );

					pushOutput( "out", ret );
					ret = nullptr;

					for( int i = 0; i < frames.size(); i++ )
						safeDelete( frames[i] );
					frames.clear();
				}
			}

			return false;
		}

		bool OCVCam::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			load<bool>( j, "running", _running );
			load<unsigned int>( j, "camID", _camID );

			if( _running )
				start();

			return ret;
		}

		bool OCVCam::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "running", _running );
			save( j, "camID", _camID );

			return ret;
		}

		void OCVCam::start()
		{
			stop();

			_impl = new Internal::OCVSourceImpl( _camID );
			if( _impl->run() )
				_running = true;
			else
			{
				std::cerr << "<error> failed to run camera with id " << _camID << std::endl;
				stop();
			}
		}

		void OCVCam::stop()
		{
			safeDelete( _impl );
			_running = false;
		}





		OCVVideoIn::OCVVideoIn() :
			Op(),
			_loop( true ),
			_impl( nullptr ),
			_inputBufferPath( 256 )
		{}

		OCVVideoIn::~OCVVideoIn()
		{
			stop();

			safeDelete( _impl );
		}

		void OCVVideoIn::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool OCVVideoIn::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			{
				ScopedImGuiDisable disable( !_path.size() );

				if( ImGui::Button( _impl ? ( _impl->getReachedEnd() ? "restart" : "stop" ) : "play" ) )
				{
					if( _impl )
					{
						if( _impl->getReachedEnd() )
							_impl->rewind();
						else
							stop();
					}
					else
						start();
				}
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
			}

			return true;
		}
#endif

		bool OCVVideoIn::process()
		{
			if( _impl )
			{
				std::vector<SampleFrame*> frames;
				_impl->fetchFrames( frames );

				if( frames.size() )
				{
					//TODO: buffer -- for now, we just use the most recent one
					SampleFrame *ret = frames.back();

					drawFrame( ret );

					pushOutput( "out", ret );
					ret = nullptr;

					for( int i = 0; i < frames.size(); i++ )
						safeDelete( frames[i] );
					frames.clear();
				}
			}

			return false;
		}

		bool OCVVideoIn::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			load<bool>( j, "loop", _loop );
			load<std::string>( j, "filePath", _path );

			bool playing = false;
			load<bool>( j, "playing", playing );

			updateBuffers();

			if( playing )
				start();

			return ret;
		}

		bool OCVVideoIn::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "loop", _loop );
			save( j, "filePath", _path );

			save( j, "playing", ( _impl ? true : false ) );

			return ret;
		}

		void OCVVideoIn::updateBuffers()
		{
			strncpy( &_inputBufferPath[0], _path.c_str(), _path.size() + 1 );
		}

		void OCVVideoIn::start()
		{
			stop();

			std::cerr << "running video " << _path << std::endl;

			_impl = new Internal::OCVSourceImpl( _path );
			if( _impl->run() )
				_impl->setLoopPlayback( _loop );
			else
			{
				std::cerr << "<error> failed to run video " << _path << std::endl;
				stop();
			}
		}

		void OCVVideoIn::stop()
		{
			safeDelete( _impl );
		}






		OCVVideoOut::OCVVideoOut() :
			Op(),
			_path( "recordings/capture.mp4" ),
			_width( 0 ),
			_height( 0 ),
			_depth( 0 ),
			_fps( 30.0 ),
			_impl( nullptr ),
			_inputBufferPath( 256 )
		{
			updateBuffers();
		}

		OCVVideoOut::~OCVVideoOut()
		{
			stop();

			safeDelete( _impl );
		}

		void OCVVideoOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

#ifdef __SUPPORT_GUI
		bool OCVVideoOut::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			{
				ScopedImGuiDisable disable( !_path.size() );
				if( ImGui::Button( _impl ? "stop" : "record" ) )
				{
					if( _impl )
						stop();
					else
						start();
				}
			}

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

				ImGui::InputFloat( "fps", &_fps );

				ImGui::Text( "size: %dx%dx%d", _width, _height, _depth );
			}

			return true;
		}
#endif

		bool OCVVideoOut::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );
			if( sf )
			{
				if( _impl )
					_impl->write( sf );
				else
				{
					_width = sf->width();
					_height = sf->height();
					_depth = sf->depth();
				}

				drawFrame( sf );
				safeDelete( sf );
			}

			return inputPending( "in" );
		}

		bool OCVVideoOut::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			stop();

			load<std::string>( j, "filePath", _path );

			load<unsigned int>( j, "width", _width );
			load<unsigned int>( j, "height", _height );
			load<unsigned int>( j, "depth", _depth );

			load<float>( j, "fps", _fps );

			bool recording = false;
			load<bool>( j, "recording", recording );

			updateBuffers();

			if( recording )
				start();

			return ret;
		}

		bool OCVVideoOut::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "filePath", _path );

			save( j, "width", _width );
			save( j, "height", _height );
			save( j, "depth", _depth );

			save( j, "fps", _fps );

			save( j, "recording", ( _impl ? true : false ) );

			return ret;
		}

		void OCVVideoOut::updateBuffers()
		{
			strncpy( &_inputBufferPath[0], _path.c_str(), _path.size() + 1 );
		}

		void OCVVideoOut::start()
		{
			stop();

			if( _depth != 3 && _depth != 1 )
			{
				std::cerr << "<error> invalid depth" << std::endl;
				return;
			}

			std::cout << "recording video " << _path << std::endl;

			_impl = new Internal::OCVSinkImpl( _path, _fps, _width, _height, _depth > 1 );
			if( !_impl->run() )
			{
				std::cerr << "<error> failed to start recording video " << _path << std::endl;
				stop();
			}
		}

		void OCVVideoOut::stop()
		{
			safeDelete( _impl );
		}






		OCVImageIn::OCVImageIn() :
			Op(),
			_path( "" ),
			_width( 0 ),
			_height( 0 ),
			_depth( 0 ),
			_inputBufferPath( 256 ),
			_image( nullptr )
		{
			updateBuffers();
		}

		OCVImageIn::~OCVImageIn()
		{
			safeDelete( _image );
		}

		void OCVImageIn::createPins()
		{
			addOutlet( new OutletPin( new DataContainer<SampleFrame>(), "out", this ) );
		}

#ifdef __SUPPORT_GUI
		bool OCVImageIn::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			{
				ScopedImGuiDisable disable( !_path.size() );

				if( ImGui::Button( "load" ) )
					loadImg();
			}

			if( ImGui::InputText( "path", &_inputBufferPath[0], _inputBufferPath.size() ) )
			{
				std::string str = trim( &_inputBufferPath[0] );

				if( str.size() )
				{
					//TODO: validate file path, check if file exists
					_path = str;
				}

				updateBuffers();
			}

			if( _image )
				ImGui::Text( "size: %dx%dx%d", _width, _height, _depth );
			else
				ImGui::Text( "<nil>" );

			return true;
		}
#endif

		bool OCVImageIn::process()
		{
			if( _image )
			{
				drawFrame( _image );

				pushOutput( "out", _image );
			}

			return false;
		}

		bool OCVImageIn::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<std::string>( j, "filePath", _path );
			updateBuffers();

			if( _path.size() )
				loadImg();

			return ret;
		}

		bool OCVImageIn::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "filePath", _path );

			return ret;
		}

		void OCVImageIn::updateBuffers()
		{
			strncpy( &_inputBufferPath[0], _path.c_str(), _path.size() + 1 );
		}

		void OCVImageIn::loadImg()
		{
			safeDelete( _image );

			_width = 0;
			_height = 0;
			_depth = 0;

			if( !_path.size() )
				return;

			try
			{
				cv::Mat m = cv::imread( _path, cv::IMREAD_UNCHANGED );
				if( !m.data )
				{
					std::cerr << "<error> failed to load image " << _path << std::endl;
					return;
				}

				std::cout << "loaded image " << _path << std::endl;

				bool convert = true;
				float s = 1.0f;
				switch( m.type() & CV_MAT_DEPTH_MASK )
				{
				case CV_8U:
					s = 1.0f / 0xff;
					break;
				case CV_8S:
					s = 1.0f / 0x7f;
					break;
				case CV_16U:
					s = 1.0f / 0xffff;
					break;
				case CV_16S:
					s = 1.0f / 0x7fff;
					break;
				case CV_32S:
					s = 1.0f / 0x7fffffff;
					break;
				case CV_32F:
					convert = false;
					break;
				case CV_64F:
				case CV_16F:
					s = 1.0f;
					break;
				}

				if( convert )
				{
					cv::Mat mf;
					m.convertTo( mf, CV_MAKETYPE( CV_32F, m.channels() ), s );

					_image = new SampleFrame( mf, getAppTime() * 1000 );
				}
				else
					_image = new SampleFrame( m, getAppTime() * 1000 );

				_width = _image->width();
				_height = _image->height();
				_depth = _image->depth();
			}
			catch( std::runtime_error &e )
			{
				std::cerr << "<error> caught exception when trying to load file " << _path << ": " << e.what() << std::endl;
			}
		}





		OCVImageOut::OCVImageOut() :
			Op(),
			_path( "recordings/capture.tiff" ),
			_width( 0 ),
			_height( 0 ),
			_depth( 0 ),
			_inputBufferPath( 256 ),
			_image( nullptr )
		{
			updateBuffers();
		}

		OCVImageOut::~OCVImageOut()
		{
			safeDelete( _image );
		}

		void OCVImageOut::createPins()
		{
			addInlet( new InletPin( new DataContainer<SampleFrame>(), "in", this ) );
		}

#ifdef __SUPPORT_GUI
		bool OCVImageOut::drawUI()
		{
			if( !Op::drawUI() )
				return false;

			{
				ScopedImGuiDisable disable( !_path.size() );

				if( ImGui::Button( "save" ) )
					saveImg();
			}

			if( ImGui::InputText( "path", &_inputBufferPath[0], _inputBufferPath.size() ) )
			{
				std::string str = trim( &_inputBufferPath[0] );

				if( str.size() )
				{
					//TODO: validate file path, check if file exists
					_path = str;
				}

				updateBuffers();
			}

			if( _image )
				ImGui::Text( "size: %dx%dx%d", _width, _height, _depth );
			else
				ImGui::Text( "<nil>" );

			return true;
		}
#endif

		bool OCVImageOut::process()
		{
			SampleFrame *sf = fetchInput<SampleFrame>( "in" );

			if( sf )
			{
				safeDelete( _image );
				_image = sf;

				drawFrame( _image );

				_width = _image->width();
				_height = _image->height();
				_depth = _image->depth();
			}

			return inputPending( "in" );
		}

		bool OCVImageOut::loadFromJSON( const nlohmann::json &j )
		{
			bool ret = Op::loadFromJSON( j );

			load<std::string>( j, "filePath", _path );
			updateBuffers();

			return ret;
		}

		bool OCVImageOut::saveToJSON( nlohmann::json &j ) const
		{
			bool ret = Op::saveToJSON( j );

			save( j, "filePath", _path );

			return ret;
		}

		void OCVImageOut::updateBuffers()
		{
			strncpy( &_inputBufferPath[0], _path.c_str(), _path.size() + 1 );
		}

		void OCVImageOut::saveImg()
		{
			if( !_image || !_path.size() )
				return;

			try
			{
				bool isPNG = endsWith( toLower( _path ), ".png" );
				bool isTIFF = ( endsWith( toLower( _path ), ".tif" ) || endsWith( toLower( _path ), ".tiff" ) );
				bool isJPEG = ( endsWith( toLower( _path ), ".jpg" ) || endsWith( toLower( _path ), ".jpeg" ) );

				if( !isTIFF && !isPNG && _image->depth() != 3 )
				{
					//NOTE: 1-channel JPG seems to work fine, though.
					if( !( isJPEG && _image->depth() == 1 ) )
						std::cerr << "<warning> " << _image->depth() << " channels may not be supported by target file type, saved file may be broken" << std::endl;
				}

				bool convert = !isTIFF;
				if( convert )
					std::cout << "image needs to be converted to byte for target file type" << std::endl;

				if( convert || !cv::imwrite( _path, _image->mat() ) )
				{
					std::cerr << "<error> failed to save image, trying type conversion" << std::endl;

					cv::Mat m;
					_image->mat().convertTo( m, CV_MAKETYPE( CV_8U, _image->depth() ), 255.0f );

					if( !cv::imwrite( _path, m ) )
					{
						std::cerr << "<error> failed to save image " << _path << std::endl;
						return;
					}
				}

				std::cout << "saved image " << _path << std::endl;
			}
			catch( std::exception &e )
			{
				std::cerr << "<error> caught exception when trying to write file " << _path << ": " << e.what() << std::endl;
			}
		}
	}
}