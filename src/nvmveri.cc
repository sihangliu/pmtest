#include "nvmveri.hh"

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
	future<void> futureObj = master_termSignal.get_future();
    MasterThreadPtr = new thread(&VeriMaster, std::move(futureObj));

	// create worker
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
        futureObj = worker_termSignal[i].get_future();
		VeriWorkerStateMap[i] = IDLE;
        WorkerThreadPool[i] = new thread(&VeriWorker, std::move(futureObj), i);
	}

	return true;
}


bool NVMVeri::termVeri()
{
	std::this_thread::sleep_for(std::chrono::seconds(10));
    printf("ask to stop\n");
    master_termSignal.set_value();
    MasterThreadPtr->join();
    printf("stopped\n");

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		worker_termSignal[i].set_value();
		WorkerThreadPool[i]->join();
	}

	return true;
}


bool NVMVeri::execVeri(vector<Metadata> *input)
{
	
    return true;
}


bool NVMVeri::getVeri(vector<Metadata> *input, VeriResult *output)
{
	//MasterThreadPtr->join();

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


void NVMVeri::VeriMaster(future<void> termSignal)
{
    while (termSignal.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        printf("b\n");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    printf("c\n");
		return;
}


void NVMVeri::VeriWorker(future<void> termSignal, int id)
{
	while (termSignal.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        printf("d\n");
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    printf("e\n");

	return;
}


bool NVMVeri::assignTask(tid_t VeriWorkerID)
{
	return true;
}
