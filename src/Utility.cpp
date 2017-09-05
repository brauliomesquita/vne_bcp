#include "Utility.h"

double get_time() {
	/*struct tms time_;
	 clock_t time_tic;
	 double cpt = sysconf(_SC_CLK_TCK);
	 times(&time_);
	 time_tic = time_.tms_utime;
	 return (double) (time_tic / cpt);*/
	struct tms time_;
	clock_t time_tic;
	double cpt = sysconf(_SC_CLK_TCK);
	time_tic = times(&time_);
	return (double) (time_tic / cpt);
}