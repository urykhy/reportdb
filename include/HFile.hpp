/*
 * _COPYRIGHT_
 */

#ifndef _H_File_HPP__
#define _H_File_HPP__

#include <Types.h>
#include <ReportException.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cassert>
#include <string.h>
#include <unistd.h>

namespace Util {

class BasicFile {
	private:
		// not-copyable
		BasicFile(const BasicFile&);
		BasicFile& operator=(const BasicFile&);

		int fd;
		ssize_t sz;

	public:
		explicit BasicFile()
		: fd (-1), sz(0) {
			;;
		}
		~BasicFile() throw() {
			close();
		}

		void open(const std::string& fname, int flags)
		{
			fd = ::open(fname.c_str(), flags, 0644);
			if ( -1 == fd)
			{
				fire_exception("fail to open "+fname, errno);
			}

			if (flags == O_RDONLY){
				struct stat fdata;
				if (-1 == fstat(fd, &fdata))
				{
					close();
					fire_exception("fail to fstat "+fname, errno);
				}
				sz = fdata.st_size;
			}
		}

		void close()
		{
			if (-1 != fd) {
				::close(fd);
				fd = -1;
				sz = 0;
			}
		}

		// FIXME: handle partial read/write
		// EINTR and similar
		void read(void* buf, ssize_t len)
		{
			ssize_t rc = ::read(fd, buf, len);
			if (rc != len )
			{
				fire_exception("fail to read", errno);
			}
		}
		void write(const void* buf, ssize_t len)
		{
			ssize_t rc = ::write(fd, buf, len);
			if (rc != len )
			{
				fire_exception("fail to write", errno);
			}
		}

		bool eof() const
		{
			ssize_t pos = lseek(fd, 0, SEEK_CUR);
			return pos == sz;
		}

		// var methods
		void set_pos(off64_t pos) {
			lseek(fd, pos, SEEK_SET);
		}
		off64_t get_pos() {
			return lseek(fd, 0, SEEK_CUR);
		}

		void sync_range(off64_t offset, off64_t nbytes, unsigned int flags) {
			assert(! ::sync_file_range(fd, offset, nbytes, flags));
		}
		void advise(off64_t offset, off64_t nbytes, unsigned int flags) {
			assert(! ::posix_fadvise(fd, offset, nbytes, flags));
		}

		bool is_open() const
		{
			return fd != -1;
		}

};

// nice read/write for some templates
template<class Impl>
struct NiceFile : public Impl {
		using Impl::read;
		template<class T>
		void read(T& t)
		{
			read(&t, sizeof(T));
		}
		void read(std::vector<unsigned char>& t)
		{
			size_t count;
			read(count);
			t.resize(count);
			read(&t[0], count);
		}
		using Impl::write;
		template<class T>
		void write(const T& t)
		{
			write(&t, sizeof(T));
		}
		void write(const std::vector<unsigned char>& t)
		{
			size_t count = t.size();
			write(count);
			write(&t[0], count);
		}
};

typedef NiceFile<BasicFile> HFile;

} // namespace Util

#endif /* _H_File_HPP__ */

