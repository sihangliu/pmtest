#ifndef __COMMON__HH__
#define __COMMON__HH__

#include <sys/time.h>
#include <queue>
using std::queue;
#include <mutex>
using std::mutex;
using std::unique_lock;
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
//#define gettid() syscall(SYS_gettid)
/*
#define gettid(pid) ( \
    asm volatile( \
    "mov $20, %%eax\n\t" \
    "int $0x80\n\t" \
    "mov %%eax, %0\n\t" \
    :"=m"(pid) \
    ); \
)
*/

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
	//void insert(int pos, FastVector<T> &inputFV, int begin, int end);
	void append(FastVector<T> &);
	void append(T *, size_t);
	int size();
	void clear();
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
	arr_vector = (T *) malloc(vector_max_size * sizeof(T));
	//printf("%lu\n", vector_max_size * sizeof(T));
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
		arr_vector = (T *) realloc(arr_vector, vector_max_size * sizeof(T));
		//printf("@@@@ push_back reallocate\n");
	}
	arr_vector[cur_size] = input;
	++cur_size;
}

template <class T>
void FastVector<T>::append(FastVector<T> &input)
{
	if (input.size() + cur_size >= vector_max_size) {
		vector_max_size = (cur_size + input.size()) * 10;
		arr_vector = (T *) realloc(arr_vector, vector_max_size * sizeof(T));
		//printf("@@@@ append reallocate\n");
	}

	memcpy(arr_vector + cur_size, input.arr_vector, input.size() * sizeof(T));
	cur_size += input.size();
}


template <class T>
void FastVector<T>::append(T *input, size_t input_size)
{
	if (input_size + cur_size >= vector_max_size) {
		vector_max_size = (cur_size + input_size) * 10;
		arr_vector = (T *) realloc(arr_vector, vector_max_size * sizeof(T));
		//printf("@@@@ append reallocate\n");
	}

	memcpy(arr_vector + cur_size, input, input_size * sizeof(T));

	cur_size += input_size;
}

/*
template <class T>
void FastVector<T>::insert(int pos, FastVector<T> &inputFV, int begin, int end)
{
	//TODO: memcpy the covered part to the end
	if (end - begin + 1 + cur_size >= vector_max_size) {
		vector_max_size = (cur_size + end - begin + 1) * 10;
		arr_vector = (T*) realloc(arr_vector, vector_max_size * sizeof(T));
	}
	memcpy(arr_vector + cur_size, arr_vector + pos + 1, end - begin + 1);
	memcpy(arr_vector + pos, inputFV.arr_vector + begin, end - begin + 1);
}
*/

template <class T>
int FastVector<T>::size()
{
	return cur_size;
}


template <class T>
void FastVector<T>::clear()
{
	cur_size = 0;
	vector_max_size = 200;
	arr_vector = (T *) realloc(arr_vector, vector_max_size * sizeof(T));
}

/*
inline int getThreadID()
{
	char name[16];
	pthread_getname_np(pthread_self(), name, 16);
	return (name[0] - 'A');
}
*/

#endif // __COMMON__HH__
