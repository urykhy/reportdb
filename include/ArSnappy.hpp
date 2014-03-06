/*
 * _COPYRIGHT_
 */

#ifndef _AR_SNAPPY_HPP__
#define _AR_SNAPPY_HPP__

#include <Types.h>
#include <ReportException.hpp>
#include <vector>

#include <snappy.h>

namespace Util {

class ArSnappy {
	private:
		// not-copyable
		ArSnappy(const ArSnappy&);
		ArSnappy& operator=( const ArSnappy&);

	public:
		typedef std::vector<char> BufferT;

		explicit ArSnappy() {
			;;
		}

		template<class T>
		void
		Compress(const std::vector<T>& data, BufferT& out)
		{
			ssize_t countBS = sizeof(T)*data.size();
			out.resize(snappy::MaxCompressedLength(countBS));
			size_t output_length=0;
			snappy::RawCompress(reinterpret_cast<const char*>(&data[0]),
								countBS,
								&out[0],
								&output_length);
			out.resize(output_length);
		}

		template<class T>
		void Decompress(const char* data, size_t size, std::vector<T>& out)
		{
			if (!snappy::RawUncompress(data,
									   size,
									   reinterpret_cast<char*>(&out[0])) )
			{
				Util::fire_exception("snappy decompress failed");
			}
		}

};

} // namespace Util

#endif /* _AR_SNAPPY_HPP__ */


