/*
 * _COPYRIGHT_
 */

#ifndef _USE_CDB_HPP__
#define _USE_CDB_HPP__

#include <stdint.h>
#include <CDBRow.hpp>
#include <Log4.hpp>

namespace CDB {

template<class T>
class Use {
	private:
		// not-copyable
		Use(const Use&);
		Use& operator=(const Use&);

		typename T::Accessor ac;
		const std::string filename;

	public:

		/**
		 @brief Constructor
		 */
		explicit Use(const std::string& fn)
		: filename(fn)
		{
			;;
		}

		// FIXME: Narrow is really const here, but TL interface
		// head to make const-correct
		template<class Narrow, class W>
		void access(Narrow& narrow, W& worker)
		{
			uint64_t columns = narrow.columns() | worker.columns();
			ac.open(filename, ROW_READ, columns);

			LOG4_DEBUG(filename << ": read columns " << columns);

			while(!ac.eof(columns))
			{
				size_t rowcount = ac.read(columns);
				for (size_t i = 0; i < rowcount; i++)
				{
					if (narrow(ac, i)){
						worker(ac, i);
					}
				}
			}
			ac.close();
		}

		/**
		 @brief Destructor
		 */
		~Use() throw() {
		}
};

} // namespace CDB

#endif /* _USE_CDB_HPP__ */


