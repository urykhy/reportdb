
#ifndef _CDB_TL_HPP__
#define _CDB_TL_HPP__

#include <Types.h>
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

			struct Push {
				template<class T, class D>
				void operator()(T& t, D& d){
					t.push_back(d);
				}
			};
			template<class Data>
			void push_back(const Data& d) {
				Push push;
				// FIXME: rm const_cast
				Data& d_const = const_cast<Data&>(d);
				ForEach2(acc, d_const, push);
			}

			struct Open {
				const std::string& fn;
				CDB::Direction di;
				const UINT_64 columns;
				Open(
					const std::string& fn_,
					CDB::Direction di_,
					const UINT_64 columns_ )
				: fn(fn_), di(di_), columns(columns_) {}

				template<class T>
				void operator()(T& t, INT_32 len)
				{
					if (1 << len & columns) {
						//std::cout << "open for [" << t.name() << "]" << std::endl;
						t.open(fn, di);
					}
				}
			};
			void open(const std::string& fn, CDB::Direction di, const UINT_64 columns)
			{
				Open op(fn, di, columns);
				ForEachN(acc, op);
			}

			struct Reserve {
				const size_t s;
				Reserve(size_t s_) : s(s_) {}
				template<class T>
				void operator()(T& t)
				{
					t.reserve(s);
				}
			};

			void reserve(size_t s)
			{
				Reserve op_res(s);
				ForEach(acc, op_res);
			}

			struct Write {
				template<class T>
				void operator()(T& t)
				{
					t.write();
				}
			};
			void write() {
				Write wr;
				ForEach(acc, wr);
			}

			struct Read {
				const UINT_64 columns;
				size_t sz;
				Read(const UINT_64 columns_) : columns(columns_), sz(0) {}

				template<class T>
				void operator()(T& t, INT_32 len){
					if (1 << len & columns) {
						size_t rc = t.read();
						if (!sz) {
							sz = rc;
						} else {
							// FIXME: wrong data
							assert(sz == rc);
						}
					}
				}
			};
			size_t read(const UINT_64 columns) {
				Read re(columns);
				ForEachN(acc, re);
				return re.sz;
			}

			struct Close {
				template<class T>
				void operator()(T& t) {
					t.close();
				}
			};
			void close() {
				Close cl;
				ForEach(acc, cl);
			}

			struct IsEof {
				bool stop;
				const UINT_64 columns;
				bool rc;
				IsEof(const UINT_64 columns_)
				: stop(false), columns(columns_), rc(false) {}

				template<class T>
				void operator()(T& t, INT_32 len)
				{
					if (!stop && 1 << len & columns) {
						//std::cout << "eof for [" << t.name() << "]" << std::endl;
						rc = t.eof();
						stop = true;
					}
				}
			};
			bool eof(const UINT_64 columns) {
				IsEof iseof(columns);
				ForEachN(acc, iseof);
				return iseof.rc;
			}
		};

		template<class T>
		struct DeriveIndex {
			typedef std::tr1::unordered_set<typename T::value_type> Result;
		};

		template<class Acc, class Index>
		struct Narrow {
			Index index;

			struct FindColumns {
				UINT_64 rc;
				FindColumns() : rc(0) {}
				template<class T>
				void operator()(T& t, INT_32 len) {
					if (!t.empty()) {
						rc |= 1 << len;
					}
				}
			};
			UINT_64 columns ()
			{
				FindColumns fc;
				ForEachN(index, fc);
				return fc.rc;
			}

			struct CheckIndex
			{
				bool rc;
				const size_t rown;
				CheckIndex(const size_t rown_) : rc(true), rown(rown_) {}

				template<class T, class V>
				void operator()(T& t, V& v)
				{
					if (rc && !t.empty()) {
						if (t.find(v.get(rown)) == t.end()) {
							rc = false;
						}
					}
				}
			};
			bool operator()(Acc& ac, const size_t rown)
			{
				CheckIndex cindex(rown);
				ForEach2(index, ac.acc, cindex);
				return cindex.rc;
			}
		};

	} // namespace TL

	template<class T>
	struct MakeRoot {
		typedef typename TL::ForEachT<T, TL::DeriveData>::Result Data;
		typedef TL::Accessor<T> Accessor;

		typedef typename TL::ForEachT<T, TL::DeriveIndex>::Result Index;
		typedef TL::Narrow<Accessor, Index> Narrow;
	};

#define CDB_BEGIN_TABLE(NAME) \
	typedef Util::TL::TypeList<Util::TL::null_type, Util::TL::null_type>

#define CDB_COLUMN(NAME, TYPE) \
	members_before_##NAME; \
	struct NAME##_base { static const char* name() { return "."#NAME; } }; \
	struct NAME##_row : public CDB::Row<TYPE> { \
		NAME##_row() : CDB::Row<TYPE>(NAME##_base::name()) {} \
	}; \
	typedef Util::TL::AddTail<members_before_##NAME, NAME##_row>::Result NAME##current; \
	static const UINT_64 ColumnName_##NAME = 1 << members_before_##NAME::length ; \
	typedef NAME##current

#define CDB_END_TABLE(NAME) \
	tmp##NAME; \
	typedef CDB::MakeRoot<tmp##NAME> NAME;

	// use ColumnName_<name> to call columns

} // namespace CDB

#endif /* _CDB_TL_HPP__ */



