/*
 * _COPYRIGHT_
 */

#ifndef _MDC_FILTER_HPP__
#define _MDC_FILTER_HPP__

#include <vector>
#include <cassert>
#include <Index.hpp>
#include <ArLZO.hpp>

namespace MDC {
namespace Filter {

// MDC filters here
// since we want different compression methods
// so we provide a bitset compression for uint32_t and less
// and LZO compression for more complex types
//
// user can write own compression classes.

class BitSet {
	private:
		// not-copyable
		BitSet(const BitSet&);
		BitSet& operator=( const BitSet&);

		const uint32_t min_size;
		Util::Index tmp_bitset;
		std::vector<unsigned char> s_buffer;
		Util::Index bs;

	public:

		BitSet (uint32_t min_size_in = COMPRESS_MIN)
			: min_size(min_size_in)
		{
			;;
		}

		// check if block should be compressed
		bool can_compress(uint32_t size) const
		{
			return size >= min_size;
		}

		template<class T>
		const MemoryBuf
		compress(const T& cube)
		{
			for(auto i = cube.begin();
					 i != cube.end();
					 ++i)
			{
				tmp_bitset.set(*i);
			}
			tmp_bitset.serialize(s_buffer);
			tmp_bitset.clear();

			return make_buf(s_buffer);
		}

		const Util::Index&
		decompress(const void* raw, uint32_t /*size*/, uint32_t /*osize*/)
		{
			bs.clear();
			bs.join(raw);
			return bs;
		}
};

template<class T>
class LZO {
	public:
		typedef std::vector<T> BufferT;

	private:
		// not-copyable
		LZO(const LZO&);
		LZO& operator=( const LZO&);

		const uint32_t min_size;
		Util::ArLZO ar;
		std::vector<unsigned char> s_buffer;
		BufferT res;

	public:

		LZO(uint32_t min_size_in = COMPRESS_MIN)
		: min_size(min_size_in)
		{
			;;
		}

		// check if block should be compressed
		bool can_compress(uint32_t size) const
		{
			return size >= min_size;
		}

		const MemoryBuf
		compress(const BufferT& cube)
		{
			ar.Compress(cube, s_buffer);
			return make_buf(s_buffer);
		}

		const BufferT&
		decompress(const void* raw, uint32_t size, uint32_t osize)
		{
			assert (osize % sizeof(T) == 0);
			//FIXME: throw an exception if data error ?

			res.resize(osize/sizeof(T));
			ar.Decompress(static_cast<const lzo_bytep>(raw), size, res);

			return res;
		}
};

} // namespace Filter
} // namespace MDC

#endif /* _MDC_FILTER_HPP__ */



