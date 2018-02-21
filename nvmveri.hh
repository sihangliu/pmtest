#ifndef NVMVERI_HH
#define NVMVERI_HH
// Libary for verification

#include <cstdlib>
#include <cstring>
#include <sys/shm.h>
#include <mqueue.h>
#include <pthread.h>
#include <queue>
using namespace std::queue;
#include <unordered_map>
using namespace std::unordered_map;
#include <string>
using namespace std::string;

#define MAX_THREAD_POOL_SIZE 1
typedef tid_t unsigned int;


struct State;
enum VeriWorkerState {IDLE, BUSY};
enum ResultType {PASS, FAIL};

struct NVMVeri {
	/* Initialize each worker */
	bool initVeri();
	/* Terminate each worker
	 * Check state has become IDLE
	 * Join all worker/master threads
	 */
	bool termVeri();
	/* Read data passed from the master thread */
	bool readMetadata();
	/* Write data passed from the master thread */
	bool writeMetadata();	

	/* Worker function. 
	 * If state is IDLE, busy waiting
	 * If state is BUSY, read metadata and verify. 
	 * When verification completes, send result to master
	 */
	static void* VeriMaster(void*);
	static void* VeriWorker(void*);
	static VeriWorkerState VeriWorkerStateMap[MAX_THREAD_POOL_SIZE];
	static pthread_mutex_t VeriWorkerLock[MAX_THREAD_POOL_SIZE];

	bool assignTask(tid_t);	
	
	// call init
	NVMVeri();
};

struct Metadata {
	State state;
	// ...
};

struct VeriResult {
	ResultType result;
}

struct State {
	enum StateVal {INIT, COMMIT};
	char StateChar[] = {"init", "commit"};
	void tranState();
};


#endif
