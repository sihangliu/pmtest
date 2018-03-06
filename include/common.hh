#ifndef __COMMON__HH__
#define __COMMON__HH__

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
	inline size_t size();
	inline T& front();
	inline T& back();
	inline void pop();
	inline void push(const T&);
};


#endif
