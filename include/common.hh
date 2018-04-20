#ifndef __COMMON__HH__
#define __COMMON__HH__

#include <sys/time.h>
#include <queue>
using std::queue;
#include <mutex>
using std::mutex;
using std::unique_lock;

#include <cstdlib>

class Timer {
public:
	void startTimer();
	void endTimer();
	unsigned long long getTime();
private:
	timeval start_tv;
	timeval end_tv;
};


template<class T>
class FastVector {
public:
	FastVector();
	~FastVector();
	void push_back(T);
	int size();
	void insert(int pos, int begin, int end);
	const T& operator[](int idx) const { return arr_vector[idx]; }
private:
	T* arr_vector;
	int curr_size;
	int vector_max_size = 200;
};


#endif
