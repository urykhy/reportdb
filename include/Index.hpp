/*
 * _COPYRIGHT_
 */

#ifndef _UTIL_INDEX_HPP__
#define _UTIL_INDEX_HPP__

#include <Types.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wempty-body"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"
#pragma GCC diagnostic warning "-fpermissive"
#define BMCOUNTOPT
#include <bm.h>
#include <bmserial.h>
#pragma GCC diagnostic pop

namespace Util {

class Index
{
#ifdef _DEBUG
		// use memory-saving method while debug
		typedef bm::bvector<bm::standard_allocator> BufT;
#else
		typedef bm::bvector<> BufT;
#endif
		BufT buf;
		bool first_join;

	public:

		Index()
		: first_join(true)
		{
		}

		void set(UINT_32 val)
		{
			buf.set_bit(val);
		}

		bool get(UINT_32 val) const
		{
			return buf.get_bit(val);
		}

		void set_range(UINT_32 left, UINT_32 right)
		{
			buf.set_range(left, right);
		}

		bool empty() const
		{
			return buf.none();
		}

		void clear() {
			buf.clear(true);
		}

		size_t size() const {
			return buf.count();
		}

		typedef BufT::enumerator enumerator;

		enumerator begin() const {
			return buf.first();
		}

		enumerator end() const {
			return buf.end();
		}

		void index_join(const Index& i)
		{
			if (first_join) {
				buf = i.buf;    // initial OR
				first_join=false;
			} else {
				buf &= i.buf;   // merge with AND
			}
		}

		void join(const void* raw)
		{
			bm::deserialize(buf, static_cast<const UCHAR_8*>(raw));
		}

		void join(const Index& i)
		{
			buf |= i.buf;
		}

		void serialize(std::vector<UCHAR_8>& res)
		{
			buf.optimize();
			BufT::statistics stats;
			buf.calc_stat(&stats);

			res.resize(stats.max_serialize_mem);
			size_t real_size = bm::serialize(buf, &res.front(), bm::BM_NO_BYTE_ORDER );
			res.resize(real_size);
		}

		size_t mem_used() const
		{
			return buf.get_blocks_manager().mem_used();
		}

};


} // namespace Util

#endif /* _UTIL_INDEX_HPP__ */



