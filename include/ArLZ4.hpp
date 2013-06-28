/*
 * _COPYRIGHT_
 */

#ifndef _AR_LZ4_HPP__
#define _AR_LZ4_HPP__

#include <Types.h>
#include <ReportException.hpp>
#include <vector>

#include <lz4.h>

namespace Util {

class ArLZ4 {
	private:
		// not-copyable
		ArLZ4(const ArLZ4&);
		ArLZ4& operator=( const ArLZ4&);

	public:
		typedef std::vector<UCHAR_8> BufferT;

		explicit ArLZ4() { ;; }

		template<class T>
		void
		Compress(const std::vector<T>& data, BufferT& out)
		{
			ssize_t countBS = sizeof(T)*data.size();

			// allocate as much memory as we need
			int maxOut = LZ4_compressBound(countBS);
			out.resize(maxOut);

			int r = LZ4_compress((const char*)&data.front(), (char*)&out.front(), countBS);
			if(r == 0) {
				Util::fire_exception("lz4 compression failed");
			}

			// trim to really used size
			out.resize(r);
		}

		// out must be large enough to hold a results
		// (keep original buffer size somewhere)
		template<class T>
		void Decompress(const unsigned char* data, size_t size, std::vector<T>& out)
		{
			ssize_t countBS = sizeof(T)*out.size();
			INT_32 r = LZ4_decompress_fast((const char*)data, (char*)&out.front(), countBS);
			if(r < 0) {
				Util::fire_exception("lz4 decompress failed", r);
			}
		}

};

} // namespace Util

#endif /* _AR_LZ4_HPP__ */



