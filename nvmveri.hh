#ifndef NVMVERI_HH
#define NVMVERI_HH
// Libary for verification

#include <cstdlib>
#include <cstring>
#include <sys/shm.h>
#include <mqueue.h>
#include <pthread.h>


struct State;

struct NVMVeri {
	bool initVeri();
	bool readMetadata();
	bool writeMetadata();	
		
	NVMVeri();
};

struct Metadata {
	State state;
	// ...
};


struct State {
	enum StateVal {INIT, COMMIT};
	char StateChar[] = {"init", "commit"};
	void tranState();
};


#endif
