
#include <CreateMDC.hpp>
#include <UseMDC.hpp>
#include <MDCFilter.hpp>
#include <set>

#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

using namespace MDC;

namespace tut
{
	test_runner_singleton runner;
	struct null_group {};
	typedef test_group<null_group> tg;
	typedef tg::object object;
	tg tg_1("mdc main test");


	struct Data {
		enum KeyCode {I_AGE, I_GENDER, I_CITY};

		struct Key {
			unsigned char  age;
			unsigned char  gender;
			uint32_t city;

#define CGEN_LESS(a,b) \
			if (a < b ) return true; \
			if (a > b ) return false;

			bool operator<(const Key& right) const {
				CGEN_LESS(age, right.age)
					CGEN_LESS(gender, right.gender)
					CGEN_LESS(city, right.city)
					return false;
			}
#undef CGEN_LESS
		};

		// val should be just a INT (upto 32bit width)
		typedef uint32_t Val;

		struct Index {
			std::map<char, Util::Index> age_index;
			std::map<char, Util::Index> gender_index;
			std::map<uint32_t, Util::Index> city_index;

			void insert(const Key& key, uint32_t cube)
			{
				age_index[key.age].set(cube);
				gender_index[key.gender].set(cube);
				city_index[key.city].set(cube);
			}

			template<class T>
			void lookup(Util::Index& cubes, const T& narrow) {
				narrow.apply(I_AGE, cubes, age_index);
				narrow.apply(I_GENDER, cubes, gender_index);
				narrow.apply(I_CITY, cubes, city_index);
			}
			size_t mem_used() { return 0; } // just a stub
		};
	};

	// conditional report
	struct Narrow1 {
		bool empty() const
		{
			return true;
		}

		template<class T>
			void apply(const Data::KeyCode key_code, Util::Index& cubes, const T& index) const
			{
			}
	};

	struct Narrow2 {
		static const unsigned char gender = 0;
		static const unsigned char age = 10;

		bool empty() const
		{
			return false;
		}

		template<class T>
			void apply(const Data::KeyCode key_code, Util::Index& cubes, const T& index) const
			{
#define CGEN_JOIN(pVal) \
				typename T::const_iterator i = index.find(pVal); \
				if (i != index.end() ) { \
					cubes.index_join(i->second); \
				}

				if (key_code == Data::I_GENDER) {
					CGEN_JOIN(gender)
				} else if (key_code == Data::I_AGE) {
					CGEN_JOIN(age)
				}
#undef CGEN_JOIN
			}
	};

	// class to make a report
	struct Worker1
	{
		uint32_t calls;

		Worker1()
		: calls(0)
		{
			;;
		}
		void operator()(const Data::Key& key, const Data::Val& val)
		{
			calls++;
		}
		void operator()(const Data::Key& key, const Util::Index& bs)
		{
			calls+=bs.size();
		}
	};

	template<>
	template<>
	void object::test<1>()
	{
		// create data

		std::string fn("__mdc_test1");
		Create<Data> mdc(fn);
		Create<Data>::RawDataT id;
		Filter::BitSet filter;

		Data::Key e;
		for (char i=10; i<20; i++) {
			e.age=i;
			for(char j=0; j<3; j++){
				e.gender=j;
				for(uint32_t k=100; k<200; k++ ) {
					e.city=k;
					id[e].push_back(k);
					id[e].push_back(k+1);
				}
			}
		} // 3K index elements
		mdc.insert(id, filter);
		mdc.close();

		// part 2, try to read
		UseMDC<Data> uc(fn);

		{
			Narrow1 narrow;
			Worker1 worker1;
			uc.access(narrow, worker1, filter);
			ensure("worker call count", 6000 == worker1.calls);
		}
		{
			Narrow2 narrow;
			Worker1 worker1;
			uc.access(narrow, worker1, filter);
			ensure("worker call count/narrow2", 200 == worker1.calls);
		}
	}

  //------------------

	struct Worker2
	{
		Util::Index uids;
		uint32_t calls;

		Worker2()
		: calls(0)
		{
			;;
		}
		void operator()(const Data::Key& key, const Data::Val& val)
		{
			calls++;
			uids.set(val);
		}
		void operator()(const Data::Key& key, const Util::Index& bs)
		{
			calls+=bs.size();
			uids.join(bs);
		}
	};

