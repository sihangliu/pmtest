#ifndef __NVMVERI_HH__
#define __NVMVERI_HH__
// Libary for verification

#include <stdio.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/shm.h>
#include <mqueue.h>
//#include <pthread.h>

#include <thread>
using std::thread;
#include <mutex>
using std::mutex;
using std::unique_lock;
#include <chrono>
#include <future>
using std::future;
#include <condition_variable>
using std::condition_variable;
#include <atomic>
using std::atomic;

#include <queue>
using std::queue;
#include <unordered_map>
using std::unordered_map;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <cstdlib>
#include <cstring>
#include <iostream>

#define MAX_THREAD_POOL_SIZE 5
typedef unsigned int tid_t;

enum VeriWorkerState {IDLE, BUSY};
enum ResultType {PASS, FAIL};

class State {
public:
	enum StateVal {INIT, COMMIT};
	const char StateChar[2][10] = {"init", "commit"};
	void tranState();
};

class Metadata {
public:
	State state;
	// ...
};

class VeriResult {
public:
	ResultType result;
};



class NVMVeri {
public:
	/* master and worker thread */
	thread *WorkerThreadPool[MAX_THREAD_POOL_SIZE];

	/* terminate signal */
	static atomic<bool> termSignal[MAX_THREAD_POOL_SIZE];

	/* get result signal */
	static atomic<bool> getResultSignal[MAX_THREAD_POOL_SIZE];
	static atomic<bool> completedStateMap[MAX_THREAD_POOL_SIZE];
	static atomic<int> completedThread;

	int assignTo;

	/* Main program works as the producer,
	* workers works as the consumers,
	* implement this like a semaphore
	*/
	// static size_t VeriNumber;
	static queue<vector<Metadata> *> VeriQueue[MAX_THREAD_POOL_SIZE];
	static mutex VeriQueueMutex[MAX_THREAD_POOL_SIZE];
	static condition_variable VeriQueueCV[MAX_THREAD_POOL_SIZE];


	/* Result queue
	*/
	static vector<VeriResult> ResultVector[MAX_THREAD_POOL_SIZE];
	static mutex ResultVectorMutex[MAX_THREAD_POOL_SIZE];
	//static condition_variable ResultVectorCV;


	/* Default constructor, call init/term */
	NVMVeri();
	~NVMVeri();

	/* Initialize each worker */
	bool initVeri();
	/* Terminate each worker
	* Check state has become IDLE
	* Join all worker/master threads
	*/
	bool termVeri();

	bool execVeri(vector<Metadata> *);

	bool getVeri(vector<VeriResult> &);

	/* Read data passed from the master thread */
	bool readMetadata();
	/* Write data passed from the master thread */
	bool writeMetadata();

	/* Worker function.
	* If state is IDLE, busy waiting
	* If state is BUSY, read metadata and verify.
	* When verification completes, send result to master
	*/
	static void VeriWorker(int id);

	bool assignTask(tid_t);
};

extern "C" void *C_createVeriInstance();
extern "C" void *C_deleteVeriInstance(void *);
extern "C" void C_execVeri(void *, void **);
extern "C" void C_getVeri(void *, void **);

#endif
