
#ifndef _CDB_TL_HPP__
#define _CDB_TL_HPP__

#include <stdint.h>
#include <cassert>
#include <TypeLists.hpp>
#include <tr1/unordered_set>

namespace CDB {
	namespace TL {
		using namespace Util::TL;

		template<class T>
		struct DeriveData {
			typedef typename T::value_type Result;
		};

		template<class Acc>
		struct Accessor {
			Acc acc;

			template<class Data>
			void push_back(const Data& d) {
				tuple_for_each2([](auto& t, auto& d){ t.push_back(d); }, acc, d);
			}

			void open(const std::string& fn, CDB::Direction di, const uint64_t columns)
			{
				tuple_for_each([&fn,&di,&columns](auto& t) mutable {
					if (t.get_column() & columns) {
						t.open(fn, di);
					}
				}, acc);
			}

			void reserve(size_t s)
			{
				tuple_for_each([&s](auto& t){ t.reserve(s); }, acc);
			}

			void write() {
				tuple_for_each([](auto& t){ t.write(); }, acc);
			}

			size_t read(const uint64_t columns) {
				size_t sz = 0;
				tuple_for_each([&columns,&sz](auto& t) mutable {
					if (t.get_column() & columns) {
						size_t rc = t.read();
						if (!sz) {
							sz = rc;
						} else {
							assert(sz == rc); // FIXME: wrong data
						}
					}
				}, acc);
				return sz;
			}

			void close() {
				tuple_for_each([](auto& t){ t.close(); }, acc);
			}

			bool eof(const uint64_t columns) {
				bool stop = false;
				bool rc = false;

				tuple_for_each([&stop,&rc,&columns](auto& t) mutable {
					if (!stop && t.get_column() & columns) {
						//std::cout << "eof for [" << t.name() << "]" << std::endl;
						rc = t.eof();
						stop = true;
					}
				}, acc);
				return rc;
			}
		};

		template<class T>
		struct DeriveIndex {
			typedef std::tr1::unordered_set<typename T::value_type> Result;
		};

		template<class Acc, class Index>
		struct Narrow {
			Index index;

			uint64_t columns ()
			{
				uint64_t counter = 0;
				uint64_t rc = 0;

				tuple_for_each([&counter,&rc](auto& t) mutable {
					if (!t.empty()) {
						rc |= 1 << counter;
					}
					counter++;
				}, index);
				return rc;
			}

			bool operator()(Acc& ac, const size_t rown)
			{
				bool rc = true;
				tuple_for_each2([&rc,&rown](auto& t, auto& v){
					if (rc && !t.empty()) {
						if (t.find(v.get(rown)) == t.end()) {
							rc = false;
						}
					}
				}, index, ac.acc);
				return rc;
			}
		};

	} // namespace TL

	template<class T>
	struct MakeRoot {
		typedef typename TL::apply_t<TL::DeriveData, T>::Result Data;
		typedef TL::Accessor<T> Accessor;

		typedef typename TL::apply_t<TL::DeriveIndex, T>::Result Index;
		typedef TL::Narrow<Accessor, Index> Narrow;
	};

#define CDB_BEGIN_TABLE(NAME) \
	typedef std::tuple<>

#define CDB_COLUMN(NAME, TYPE) \
	members_before_##NAME; \
	static const uint64_t ColumnName_##NAME = 1 << std::tuple_size<members_before_##NAME>::value ; \
	struct NAME##_base { static const char* name() { return "."#NAME; } }; \
	struct NAME##_row : public CDB::Row<TYPE> { \
		NAME##_row() : CDB::Row<TYPE>(NAME##_base::name(), ColumnName_##NAME) {} \
	}; \
	typedef Util::TL::append_tail<NAME##_row, members_before_##NAME>::Result NAME##current; \
	typedef NAME##current

#define CDB_END_TABLE(NAME) \
	tmp##NAME; \
	typedef CDB::MakeRoot<tmp##NAME> NAME;

	// use ColumnName_<name> to call columns

} // namespace CDB

#endif /* _CDB_TL_HPP__ */



