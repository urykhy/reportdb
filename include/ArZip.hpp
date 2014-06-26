
#ifndef _UTIL_AR_LIBZ_HPP__
#define _UTIL_AR_LIBZ_HPP__

#include <zlib.h>
#include <vector>

namespace Util {

	class ArZip
	{
			z_stream def;
			z_stream inf;
		public:
			typedef std::vector<unsigned char> BufferT;

			ArZip(int level = Z_BEST_COMPRESSION)
			{
				def.zalloc = Z_NULL;
				def.zfree = Z_NULL;
				def.opaque = Z_NULL;
				if (deflateInit(&def, level) != Z_OK)
				{
					throw "ArZip: fail to deflateInit";
				}

				inf.zalloc = Z_NULL;
				inf.zfree = Z_NULL;
				inf.opaque = Z_NULL;
				inf.avail_in = 0;
				inf.next_in = Z_NULL;
				int ret = inflateInit(&inf);
				if (ret != Z_OK)
				{
					throw "ArZip: fail to inflateInit";
				}

			}

			~ArZip() throw ()
			{
				deflateEnd(&def);
				inflateEnd(&inf);
			}

			size_t bound(size_t len)
			{
				return compressBound(len);
			}

			// out buffer must be enough len to hold `len` bytes
			size_t
			compress(const void* data, size_t len, void* out, size_t out_len)
			{
				deflateReset(&def);
				def.avail_in = len;
				def.next_in = (Bytef*)data;
				def.avail_out = out_len;
				def.next_out = (Bytef*)out;
				int ret = deflate(&def, Z_FINISH);
				if (ret != Z_STREAM_END)
				{
					throw "ArZip: fail to deflate";
				}
				return out_len - def.avail_out;
			}

			template<class T>
			void
			Compress(const std::vector<T>& data, BufferT& out)
			{
				size_t inSize = data.size() * sizeof(T);
				size_t outSize = bound(inSize);
				out.resize(outSize);
				size_t r = compress(&data[0], inSize, &out[0], outSize);
				out.resize(r);
			}

			// out buffer should be enough len to hold result
			// (keep it somewhere?)
			// return decompressed size
			size_t
			decompress(const void* data, size_t len, void* out, size_t out_max)
			{
				inflateReset(&inf);
				inf.avail_in = len;
				inf.next_in = (Bytef*)data;
				inf.avail_out = out_max;
				inf.next_out = (Bytef*)out;
				size_t ret = inflate(&inf, Z_FINISH);
				if (ret != Z_STREAM_END)
				{
					throw "ArZip: fail to inflate";
				}
				return out_max - inf.avail_out;
			}

			template<class T>
			void Decompress(const void* data, size_t size, std::vector<T>& out)
			{
				decompress(data, size, &out[0], out.size() * sizeof(T));
			}
	};

} // namespace Util

#endif /* _UTIL_AR_LIBZ_HPP__ */

