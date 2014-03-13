/*
 * _COPYRIGHT_
 */

#ifndef _REPORT_EXCEPTION_HPP__
#define _REPORT_EXCEPTION_HPP__

#include <stdint.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <errno.h>

namespace Util {

	inline void fire_exception(const std::string& msg)
	{
		throw std::runtime_error(msg);
	}

	template<class T>
	inline void fire_exception(const std::string& msg, const T rc)
	{
		std::ostringstream s;
		s << msg << ", error code: " << rc;

		throw std::runtime_error(s.str());
	}

} // namespace Util

#endif /* _REPORT_EXCEPTION_HPP__ */

