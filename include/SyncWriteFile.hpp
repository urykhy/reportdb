
#ifndef _BUFFER_FILE_HPP__
#define _BUFFER_FILE_HPP__

#include <HFile.hpp>

namespace Util {
namespace aux {
// buffer to use in BufferFile
class WriteCounter {
		ssize_t used;
		off64_t write_pos;
	public:
		WriteCounter()
		: used(0), write_pos(0)
		{
		}
		~WriteCounter() throw () {
		}

		ssize_t write(BasicFile& fd, const void* s, size_t l) {
			if (!used) {
				write_pos = fd.get_pos();
			}
			//std::cerr << "write " << l << " bytes; range start at " << write_pos << std::endl;
			fd.write(s, l);
			used += l;
			return used;
		}
		void sync(BasicFile& fd) {
			fd.sync_range(write_pos, used, SYNC_FILE_RANGE_WRITE);
			//std::cerr << "sync from " << write_pos << ", size " << used << std::endl;
		}
		void wait(BasicFile& fd) {
			if (!empty()) {
				//std::cerr << "wait from "<< write_pos << ", size " << used << std::endl;
				fd.sync_range(write_pos, used, SYNC_FILE_RANGE_WAIT_BEFORE | SYNC_FILE_RANGE_WRITE | SYNC_FILE_RANGE_WAIT_AFTER);
				fd.advise(write_pos, used, POSIX_FADV_DONTNEED);
				clear();
			}
		}
		void clear() {
			used = 0;
			write_pos = 0;
		}
		bool empty() const {
			return used == 0;
		}
};

// write file
// sync + drop written parts from pagecache
class SyncWriteImpl {
		BasicFile impl;
		aux::WriteCounter buf[2];
		int current;

		void flush() {
			if (!buf[current].empty())
			{
				buf[current].sync(impl); // write normal buffer
				current = (current + 1) % 2;
				buf[current].wait(impl);  // wait for prevoius buffer to be written
			}
		}

	public:
		SyncWriteImpl() : current(0) {}
		~SyncWriteImpl() throw() { close(); }

		void open(const std::string& fname, int flags)
		{
			impl.open(fname, flags);
		}
		void close() {
			flush();		   // flush current chunk
			buf[0].wait(impl);
			buf[1].wait(impl); // wait for data to be written
			impl.close();
			current = 0;
		}

		void read(void* buf, ssize_t len) {
			impl.read(buf, len);
		}

		bool eof() const {
			return impl.eof();
		}

		void write(const void* data, ssize_t len)
		{
			const ssize_t window = 1024*4096;
			ssize_t used = buf[current].write(impl, data, len);
			if (used >= window) {
				std::cerr << "flush" << std::endl;
				flush();
			}
		}
};
} // aux

typedef Util::aux::NiceFile<Util::aux::SyncWriteImpl> SyncWriteFile;

} // namespace Util

#endif /* _BUFFER_FILE_HPP__ */



