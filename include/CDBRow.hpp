/*
 * _COPYRIGHT_
 */

#ifndef _CDB_ROW_HPP__
#define _CDB_ROW_HPP__

#include <stdint.h>
#include <Log4.hpp>
#include <ArLZO.hpp>
//#include <ArZip.hpp>

#include <SyncWriteFile.hpp>
#include <stdlib.h>
#include <cassert>

namespace CDB {

	enum Direction {ROW_READ, ROW_WRITE};

template<class T, class AR = Util::ArLZO>
class Row {
	private:
		// not-copyable
		Row(const Row&);
		Row& operator=( const Row&);

		typedef std::vector<T> BufT;
		BufT buf;
		Util::SyncWriteFile disk;
		//Util::HFile disk;
		const std::string suffix;
		const uint64_t id;

		AR lzo;
		typedef typename AR::BufferT ArBufferT;
		ArBufferT tmp;

	public:
		typedef T value_type;

		explicit Row(const std::string& suffixIn, uint64_t idIn)
		: suffix(suffixIn), id(idIn)
		{
		}

		uint64_t get_column() const {
			return id;
		}

		void reserve(size_t s)
		{
			buf.reserve(s);
		}

		const std::string name() const {
			return suffix;
		}

		void push_back(const T val)
		{
			buf.push_back(val);
		}

		size_t size() const
		{
			return buf.size();
		}

		T get(size_t i) const
		{
			return buf[i];
		}

		bool eof() const
		{
			return disk.eof();
		}

		// open file
		void open(std::string fn, Direction di)
		{
			fn.append(suffix);
			if (di == ROW_WRITE)
			{
				disk.open(fn, O_CREAT|O_WRONLY|O_APPEND);
			} else if ( di == ROW_READ)
			{
				disk.open(fn, O_RDONLY);
			}
		}

		void write()
		{
			if (buf.empty()) {
				return;
			}

			LOG4_TRACE(suffix << ": compressing.. " << buf.size() * sizeof(value_type) << " bytes");
			// compress and write
			lzo.Compress(buf, tmp);

			size_t elements = buf.size();
			LOG4_TRACE(suffix << ": writing.. " << tmp.size() << " bytes");
			disk.write(elements);
			disk.write(tmp);

			// clear buf for new data
			buf.clear();
		}

		size_t read()
		{
			size_t elements = 0;
			disk.read(elements);
			disk.read(tmp);

			buf.resize(elements);
			lzo.Decompress(&tmp[0], tmp.size(), buf);

			LOG4_TRACE("Read block: " << suffix << ": "
						<< elements << " elements, in "
						<< tmp.size() << " c-bytes" );
			return elements;
		}

		void close()
		{
			disk.close();
		}

};

} // namespace CDB

#endif /* _CDB_ROW_HPP__ */