	template<>
	template<>
	void object::test<2>()
	{
		// create data

		std::string fn("__mdc_test2");
		Create<Data> mdc(fn);
		Create<Data>::RawDataT id;
		Filter::BitSet filter;

		Data::Key e;
		for (char i=10; i<20; i++) {
			e.age=1;
			e.gender=0;
			e.city=0;
			id[e].push_back(i);
		} // 10 elements
		mdc.insert(id, filter);
		id.clear();

		for (char i=20; i<40; i++) {
			e.age=2;
			e.gender=0;
			e.city=0;
			id[e].push_back(i-10);
		} // 20 elements
		mdc.insert(id, filter);

		mdc.close();

		// part 2, try to read
		UseMDC<Data> uc(fn);

		{
			Narrow1 narrow;
			Worker2 worker2;
			uc.access(narrow, worker2, filter);
			ensure("worker call count", 30 == worker2.calls);
			ensure("uids count", 20 == worker2.uids.size());
		}
	}

	struct Data3 {
		enum KeyCode {I_CITY};

		struct Key {
			uint32_t city;

			bool operator<(const Key& right) const {
				return city < right.city;
			}
		};

		struct Val {
			uint32_t uid;
			uint32_t group;
		};

		struct Index {
			std::map<uint32_t, Util::Index> city_index;

			void insert(const Key& key, uint32_t cube)
			{
				city_index[key.city].set(cube);
			}

			template<class T>
			void lookup(Util::Index& cubes, const T& narrow) {
				narrow.apply(I_CITY, cubes, city_index);
			}
			size_t mem_used() { return 0; } // just a stub
		};
	};

	struct Narrow3 {
		typedef std::set<uint32_t> LimCityT;
		LimCityT lim_city;
		bool empty() const
		{
			return lim_city.empty();
		}

		template<class T>
		void apply(const Data3::KeyCode key_code, Util::Index& cubes, const T& index) const
		{
			// we can ignore a key_code since index have only 1 dimenstion

			Util::Index tmp;
			for(LimCityT::const_iterator l = lim_city.begin();
				l != lim_city.end();
				++l)
			{
				typename T::const_iterator i = index.find(*l);
				if (i!=index.end())
				{
					tmp.join(i->second);
				}
			}
			cubes.index_join(tmp);
		}
	};

	struct Worker3
	{
		Util::Index uids;
		uint32_t calls;

		Worker3()
		: calls(0)
		{
			;;
		}
		void operator()(const Data3::Key& key, const Data3::Val& val)
		{
			calls++;
			uids.set(val.uid);
		}
		void operator()(const Data3::Key& key, const std::vector<Data3::Val>& val)
		{
			calls+=val.size();
			for (uint32_t i = 0; i<val.size(); ++i)
			{
				uids.set(val[i].uid);
			}
		}
	};

	template<>
	template<>
	void object::test<3>()
	{
		std::string fn("__mdc_test3");
		Create<Data3> mdc(fn);
		Create<Data3>::RawDataT id;
		Filter::LZO<Data3::Val> filter;

		for (uint32_t i=10; i<20; i++) {
			Data3::Key e;
			e.city=i;
			for (uint32_t j=9; j<i; j++){
				Data3::Val v;
				v.uid = j;
				v.group = 0;
				id[e].push_back(v);
			} // 1 to 11 value elements
		} // 10 index elements

		mdc.insert(id, filter);
		mdc.close();

		// read
		UseMDC<Data3> uc(fn);
		{
			Narrow3 narrow;
			narrow.lim_city.insert(10);
			Worker3 worker;
			uc.access(narrow, worker, filter);
			ensure("1:worker call count", 1 == worker.calls);
			ensure("1:uids count", 1 == worker.uids.size());
		}
		{
			Narrow3 narrow;
			narrow.lim_city.insert(10);
			narrow.lim_city.insert(15);
			Worker3 worker;
			uc.access(narrow, worker, filter);
			ensure("2:worker call count", 1+6 == worker.calls);
			ensure("2:uids count", 6 == worker.uids.size());
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


