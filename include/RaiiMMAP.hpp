/*
 * _COPYRIGHT_
 */

#ifndef _RAII_MMAP_HPP__
#define _RAII_MMAP_HPP__

#include <Types.h>
#include <ReportException.hpp>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace Util {

class RaiiMMAP {
	private:
		// not-copyable
		RaiiMMAP(const RaiiMMAP&);
		RaiiMMAP& operator=( const RaiiMMAP&);

		int fd;
		ssize_t fsize;
		void* data;

	public:
		/**
		 @brief Constructor
		 @param fname - filename to mmap
		 */
		explicit RaiiMMAP(const std::string& fname)
		: fd(-1), fsize(0), data(0)
		{
			fd = open (fname.c_str(), O_RDONLY);
			if (-1 == fd)
			{
				Util::fire_exception("fail to open "+fname, errno);
			}

			struct stat fdata;
			if (-1 == fstat(fd, &fdata))
			{
				close(fd);
				Util::fire_exception("fail to fstat "+fname, errno);
			}
			fsize = fdata.st_size;

			data=mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0 );
			if (MAP_FAILED == data){
				close(fd);
				Util::fire_exception("fail to mmap "+fname, errno);
			}
		}

		/**
		 @brief Destructor
		 */
		~RaiiMMAP() throw() {
			munmap(data, fsize);
			close(fd);
		}

		/**
		 @brief Get pointer to begin of mapping
		 @return pointer
		 */
		void* addr(){
			return data;
		}

		/**
		  @brief Get const pointer to begin of mapping
		  @return const pointer
		 */
		const void* addr() const {
			return data;
		}

		/**
		 @brief Get file size
		 @return file size
		 */
		ssize_t size() const {
			return fsize;
		}

};

} // namespace Util

#endif /* _RAII_MMAP_HPP__ */



