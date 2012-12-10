
#ifndef _LOG4_HPP__
#define _LOG4_HPP__

#include <Types.h>
#include <sys/time.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace Util {
	struct ILogger
	{
		virtual ~ILogger () {}
		virtual void log(int level, const std::string& message) = 0;

		enum {
			ERROR,
			WARNING,
			INFO,
			DEBUG,
			TRACE
		};
	};

	namespace Logger {

		class Manager
		{
			private:
				static ILogger* logger_;
				static int log_level_;
			public:
				static void setup (ILogger* logger, int log_level = ILogger::DEBUG) {
					logger_ = logger;
					log_level_ = log_level;
				}

				static bool do_log(int level) {
					return logger_ != 0 && level <= log_level_;
				}

				static void log (int level, const std::string& message) {
					logger_->log (level, message);
				}
		};

		class DefaultLogger : public ILogger
		{
				static const std::string error;
				static const std::string warning;
				static const std::string info;
				static const std::string debug;
				static const std::string trace;
				static const std::string nx_level;

				const std::string& log_level(int level) {
					switch(level) {
						case ERROR: return error;
						case WARNING: return warning;
						case INFO: return info;
						case DEBUG: return debug;
						case TRACE: return trace;
						default: return nx_level;
					}
				}
			public:
				virtual void log(int level, const std::string& message)
				{
					struct timeval tv;
					gettimeofday(&tv, NULL);
					struct tm result;
					localtime_r(&tv.tv_sec, &result);

					std::stringstream m;
					m << result.tm_year+1900 << "-"
					  << std::setfill('0') << std::setw(2) << result.tm_mon  << "-"
					  << std::setw(2) << result.tm_mday << " "
					  << std::setw(2) << result.tm_hour << ":"
					  << std::setw(2) << result.tm_min  << ":"
					  << std::setw(2) << result.tm_sec  << "."
					  << std::setw(3) << int(tv.tv_usec/1000) << " "
					  << std::setw(10)<< pthread_self() << " "
					  << std::setfill(' ') << std::setw(7) << log_level(level) << ":"
					  << message
					  << std::endl;
					std::cout << m.str();
				}
		};

#define LOG4_GENERIC(level, message) \
	do { \
		if(Util::Logger::Manager::do_log(level)) { \
			std::stringstream m; \
			m << message; \
			Util::Logger::Manager::log(level, m.str()); \
		} \
	} while(0)

#define LOG4_TRACE(message)   LOG4_GENERIC(Util::ILogger::TRACE, message)
#define LOG4_DEBUG(message)   LOG4_GENERIC(Util::ILogger::DEBUG, message)
#define LOG4_INFO(message)    LOG4_GENERIC(Util::ILogger::INFO, message)
#define LOG4_WARNING(message) LOG4_GENERIC(Util::ILogger::WARNING, message)
#define LOG4_ERROR(message)   LOG4_GENERIC(Util::ILogger::ERROR, message)

	} // namespace Logger
} // namespace Util

#endif /* _LOG4_HPP__ */



