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

namespace sqid
{
	namespace OCV
	{
		namespace Internal
		{
			class OCVSourceImpl;
			class OCVSinkImpl;
		}

		//NOTE: OpenCV capture seems to be leaking memory
		//https://github.com/opencv/opencv/issues/13255
		// also, VLD crashes sometimes when trying to resolve leaks
		class OCVCam : public Op
		{
		private:
			unsigned int _camID;
			bool _running;

			Internal::OCVSourceImpl *_impl;

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			OCVCam();
			virtual ~OCVCam();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class OCVVideoIn : public Op
		{
		private:
			bool _loop;

			std::string _path;

			Internal::OCVSourceImpl *_impl;

			std::vector<char> _inputBufferPath;

			void updateBuffers();

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			OCVVideoIn();
			virtual ~OCVVideoIn();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class OCVVideoOut : public Op
		{
		private:
			std::string _path;

			unsigned int _width;
			unsigned int _height;
			unsigned int _depth;

			float _fps;

			Internal::OCVSinkImpl *_impl;

			std::vector<char> _inputBufferPath;

			void updateBuffers();

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			OCVVideoOut();
			virtual ~OCVVideoOut();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class OCVImageIn : public Op
		{
		private:
			std::string _path;

			unsigned int _width;
			unsigned int _height;
			unsigned int _depth;

			std::vector<char> _inputBufferPath;

			SampleFrame *_image;

			void updateBuffers();

		protected:
			virtual bool process();

			void loadImg();

		public:
			OCVImageIn();
			virtual ~OCVImageIn();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};

		class OCVImageOut : public Op
		{
		private:
			std::string _path;

			unsigned int _width;
			unsigned int _height;
			unsigned int _depth;

			std::vector<char> _inputBufferPath;

			SampleFrame *_image;

			void updateBuffers();

		protected:
			virtual bool process();

			void saveImg();

		public:
			OCVImageOut();
			virtual ~OCVImageOut();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};
	}
}