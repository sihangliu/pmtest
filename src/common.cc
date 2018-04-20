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

template<class T>
FastVector<T>::FastVector()
{
	curr_size = 0;
	arr_vector = (T*) malloc (VECTOR_MAX_SIZE * sizeof(T));
}

template<class T>
FastVector<T>::~FastVector()
{
	free(arr_vector);
}

template<class T>
void FastVector<T>::push_back(T input)
{
	if (!curr_size < vector_max_size) {
		vector_max_size *= 10;
		arr_vector = (T*) realloc (arr_vector, vector_max_size * sizeof(T));
	}
	arr_vector[curr_size] = input;
	++curr_size;
}

template<class T>
void FastVector<T>::insert(int pos, int begin, int end)
{
	if (!(end - begin + curr_size < vector_max_size)) {
		vector_max_size = (curr_size + end - begin) * 2;
		arr_vector = (T*) realloc (arr_vector, vector_max_size * sizeof(T));
	}
	if (curr_size - pos > 0)
		memcpy(arr_vector + , arr_vector + pos, curr_size - pos, )
}


template<class T>
int FastVector<T>::size() 
{
	return curr_size;
}

