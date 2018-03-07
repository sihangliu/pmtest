#include "nvmveri.hh"

size_t NVMVeri::VeriNumber;
queue<vector<Metadata> *> NVMVeri::VeriQueue;
mutex NVMVeri::VeriQueueMutex;
condition_variable NVMVeri::VeriQueueCV;

vector<VeriResult> NVMVeri::ResultVector;
mutex NVMVeri::ResultVectorMutex;

atomic<bool> NVMVeri::termSignal[MAX_THREAD_POOL_SIZE];

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

	VeriNumber = 0;
	return true;
}


bool NVMVeri::termVeri()
{
	// printf("ask to stop\n");
	//master_termSignal.set_value();
	//MasterThreadPtr->join();

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		termSignal[i] = true;
	}

	unique_lock<mutex> lock(VeriQueueMutex);
	VeriQueueCV.notify_all();
	lock.unlock();

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
	unique_lock<mutex> lock(VeriQueueMutex);
	VeriNumber++;
	VeriQueue.push(input);
	// printf("pushed\n");
	VeriQueueCV.notify_one();

	return true;
}


// get the result of all previous inputs' verification
//
bool NVMVeri::getVeri(vector<VeriResult> &output)
{
	// printf("start getVeri\n");
	//MasterThreadPtr->join();
	while(true) {
		unique_lock<mutex> veri_lock(VeriQueueMutex);
		unique_lock<mutex> result_lock(ResultVectorMutex);
		//// printf("@");
		if (VeriNumber == ResultVector.size()) {
			output = ResultVector;
			VeriNumber = 0;
			ResultVector.clear();
			break;
		}
	}
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
		// printf("startVeriWorkerloop %d, %d\n", id, bool(termSignal[id]));
		fflush(stdout);
		unique_lock<mutex> veri_lock(VeriQueueMutex);
		// printf("size = %lu\n", VeriQueue.size());

		// while (VeriQueue.size() == 0 && !termSignal[id]) {
		// 	VeriQueueCV.wait(veri_lock);
		// }
		while (!termSignal[id]) {
			if(VeriQueue.size() != 0) break;
			VeriQueueCV.wait(veri_lock);
		}

		if (termSignal[id]) break;

		// VeriQueueCV.wait(
		// 	veri_lock,
		// 	[id] {
		// 		if (termSignal[id])
		// 			return false;
		// 		return (VeriQueue.size() != 0);
		// 	}
		// );

		// printf("semaphore > 0\n");

		// printf("read\n");
		for(int i = 0; i < 4; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		VeriQueue.pop();
		veri_lock.unlock();
		VeriResult temp;
		unique_lock<mutex> result_lock(ResultVectorMutex);
		ResultVector.push_back(temp);
		result_lock.unlock();

		// printf("processed\n");
	}

	return;
}


bool NVMVeri::assignTask(tid_t VeriWorkerID)
{
	return true;
}
