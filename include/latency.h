#include <sys/time.h>
#include <stdint.h>

#define FREQ 2.5f

static inline uint64_t rdtsc() {
	uint32_t hi, lo;
	asm volatile("rdtscp\n"
	             "movl %%edx, %0\n"
	             "movl %%eax, %1\n"
	             "cpuid"
               : "=r" (hi), "=r" (lo) : : "%rax", "%rbx", "%rcx", "%rdx");
    return (((uint64_t)hi) << 32) | (uint64_t)lo;
}


// wait for nanoseconds
void inline waitFor(unsigned int duration) 
{
	uint64_t start, end;
	start = rdtsc();
	do {
		end = rdtsc();
	} while ((end - start) / FREQ < duration);
}

// extra latency for NVM
#define STT_WRITE_LATENCY waitFor(30)
#define PCM_WRITE_LATENCY waitFor(120)
#define PCM_READ_LATENCY waitFor(25)
