/*
 * _COPYRIGHT_
 */

#ifndef _MDC_INTERNAL_HPP__
#define _MDC_INTERNAL_HPP__

#include <stdint.h>

namespace MDC {
	using std::size_t;

	const char* SUFFIX_DATA=".data";
	const char* SUFFIX_PK=".pk";
	const char* SUFFIX_KEYS=".keys";

	// minimal size (bytes) to compress in filter
	const static uint32_t COMPRESS_MIN = 32;

	struct PrimaryKey
	{
		uint64_t offset;
		uint32_t size;   // on disk size
		uint32_t osize;  // original data size in bytes(decpmpressed)

		PrimaryKey() : offset(0), size(0), osize(0)
		{
			;;
		}
	};

	typedef std::pair<const void*, size_t> MemoryBuf;

	template<class T>
	const MemoryBuf
	make_buf(const std::vector<T>& buf)
	{
		MemoryBuf dat;
		dat.first = &buf.front();
		dat.second = buf.size() * sizeof(T);
		return dat;
	}



} // namespace MDC

#endif /* _MDC_INTERNAL_HPP__ */



