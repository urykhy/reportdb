/*
 * g++ test-aio.cpp -I.
 * AIO simple test
 */

#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

#include <ArZip.hpp>
using namespace Util;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


namespace tut
{
	struct null_group {};
	typedef test_group<null_group> tg;
	typedef tg::object object;
	tg tg_1("zlib test");

	template<>
	template<>
	void object::test<1>()
	{
		//ensure("true", true);
		int fd = open("/bin/bash", O_RDONLY, 0644);
		if (fd == -1) {
			throw "file not found";
		}

		const size_t num = 100;
		std::vector <unsigned char> buf_orig(num);
		std::vector <unsigned char> buf_c(num);
		std::vector <unsigned char> buf_dc(num);
		ensure("read bash", (ssize_t)num == read(fd, &buf_orig[0], num));

		ArZip manager;
		size_t c_len = manager.compress(&buf_orig[0], num, &buf_c[0], num);
		std::cerr << "compressed to " << c_len << " bytes" << std::endl;
		ensure("decompress: size", num == manager.decompress(&buf_c[0], c_len, &buf_dc[0], num));
		ensure("decompress: check", buf_orig==buf_dc);

		close(fd);
	}
} // namespace tut
int main()
{
	tut::reporter reporter;
	tut::runner.get().set_callback(&reporter);
	tut::runner.get().run_tests();
	return !reporter.all_ok();
}


