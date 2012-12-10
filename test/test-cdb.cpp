
#include <CreateCDB.hpp>
#include <UseCDB.hpp>
#include <Index.hpp>

#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

namespace tut
{
    test_runner_singleton runner;
	struct null_group {};
	typedef test_group<null_group> tg;
	typedef tg::object object;
	tg tg_1("cdb main test");

	struct Root {
		// must a bit set (UseCDB::access)
		enum KeyCode {I_AGE    = 1 << 0,
			I_GENDER = 1 << 1,
			I_CITY   = 1 << 2};

		struct Data {
			UINT_8  age;
			UINT_8  gender;
			UINT_32 city;
			UINT_32 uid;

#define CGEN_LESS(a,b) \
			if (a < b ) return true; \
			if (a > b ) return false;

			bool operator<(const Data& right) const {
				CGEN_LESS(age, right.age)
					CGEN_LESS(gender, right.gender)
					CGEN_LESS(city, right.city)
					CGEN_LESS(uid, right.uid)
					return false;
			}
#undef CGEN_LESS
		};

		struct Accessor {
			CDB::Row<UINT_8> age;
			CDB::Row<UINT_8> gender;
			CDB::Row<UINT_32> city;
			CDB::Row<UINT_32> uid;

			void push_back(const Data& d) {
				age.push_back(d.age);
				gender.push_back(d.gender);
				city.push_back(d.city);
				uid.push_back(d.uid);
			}

			void open(const std::string& fn, CDB::Direction di, UINT_64 columns)
			{
#define CGEN_NAR(field, code) \
				if (columns & code){ \
					field.open(fn, di); \
				}
				CGEN_NAR(age, I_AGE)
					CGEN_NAR(gender, I_GENDER)
					CGEN_NAR(city, I_CITY)
#undef CGEN_NAR

					uid.open(fn, di);
			}
			void reserve (size_t n)
			{
				age.reserve(n);
				gender.reserve(n);
				city.reserve(n);
				uid.reserve(n);
			}
			void write()
			{
				age.write();
				gender.write();
				city.write();
				uid.write();
			}

			// FIXME: remove always-read-uid-column ?
			size_t read(UINT_64 columns)
			{
#define CGEN_NAR(field, code) \
				if (columns & code){ \
					field.read(); \
				}
				CGEN_NAR(age, I_AGE)
					CGEN_NAR(gender, I_GENDER)
					CGEN_NAR(city, I_CITY)
#undef CGEN_NAR

					return uid.read();
			}

			void close()
			{
				age.close();
				gender.close();
				city.close();
				uid.close();
			}

			bool eof(UINT_64 columns)
			{
				return uid.eof();
			}

			Accessor()
			: age(".age"),
			  gender(".gender"),
			  city(".city"),
			  uid(".uid")
			{
				;;
			}
		};

	};

	// conditional report
	struct Narrow1 {
		// util::index is a fast bitset with 32bit key
		// used as faster std::set replacement
		Util::Index ages;
		Util::Index genders;
		Util::Index cities;

		UINT_64 columns () const
		{
			UINT_64 res=0;

			if(!ages.empty()) res |= Root::I_AGE;
			if(!genders.empty()) res |= Root::I_GENDER;
			if(!cities.empty()) res |= Root::I_CITY;

			return res;
		}

		template<class K, class T>
			bool check_in(const K& dict, const T val) const
			{
				return dict.get(val);
			}

		bool operator()(const Root::Accessor& ac, size_t rown) const {
			if (!ages.empty()	 and !check_in(ages, ac.age.get(rown))) return false;
			if (!genders.empty() and !check_in(genders, ac.gender.get(rown))) return false;
			if (!cities.empty()  and !check_in(cities, ac.city.get(rown))) return false;

			return true;
		}
	};

	// class to make a report
	struct Worker1
	{
		UINT_32 calls;

		Worker1()
		: calls(0)
		{
			;;
		}
		void operator()(const Root::Accessor& ac, size_t idx)
		{
			calls++;
		}

		// ask no additional columns
		UINT_64 columns()
		{
			return 0;
		}

	};

	template<>
	template<>
	void object::test<1>()
	{
		// create data

		std::string fn("__cdb_test1");
		CDB::Create<Root> cdb(fn);
		CDB::Create<Root>::RawDataT id;

		// id should be sorted
		// w/o duplicate keys

		Root::Data v;
		for (INT_8 i=10; i<20; i++) {
			v.age=i;
			for(INT_8 j=0; j<3; j++){
				v.gender=j;
				for(UINT_32 k=100; k<200; k++ ) {
					v.city=k;
					v.uid=k;
					id.insert(v);
					v.uid++;
					id.insert(v);
				}
			}
		} // 3K index elements
		cdb.insert(id);
		cdb.close();

		{
			CDB::Use<Root> uc(fn);
			Narrow1 narrow;
			Worker1 worker1;
			uc.access(narrow, worker1);
			ensure("worker call count", 6000 == worker1.calls);
		}
		{
			CDB::Use<Root> uc(fn);
			Narrow1 narrow;
			narrow.genders.set(0);
			narrow.ages.set(10);
			Worker1 worker1;
			uc.access(narrow, worker1);
			ensure("worker call count/narrow", 200 == worker1.calls);
		}

	}

} // namespace tut

int main()
{
	tut::reporter reporter;
	tut::runner.get().set_callback(&reporter);
	tut::runner.get().run_tests();
	return !reporter.all_ok();
}


