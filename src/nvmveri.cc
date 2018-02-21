#include "nvmveri.hh"

void* NVMVeri::VeriWorker(void* a) {
	tid_t tid = *((int*) a);
	return NULL;
}

void* NVMVeri::VeriMaster(void* a) {
	return NULL;
}

bool NVMVeri::init() {
	// create master
	pthread_t MasterThread;
	pthread_create(&MasterThread, NULL, &VeriMaster, (void*)0);

	// create worker
	for (int i = 0; i < MAX_THREAD_POOL_SIZE) {
		pthread_t VeriThread;
		tid_t VeriWorkerID = i;
		VeriWorkerStateMap[VeriWorkerID] = IDLE;
		VeriWorkerLock[i] = PTHREAD_MUTEX_INITIALIZER;
		pthread_create(&VeriThread, NULL, &VeriWorker, VeriWorkerID);
	}
}

bool NVMVeri::readMetadata() {

}

bool NVMVeri::writeMetadata() {

}

bool NVMVeri::assignTask(tid_t VeriWorkerID) {

}
