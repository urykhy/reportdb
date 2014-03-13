/*
 * _COPYRIGHT_
 */

#ifndef _AR_LZO_HPP__
#define _AR_LZO_HPP__

#include <stdint.h>
#include <ReportException.hpp>
#include <vector>

#include <lzo1x.h>

namespace Util {

class ArLZO {
	private:
		// not-copyable
		ArLZO(const ArLZO&);
		ArLZO& operator=( const ArLZO&);

		std::vector<unsigned char> lzoTemp;

	public:
		typedef std::vector<unsigned char> BufferT;

		explicit ArLZO() : lzoTemp(LZO1X_999_MEM_COMPRESS){
			;;
		}

		// FIXME: store original size in bytes
		// and compressed size in out ?

		template<class T>
		void
		Compress(const std::vector<T>& data, BufferT& out)
		{
			ssize_t countBS = sizeof(T)*data.size();
			lzo_uint outLen=(countBS + countBS / 16 + 64 + 3);   // from simple.c in LZO distrib

			// allocate as much memory as we need
			out.resize(outLen);

			int32_t r = lzo1x_999_compress(
				reinterpret_cast<const lzo_bytep>(&data.front()),
				countBS,
				&out.front(),
				&outLen,
				&lzoTemp[0]);
			if(r != LZO_E_OK){
				Util::fire_exception("lzo compression failed");
			}

			// trim to really used size
			out.resize(outLen);
		}

		// out must be large enough to hold a results
		// (keep original buffer size somewhere)
		template<class T>
		void Decompress(const lzo_bytep data, size_t size, std::vector<T>& out)
		{
			lzo_uint countBsLZO=size*sizeof(T);
			int32_t r = lzo1x_decompress(
				data,
				size,
				reinterpret_cast<lzo_bytep>(&out.front()),
				&countBsLZO,
				NULL);

			if(r != LZO_E_OK){
				Util::fire_exception("lzo decompress failed", r);
			}
		}

};

} // namespace Util

#endif /* _AR_LZO_HPP__ */



