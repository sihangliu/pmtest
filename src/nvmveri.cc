#include "nvmveri.hh"
#include <stdarg.h>

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

void *metadataPtr;
int existVeriInstance = 0;

void Metadata::print()
{
	printf("%s ", MetadataTypeStr[type]);
	if (type == _OPINFO) {
		printf("\n");
	}
	else if (type == _ASSIGN) {
		printf("%p %lu\n", assign.addr, assign.size);
	}
	else if (type == _PERSIST) {
	}
	else if (type == _FLUSH) {
		printf("%p %lu\n", flush.addr, flush.size);
	}
	else if (type == _FENCE) {
		printf("\n");
	}
	else {}
}


NVMVeri::NVMVeri()
{
	initVeri();
}


NVMVeri::~NVMVeri()
{
	termVeri();
}


const bool _debug = true;

void log(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (_debug == true)
		printf(format, args);
	va_end(args);
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

	existVeriInstance = 0;
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
			vector<Metadata *> *veriptr = VeriQueue[id].front();
			VeriQueue[id].pop();
			veri_lock.unlock();

			VeriProc(veriptr);

			VeriResult temp;
			unique_lock<mutex> result_lock(ResultVectorMutex[id]);
			ResultVector[id].push_back(temp);
		}

		//printf("processed %d\n", id);
	}

	return;
}


void NVMVeri::VeriProc(vector<Metadata *> *veriptr)
{
	// usually sizeof(size_t) = 8 on 64-bit system
	interval_set<size_t> PersistInfo;
	interval_map<size_t, int> OrderInfo;
	auto prev = veriptr->begin();
	auto cur = veriptr->begin();
	for (; cur != veriptr->end(); cur++) {
		if ((*cur)->type == _FENCE) {
			// process all Metadata in frame [prev, cur) if (*cur->type == _FENCE)	
			size_t startaddr, endaddr;
			for (auto i = prev; i != cur; i++) {
				if ((*i)->type == _ASSIGN) {
					startaddr = (size_t)((*i)->assign.addr);
					endaddr = startaddr + (*i)->assign.size;
					printf("%s %p %lu\n", MetadataTypeStr[_ASSIGN], (*i)->assign.addr, (*i)->assign.size);
					PersistInfo += interval<size_t>::right_open(startaddr, endaddr);

				}
				else if ((*i)->type == _FLUSH) {
					startaddr = (size_t)((*i)->flush.addr);
					endaddr = startaddr + (*i)->flush.size;
					printf("%s %p %lu\n", MetadataTypeStr[_FLUSH], (*i)->flush.addr, (*i)->flush.size);
					PersistInfo -= interval<size_t>::right_open(startaddr, endaddr);
				}
				else if ((*i)->type == _PERSIST) {
					startaddr = (size_t)((*i)->persist.addr);
					endaddr = startaddr + (*i)->persist.size;
					printf("%s %p %lu\n", MetadataTypeStr[_PERSIST], (*i)->persist.addr, (*i)->persist.size);
					for (size_t j = startaddr; j < endaddr; j += 4) {
						if (within(j, PersistInfo))
							printf("Addr %lu not persist.\n", j);
					}
				
				}
				else if ((*i)->type == _ORDER) {
				
				}
				else if ((*i)->type == _FENCE) {
				
				}
				else {
				
				}
			}
			prev = cur;
		}
	}
	std::cout << PersistInfo << std::endl;
	// processing tail value of [prev, cur):
	// prev point to last _FENCE, cur point to veriptr->end
	// will not execute Assign or Flush because of no Fence at the end
	for (auto i = prev; i != cur; i++) {
		
		if ((*i)->type == _ORDER) {
		
		}
		else if ((*i)->type == _FENCE) {
		
		}
		else {
		
		}
	}
}


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


void C_createMetadata_OpInfo(void *metadata_vector, char *name, void *address, size_t size)
{
	if (existVeriInstance) {
	Metadata *m = new Metadata;
	m->type = _OPINFO;

	strcpy(m->op.opName, name);
	m->op.address = (unsigned long long)address;
	m->op.size = size;
		
	//log("opinfo_aa\n");

	((vector<Metadata *> *)metadata_vector)->push_back(m);
	}
	else {
		//log("opinfo\n");
	}
}


void C_createMetadata_Assign(void *metadata_vector, void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _ASSIGN;
		
		//log("assign_aa\n");
		
		m->assign.addr = addr;
		m->assign.size = size;
		((vector<Metadata *> *)metadata_vector)->push_back(m);
	}
	else {
		//log("assign\n");
	}
}


void C_createMetadata_Flush(void *metadata_vector, void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _FLUSH;
		
		m->flush.addr = addr;
		m->flush.size = size;
		((vector<Metadata *> *)metadata_vector)->push_back(m);
	}
	else {
		//log("flush\n");
	}
}


void C_createMetadata_Commit(void *metadata_vector)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _COMMIT;

		((vector<Metadata *> *)metadata_vector)->push_back(m);
	}
	else {
		//log("commit\n");
	}
}


void C_createMetadata_Barrier(void *metadata_vector)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _BARRIER;

		((vector<Metadata *> *)metadata_vector)->push_back(m);
	}
	else {
		//log("barrier\n");
	}
}


void C_createMetadata_Fence(void *metadata_vector)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _FENCE;

		((vector<Metadata *> *)metadata_vector)->push_back(m);
	}
	else {
		//log("fence\n");
	}
}

void C_createMetadata_Persist(void *metadata_vector, void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _PERSIST;

		m->persist.addr = addr;
		m->persist.size = size;

		((vector<Metadata *> *)metadata_vector)->push_back(m);
	}
	else {
		//log("persist\n");
	}
}

void C_createMetadata_Order(void *metadata_vector, void *early_addr, size_t early_size, void *late_addr, size_t late_size)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _ORDER;

		((vector<Metadata *> *)metadata_vector)->push_back(m);
	}
	else {
		//log("order\n");
	}
}
