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


template <class T>
class FastVector {
public:
	FastVector();
	~FastVector();
	void push_back(T);
	int size();
	void insert(int pos, FastVector<T> &inputFV, int begin, int end);
	const T& operator[](int idx) const { return arr_vector[idx]; }
private:
	T* arr_vector;
	int cur_size;
	int vector_max_size;
};


template <class T>
FastVector<T>::FastVector()
{
	cur_size = 0;
	vector_max_size = 200;
	arr_vector = (T*) malloc (vector_max_size * sizeof(T));
}

template <class T>
FastVector<T>::~FastVector()
{
	free(arr_vector);
}

template <class T>
void FastVector<T>::push_back(T input)
{
	if (cur_size >= vector_max_size) {
		vector_max_size *= 10;
		arr_vector = (T*) realloc(arr_vector, vector_max_size * sizeof(T));
	}
	arr_vector[cur_size] = input;
	++cur_size;
}

template <class T>
void FastVector<T>::insert(int pos, FastVector<T> &inputFV, int begin, int end)
{
	//TODO: memcpy the covered part to the end
	if (end - begin + 1 + cur_size >= vector_max_size) {
		vector_max_size = (cur_size + end - begin + 1) * 2;
		arr_vector = (T*) realloc(arr_vector, vector_max_size * sizeof(T));
	}
	memcpy(arr_vector + pos, inputFV.arr_vector + begin, end - begin + 1);
}

template <class T>
int FastVector<T>::size() 
{
	return cur_size;
}


#endif
