#include "nvmveri.hh"
#include <stdarg.h>

#define COLOR_RED "\x1B[31m"
#define COLOR_RESET "\x1B[0m"

//size_t NVMVeri::VeriNumber;
/*
queue<vector<Metadata *> *> NVMVeri::VeriQueue[MAX_THREAD_POOL_SIZE];
mutex NVMVeri::VeriQueueMutex[MAX_THREAD_POOL_SIZE];
condition_variable NVMVeri::VeriQueueCV[MAX_THREAD_POOL_SIZE];

vector<VeriResult> NVMVeri::ResultVector[MAX_THREAD_POOL_SIZE];
mutex NVMVeri::ResultVectorMutex[MAX_THREAD_POOL_SIZE];

atomic<bool> NVMVeri::termSignal[MAX_THREAD_POOL_SIZE];
atomic<bool> NVMVeri::getResultSignal[MAX_THREAD_POOL_SIZE];

atomic<bool> NVMVeri::completedStateMap[MAX_THREAD_POOL_SIZE];
atomic<int> NVMVeri::completedThread;
*/

#ifndef NVMVERI_KERNEL_CODE
const char MetadataTypeStr[20][30] = {"_OPINFO", "_ASSIGN", "_FLUSH", "_COMMIT", "_BARRIER", "_FENCE", "_PERSIST", "_ORDER", "_TRANSACTIONDELIM", "_ENDING", "_TRANSACTIONBEGIN", "_TRANSACTIONEND"};

__thread void *metadataPtr;
//void *metadataManagerPtr;
__thread int thread_id;
__thread int existVeriInstance = 0;
__thread FastVector<Metadata *> *transactionLog = NULL;
__thread int nvmveri_cur_idx;
__thread void **metadataVectorPtr;
void* veriInstancePtr;

ThreadInfo thread_info;

void Metadata_print(Metadata *m)
{
	printf("%s ", MetadataTypeStr[m->type]);
	if (m->type == _ASSIGN) {
		printf("%p %u", m->assign.addr, m->assign.size);
	}
	else if (m->type == _FLUSH) {
		printf("%p %u", m->flush.addr, m->flush.size);
	}
	else if (m->type == _PERSIST) {
		printf("%p %u", m->persist.addr, m->persist.size);
	}
	else if (m->type == _ORDER) {
		printf("%p %u %p %u", m->order.early_addr, m->order.early_size, m->order.late_addr, m->order.late_size);
	}
	else {}
	printf("\n");
}


NVMVeri::NVMVeri()
{
	initVeri();
}


NVMVeri::~NVMVeri()
{
	termVeri();
}


const bool _debug = DEBUG_FLAG;

void log(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	if (_debug == true)
		vprintf(format, args);
	va_end(args);
}


bool NVMVeri::initVeri()
{
	// create worker
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].WorkerThreadPool = new thread(&NVMVeri::VeriWorker, this, i);
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].termSignal = false;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].getResultSignal = false;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].completedStateMap = false;
	}

	//completedThread = 0;
	assignTo = 0;

	existVeriInstance = 0;
	transactionLog = NULL;
	return true;
}


bool NVMVeri::termVeri()
{
	// printf("ask to stop\n");

	// for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
	// 	delete WorkerThreadPool[i];
	// }


	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].termSignal = true;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> lock(VeriQueueMutex[i]);
		VeriQueueCV[i].notify_all();
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].WorkerThreadPool->join();
	}

	// printf("stopped\n");
	return true;
}


// execute verification of input
bool NVMVeri::execVeri(FastVector<Metadata *> *input)
{
	//printf("@execVeri\n");
	unique_lock<mutex> lock(VeriQueueMutex[assignTo]);
	VeriQueue[assignTo].push(input);
	lock.unlock();
	VeriQueueCV[assignTo].notify_one();

	assignTo = (assignTo + 1) % MAX_THREAD_POOL_SIZE;

	return true;
}


