/*---------------------------------------------------------------------------------------------
* Copyright (C) 2025 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of the sqid Visual Programming Environment
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. if not, see <http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <processing/op.h>

namespace sqid
{
	class ProcessorCreatorBase
	{
	public:
		virtual ~ProcessorCreatorBase() {}

		virtual Op *create() const = 0;
	};

	template<typename T>
	class ProcessorCreator : public ProcessorCreatorBase
	{
		typedef T _ValueType;
		typedef ProcessorCreator<T> _MyType;

	public:
		virtual Op *create() const
		{
			return new T();
		}
	};

#define REGISTER_OP_TYPE( _TYPE ) \
	opFactory().registerType( new ProcessorCreator<_TYPE>(), _TYPE::Name(), _TYPE::Path(), _TYPE::ClassID() );

	class SQID_API OpDesc
	{
	private:
		std::string _name;
		std::string _path;
		GUID _guid;

	public:
		OpDesc( const std::string &name, const std::string &path, const GUID &guid ) :
			_name( name ), _path( path ), _guid( guid )
		{}

		const std::string &name() const { return _name; }
		const std::string &path() const { return _path; }
		const GUID &guid() const { return _guid; }
	};

	class SQID_API OpFactory
	{
	private:
		std::map<GUID, ProcessorCreatorBase*, CompareGUID> _creators;
		std::vector<OpDesc> _supported;

	public:
		OpFactory();
		~OpFactory();

		void shutdown();

		Op *create( const GUID &classID );

		void printSupported();
		const std::vector<OpDesc> &getSupported();

		void registerType( ProcessorCreatorBase *creator, const std::string &name, const std::string &path, const GUID &classID );
	};

	SQID_API OpFactory& SQID_API_CALL opFactory();
}