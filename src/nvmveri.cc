#include "nvmveri.hh"

//size_t NVMVeri::VeriNumber;
queue<vector<Metadata *> *> NVMVeri::VeriQueue[MAX_THREAD_POOL_SIZE];
mutex NVMVeri::VeriQueueMutex[MAX_THREAD_POOL_SIZE];
condition_variable NVMVeri::VeriQueueCV[MAX_THREAD_POOL_SIZE];

vector<VeriResult> NVMVeri::ResultVector[MAX_THREAD_POOL_SIZE];
mutex NVMVeri::ResultVectorMutex[MAX_THREAD_POOL_SIZE];

atomic<bool> NVMVeri::termSignal[MAX_THREAD_POOL_SIZE];
atomic<bool> NVMVeri::getResultSignal[MAX_THREAD_POOL_SIZE];

atomic<bool> NVMVeri::completedStateMap[MAX_THREAD_POOL_SIZE];
atomic<int> NVMVeri::completedThread;



NVMVeri::NVMVeri()
{
	initVeri();
}


NVMVeri::~NVMVeri()
{
	termVeri();
}


bool NVMVeri::initVeri()
{
	// create worker
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerThreadPool[i] = new thread(&VeriWorker, i);
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		termSignal[i] = false;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		getResultSignal[i] = false;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		completedStateMap[i] = false;
	}

	completedThread = 0;
	assignTo = 0;

	return true;
}


bool NVMVeri::termVeri()
{
	// printf("ask to stop\n");

	// for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
	// 	delete WorkerThreadPool[i];
	// }


	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		termSignal[i] = true;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> lock(VeriQueueMutex[i]);
		VeriQueueCV[i].notify_all();
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerThreadPool[i]->join();
	}
	// printf("stopped\n");
	return true;
}


// execute verification of input
bool NVMVeri::execVeri(vector<Metadata *> *input)
{

	unique_lock<mutex> lock(VeriQueueMutex[assignTo]);
	VeriQueue[assignTo].push(input);
	lock.unlock();
	VeriQueueCV[assignTo].notify_one();

	assignTo = (assignTo + 1) % MAX_THREAD_POOL_SIZE;

	return true;
}


// get the result of all previous inputs' verification
//
bool NVMVeri::getVeri(vector<VeriResult> &output)
{
	// printf("start getVeri\n");

	for (int i = 0 ; i < MAX_THREAD_POOL_SIZE; i++) {
		// unique_lock<mutex> lock(VeriQueueMutex[i]);
		getResultSignal[i] = true;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> lock(VeriQueueMutex[i]);
		VeriQueueCV[i].notify_all();
	}

	// Wait until all worker threads are complete
	while (completedThread != MAX_THREAD_POOL_SIZE) {
	};

	// Merge results
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> result_lock(ResultVectorMutex[i]);
		output.insert(output.end(), ResultVector[i].begin(), ResultVector[i].end());
	}

	// Reset worker state
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		getResultSignal[i] = false;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> lock(VeriQueueMutex[i]);
		VeriQueueCV[i].notify_all();
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		completedStateMap[i] = false;
	}

	completedThread = 0;

	// printf("end getVeri\n");
	return true;
}

/*
void NVMVeri::readMetadata()
{

}


void NVMVeri::writeMetadata()
{

}
*/

void NVMVeri::VeriWorker(int id)
{
	while (true) {
		//printf("startVeriWorkerloop %d, %d\n", id, bool(termSignal[id]));
		unique_lock<mutex> veri_lock(VeriQueueMutex[id]);

		// when no termSignal and no getResultSignal and VeriQueue is empty
		while (!termSignal[id] && !getResultSignal[id] && VeriQueue[id].size() == 0) {
		//	assert(completedStateMap[id] != true);
			VeriQueueCV[id].wait(veri_lock);
		}
		if (termSignal[id]) break;


		// when getResultSignal and this thread is not completed yet and VeriQueue is empty
		if (!completedStateMap[id] && getResultSignal[id] && VeriQueue[id].size() == 0) {
			completedThread++;
			completedStateMap[id] = true;
			continue;
		}

		if(VeriQueue[id].size() > 0) {
			VeriQueue[id].pop();
			veri_lock.unlock();
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			VeriResult temp;
			unique_lock<mutex> result_lock(ResultVectorMutex[id]);
			ResultVector[id].push_back(temp);
		}

		//printf("processed %d\n", id);
	}

	return;
}

/*
bool NVMVeri::assignTask(tid_t VeriWorkerID)
{
	return true;
}
*/

void *C_createVeriInstance()
{
	NVMVeri *result = new NVMVeri();
	return (void *)result;
}

void *C_deleteVeriInstance(void *veriInstance)
{
	NVMVeri *in = (NVMVeri *)veriInstance;
	delete in;
}

void C_execVeri(void *veriInstance, void *metadata_vector)
{
	NVMVeri *in = (NVMVeri *)veriInstance;
	in->execVeri((vector<Metadata *> *)metadata_vector);
}

void C_getVeri(void *veriInstance, void *veriResult)
{
	NVMVeri *in = (NVMVeri *)veriInstance;
	vector<VeriResult> r;
	in->getVeri(r);
	// TODO: cast veriResult
}


void *C_createMetadataVector()
{
	return (void *)(new vector<Metadata *>);
}


void C_deleteMetadataVector(void *victim)
{
	vector<Metadata *> *temp = (vector<Metadata *> *)victim;
	for (auto i = temp->begin(); i != temp->end(); i++) {
		delete (*i);
	}
}


void C_createMetadata(void *metadata_vector, char *name, void *address, int size)
{
	Metadata *m = new Metadata;

	strcpy(m->op.opName, name);
	m->op.address = (unsigned long long)address;
	m->op.size = size;

	((vector<Metadata *> *)metadata_vector)->push_back(m);
}
