#ifndef __NVMVERI_HH__
#define __NVMVERI_HH__


#ifndef DEBUG_FLAG
#define DEBUG_FLAG false
#endif // DEBUG_FLAG

/***********************************************
   library part that may be used by C program
***********************************************/
#include "stddef.h"
// typedef unsigned long long addr_t;

typedef enum MetadataType {_OPINFO, _ASSIGN, _FLUSH, _COMMIT, _BARRIER, _FENCE, _PERSIST, _ORDER, _TRANSACTIONDELIM, _ENDING, _TRANSACTIONBEGIN, _TRANSACTIONEND, _TRANSACTIONADD } MetadataType;

// the corresponding MetadataTypeStr is defined in nvmveri.cc

// typedef struct Metadata_OpInfo {
// 	enum State {NONE, WORK, COMMIT, ABORT, FINAL} state;
// 	char opName[MAX_OP_NAME_SIZE]; // function name
// 	addr_t address;		    	   // address of object being operated
// 	int size;					   // size of object
// } Metadata_OpInfo;
#define FILENAME_LEN 10

typedef struct Metadata_Assign {
	unsigned short size;
	unsigned short line_num;
	char file_name[FILENAME_LEN];
	void *addr;
} Metadata_Assign;

typedef struct Metadata_Flush {
	unsigned short size;
	void *addr;
} Metadata_Flush;

typedef struct Metadata_Persist {
	unsigned short size;
	unsigned short line_num;
	char file_name[FILENAME_LEN];
	void *addr;
}  Metadata_Persist;

typedef struct Metadata_Order {
	unsigned short early_size;
	unsigned short late_size;
	unsigned short line_num;
	char file_name[FILENAME_LEN];
	void *early_addr;
	void *late_addr;
} Metadata_Order;


typedef struct Metadata_TransactionAdd {
	unsigned short size;
	void *addr;
} Metadata_TransactionAdd;

typedef struct Metadata {
	MetadataType type;
	union {
		//Metadata_OpInfo op;
		Metadata_Assign assign;
		Metadata_Flush flush;
		Metadata_Persist persist;
		Metadata_Order order;
		Metadata_TransactionAdd transactionadd;
	};
} Metadata;

void Metadata_print(Metadata *);

# ifndef NVMVERI_KERNEL_CODE

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
#include <map>
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

#define MAX_THREAD_POOL_SIZE NUM_CORES
#define MAX_OP_NAME_SIZE 50
typedef unsigned int tid_t;

typedef struct {
	int cur_thread_id;
	mutex lock;
} ThreadInfo;

/*
typedef struct {
	//int tid;		  // system thread_id
	//int threadIndex; // 0, 1 , 2 ...#threads
	int metadataVectorCurIndex;
	bool valid;
	bool existVeriInstance;
	void* metadataPtr;
	void** metadataVectorArrayPtr;
} MetadataPtrInfo;

class metadataManager {
public:
	MetadataPtrInfo *metadataPtrInfoArray;
	int num_threads;
	int cur_num_threads;
	// Mutex for metadataPtrArray
	mutex metadataManagerLock;

	metadataManager(int);
	~metadataManager();

	// void addThreads(int);
	void registerThread();
	void setExistVeriInstance();
	void unsetExistVeriInstance();
	void incrMetadataVectorCurIndex();
	void resetMetadataVectorCurIndex();
	int getCurThreadIndex();
	MetadataPtrInfo* getMetadataPtrInfo();
};
*/

template <typename Type> struct inplace_assign: public identity_based_inplace_combine<Type>
{
	typedef inplace_assign<Type> type;

	void operator()(Type& object, const Type& operand) const
	{ object = operand; }
};

typedef interval_set<size_t> interval_set_addr;
typedef interval_map<size_t, int, partial_enricher, std::less, inplace_assign> interval_map_addr_timestamp;


enum VeriWorkerState {IDLE, BUSY};
enum ResultType {PASS, FAIL};

class VariableInfo {
public:
	void* addr;
	int size;
};

class VeriResult {
public:
	ResultType result;
};

class NVMVeriWorkerInfo {
public:
	//char padding1[64-sizeof(atomic<bool>)*3 - sizeof(thread *)];
	thread *WorkerThreadPool;
	bool termSignal;
	bool getResultSignal;
	bool completedStateMap;
	char padding2[64-sizeof(bool)*3 - sizeof(thread *)];
};

class NVMVeri {
public:
	/* master and worker thread */
	//thread *WorkerThreadPool[MAX_THREAD_POOL_SIZE];

	/* terminate signal */
	//atomic<bool> termSignal[MAX_THREAD_POOL_SIZE];

	/* get result signal */
	//atomic<bool> getResultSignal[MAX_THREAD_POOL_SIZE];
	//atomic<bool> completedStateMap[MAX_THREAD_POOL_SIZE];
	//atomic<int> completedThread;

	NVMVeriWorkerInfo WorkerInfo[MAX_THREAD_POOL_SIZE]; 

	int assignTo;

	/* Main program works as the producer,
	* workers works as the consumers,
	* implement this like a semaphore
	*/
	// static size_t VeriNumber;
	queue<FastVector<Metadata *> *> VeriQueue[MAX_THREAD_POOL_SIZE];
	mutex VeriQueueMutex[MAX_THREAD_POOL_SIZE];
	condition_variable VeriQueueCV[MAX_THREAD_POOL_SIZE];


	/* Result queue */
	FastVector<VeriResult> ResultVector[MAX_THREAD_POOL_SIZE];
	mutex ResultVectorMutex[MAX_THREAD_POOL_SIZE];
	//static condition_variable ResultVectorCV;

	/* Name to address map */
	unordered_map<string, VariableInfo> VariableNameAddressMap;

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

/* Thread control */
extern "C" void C_initThread();
extern "C" void C_getNewMetadataPtr();

/* Nvmveri */
extern "C" void *C_createVeriInstance();
extern "C" void C_deleteVeriInstance(void *);
extern "C" void C_execVeri(void *, void *);
extern "C" void C_getVeri(void *, void *);
extern "C" void C_getVeriDefault(void *);

extern "C" void *C_createMetadataVector();
extern "C" void C_deleteMetadataVector(void *);
extern "C" void C_createMetadata_OpInfo(void *, char *, void *, size_t);
extern "C" void C_createMetadata_Assign(void *, void *, size_t, const char[], unsigned short);
extern "C" void C_createMetadata_Flush(void *, void *, size_t);
extern "C" void C_createMetadata_Commit(void *);
extern "C" void C_createMetadata_Barrier(void *);
extern "C" void C_createMetadata_Fence(void *);
extern "C" void C_createMetadata_Persist(void *, void *, size_t, const char[], unsigned short);
extern "C" void C_createMetadata_Order(void *, void *, size_t, void *, size_t, const char[], unsigned short);
extern "C" void C_registerVariable(char*, void*, size_t);
extern "C" void C_unregisterVariable(char*);
extern "C" void* C_getVariable(char*, size_t*);
extern "C" void C_transactionBegin(void *);
extern "C" void C_transactionEnd(void *);

extern __thread void *metadataPtr;
extern __thread int existVeriInstance;
extern __thread FastVector<Metadata *> *transactionLog;

//extern void *metadataManagerPtr;
extern __thread int thread_id;
extern __thread int nvmveri_cur_idx;
extern __thread void **metadataVectorPtr;
extern __thread int nvmveri_cur_idx;
//extern ThreadInfo thread_info;

extern void* veriInstancePtr;

#endif // !NVMVERI_KERNEL_CODE
#endif // __NVMVERI_HH__
