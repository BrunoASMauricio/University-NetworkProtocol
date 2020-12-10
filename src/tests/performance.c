#include "performance.h"

void
performMeasurements()
{
	struct timespec res;
	struct timespec res2;

	long avg = 0;
	long total = 0;

	for(int i = 0; i < 100; i++)
	{
		clock_gettime(CLOCK_REALTIME, &res);
		clock_gettime(CLOCK_REALTIME, &res2);
		avg += ((res2.tv_sec - res.tv_sec) * (int64_t)1000000000UL) + (res2.tv_nsec - res.tv_nsec);
		total++;
		// sleep(1); <-- this changes the average, why
	}

	printf("clock_gettime(CLOCK_REAL_TIME) average sampling delay: %lld\n", avg/total);
}
