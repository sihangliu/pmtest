#include "nvmveri.hh"

//size_t NVMVeri::VeriNumber;
queue<vector<Metadata> *> NVMVeri::VeriQueue[MAX_THREAD_POOL_SIZE];
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
	//future<void> futureObj[MAX_THREAD_POOL_SIZE];
	// future<void> futureObj = master_termSignal.get_future();
	// MasterThreadPtr = new thread(&VeriMaster, std::move(futureObj));

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

	//VeriNumber = 0;


	return true;
}


bool NVMVeri::termVeri()
{
	 printf("ask to stop\n");
	//master_termSignal.set_value();
	//MasterThreadPtr->join();



	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		termSignal[i] = true;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> lock(VeriQueueMutex[i]);
		VeriQueueCV[i].notify_all();
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		// printf("fck%d\n", i);
		WorkerThreadPool[i]->join();
		// printf("stop thread %d\n", i);
	}
	// printf("stopped\n");
	return true;
}


// execute verification of input
bool NVMVeri::execVeri(vector<Metadata> *input)
{

	unique_lock<mutex> lock(VeriQueueMutex[assignTo]);
	//VeriNumber++;
	VeriQueue[assignTo].push(input);
	lock.unlock();
	// printf("Queue %d size = %d\n", assignTo, int(VeriQueue[assignTo].size()));
	VeriQueueCV[assignTo].notify_one();

	assignTo = (assignTo + 1) % MAX_THREAD_POOL_SIZE;

	return true;
}


// get the result of all previous inputs' verification
//
bool NVMVeri::getVeri(vector<VeriResult> &output)
{
	// printf("start getVeri\n");
	//MasterThreadPtr->join();

	for (int i = 0 ; i < MAX_THREAD_POOL_SIZE; i++) {
		// unique_lock<mutex> lock(VeriQueueMutex[i]);
		getResultSignal[i] = true;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> lock(VeriQueueMutex[i]);
		// printf("waking\n");
		VeriQueueCV[i].notify_all();
		// printf("waked\n");
	}

//	printf("suck1\n");
  // Wait until all worker threads are complete
	while (completedThread != MAX_THREAD_POOL_SIZE) {
	//	printf("suck %d\n", int(completedThread));
	};
	//printf("suck2\n");



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
		// printf("waking\n");
		VeriQueueCV[i].notify_all();
		// printf("waked\n");
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		completedStateMap[i] = false;
	}

	completedThread = 0;

	// printf("end getVeri\n");
	return true;
}


bool NVMVeri::readMetadata()
{
	return true;
}


bool NVMVeri::writeMetadata()
{
	return true;
}


void NVMVeri::VeriWorker(int id)
{
	while (true) {
		//printf("startVeriWorkerloop %d, %d\n", id, bool(termSignal[id]));
		unique_lock<mutex> veri_lock(VeriQueueMutex[id]);

		// printf("id = %d, %d, %d, %d\n", id, int(termSignal[id]), int(getResultSignal[id]), int(VeriQueue[id].size()));
		// when no termSignal and no getResultSignal and VeriQueue is empty
		while (!termSignal[id] && !getResultSignal[id] && VeriQueue[id].size() == 0) {
			printf("c %d\n", id);
		//	assert(completedStateMap[id] != true);
			VeriQueueCV[id].wait(veri_lock);
			printf("d %d\n", id);
		}
		//printf("id = %d, %d\n", id, int(termSignal[id]));
		//if (id > 0) printf("a %d\n", id);
		if (termSignal[id]) break;


		//fflush(stdout);
		// when getResultSignal and this thread is not completed yet and VeriQueue is empty
		if (!completedStateMap[id] && getResultSignal[id] && VeriQueue[id].size() == 0) {
			completedThread++;
			printf("complete %d\n", id);
			completedStateMap[id] = true;
			continue;
		}

		//if (id > 0) printf("%d size = %d\n", id, int(VeriQueue[id].size()));
		if(VeriQueue[id].size() > 0) {
			VeriQueue[id].pop();
			veri_lock.unlock();
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
			//for (int x = 0; x < 1e1; ++x) {}
			VeriResult temp;
			unique_lock<mutex> result_lock(ResultVectorMutex[id]);
			ResultVector[id].push_back(temp);
		}

		//printf("processed %d\n", id);
	}

	return;
}


bool NVMVeri::assignTask(tid_t VeriWorkerID)
{
	return true;
}
