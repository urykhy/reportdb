/*
 * _COPYRIGHT_
 */

#ifndef _MDC_TL_HPP__
#define _MDC_TL_HPP__

#include <Types.h>
#include <set>
#include <algorithm>
#include <TypeLists.hpp>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

namespace MDC {
	namespace TL {

		using namespace Util::TL;

		// make a index
		template<class T>
		struct DeriveIndex
		{
			typedef std::tr1::unordered_map<T, Util::Index> Result;
		};

		// make a narrow's limit
		template<class T>
		struct DeriveLimit
		{
			typedef std::tr1::unordered_set<T> Result;
		};

		// functor to insert a value in index
		struct Inserter {
			UINT_32 cube;
			Inserter(UINT_32 cubeIn)
			: cube(cubeIn) {}

			template<class Key, class Idx>
			void operator()(Key& key, Idx& idx) {
				idx[key].set(cube);
			}
		};

		// apply a narrow limit's to cubes
		struct Narrower {
			Util::Index& cubes;
			Narrower(Util::Index& cubesIn)
			: cubes(cubesIn) {}

			template<class IndexT, class LimitT>
			void operator()(IndexT& index, LimitT& limit) {
				// collect via OR cube-ids in tmp
				// and then join via AND

				if (limit.size()) {	// handle only if we have limits here
					Util::Index tmp;
					// walk over limit values
					for(auto i=limit.begin();
							 i != limit.end();
							++i)
					{ // ask index for every val in limit
						auto j = index.find(*i);
						if (j != index.end()){
							// join cube indexes
							tmp.join(j->second);
						}
					}
					cubes.index_join(tmp);
				}
			}
		};

		struct MemUsageCounter
		{
			size_t counter;
			MemUsageCounter()
			: counter(0)
			{
			}

			template<class T>
			void operator()(T& t)
			{
				for(auto i = t.begin();
						 i != t.end();
						 ++i)
				{
					counter += i->second.mem_used();
				}
			}
		};

		// generate main Index structure
		template<class Key, class Idx>
		struct MakeIndex {
			Idx index;
			void insert(Key& key, UINT_32 cube)
			{	// insert a value to index
				Inserter inserter(cube);
				tuple_for_each2(inserter, key, index);
			}
			template<class N>
			void lookup(Util::Index& cubes, N& narrow)
			{	// find clusters's we going to process
				Narrower na(cubes);
				tuple_for_each2(na, index, narrow.limit);
			}
			size_t mem_used()
			{	// return index size (bitset only)
				MemUsageCounter muc;
				tuple_for_each(muc, index);
				return muc.counter;
			}
		};

		// functor to check if narrow if empty
		struct CheckIfEmpty
		{
			bool empty;
			CheckIfEmpty()
			: empty(true)
			{
			}

			template<class Limit>
			void operator()(Limit& lim)
			{
				if (!lim.empty()){
					empty = false;
				}
			}
		};

		// generate main Narrow structure
		template<class Limit>
		struct MakeNarrow
		{
			typedef Limit LimitT;
			LimitT limit;
			bool empty()
			{
				CheckIfEmpty ce;
				tuple_for_each(ce, limit);
				return ce.empty;
			}
		};

	} // namespace TL

	// main MDC
	template<class KeyT, class ValueT>
	struct Client
	{
		typedef KeyT Key;
		typedef ValueT Val;

		typedef typename TL::apply_t<TL::DeriveIndex, Key>::Result IndexTL;
		typedef TL::MakeIndex<Key, IndexTL> Index;

		typedef typename TL::apply_t<TL::DeriveLimit, Key>::Result LimitTL;
		typedef TL::MakeNarrow<LimitTL> Narrow;
	};


} // namespace MDC

#endif /* _MDC_TL_HPP__ */


