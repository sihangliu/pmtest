#ifndef __LATENCY_H__
#define __LATENCY_H__

#include <sys/time.h>
#include <stdint.h>

#define FREQ 2.5f

// wait for nanoseconds
#define pmLatency(duration) \
	uint64_t start, end;   \
	uint32_t hi, lo; \
	asm volatile("rdtscp\n" \
	             "movl %%edx, %0\n" \
	             "movl %%eax, %1\n" \
	             "cpuid"            \
               : "=r" (hi), "=r" (lo) : : "%rax", "%rbx", "%rcx", "%rdx"); \
    start = (((uint64_t)hi) << 32) | (uint64_t)lo; \
	do { \
		asm volatile("rdtscp\n" \
					 "movl %%edx, %0\n" \
					 "movl %%eax, %1\n" \
					 "cpuid"           \
				   : "=r" (hi), "=r" (lo) : : "%rax", "%rbx", "%rcx", "%rdx"); \
		end = (((uint64_t)hi) << 32) | (uint64_t)lo; \
	} while ((end - start) / FREQ < duration); 

// extra latency for NVM
#define STT_WRITE_LATENCY 30
#define PCM_WRITE_LATENCY 120
#define PCM_READ_LATENCY 25

#endif
