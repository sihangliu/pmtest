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
    std::promise<void> master_termSignal;
    std::future<void> futureObj = master_termSignal.get_future();
    std::thread MasterThread(&VeriMaster, std::move(futureObj));

	// create worker
    std::promise<void> worker_termSignal[MAX_THREAD_POOL_SIZE];
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
        futureObj = worker_termSignal[i].get_future();
        VeriWorkerStateMap[VeriWorkerID] = IDLE;
        std::thread WorkerThread(&VeriWorker, std::move(futureObj));
	}

	return true;
}


bool NVMVeri::termVeri()
{
    printf("ask to stop\n");
    master_termSignal.set_value();
    MasterThread.join();
    printf("stopped\n");
	return true;
}


bool execVeri(vector<Metadata> *input)
{

    return true;
}


bool getVeri(vector<Metadata> *input, VeriResult *output)
{
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


static void NVMVeri::VeriMaster(std::future<void> termSignal)
{
    printf("a\n");
    while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        printf("b\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    printf("c\n");
	return NULL;
}


static void NVMVeri::VeriWorker(std::future<void> termSignal)
{
	return NULL;
}


bool NVMVeri::assignTask(tid_t VeriWorkerID)
{
	return true;
}
