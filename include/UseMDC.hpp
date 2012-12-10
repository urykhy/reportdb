/*
 * _COPYRIGHT_
 */

#ifndef _USE_MDC_HPP__
#define _USE_MDC_HPP__

#include <Types.h>
#include <MDCInternal.hpp>
#include <RaiiMMAP.hpp>
#include <Index.hpp>
#include <Log4.hpp>

namespace MDC {

template<class CubeData>
class UseMDC {
	private:
		// not-copyable
		UseMDC(const UseMDC&);
		UseMDC& operator=( const UseMDC&);

		const std::string& fn_;
		Util::RaiiMMAP map_keys;
		Util::RaiiMMAP map_pk;
		Util::RaiiMMAP map_data;
		ssize_t cube_max;
		typename CubeData::Index index;

		typedef typename CubeData::Key Key;
		typedef typename CubeData::Val Val;

		void build_index()
		{
			void* ptr = map_keys.addr();
			Key* key = static_cast<Key*>(ptr);

			for (ssize_t i = 0; i < cube_max; ++i)
			{
				index.insert(key[i], i);
			}
		}

		PrimaryKey find_cube(UINT_32 cube)
		{
			const void* ptr = map_pk.addr();
			const PrimaryKey* cube_info = static_cast<const PrimaryKey*>(ptr);
			return cube_info[cube];
		}

		Key find_key(UINT_32 cube)
		{
			void* ptr = map_keys.addr();
			Key* key = static_cast<Key*>(ptr);
			return key[cube];
		}

		template<class W, class F>
		void process(
				const PrimaryKey cube_key,
				const Key& key,
				W& worker,
				F& filter)
		{
			const UCHAR_8* ptr = static_cast<const UCHAR_8*>(map_data.addr());
			ptr += cube_key.offset;

			if (cube_key.size < cube_key.osize)
			{
				// compressed data
				worker(key, filter.decompress(ptr, cube_key.size, cube_key.osize));
			} else {
				// normal data
				const UCHAR_8* end = ptr + cube_key.size;
				while(ptr < end)
				{
					const Val* val = static_cast<const Val*>( static_cast<const void*>(ptr) );
					worker(key, *val);
					ptr += sizeof(Val);
				}
			}
		}

	public:

		/**
		 @brief Constructor
		 */
		explicit UseMDC(const std::string& fn)
		: fn_(fn),
		  map_keys(fn + SUFFIX_KEYS),
		  map_pk(fn + SUFFIX_PK),
		  map_data(fn + SUFFIX_DATA),
		  cube_max(map_keys.size() / sizeof(Key))
		{
			LOG4_DEBUG("building index for " << fn);
			build_index();
		}

		/**
		 @brief Destructor
		 */
		~UseMDC() throw() {
			;;
		}

		// FIXME: constness
		// it's hard to get it work with MPL
		template<class Narrow, class W, class F>
		void access(Narrow& narrow, W& worker, F& filter)
		{
			Util::Index cubes;

			if (narrow.empty()) {
				cubes.set_range(0,cube_max-1);
			} else {
				index.lookup(cubes, narrow);
			}
			LOG4_DEBUG("access " << fn_ << ":" << cube_max << "/" << cubes.size());

			// walk over
			for(auto i=cubes.begin();
					 i < cubes.end();
					 ++i)
			{
				// handle cube
				Key key = find_key(*i);
				PrimaryKey primary = find_cube(*i);
				process(primary, key, worker, filter);
			}
		}

		size_t mem_used()
		{
			return index.mem_used();
		}

};

} // namespace MDC

#endif /* _USE_MDC_HPP__ */