// get the result of all previous inputs' verification
//
bool NVMVeri::getVeri(FastVector<VeriResult> &output)
{
	// printf("start getVeri\n");

	for (int i = 0 ; i < MAX_THREAD_POOL_SIZE; i++) {
		// unique_lock<mutex> lock(VeriQueueMutex[i]);
		WorkerInfo[i].getResultSignal = true;
	}


	// Wait until all worker threads are complete
	/*
	while (completedThread != MAX_THREAD_POOL_SIZE) {
		for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
			unique_lock<mutex> lock(VeriQueueMutex[i]);
			VeriQueueCV[i].notify_all();
		}
	};
	*/

	while (true) {
		int tmp = 0;
		for (int i = 0; i < MAX_THREAD_POOL_SIZE; ++i) {
			tmp += WorkerInfo[i].completedStateMap;
		}

		if (tmp == MAX_THREAD_POOL_SIZE) break;
		for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
			unique_lock<mutex> lock(VeriQueueMutex[i]);
			VeriQueueCV[i].notify_all();
		}
	};


	// Merge results
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> result_lock(ResultVectorMutex[i]);
		//output.insert(output.size(), ResultVector[i], 0, ResultVector[i].size() - 1);
		output.append(ResultVector[i]);
	}

	// Reset worker state
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].getResultSignal = false;
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		unique_lock<mutex> lock(VeriQueueMutex[i]);
		VeriQueueCV[i].notify_all();
	}

	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].completedStateMap = false;
	}

	//completedThread = 0;

	// printf("end getVeri\n");
	return true;
}


