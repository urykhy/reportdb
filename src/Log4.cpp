
#include <Log4.hpp>

namespace Util {
	namespace Logger {
		ILogger* Manager::logger_ = 0;
		int Manager::log_level_ = 0;

		const std::string DefaultLogger::error="ERROR";
		const std::string DefaultLogger::warning="WARNING";
		const std::string DefaultLogger::info="INFO";
		const std::string DefaultLogger::debug="DEBUG";
		const std::string DefaultLogger::trace="TRACE";
		const std::string DefaultLogger::nx_level="UNKNOWN";

	}// namespace Logger
} // namespace Util



