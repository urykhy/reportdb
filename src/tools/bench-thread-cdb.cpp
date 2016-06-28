
#include <iostream>
#include <CreateCDB.hpp>
#include <UseCDB.hpp>
#include <CDBtl.hpp>
#include <Index.hpp>

#include <TbbTrampoline.hpp>
#include <TimeMeter.hpp>

#include <getopt.h>
#include <sys/time.h>
#include <tr1/random>
#include <deque>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

static const uint32_t MAX_ROWS=10000000; // 10M

	// define a Table
	CDB_BEGIN_TABLE(Root)
		CDB_COLUMN(gender, unsigned char)
		CDB_COLUMN(uid, uint32_t)
		CDB_COLUMN(slot, uint32_t)
	CDB_END_TABLE(Root)

#define MAX_GENDER 3
#define MAX_SLOTS  72

std::string human(double d)
{
    if (d < 1024) return std::to_string(int(d));
    if (d < 1024 * 1024) return std::to_string((d/1024)) + "K";
    if (d < 1024 * 1024 * 1024) return std::to_string(int(d/(1024*1024))) + "M";
}

// class to make a report
struct Worker1
{
	uint32_t calls;
	Util::Index uid[MAX_GENDER];

	Worker1()
	: calls(0)
	{
		;;
	}
	void operator()(const Root::Accessor& ac, size_t rown)
	{
		unsigned char gender_ = std::get<0>(ac.acc).get(rown); //CDB::TL::get<0>(ac.acc).get(rown);
		uint32_t uid_ = std::get<1>(ac.acc).get(rown); //CDB::TL::get<1>(ac.acc).get(rown);
		// count uid's per gender
		uid[ gender_ ].set( uid_ );
		calls++;
	}

	// ask no additional columns
	uint64_t columns()
	{
		return ColumnName_uid | ColumnName_gender;
	}
};

std::string base_fn("__cdb_tbench");

typedef std::deque<std::string> FileListT;

class DJB2StringHash {
	public:
	template<class T>
	size_t operator()(const T& s) const throw()
	{
		// djb2 hash by dan bernstein
		unsigned long hash = 5381;
		int c;

		for (const char* str = s.data();
			 str < s.data() + s.length();
			 ++str )
		{
			c = static_cast<unsigned char>(*str);
			hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		}

		return hash;
	}
};

struct Generate {
	void operator()(const std::string& fname)
	{
		CDB::Create<Root> cdb(fname);
		CDB::Create<Root>::RawDataT id;

		// id should be sorted
		// w/o duplicate keys

		LOG4_INFO("Generating: " << fname << ", please wait...");

		DJB2StringHash hash;
		size_t seed = hash(fname);
		std::tr1::mt19937 seq(seed);

		Root::Data v;

		int i_all = 0;
		unsigned int i = 0;
		while( i < MAX_ROWS)
		{
			size_t cur = seq();
			unsigned char gender = cur % MAX_GENDER; // only 0,1,2
			uint32_t uid = cur % MAX_ROWS;
			uint32_t slot = cur % MAX_SLOTS;

			v = std::tie(gender, uid, slot);
			//CDB::TL::SetArg(v, gender, uid, slot);
			if (id.find(v) == id.end())
			{
				id.insert(v);
				++i;
			}
			++i_all;
		}
		LOG4_INFO("Generation done (" << i << "/" << i_all << ")" );

		/*
		for (unsigned char gender = 0;
			 gender < MAX_GENDER;
			 ++gender)
		{
			for (uint32_t uid = gender; uid < MAX_ROWS; uid += MAX_GENDER)
			{
				CDB::TL::SetArg(v, gender, uid);
				id.insert(v);
			}
		}*/
		cdb.insert(id);
		cdb.close();
		LOG4_INFO("Writing done");
	}
};

struct Bench {
	uint32_t calls;
	double worktime;
	double jointime;
	Util::Index uid[MAX_GENDER];
	Root::Narrow narrow; // shared access

	typedef Worker1 TmpResult;

	Bench() : calls(0), worktime(0), jointime(0)
	{
		;;
	}

	// work
	// run in parallel
	void work(const std::string& fname, TmpResult& worker)
	{
		Util::TimeMeter tm;

		CDB::Use<Root> uc(fname);
		uc.access(narrow, worker);

		worktime += tm.get();
	}

	// join
	// run in order
	void join(const std::string& fname, TmpResult& worker)
	{
		Util::TimeMeter tm;
		calls += worker.calls;
		uid[0].join(worker.uid[0]);
		uid[1].join(worker.uid[1]);
		uid[2].join(worker.uid[2]);
		jointime += tm.get();
	}
};

int main(int argc, char** argv)
{
	bool opt_generate = 0;
	bool opt_bench = 0;
	int opt_count = 1;
	int opt_thr_count = -1;
	bool opt_verbose = 0;

	po::options_description desc("Program options");
	desc.add_options()
		("help,h", "show usage information")
		("generate",po::bool_switch(&opt_generate), "generate test data")
		("bench", po::bool_switch(&opt_bench), "benchmark")
		("count", po::value<int>(&opt_count), "number of files to use (1)")
		("threads", po::value<int>(&opt_thr_count), "number of threads to run (auto)")
		("verbose", po::bool_switch(&opt_verbose), "verbose logging");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	if (argc == 1 || vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	Util::Logger::DefaultLogger defaultLogger;
	Util::Logger::Manager::setup(&defaultLogger, opt_verbose ? Util::ILogger::TRACE : Util::ILogger::DEBUG);

	std::cout << "processing with " << opt_count
		<< " files ("
		<< (opt_thr_count != -1 ? opt_thr_count : tbb::task_scheduler_init::default_num_threads())
		<< " threads)" << std::endl;

	tbb::task_scheduler_init init(opt_thr_count);

	FileListT fl;
	for (int i=0; i < opt_count; ++i)
	{
		std::stringstream fn;
		fn << base_fn << "-" << i;
		fl.push_back(fn.str());
	}

	if (opt_generate) {
		Generate gen;
		Util::parallel(fl, gen);
	}
	if (opt_bench) {
		Bench bench;
		Util::TimeMeter total_time;

		Util::pipeline(fl, bench);

		double ela = total_time.get();
		std::cout << "Results: " << bench.calls << " in " << ela << "; "
			<< human(bench.calls/ela) << " calls per second" << std::endl;
		std::cout << "Work: " << bench.worktime
			<< ", join: " << bench.jointime
			<< std::endl;

	}

	return 0;
}


