#ifndef NVMVERI_HH
#define NVMVERI_HH
// Libary for verification

#include <cstdlib>
#include <cstring>

#include <stdio.h>
#include <assert.h>

#include <thread>
#include <mutex>
#include <chrono>
#include <future>

#include <sys/mman.h>
#include <sys/shm.h>
#include <mqueue.h>
//#include <pthread.h>

#include <queue>
using std::queue;
#include <unordered_map>
using std::unordered_map;
#include <string>
using std::string;
#include <vector>
using std::vector;

#define MAX_THREAD_POOL_SIZE 1
typedef unsigned int tid_t;

enum VeriWorkerState {IDLE, BUSY};
enum ResultType {PASS, FAIL};


class State {
public:
	enum StateVal {INIT, COMMIT};
	const std::string StateChar[2] = {"init", "commit"};
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
	static VeriWorkerState VeriWorkerStateMap[MAX_THREAD_POOL_SIZE];
	static std::mutex VeriWorkerLock[MAX_THREAD_POOL_SIZE];

	// Default constructor, call init/term
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

    bool getVeri(vector<Metadata> *, VeriResult *);

	/* Read data passed from the master thread */
	bool readMetadata();
	/* Write data passed from the master thread */
	bool writeMetadata();

	/* Worker function.
	 * If state is IDLE, busy waiting
	 * If state is BUSY, read metadata and verify.
	 * When verification completes, send result to master
	 */
	//static void *VeriMaster(void *);
	//static void *VeriWorker(void *);
    static void VeriMaster(std::future<void>);
    static void VeriWorker(std::future<void>);

	bool assignTask(tid_t);
};
#endif
