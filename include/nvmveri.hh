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

#define MAX_THREAD_POOL_SIZE 1
#define MAX_OP_NAME_SIZE 50
typedef unsigned int tid_t;
typedef unsigned long long addr_t;

enum VeriWorkerState {IDLE, BUSY};
enum ResultType {PASS, FAIL};

//class State {
//public:
	//enum StateVal {NONE, WORK, COMMIT, ABORT, FINAL};
	//const char StateChar[2][10] = {"init", "commit"};
	//void tranState();
//};


enum MetadataType {_OPINFO, _ASSIGN, _PERSIST, _ORDER};

struct Metadata_OpInfo {
	enum State {NONE, WORK, COMMIT, ABORT, FINAL};
	char opName[MAX_OP_NAME_SIZE]; // function name
	addr_t address;		    	   // address of object being operated
	int size;					   // size of object
};

struct Metadata_Assign {
	void *lhs;
	unsigned int lhs_size;
	void *rhs;
	unsigned int rhs_size;
};

struct Metadata_Persist {

};

struct Metadata_Order {

};

class Metadata {
public:
	MetadataType type;
	union {
		Metadata_OpInfo op;
		Metadata_Assign assign;
		Metadata_Persist persist;
		Metadata_Order order;
	};
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
	static queue<vector<Metadata *> *> VeriQueue[MAX_THREAD_POOL_SIZE];
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

	bool execVeri(vector<Metadata *> *);

	bool getVeri(vector<VeriResult> &);

	/* Read data passed from the master thread */
	//void readMetadata();
	/* Write data passed from the master thread */
	//void writeMetadata();

	/* Worker function.
	* If state is IDLE, busy waiting
	* If state is BUSY, read metadata and verify.
	* When verification completes, send result to master
	*/
	static void VeriWorker(int id);

	//bool assignTask(tid_t);
};

extern "C" void *C_createVeriInstance();
extern "C" void *C_deleteVeriInstance(void *);
extern "C" void C_execVeri(void *, void *);
extern "C" void C_getVeri(void *, void *);

extern "C" void *C_createMetadataVector();
extern "C" void C_deleteMetadataVector(void *);
extern "C" void C_createMetadata_OpInfo(void *, char *, void *, size_t);
extern "C" void C_createMetadata_Assign(void *, void *, size_t);
extern "C" void C_createMetadata_Persist(void *);
extern "C" void C_createMetadata_Order(void *);

extern void *metadataPtr;
#endif
