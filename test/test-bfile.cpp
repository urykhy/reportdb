
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

#include <SyncWriteFile.hpp>

namespace tut
{
    test_runner_singleton runner;
	struct null_group {};
	typedef test_group<null_group> tg;
	typedef tg::object object;
	tg tg_1("bfile main test");

	template<>
	template<>
	void object::test<1>()
	{
		Util::SyncWriteFile f;
		f.open("__testfile1.bin", O_CREAT|O_WRONLY|O_APPEND);

		char* buf = (char*)malloc(12346789);
		for (size_t i = 0 ; i < 12346789; i++) {
			*(buf+i) = (i%26) + 'A';
		}
		f.write(buf,  6000000);
		f.write(buf+6000000, 6346789);
		f.close();

		f.open("__testfile1.bin",O_RDONLY);
		f.read(buf, 12346789);
		for (size_t i = 0 ; i < 12346789; i++) {
			ensure("data written ok", *(buf+i) == (char)((i%26) + 'A'));
			//if (*((unsigned char*)buf+i) != (i%26) + 'A') {
			//	std::cerr << "err pos " << i << std::endl;
			//}
		}

		free(buf);
	}

} // namespace tut

int main()
{
	tut::reporter reporter;
	tut::runner.get().set_callback(&reporter);
	tut::runner.get().run_tests();
	return !reporter.all_ok();
}


