/*
 * _COPYRIGHT_
 */

#ifndef _MDC_TL_HPP__
#define _MDC_TL_HPP__

#include <stdint.h>
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

		// generate main Index structure
		template<class Key, class Idx>
		struct MakeIndex {
			Idx index;
			void insert(Key& key, uint32_t cube)
			{	// insert a value to index
				tuple_for_each2([&cube](auto& key, auto& idx) {
					idx[key].set(cube);
				}, key, index);
			}

			template<class N>
			void lookup(Util::Index& cubes, const N& narrow)
			{	// find clusters's we going to process

				tuple_for_each2([&cubes](auto& index, auto& limit) mutable
				{
					// collect via OR cube-ids in tmp
					// and then join via AND
					if (limit.size())
					{	// handle only if we have limits here
						Util::Index tmp;
						// walk over limit values
						for (auto& i : limit)
						{	// ask index for every val in limit
							auto j = index.find(i);
							if (j != index.end())
							{	// join cube indexes
								tmp.join(j->second);
							}
						}
						cubes.index_join(tmp);
					}
				}, index, narrow.limit);
			}

			size_t mem_used()
			{	// return index size (bitset only)
				size_t counter = 0;
				tuple_for_each([&counter](auto& t) mutable {
					for (auto& i : t) {
						counter += i.second.mem_used();
					}
				}, index);
				return counter;
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
				bool empty = true;
				tuple_for_each([&empty](auto& lim) mutable {
					if (!lim.empty()) {
						empty = false;
					}
				}, limit);
				return empty;
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


