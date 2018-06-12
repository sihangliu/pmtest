#include "common.hh"

void Timer::startTimer()
{
	gettimeofday(&start_tv, NULL);
}

void Timer::endTimer()
{
	gettimeofday(&end_tv, NULL);
}

unsigned long long Timer::getTime()
{
	return (end_tv.tv_sec - start_tv.tv_sec) * 1000000
				+ (end_tv.tv_usec - start_tv.tv_usec);
}

