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
	class FileWriter;

	namespace File
	{
		namespace Internal
		{
			class FileSourceImpl;
		}

		class FileIn : public Op
		{
		private:
			bool _loop;
			bool _realtime;

			std::string _path;

			Internal::FileSourceImpl *_impl;

			std::vector<char> _inputBufferPath;

			void updateBuffers();

		protected:
			virtual bool process();

			void start();
			void stop();

		public:
			FileIn();
			virtual ~FileIn();

#ifdef __SUPPORT_GUI
			virtual bool drawUI();
#endif

			virtual void createPins();

			virtual bool loadFromJSON( const nlohmann::json &j );
			virtual bool saveToJSON( nlohmann::json &j ) const;

			DECLARE_OP_DESC;
		};


		class FileOut : public Op
		{
		private:
			bool _writing;
			bool _append;

			std::string _path;
			bool _pathValid;

			FileFormat _format;

			FileWriter *_writer;

			std::vector<char> _inputBufferPath;

			void validateExtension();
			void updateBuffers();

			std::string makePathString();
			bool checkPathString();

			bool createWriter();
			bool closeWriter();

		protected:
			virtual bool process();

		public:
			explicit FileOut( const std::string &path = "recordings/<s>-<c>" );
			virtual ~FileOut();

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