/*
 * _COPYRIGHT_
 */

#ifndef _CREATE_CDB_HPP__
#define _CREATE_CDB_HPP__

#include <set>
#include <limits>
#include <CDBRow.hpp>
#include <Log4.hpp>

namespace CDB {

template<class T>
class Create {
	private:
		// not-copyable
		Create(const Create&);
		Create& operator=(const Create&);

		typename T::Accessor ac;
		size_t counter;

		static const size_t MAX_ROWS = 1000000;

	public:
		typedef std::set<typename T::Data> RawDataT;

		/**
		 @brief Constructor
		 */
		explicit Create(const std::string& fn)
		: counter(0)
		{
			ac.open(fn, ROW_WRITE, std::numeric_limits<uint64_t>::max());
		}

		void insert(const RawDataT& data) {
			LOG4_TRACE("about to insert " << data.size() << " elements");
			ac.reserve(MAX_ROWS);
			for (auto i = data.begin();
					  i != data.end();
					  ++i, ++counter )
			{
				if (counter >= MAX_ROWS)
				{
					ac.write();
					counter = 0;
				}
				ac.push_back(*i);
			}
		}

		void close() {
			ac.write();
			ac.close();
		}

		/**
		 @brief Destructor
		 */
		~Create() throw() {
			;;
		}
};

} // namespace CDB

#endif /* _CREATE_CDB_HPP__ */


