
#include <CreateMDC.hpp>
#include <UseMDC.hpp>
#include <MDCtl.hpp>
#include <MDCFilter.hpp>

#include <CreateCDB.hpp>
#include <UseCDB.hpp>
#include <CDBtl.hpp>

#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

using namespace MDC;


namespace tut
{
	test_runner_singleton runner;
	struct null_group {};
	typedef test_group<null_group> tg;
	typedef tg::object object;
	tg tg_1("TL interface test");

	//
	// MDC/TL interface test
	//
	typedef std::tuple<uint32_t, uint32_t, uint32_t> mdc_Key;
	typedef uint32_t mdc_Val;
	typedef Client<mdc_Key, mdc_Val> Data;

	struct Worker1
	{
		uint32_t calls;

		Worker1()
		: calls(0) { }
		void operator()(const mdc_Key& key, const mdc_Val& val)
		{
			calls++;

			// sample for `easy` access to mdc_key value
			//std::cout << "first element value is "
			//  << TL::get<0>(key)
			//  << std::endl;
		}
		void operator()(const mdc_Key& key, const Util::Index& bs)
		{
			calls+=bs.size();
		}
	};

	template<>
	template<>
	void object::test<1>()
	{
		std::string fn("__mdc_tl_test1");
		Create<Data> mdc(fn);
		Create<Data>::RawDataT id;
		Filter::BitSet filter;

		// fill with data
		Data::Key e;
		for (char i=10; i<20; i++) {
			e = TL::ctie(i, 1, 1);
			for(uint32_t k=0; k<5; k++ ) {
				id[e].push_back(k*100+1);     // value
			}
		}
		mdc.insert(id, filter);
		mdc.close();

		// process data
		UseMDC<Data> uc(fn);

		{
			Data::Narrow narrow;
			Worker1 worker1;
			uc.access(narrow, worker1, filter);
			ensure("worker call count", 10*5 == worker1.calls);
		}
		{
			Data::Narrow narrow;
			std::get<0>(narrow.limit).insert(11);
			std::get<0>(narrow.limit).insert(12);
			Worker1 worker1;
			uc.access(narrow, worker1, filter);
			ensure("worker call count/2", 10 == worker1.calls);
			ensure("index mem_used", uc.mem_used() > 0);
			ensure("narrow is_empty()", narrow.empty() == false );
		}

		// test empty index bug.
		{
			Data::Narrow narrow;
			std::get<0>(narrow.limit).insert(11);
			std::get<1>(narrow.limit).insert(2);	// index should have 0 elements now
			std::get<2>(narrow.limit).insert(1); // index should still have 0 elements
			Worker1 worker1;
			uc.access(narrow, worker1, filter);
			ensure("worker call count/3", 0 == worker1.calls);
		}

	}

	//
	// CDB/TL interface test
	//
	CDB_BEGIN_TABLE(Root)
		CDB_COLUMN(age, unsigned char)
		CDB_COLUMN(city, uint32_t)
		CDB_COLUMN(uid, uint32_t)
	CDB_END_TABLE(Root)

	struct Worker2
	{
		uint32_t calls;

		Worker2()
		: calls(0)
		{
			;;
		}
		void operator()(const Root::Accessor& ac, size_t idx)
		{
			//std::cout << "EEL " <<  TL::get<2>(ac.acc).get(idx) << std::endl;
			calls++;
		}

		uint64_t columns()
		{
			return ColumnName_uid;
		}

	};

	template<>
	template<>
	void object::test<2>()
	{
		std::string fn("__cdb_tl_test1");
		CDB::Create<Root> cdb(fn);
		CDB::Create<Root>::RawDataT id;

		Root::Data d;
		for(unsigned char age = 10; age < 20; age++)
			for (uint32_t city = 100; city < 110; city++)
			{
				uint32_t lead_uid = city*100 + age * 10;
				for (uint32_t uid = 0; uid < 5; uid++)
				{
					d = TL::ctie(age, city, uid + lead_uid);
					id.insert(d);
				}
			}

		cdb.insert(id);
		cdb.close();
		{	// basic access
			CDB::Use<Root> uc(fn);
			Root::Narrow narrow;
			Worker2 worker;
			uc.access(narrow, worker);
			ensure("worker call count", 500 == worker.calls);
		}
		{	// narrow
			//std::cout << "test2" << std::endl;
			ensure("ColumnName_age is 1", ColumnName_age == 1);
			ensure("ColumnName_city is 2", ColumnName_city == 2);
			ensure("ColumnName_uid is 4", ColumnName_uid == 4);
			CDB::Use<Root> uc(fn);
			Root::Narrow narrow;
			std::get<0>(narrow.index).insert(10); // age 10
			std::get<1>(narrow.index).insert(109) ; // city 109
			// UID not limited
			Worker2 worker;
			uc.access(narrow, worker);
			ensure("worker call count/with narrow", 5 == worker.calls);
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


