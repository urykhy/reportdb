/*
 * _COPYRIGHT_
 */

#ifndef _CREATE_MDC_HPP__
#define _CREATE_MDC_HPP__

#include <stdint.h>
#include <map>
#include <vector>

#include <MDCInternal.hpp>
#include <Index.hpp>
#include <HFile.hpp>
#include <Log4.hpp>

namespace MDC {

template<class CubeData>
class Create {
	private:
		// not-copyable
		Create(const Create&);
		Create& operator=( const Create&);

		typedef std::vector<typename CubeData::Key> KeysT;
		KeysT keys;

		typedef std::vector<PrimaryKey> PKT;
		PKT pk;

		const std::string& filename;
		uint64_t currentPos;
		ssize_t counter;
		ssize_t pak_counter;
		Util::HFile data_writer;

	public:
		typedef std::vector<typename CubeData::Val> ValListT;
		typedef std::map<typename CubeData::Key, ValListT> RawDataT;

	private:

		// cbuf - compressed data
		// obuf - original data
		template<class T>
		void add_cube(const MemoryBuf cbuf, const T& obuf) {
			uint32_t osize = obuf.size() * sizeof(typename T::value_type);

			if (cbuf.second < osize) {
				add_cube_pk(cbuf, osize);
				pak_counter++;
			} else {
				add_cube_pk(make_buf(obuf));
			}
		}

		void add_cube_pk(const MemoryBuf buf, uint32_t osize=0) {
			PrimaryKey k;
			k.offset = currentPos;
			k.size = buf.second;

			if (!osize) {
				k.osize = buf.second;
			} else {
				k.osize = osize;
			}

			write_cube(buf);

			currentPos += k.size;
			pk.push_back(k);
		}

		void write_cube(const MemoryBuf buf) {
			if ( !data_writer.is_open() ) {
				data_writer.open(filename + SUFFIX_DATA, O_CREAT|O_WRONLY|O_APPEND);
			}
			data_writer.write(buf.first, buf.second);
		}

		void write(const char* suffix, const void* data, size_t len)
		{
			std::string name = filename + suffix;
			Util::HFile wr;
			wr.open(name, O_CREAT|O_WRONLY|O_APPEND);
			wr.write(data, len);
		}

		void write(const PKT& buf) {
			write(SUFFIX_PK, &buf[0], buf.size() * sizeof(PKT::value_type));
		}

		void write(const KeysT& buf) {
			write(SUFFIX_KEYS, &buf[0], buf.size() * sizeof(typename KeysT::value_type));
		}

	public:

		/**
		 @brief Constructor
		 */
		explicit Create(const std::string& fn)
		: filename(fn), currentPos(0), counter(0), pak_counter(0)
		{
			;;
		}

		template<class F>
		void insert(const RawDataT& data, F& filter) {
			LOG4_TRACE("about to insert " << data.size() << " elements");
			keys.reserve(data.size() + keys.size());
			pk.reserve(data.size() + pk.size());

			for(auto i = data.begin();
					 i != data.end();
					 ++i)
			{
				keys.push_back(i->first);

				uint32_t bs = i->second.size() * sizeof(typename CubeData::Val);

				if (filter.can_compress(bs)) {
					add_cube(filter.compress(i->second), i->second);
				} else {
					add_cube_pk(make_buf(i->second));
				}

				counter++;
			}
		}

		void close() {
			write(pk);
			write(keys);
			data_writer.close();

			LOG4_DEBUG(counter << " index elements added, (including "
				<< pak_counter << " compressed)");

		}

		/**
		 @brief Destructor
		 */
		~Create() throw() {
			;;
		}

};

} // namespace MDC

#endif /* _CREATE_MDC_HPP__ */