void NVMVeri::VeriWorker(int id)
{
	while (WorkerInfo[id].termSignal == true);

	while (true) {
		//printf("startVeriWorkerloop %d, %d\n", id, bool(termSignal[id]));
//		if (id == 0)
//printf("a%d, %d, %d\n", int(completedThread), int(completedStateMap[id]), int(id));
		unique_lock<mutex> veri_lock(VeriQueueMutex[id]);
		// when no termSignal and no getResultSignal and VeriQueue is empty
//		if (id == 0)
//printf("b%d, %d, %d\n", int(completedThread), int(completedStateMap[id]), int(id));
		while (!WorkerInfo[id].termSignal && !WorkerInfo[id].getResultSignal && VeriQueue[id].size() == 0) {
		//	assert(completedStateMap[id] != true);
			VeriQueueCV[id].wait(veri_lock);
		}
//		if (id == 0)
//printf("c%d, %d, %d\n", int(completedThread), int(completedStateMap[id]), int(id));
		if (WorkerInfo[id].termSignal) break;


		// when getResultSignal and this thread is not completed yet and VeriQueue is empty
		if (!WorkerInfo[id].completedStateMap && WorkerInfo[id].getResultSignal && VeriQueue[id].size() == 0) {
			//completedThread++;
			WorkerInfo[id].completedStateMap = true;
			continue;
		}

		if (VeriQueue[id].size() > 0) {
			FastVector<Metadata *> *veriptr = VeriQueue[id].front();
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

/********************************************
 Check README.md for detailed implementation
 ********************************************/
// use negative integers to represent timestamp that is not an exact value
int timestamp_atleast(int t)
{
	return (t >= 0 ? -t : t);
}

int timestamp_exactly(int t)
{
	return (t <= 0 ? -t : t);
}

bool timestamp_isexacttime(int t)
{
	return (t >= 0);
}

inline void VeriProc_Assign(Metadata *cur, interval_set_addr &PersistInfo, interval_map_addr_timestamp &OrderInfo, int &timestamp)
{
	size_t startaddr = (size_t)(cur->assign.addr);
	size_t endaddr = startaddr + cur->assign.size;
	discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
	log(
		"%s %p %lu\n",
		MetadataTypeStr[_ASSIGN],
		cur->assign.addr,
		cur->assign.size);
	PersistInfo += addrinterval;
	OrderInfo += make_pair(addrinterval, timestamp_atleast(timestamp));
}

inline void VeriProc_Flush(Metadata *cur, interval_set_addr &PersistInfo, interval_map_addr_timestamp &OrderInfo, int &timestamp)
{
	size_t startaddr = (size_t)(cur->flush.addr);
	size_t endaddr = startaddr + cur->flush.size;
	discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
	log("%s %p %lu\n",
		MetadataTypeStr[_FLUSH],
		cur->flush.addr,
		cur->flush.size);
	PersistInfo -= addrinterval;
	OrderInfo += make_pair(addrinterval, timestamp_exactly(timestamp));
}

inline void VeriProc_Fence(int &timestamp)
{
	log("%s\n", MetadataTypeStr[_FENCE]);
	timestamp++;
}

inline void VeriProc_Persist(Metadata *cur, interval_set_addr &PersistInfo, interval_map_addr_timestamp &OrderInfo, int &timestamp)
{
	size_t startaddr = (size_t)(cur->persist.addr);
	size_t endaddr = startaddr + cur->persist.size;
	discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
	log("%s %p %lu\n",
		MetadataTypeStr[_PERSIST],
		cur->persist.addr,
		cur->persist.size);
	auto iter = PersistInfo.find(addrinterval);

	if (iter != PersistInfo.end()) {
		addrinterval = addrinterval & (*iter);
		char filename_temp[FILENAME_LEN + 1];
		strncpy(filename_temp, cur->persist.file_name, FILENAME_LEN);
		filename_temp[FILENAME_LEN] = '\0';
		printf(
			COLOR_RED "PERSIST ERROR: " COLOR_RESET
			"%s:%hu: Address range [0x%lx, 0x%lx) not persisted.\n",
			filename_temp,
			cur->persist.line_num,
			addrinterval.lower(),
			addrinterval.upper());
	}
}

void VeriProc_Order(Metadata *cur, interval_set_addr &PersistInfo, interval_map_addr_timestamp &OrderInfo, int &timestamp)
{
	size_t startaddr = (size_t)(cur->order.early_addr);
	size_t endaddr = startaddr + cur->order.early_size;
	discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);

	startaddr = (size_t)(cur->order.late_addr);
	endaddr = startaddr + cur->order.late_size;
	discrete_interval<size_t> addrinterval_late = interval<size_t>::right_open(startaddr, endaddr);

	log(
		"%s %p %lu %p %lu\n",
		MetadataTypeStr[_ORDER],
		cur->order.early_addr,
		cur->order.early_size,
		cur->order.late_addr,
		cur->order.late_size);

	// check maximum timestamp of the "early" address range is strictly smaller than the minimum timestamp of the "late" address range
	if (within(addrinterval, OrderInfo) && within(addrinterval_late, OrderInfo)) {
		auto intersec = addrinterval & OrderInfo;
		auto intersec_late = addrinterval_late & OrderInfo;
		int early_max = 0, late_min = std::numeric_limits<int>::max();

		for (auto j = intersec.begin(); j != intersec.end(); j++) {
			if (!timestamp_isexacttime(j->second)) {
				early_max = std::numeric_limits<int>::max();
				break;
			}
			early_max = std::max(early_max, j->second);
		}
		for (auto j = intersec_late.begin(); j != intersec_late.end(); j++) {
			late_min = std::min(late_min, timestamp_exactly(j->second));
		}

		if (early_max >= late_min) {
			char filename_temp[FILENAME_LEN + 1];
			strncpy(filename_temp, cur->order.file_name, FILENAME_LEN);
			filename_temp[FILENAME_LEN] = '\0';
			printf(
				COLOR_RED "ORDER ERROR: " COLOR_RESET
				"%s:%hu: Address range [0x%lx, 0x%lx) not before [0x%lx, 0x%lx).\n",
				filename_temp,
				cur->order.line_num,
				(size_t)(cur->order.early_addr),
				(size_t)(cur->order.early_addr) + cur->order.early_size,
				(size_t)(cur->order.late_addr),
				(size_t)(cur->order.late_addr) + cur->order.late_size);
		}
	}
	else {
		char filename_temp[FILENAME_LEN + 1];
		strncpy(filename_temp, cur->order.file_name, FILENAME_LEN);
		filename_temp[FILENAME_LEN] = '\0';
		printf(
			COLOR_RED "ORDER ERROR: " COLOR_RESET
			"%s:%hu: Queried address range not yet assigned.\n",
			filename_temp,
			cur->order.line_num);
	}
}

void NVMVeri::VeriProc(FastVector<Metadata *> *veriptr)
{
	// usually sizeof(size_t) = 8 on 64-bit system
	interval_set_addr PersistInfo;
	interval_map_addr_timestamp OrderInfo;
	int timestamp = 1; // because we want to use the sign of timestamp to store whether this is an exact timestamp, so start from 1
	size_t startaddr, endaddr;

	int prev = 0;
	int cur = 0;

	for (; cur != veriptr->size(); cur++) {
		if (((*veriptr)[cur])->type == _FENCE) {
			// process all Metadata in frame [prev, cur) if (*cur->type == _FENCE)
			for (int i = prev; i != cur; i++) {
				if (((*veriptr)[i])->type == _ASSIGN) {
					VeriProc_Assign(((*veriptr)[i]), PersistInfo, OrderInfo, timestamp);
				}
				else if (((*veriptr)[i])->type == _FLUSH) {
					// log("flush verified %p, %lu, %d\n", ((*veriptr)[i])->flush.addr, ((*veriptr)[i])->flush.size, ((*veriptr)[i])->type);
					VeriProc_Flush(((*veriptr)[i]), PersistInfo, OrderInfo, timestamp);
				}
				else if (((*veriptr)[i])->type == _FENCE) {
					VeriProc_Fence(timestamp);
				}
				else if (((*veriptr)[i])->type == _PERSIST) {
					VeriProc_Persist(((*veriptr)[i]), PersistInfo, OrderInfo, timestamp);
				}
				else if (((*veriptr)[i])->type == _ORDER) {
					VeriProc_Order(((*veriptr)[i]), PersistInfo, OrderInfo, timestamp);
				}
				else {
				}
			}
			prev = cur;
		}
	}
	// processing tail value of [prev, cur):
	// prev point to last _FENCE, cur point to veriptr->end
	for (int i = prev; i != cur; i++) {
		if (((*veriptr)[i])->type == _ASSIGN) {
			VeriProc_Assign(((*veriptr)[i]), PersistInfo, OrderInfo, timestamp);
		}
		else if (((*veriptr)[i])->type == _FLUSH) {
			// do nothing
			//printf("flush not verified %p\n", ((*veriptr)[i])->flush.addr);

		}
		else if (((*veriptr)[i])->type == _FENCE) {
			VeriProc_Fence(timestamp);
		}
		else if (((*veriptr)[i])->type == _PERSIST) {
			VeriProc_Persist(((*veriptr)[i]), PersistInfo, OrderInfo, timestamp);
		}
		else if (((*veriptr)[i])->type == _ORDER) {
			VeriProc_Order(((*veriptr)[i]), PersistInfo, OrderInfo, timestamp);
		}
		else {
		}
	}
}

/* C programming interface for Nvmveri */

void *C_createVeriInstance()
{
	NVMVeri *result = new NVMVeri();
	veriInstancePtr = result;
	return (void *)result;
}

void C_deleteVeriInstance(void *veriInstance)
{
	NVMVeri *in = (NVMVeri *)veriInstance;
	delete in;
}

void C_execVeri(void *veriInstance, void *metadata_vector)
{
	NVMVeri *in = (NVMVeri *)veriInstance;
	in->execVeri((FastVector<Metadata *> *)metadata_vector);
}

void C_getVeri(void *veriInstance, void *veriResult)
{
	NVMVeri *in = (NVMVeri *)veriInstance;
	FastVector<VeriResult> r;
	in->getVeri(r);
	// TODO: cast veriResult
}


void C_getVeriDefault(void *veriInstance)
{
	NVMVeri *in = (NVMVeri *)veriInstance;
	FastVector<VeriResult> r;
	in->getVeri(r);
	// TODO: cast veriResult
}

void *C_createMetadataVector()
{
	FastVector<Metadata *> *vec= new FastVector<Metadata *>;
	//vec->reserve(100);
	return (void *)(vec);
}

void C_deleteMetadataVector(void *victim)
{
	FastVector<Metadata *> *temp = (FastVector<Metadata *> *)victim;
	for (int i = 0; i != temp->size(); i++) {
		delete ((*temp)[i]);
	}
}

void C_initThread() {
	unique_lock<mutex> lock(thread_info.lock);
	thread_id = thread_info.cur_thread_id;
	++(thread_info.cur_thread_id);
	existVeriInstance = 0;
	transactionLog = NULL;
	nvmveri_cur_idx = 0;
}

void C_getNewMetadataPtr() {
	metadataPtr = metadataVectorPtr[nvmveri_cur_idx];
	nvmveri_cur_idx++;
}

void C_createMetadata_Assign(void *metadata_vector, void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _ASSIGN;


		m->assign.addr = addr;
		m->assign.size = size;
		log("create metadata assign %p, %lu, %d\n", m->assign.addr, m->assign.size, m->type);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);

		// if assign is inside a transaction, then the lhs needs to be persisted.
		if (transactionLog != NULL) {
			m = new Metadata;
			m->type = _PERSIST;

			//log("persist_aa\n");
			m->persist.addr = addr;
			m->persist.size = size;
			m->persist.line_num = 0;
			strncpy(m->persist.file_name, "in TX", FILENAME_LEN);
			log("create persisted assign %p, %lu, %d\n", m->persist.addr, m->persist.size, m->type);
			transactionLog->push_back(m);
		}
	}
	// else {
		//log("assign\n");
	// }
}

