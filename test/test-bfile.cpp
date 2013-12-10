
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

		size_t fs = 22346789;
		char* buf = (char*)malloc(fs);
		for (size_t i = 0 ; i < fs; i++) {
			*(buf+i) = (i%26) + 'A';
		}

		for (size_t i = 0; i < fs; ) {
			size_t len = 3000000 + (rand() % 1000);
			if (i+len > fs) {
				len = fs - i;
			}
			f.write(buf+i, len);
			i += len;
		}
		f.close();

		f.open("__testfile1.bin",O_RDONLY);
		f.read(buf, fs);
		for (size_t i = 0 ; i < fs; i++) {
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


