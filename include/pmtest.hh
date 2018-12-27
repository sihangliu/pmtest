#ifndef __PMTEST_HH__
#define __PMTEST_HH__


/***********************************************
   library part that may be used by C program
***********************************************/
#ifndef DEBUG_FLAG
#define DEBUG_FLAG -1
#endif // DEBUG_FLAG >= 0

#if DEBUG_FLAG >= 0
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...) 
#endif // DEBUG_FLAG >= 0

#if DEBUG_FLAG >= 1
#define LOG_NOTE(...) printf(__VA_ARGS__)
#else
#define LOG_NOTE(...) 
#endif // DEBUG_FLAG >= 0

#include "stddef.h"
// typedef unsigned long long addr_t;

typedef enum MetadataType {
	_ASSIGN, _FLUSH, _COMMIT, _BARRIER, _FENCE,
	_PERSIST, _ORDER,
	_TRANSACTIONDELIM, _ENDING,
	_TRANSACTIONBEGIN, _TRANSACTIONEND, _TRANSACTIONADD,
	_EXCLUDE, _INCLUDE
} MetadataType;

// the corresponding MetadataTypeStr is defined in pmtest.cc


#define FILENAME_LEN 48


typedef struct Metadata {
	void *addr;
	void *addr_late;
	MetadataType type;
	unsigned int size;
	unsigned int size_late;
	unsigned int line_num;
	char file_name[FILENAME_LEN];
} Metadata;

/***********************************************
   part that can only be used by user space C++ program
***********************************************/
# ifndef PMTEST_KERNEL_CODE

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
#include "global.h"

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

class PMTestWorkerInfo {
public:
	//char padding1[64-sizeof(atomic<bool>)*3 - sizeof(thread *)];
	thread *WorkerThreadPool;
	atomic<bool> termSignal;
	atomic<bool> getResultSignal;
	atomic<bool> completedStateMap;
	char padding2[64-sizeof(bool)*3 - sizeof(thread *)];
};

class PMTest {
public:
	/* master and worker thread */
	//thread *WorkerThreadPool[MAX_THREAD_POOL_SIZE];

	/* terminate signal */
	//atomic<bool> termSignal[MAX_THREAD_POOL_SIZE];

	/* get result signal */
	//atomic<bool> getResultSignal[MAX_THREAD_POOL_SIZE];
	//atomic<bool> completedStateMap[MAX_THREAD_POOL_SIZE];
	//atomic<int> completedThread;

	PMTestWorkerInfo WorkerInfo[MAX_THREAD_POOL_SIZE]; 

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
	PMTest();
	~PMTest();

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

/* PMTest */
extern "C" void *C_createVeriInstance();
extern "C" void C_deleteVeriInstance(void *);
extern "C" void C_execVeri(void *, void *);
extern "C" void C_getVeri(void *, void *);
extern "C" void C_getVeriDefault(void *);

extern "C" void *C_createMetadataVector();
extern "C" void C_deleteMetadataVector(void *);
extern "C" void C_createMetadata_Assign(void *, void *, size_t, const char[], unsigned int);
extern "C" void C_createMetadata_Flush(void *, void *, size_t, const char[], unsigned int);
extern "C" void C_createMetadata_Commit(void *, const char[], unsigned int);
extern "C" void C_createMetadata_Barrier(void *, const char[], unsigned int);
extern "C" void C_createMetadata_Fence(void *, const char[], unsigned int);
extern "C" void C_createMetadata_Persist(void *, void *, size_t, const char[], unsigned int);
extern "C" void C_createMetadata_Order(void *, void *, size_t, void *, size_t, const char[], unsigned int);
extern "C" void C_createMetadata_TransactionBegin(void *, const char[], unsigned int);
extern "C" void C_createMetadata_TransactionEnd(void *, const char[], unsigned int);
extern "C" void C_createMetadata_TransactionAdd(void *, void *, size_t, const char[], unsigned int);
extern "C" void C_createMetadata_Exclude(void *, void *, size_t, const char[], unsigned int);
extern "C" void C_createMetadata_Include(void *, void *, size_t, const char[], unsigned int);
extern "C" void C_registerVariable(char *, void *, size_t);
extern "C" void C_unregisterVariable(char *);
extern "C" void* C_getVariable(char *, size_t *);
extern "C" void C_initVeri(void **, int);
extern "C" void C_sendTrace(void *);
extern "C" void C_exitVeri(void *);

#endif // !PMTEST_KERNEL_CODE
#endif // __PMTEST_HH__