void C_createMetadata_Flush(void *metadata_vector, void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _FLUSH;

		m->flush.addr = addr;
		m->flush.size = size;
		log("create metadata flush %p, %lu, %d\n", m->flush.addr, m->flush.size, m->type);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
	// else {
		//log("flush\n");
	// }
}


void C_createMetadata_Commit(void *metadata_vector)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _COMMIT;

		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
	// else {
		//log("commit\n");
	// }
}


void C_createMetadata_Barrier(void *metadata_vector)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _BARRIER;

		//log("flush_aa\n");
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
	// else {
		//log("barrier\n");
	// }
}


void C_createMetadata_Fence(void *metadata_vector)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _FENCE;
		//log("fence_aa\n");

		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
	// else {
		//log("fence\n");
	// }
}


void C_createMetadata_Persist(void *metadata_vector, void *addr, size_t size, const char file_name[], unsigned short line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _PERSIST;

		//log("persist_aa\n");
		m->persist.addr = addr;
		m->persist.size = size;
		m->persist.line_num = line_num;
		strncpy(m->persist.file_name, (file_name+strlen(file_name)-FILENAME_LEN), FILENAME_LEN);
		log("create metadata persist %p, %lu, %d\n", m->persist.addr, m->persist.size, m->type);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
	// else {
		//log("persist\n");
	// }
}


