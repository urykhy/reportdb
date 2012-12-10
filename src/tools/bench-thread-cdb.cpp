
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

static const UINT_32 MAX_ROWS=10000000; // 10M

	// define a Table
    CDB_BEGIN_TABLE(Root)
		CDB_COLUMN(gender, UINT_8)
	    CDB_COLUMN(uid, UINT_32)
		CDB_COLUMN(slot, UINT_32)
	CDB_END_TABLE(Root)

#define MAX_GENDER 3
#define MAX_SLOTS  72

// class to make a report
struct Worker1
{
	UINT_32 calls;
	Util::Index uid[MAX_GENDER];

	Worker1()
	: calls(0)
	{
		;;
	}
	void operator()(const Root::Accessor& ac, size_t rown)
	{
		UINT_8 gender_ = CDB::TL::get<0>(ac.acc).get(rown);
		UINT_32 uid_ = CDB::TL::get<1>(ac.acc).get(rown);
		// count uid's per gender
		uid[ gender_ ].set( uid_ );
		calls++;
	}

	// ask no additional columns
	UINT_64 columns()
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
			UINT_8 gender = cur % MAX_GENDER; // only 0,1,2
			UINT_32 uid = cur % MAX_ROWS;
			UINT_32 slot = cur % MAX_SLOTS;

			CDB::TL::SetArg(v, gender, uid, slot);
			if (id.find(v) == id.end())
			{
				id.insert(v);
				++i;
			}
			++i_all;
		}
		LOG4_INFO("Generation done (" << i << "/" << i_all << ")" );

		/*
		for (UINT_8 gender = 0;
			 gender < MAX_GENDER;
			 ++gender)
		{
			for (UINT_32 uid = gender; uid < MAX_ROWS; uid += MAX_GENDER)
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
	UINT_32 calls;
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

void usage()
{
	std::cout << "Usage: " << std::endl
		<< "       --generate     - generate test data" << std::endl
		<< "       --bench        - benchmark" <<std::endl
		<< "       --count        - number of files to use (1)" <<std::endl
		<< "       --threads      - number of threads to run (auto)" <<std::endl
		<< std::endl;
}

int main(int argc, char** argv)
{
	int opt_generate = 0;
	int opt_bench = 0;
	int opt_count = 1;
	int opt_thr_count = -1;
	int opt_verbose = 0;

	if (argc == 1) {
		usage();
		return -1;
	}

	static struct option long_options[] = {
		{"generate",              0, &opt_generate, 1},
		{"bench",                 0, &opt_bench, 1},
		{"count",  required_argument, NULL, 3},
		{"threads",required_argument, NULL, 4},
		{"verbose",               0, &opt_verbose, 1},
		{0,0,0,0} };

	int rc = 0;
	while (rc != -1)
	{
		rc = getopt_long(argc, argv, "", long_options, NULL);
		switch (rc){
			case 3: opt_count=atol(optarg); break;
			case 4: opt_thr_count=atol(optarg); break;
			case '?':
					exit(-1);
			default:
					break;
		}
	}
	if (optind != argc) {
		std::cout << "Unrecognized option '" << argv[optind] << "'" << std::endl;
		usage();
		return -1;
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

		W_FLOAT ela = total_time.get();
		std::cout << "Results: " << bench.calls << " in " << ela << "; "
			<< bench.calls/ela << " calls per second" << std::endl;
		std::cout << "Work: " << bench.worktime
			<< ", join: " << bench.jointime
			<< std::endl;

	}

	return 0;
}


