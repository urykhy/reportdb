/*
 * _COPYRIGHT_
 */

#ifndef _MDC_INTERNAL_HPP__
#define _MDC_INTERNAL_HPP__

#include <Types.h>

namespace MDC {
	using std::size_t;

	const char* SUFFIX_DATA=".data";
	const char* SUFFIX_PK=".pk";
	const char* SUFFIX_KEYS=".keys";

	// minimal size (bytes) to compress in filter
	const static UINT_32 COMPRESS_MIN = 32;

	struct PrimaryKey
	{
		UINT_64 offset;
		UINT_32 size;   // on disk size
		UINT_32 osize;  // original data size in bytes(decpmpressed)

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