void C_createMetadata_Order(void *metadata_vector, void *early_addr, size_t early_size, void *late_addr, size_t late_size, const char file_name[], unsigned short line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _ORDER;
		m->order.early_addr = early_addr;
		m->order.early_size = early_size;
		m->order.late_addr = late_addr;
		m->order.late_size = late_size;
		m->order.line_num = line_num;
		strncpy(m->order.file_name, (file_name+strlen(file_name)-FILENAME_LEN), FILENAME_LEN);

		//log("order_aa\n");
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
	// else {
		//log("order\n");
	// }
}

void C_registerVariable(char* name, void* addr, size_t size)
{
	string variableName(name);
	variableName += std::to_string(thread_id);
	if (((NVMVeri*)veriInstancePtr)->VariableNameAddressMap.find(variableName) == ((NVMVeri*)veriInstancePtr)->VariableNameAddressMap.end()) {
		printf("Register name=%s, addr=%p, size=%lu, ", name, addr, size);
		printf("variableName=%s\n", variableName.c_str());
		VariableInfo new_var;
		new_var.addr = addr;
		new_var.size = size;
		((NVMVeri*)veriInstancePtr)->VariableNameAddressMap[variableName] = new_var; //std::pair<void*, int>(addr, (int)size);
	}
}

void C_unregisterVariable(char* name)
{
	string variableName(name);
	variableName += std::to_string(thread_id);
	((NVMVeri*)veriInstancePtr)->VariableNameAddressMap.erase(variableName);
}

void* C_getVariable(char* name, size_t* size)
{
	string variableName(name);
	variableName += std::to_string(thread_id);
	//printf("%s\n", variableName.c_str());
	*size = ((NVMVeri*)veriInstancePtr)->VariableNameAddressMap[variableName].size;
	return ((NVMVeri*)veriInstancePtr)->VariableNameAddressMap[variableName].addr;
}

void C_transactionBegin(void *metadata_vector)
{
	if (transactionLog == NULL) {
		transactionLog = new FastVector<Metadata *>;
	}

}

void C_transactionEnd(void *metadata_vector)
{
	if (transactionLog != NULL) {
		((FastVector<Metadata *> *)metadata_vector)->append(*transactionLog);
		delete transactionLog;
		transactionLog = NULL;
	}
}


#endif // !NVMVERI_KERNEL_CODE
