#ifndef __COMMON__HH__
#define __COMMON__HH__

#include <sys/time.h>
#include <queue>
using std::queue;
#include <mutex>
using std::mutex;
using std::unique_lock;

#include <cstdlib>

template<typename T>
class ConcurrentQueue {
private:
	mutex QueueMutex;
	queue<T> q;
public:
	size_t size();
	T& front();
	T& back();
	void pop();
	void push(const T&);
};

class Timer {
public:
	void startTimer();
	void endTimer();
	unsigned long long getTime();
private:
	timeval start_tv;
	timeval end_tv;
};

#endif
