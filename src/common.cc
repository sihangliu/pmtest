#include "common.hh"

template<typename T>
inline size_t ConcurrentQueue<T>::size()
{
	unique_lock<mutex> lock(QueueMutex);
	return q.size();
}


template<typename T>
inline T& ConcurrentQueue<T>::front()
{
	unique_lock<mutex> lock(QueueMutex);
	return q.front();
}


template<typename T>
inline T& ConcurrentQueue<T>::back()
{
	unique_lock<mutex> lock(QueueMutex);
	return q.back();
}


template<typename T>
inline void ConcurrentQueue<T>::pop()
{
	unique_lock<mutex> lock(QueueMutex);
	q.pop();
}


template<typename T>
inline void ConcurrentQueue<T>::push(const T& value)
{
	unique_lock<mutex> lock(QueueMutex);
	q.push(value);
}
