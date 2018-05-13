#ifndef __NVMVERI_HH__
#define __NVMVERI_HH__


// library part that may be used by C program
#include "stddef.h"
// typedef unsigned long long addr_t;

typedef enum MetadataType {_OPINFO, _ASSIGN, _FLUSH, _COMMIT, _BARRIER, _FENCE, _PERSIST, _ORDER, _TRANSACTIONDELIM, _ENDING} MetadataType;
const char MetadataTypeStr[][20] = {"_OPINFO", "_ASSIGN", "_FLUSH", "_COMMIT", "_BARRIER", "_FENCE", "_PERSIST", "_ORDER", "_TRANSACTIONDELIM", "_ENDING"};

// typedef struct Metadata_OpInfo {
// 	enum State {NONE, WORK, COMMIT, ABORT, FINAL} state;
// 	char opName[MAX_OP_NAME_SIZE]; // function name
// 	addr_t address;		    	   // address of object being operated
// 	int size;					   // size of object
// } Metadata_OpInfo;

typedef struct Metadata_Assign {
    void *addr;
    size_t size;
} Metadata_Assign;

typedef struct Metadata_Flush {
    void *addr;
    size_t size;
} Metadata_Flush;

typedef struct Metadata_Commit {

} Metadata_Commit;

typedef struct Metadata_Barrier {

} Metadata_Barrier;

typedef struct Metadata_Fence {

} Metadata_Fence;

typedef struct Metadata_Persist {
    void *addr;
    size_t size;
}  Metadata_Persist;

typedef struct Metadata_Order {
    size_t early_size;
    size_t late_size;
    void *early_addr;
    void *late_addr;
} Metadata_Order;

typedef struct Metadata_TransactionDelim {

} Metadata_TransactionDelim;

typedef struct Metadata_Ending {

} Metadata_Ending;

typedef struct Metadata {
	MetadataType type;
	union {
		//Metadata_OpInfo op;
		Metadata_Assign assign;
		Metadata_Flush flush;
		Metadata_Commit commit;
		Metadata_Barrier barrier;
		Metadata_Fence fence;
		Metadata_Persist persist;
		Metadata_Order order;
		Metadata_TransactionDelim delim;
		Metadata_Ending ending;
	};
} Metadata;

# ifndef NVMVERI_KERNEL_CODE

#define DEBUG_FLAG false
// C++ Libary for verification

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

#include <boost/icl/interval_set.hpp>
#include <boost/icl/interval_map.hpp>
using namespace boost::icl;

#include "common.hh"

#define MAX_THREAD_POOL_SIZE 1
#define MAX_OP_NAME_SIZE 50
typedef unsigned int tid_t;
typedef interval_set<size_t> interval_set_addr;
typedef interval_map<size_t, int, partial_enricher, std::less, inplace_max> interval_map_addr_timestamp;

enum VeriWorkerState {IDLE, BUSY};
enum ResultType {PASS, FAIL};

class VeriResult {
public:
	ResultType result;
};


class NVMVeri {
public:
	/* master and worker thread */
	thread *WorkerThreadPool[MAX_THREAD_POOL_SIZE];

	/* terminate signal */
	atomic<bool> termSignal[MAX_THREAD_POOL_SIZE];

	/* get result signal */
	atomic<bool> getResultSignal[MAX_THREAD_POOL_SIZE];
	atomic<bool> completedStateMap[MAX_THREAD_POOL_SIZE];
	atomic<int> completedThread;

	int assignTo;

	/* Main program works as the producer,
	* workers works as the consumers,
	* implement this like a semaphore
	*/
	// static size_t VeriNumber;
	queue<FastVector<Metadata *> *> VeriQueue[MAX_THREAD_POOL_SIZE];
	mutex VeriQueueMutex[MAX_THREAD_POOL_SIZE];
	condition_variable VeriQueueCV[MAX_THREAD_POOL_SIZE];


	/* Result queue
	*/
	FastVector<VeriResult> ResultVector[MAX_THREAD_POOL_SIZE];
	mutex ResultVectorMutex[MAX_THREAD_POOL_SIZE];
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

	bool execVeri(FastVector<Metadata *> *);

	bool getVeri(FastVector<VeriResult> &);

	/* Read data passed from the master thread */
	//void readMetadata();
	/* Write data passed from the master thread */
	//void writeMetadata();

	/* Worker function.
	* If state is IDLE, busy waiting
	* If state is BUSY, read metadata and verify.
	* When verification completes, send result to master
	*/
	void VeriWorker(int);

	void VeriProc(FastVector<Metadata *> *);

	//bool assignTask(tid_t);
};

extern "C" void *C_createVeriInstance();
extern "C" void C_deleteVeriInstance(void *);
extern "C" void C_execVeri(void *, void *);
extern "C" void C_getVeri(void *, void *);

extern "C" void *C_createMetadataVector();
extern "C" void C_deleteMetadataVector(void *);
extern "C" void C_createMetadata_OpInfo(void *, char *, void *, size_t);
extern "C" void C_createMetadata_Assign(void *, void *, size_t);
extern "C" void C_createMetadata_Flush(void *, void *, size_t);
extern "C" void C_createMetadata_Commit(void *);
extern "C" void C_createMetadata_Barrier(void *);
extern "C" void C_createMetadata_Fence(void *);
extern "C" void C_createMetadata_Persist(void *, void *, size_t);
extern "C" void C_createMetadata_Order(void *, void *, size_t, void *, size_t);

extern void *metadataPtr;
#endif // NVMVERI_C_CODE
#endif // __NVMVERI_HH__
