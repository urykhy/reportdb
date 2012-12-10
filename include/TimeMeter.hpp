/*
 * _COPYRIGHT_
 */

#ifndef _TIME_METER_HPP__
#define _TIME_METER_HPP__

#include <sys/time.h>
#include <time.h>

namespace Util {

class TimeMeter {
	private:
		struct timeval start;
	public:

		TimeMeter() {
			gettimeofday(&start,NULL);
		}

		double get() {
			struct timeval stop;
			gettimeofday(&stop,NULL);
			double ela=( (double)stop.tv_sec+(  double)stop.tv_usec/1000000.0f )-( (double)start.tv_sec+(double)start.tv_usec/1000000.0f );
			return ela;
		}

		~TimeMeter() throw ()
		{
			;;
		}
};

} // namespace Util

#endif /* _TIME_METER_HPP__ */

