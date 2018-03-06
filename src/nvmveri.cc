#include "nvmveri.hh"

size_t NVMVeri::VeriNumber;
queue<vector<Metadata> *> NVMVeri::VeriQueue;
mutex NVMVeri::VeriQueueMutex;
condition_variable NVMVeri::VeriQueueCV;

vector<VeriResult> NVMVeri::ResultVector;
mutex NVMVeri::ResultVectorMutex;

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
		futureObj[i] = worker_termSignal[i].get_future();
		VeriWorkerStateMap[i] = IDLE;
		WorkerThreadPool[i] = new thread(&VeriWorker, std::move(futureObj[i]), i);
	}
    
    VeriNumber = 0;
	return true;
}


bool NVMVeri::termVeri()
{
	printf("ask to stop\n");
	//master_termSignal.set_value();
	//MasterThreadPtr->join();

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		worker_termSignal[i].set_value();
	}
    //unique_lock<mutex> lock(VeriQueueMutex);	
    //VeriQueueCV.notify_all();

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		printf("fck\n");
        WorkerThreadPool[i]->join();
        printf("stop thread %d\n", i);
	}
	printf("stopped\n");
	return true;
}


// execute verification of input
bool NVMVeri::execVeri(vector<Metadata> *input)
{
    unique_lock<mutex> lock(VeriQueueMutex);	
	VeriNumber++;
    VeriQueue.push(input);
    printf("pushed\n");
    VeriQueueCV.notify_one();

    return true;
}


// get the result of all previous inputs' verification
//
bool NVMVeri::getVeri(vector<VeriResult> &output)
{
    printf("start getVeri\n");
	//MasterThreadPtr->join();
    while(true) {
        unique_lock<mutex> veri_lock(VeriQueueMutex);
        unique_lock<mutex> result_lock(ResultVectorMutex);
        //printf("@");
        if (VeriNumber == ResultVector.size()) {
            output = ResultVector;
            VeriNumber = 0;
            ResultVector.clear();
            break;
        }
    }
    printf("end getVeri\n");

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
	while (termSignal.wait_for(std::chrono::nanoseconds(1)) == std::future_status::timeout) {
	    printf("startVeriWorkerloop\n");
        fflush(stdout);
        unique_lock<mutex> veri_lock(VeriQueueMutex);
        while (VeriQueue.size() == 0) {
            printf("a\n");
	        if (termSignal.wait_for(std::chrono::nanoseconds(1)) != std::future_status::timeout) {
                printf("b\n");
                break;
            }
            printf("c\n");
            VeriQueueCV.wait(veri_lock);
            printf("d\n");
        }
        printf("semaphore > 0\n");
        if (VeriQueue.size() != 0) {
            printf("read\n");
            for(int i = 0; i < 4; i++) {
		        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                printf("zzz %d\n", id);
            }
            VeriQueue.pop();
            veri_lock.unlock();

            VeriResult temp;
            temp.teststr = "result_";
            unique_lock<mutex> result_lock(ResultVectorMutex);
            ResultVector.push_back(temp);
            result_lock.unlock();
        }
        printf("processed\n");
    }

	return;
}


bool NVMVeri::assignTask(tid_t VeriWorkerID)
{
	return true;
}
