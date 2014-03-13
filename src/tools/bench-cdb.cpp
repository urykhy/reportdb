
#include <iostream>
#include <CreateCDB.hpp>
#include <UseCDB.hpp>
#include <Index.hpp>

#include <getopt.h>
#include <sys/time.h>

struct Root {
	// must a bit set (UseCDB::access)
	enum KeyCode { I_GENDER = 1 << 1 };

	struct Data {
		unsigned char  gender;
		uint32_t uid;

#define CGEN_LESS(a,b) \
		if (a < b ) return true; \
		if (a > b ) return false;

		bool operator<(const Data& right) const {
			CGEN_LESS(gender, right.gender)
				CGEN_LESS(uid, right.uid)
				return false;
		}
#undef CGEN_LESS
	};

	struct Accessor {
		CDB::Row<unsigned char> gender;
		CDB::Row<uint32_t> uid;

		void push_back(const Data& d) {
			gender.push_back(d.gender);
			uid.push_back(d.uid);
		}

		void open(const std::string& fn, CDB::Direction di, uint64_t columns)
		{
			// XXX: we ignore columsn here, this is a test
			gender.open(fn, di);
			uid.open(fn, di);
		}

		void reserve(size_t n)
		{
			gender.reserve(n);
			uid.reserve(n);
		}

		void write()
		{
			gender.write();
			uid.write();
		}

		size_t read(uint64_t columns)
		{
			gender.read();
			return uid.read();
		}

		void close()
		{
			gender.close();
			uid.close();
		}

		bool eof(uint64_t columns)
		{
			return uid.eof();
		}

		Accessor()
		: gender(".gender", 1),
		uid(".uid", 2)
		{
			;;
		}
	};

};

// conditional report
struct Narrow1 {

	uint64_t columns () const
	{
		return 0;
	}

	bool operator()(const Root::Accessor& ac, size_t rown) const {
		return true;
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
	void operator()(const Root::Accessor& ac, size_t idx)
	{
		calls++;
	}

	// ask no additional columns
	uint64_t columns()
	{
		return 0;
	}

};

std::string fn("__cdb_bench");

void generate(){
	// create data

	CDB::Create<Root> cdb(fn);
	CDB::Create<Root>::RawDataT id;

	// id should be sorted
	// w/o duplicate keys

	const uint32_t MAX_ROWS=10000000; // 10M

	std::cout << "Generating: please wait..." << std::endl;

	Root::Data v;

	for (v.gender = 0;
		 v.gender < 3;
		 ++v.gender)
	{
		for (uint32_t uid=0; uid<MAX_ROWS/3.0; uid++)
		{
			v.uid = uid;
			id.insert(v);
		}
	}

	cdb.insert(id);
	cdb.close();
}

void bench(){
	struct timeval start, stop;
	gettimeofday(&start, NULL);

	std::cout << "Benchmarking: please wait..." << std::endl;

	CDB::Use<Root> uc(fn);
	Narrow1 narrow;
	Worker1 worker1;
	uc.access(narrow, worker1);

	gettimeofday(&stop, NULL);
	double ela=( (double)stop.tv_sec + (double)stop.tv_usec/1000000.0f )-
		( (double)start.tv_sec + (double)start.tv_usec/1000000.0f );

	std::cout << "Results: " << worker1.calls << " in " << ela << "; "
		<< worker1.calls/ela << " calls per second" << std::endl;

}


int main(int argc, char** argv)
{

  int opt_generate = 0;
  int opt_bench = 0;

  Util::Logger::DefaultLogger defaultLogger;
  Util::Logger::Manager::setup(&defaultLogger);

  if (argc == 1) {
	  std::cout << "Usage: " << std::endl
		  << "       --generate     - generate test data" << std::endl
		  << "       --bench        - benchmark" <<std::endl
		  << std::endl;
	  return 0;
  }

  static struct option long_options[] = {
                     {"generate", 0, &opt_generate, 1},
                     {"bench", 0, &opt_bench, 1},
                     {0,0,0,0} };
  int rc = 0;
  while (rc != -1)
  {
	  rc = getopt_long(argc, argv, "", long_options, NULL);
	  switch (rc){
		  case '?':
			  exit(-1);
		  default:
			  break;
	  }
  }

  if (opt_generate) {
	  generate();
  }
  if (opt_bench) {
	  bench();
  }

  return 0;
}


