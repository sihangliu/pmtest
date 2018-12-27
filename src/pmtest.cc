#include "pmtest.hh"
#include <stdarg.h>

#define COLOR_RED "\x1B[31m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_RESET "\x1B[0m"

//size_t PMTest::VeriNumber;
/*
queue<vector<Metadata *> *> PMTest::VeriQueue[MAX_THREAD_POOL_SIZE];
mutex PMTest::VeriQueueMutex[MAX_THREAD_POOL_SIZE];
condition_variable PMTest::VeriQueueCV[MAX_THREAD_POOL_SIZE];

vector<VeriResult> PMTest::ResultVector[MAX_THREAD_POOL_SIZE];
mutex PMTest::ResultVectorMutex[MAX_THREAD_POOL_SIZE];

atomic<bool> PMTest::termSignal[MAX_THREAD_POOL_SIZE];
atomic<bool> PMTest::getResultSignal[MAX_THREAD_POOL_SIZE];

atomic<bool> PMTest::completedStateMap[MAX_THREAD_POOL_SIZE];
atomic<int> PMTest::completedThread;
*/

#ifndef PMTEST_KERNEL_CODE
const char MetadataTypeStr[20][30] = {
	"_ASSIGN", "_FLUSH", "_COMMIT", "_BARRIER", "_FENCE",
	"_PERSIST", "_ORDER",
	"_TRANSACTIONDELIM", "_ENDING",
	"_TRANSACTIONBEGIN", "_TRANSACTIONEND", "_TRANSACTIONADD", "_EXCLUDE", "_INCLUDE"
};

__thread void *metadataPtr;
//void *metadataManagerPtr;
__thread int thread_id;
__thread int existVeriInstance = 0;
__thread int pmtest_cur_idx;
__thread void **metadataVectorPtr;
void* veriInstancePtr;

ThreadInfo thread_info;


PMTest::PMTest()
{
	initVeri();
}


PMTest::~PMTest()
{
	termVeri();
}


void error_msg(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}


bool PMTest::initVeri()
{
	// create worker
	for (int i = 0; i < MAX_THREAD_POOL_SIZE; i++) {
		WorkerInfo[i].WorkerThreadPool = new thread(&PMTest::VeriWorker, this, i);
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
	return true;
}


bool PMTest::termVeri()
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
bool PMTest::execVeri(FastVector<Metadata *> *input)
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
bool PMTest::getVeri(FastVector<VeriResult> &output)
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
	}
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
	}


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


void PMTest::VeriWorker(int id)
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

inline int VeriProc_Assign(Metadata *cur, interval_set_addr &ExcludeInfo, interval_set_addr &PersistInfo, interval_set_addr &TransactionAddInfo, interval_map_addr_timestamp &OrderInfo, FastVector<Metadata *> &TransactionPersistInfo, int &timestamp, int &transactionCount)
{
	
	if (cur->size > 0) {
		size_t startaddr = (size_t)(cur->addr);
		size_t endaddr = startaddr + cur->size;
		discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
		
		LOG(
			"%s %p %d %s %u\n",
			MetadataTypeStr[_ASSIGN],
			cur->addr,
			cur->size,
			cur->file_name,
			cur->line_num);
	#ifdef PMTEST_EXCLUDE
		auto it = ExcludeInfo.find(addrinterval);
		if (it != ExcludeInfo.end()) {
			return -2;
		}
	#endif // PMTEST_EXCLUDE
		PersistInfo += addrinterval;
		OrderInfo += make_pair(addrinterval, timestamp_atleast(timestamp));
		if (transactionCount > 0) {
			TransactionPersistInfo.push_back(cur);

			auto iter = TransactionAddInfo.find(addrinterval);
			if (iter == TransactionAddInfo.end()) {
				char filename_temp[FILENAME_LEN + 1];
				strncpy(filename_temp, cur->file_name, FILENAME_LEN);
				filename_temp[FILENAME_LEN] = '\0';
				printf(
					COLOR_RED "ASSIGN ERROR: " COLOR_RESET
					"%s:%u: Address range [0x%lx, 0x%lx) is not TransactionAdded before modified.\n",
					filename_temp,
					cur->line_num,
					addrinterval.lower(),
					addrinterval.upper());
				return -1;
			}
		}
	}
	return 0;
}

inline int VeriProc_Flush(Metadata *cur, interval_set_addr &ExcludeInfo, interval_set_addr &PersistInfo, interval_map_addr_timestamp &OrderInfo, int &timestamp)
{
	if (cur->size > 0) {
		size_t startaddr = (size_t)(cur->addr);
		size_t endaddr = startaddr + cur->size;
		discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
		LOG("%s %p %d %s %u\n",
			MetadataTypeStr[_FLUSH],
			cur->addr,
			cur->size,
			cur->file_name,
			cur->line_num);
	#ifdef PMTEST_EXCLUDE
		auto it = ExcludeInfo.find(addrinterval);
		if (it != ExcludeInfo.end()) {
			return -2;
		}
	#endif // PMTEST_EXCLUDE
	#ifdef PMTEST_WARN
		auto iter = PersistInfo.find(addrinterval);
		if (iter == PersistInfo.end()) {
			char filename_temp[FILENAME_LEN + 1];
			strncpy(filename_temp, cur->file_name, FILENAME_LEN);
			filename_temp[FILENAME_LEN] = '\0';
			printf(
				COLOR_YELLOW "FLUSH WARNING: " COLOR_RESET
				"%s:%u: Address range [0x%lx, 0x%lx) is not modified, no need to flush.\n",
				filename_temp,
				cur->line_num,
				addrinterval.lower(),
				addrinterval.upper());
		}
		else
			PersistInfo -= addrinterval;
	#else
		PersistInfo -= addrinterval;
	#endif // PMTEST_WARN
		OrderInfo += make_pair(addrinterval, timestamp_exactly(timestamp));
	}
	return 0;
}

inline int VeriProc_Fence(Metadata *cur, int &timestamp)
{
	LOG("%s %s %u\n",
		MetadataTypeStr[_FENCE],
		cur->file_name,
		cur->line_num);
	timestamp++;
	return 0;
}

inline int VeriProc_Persist(Metadata *cur, interval_set_addr &ExcludeInfo, interval_set_addr &PersistInfo)
{
	if (cur->size > 0) {
		size_t startaddr = (size_t)(cur->addr);
		size_t endaddr = startaddr + cur->size;
		discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
		LOG("%s %p %d %s %u\n",
			MetadataTypeStr[_PERSIST],
			cur->addr,
			cur->size,
			cur->file_name,
			cur->line_num);
	#ifdef PMTEST_EXCLUDE
		auto it = ExcludeInfo.find(addrinterval);
		if (it != ExcludeInfo.end()) {
			return -2;
		}
	#endif // PMTEST_EXCLUDE

		auto iter = PersistInfo.find(addrinterval);

		if (iter != PersistInfo.end()) {
			addrinterval = addrinterval & (*iter);
			char filename_temp[FILENAME_LEN + 1];
			strncpy(filename_temp, cur->file_name, FILENAME_LEN);
			filename_temp[FILENAME_LEN] = '\0';
			printf(
				COLOR_RED "PERSIST ERROR: " COLOR_RESET
				"%s:%u: Address range [0x%lx, 0x%lx) not persisted.\n",
				filename_temp,
				cur->line_num,
				addrinterval.lower(),
				addrinterval.upper());
			return -1;
		}
	}
	return 0;
}

inline int VeriProc_Order(Metadata *cur, interval_set_addr &ExcludeInfo, interval_map_addr_timestamp &OrderInfo, int &timestamp)
{
	if (cur->size > 0 && cur->size_late > 0) {
		size_t startaddr = (size_t)(cur->addr);
		size_t endaddr = startaddr + cur->size;
		discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);

		startaddr = (size_t)(cur->addr_late);
		endaddr = startaddr + cur->size_late;
		discrete_interval<size_t> addrinterval_late = interval<size_t>::right_open(startaddr, endaddr);
		LOG("%s %p %d %p %d %s %u\n",
			MetadataTypeStr[_ORDER],
			cur->addr,
			cur->size,
			cur->addr_late,
			cur->size_late,
			cur->file_name,
			cur->line_num);
	#ifdef PMTEST_EXCLUDE
		auto it = ExcludeInfo.find(addrinterval);
		if (it != ExcludeInfo.end()) {
			return -2;
		}
		it = ExcludeInfo.find(addrinterval_late);
		if (it != ExcludeInfo.end()) {
			return -2;
		}
	#endif // PMTEST_EXCLUDE

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
				strncpy(filename_temp, cur->file_name, FILENAME_LEN);
				filename_temp[FILENAME_LEN] = '\0';
				printf(
					COLOR_RED "ORDER ERROR: " COLOR_RESET
					"%s:%u: Address range [0x%lx, 0x%lx) not before [0x%lx, 0x%lx).\n",
					filename_temp,
					cur->line_num,
					(size_t)(cur->addr),
					(size_t)(cur->addr) + cur->size,
					(size_t)(cur->addr_late),
					(size_t)(cur->addr_late) + cur->size_late);
				return -1;
			}
		}
		else {
			char filename_temp[FILENAME_LEN + 1];
			strncpy(filename_temp, cur->file_name, FILENAME_LEN);
			filename_temp[FILENAME_LEN] = '\0';
			printf(
				COLOR_RED "ORDER ERROR: " COLOR_RESET
				"%s:%u: Queried address range not yet assigned.\n",
				filename_temp,
				cur->line_num);
			return -1;
		}
	}
	return 0;
}

inline void VeriProc_TransactionBegin(Metadata *cur, FastVector<Metadata *> &TransactionPersistInfo,int &transactionCount)
{
	if (transactionCount == 0) {
		TransactionPersistInfo.clear();
	}
	transactionCount++;
	LOG("%s %s %u\n",
		MetadataTypeStr[_TRANSACTIONBEGIN],
		cur->file_name,
		cur->line_num);
}

inline void VeriProc_TransactionEnd(Metadata *cur, interval_set_addr &ExcludeInfo, interval_set_addr &PersistInfo, FastVector<Metadata *> &TransactionPersistInfo, int &transactionCount)
{
	transactionCount--;
	LOG("%s %s %u\n",
		MetadataTypeStr[_TRANSACTIONEND],
		cur->file_name,
		cur->line_num);
	if (transactionCount == 0) {
		for (int i = 0; i < TransactionPersistInfo.size(); i++) {
			TransactionPersistInfo[i]->type = _PERSIST;
			VeriProc_Persist(TransactionPersistInfo[i], ExcludeInfo, PersistInfo);
			// This is not the exactly correct usage, but the behavior is correct because _ASSIGN and _PERSIST metadata have the same layout.
		}
	}
}

inline void VeriProc_TransactionAdd(Metadata *cur, interval_set_addr &TransactionAddInfo, int &transactionCount)
{
	if (transactionCount > 0) {
		if (cur->size > 0) {
			LOG("%s %p %d %s %u\n",
				MetadataTypeStr[_TRANSACTIONADD],
				cur->addr,
				cur->size,
				cur->file_name,
				cur->line_num);
			size_t startaddr = (size_t)(cur->addr);
			size_t endaddr = startaddr + cur->size;
			discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
		#ifdef PMTEST_WARN
			auto iter = TransactionAddInfo.find(addrinterval);
			if (iter != TransactionAddInfo.end()) {
				char filename_temp[FILENAME_LEN + 1];
				strncpy(filename_temp, cur->file_name, FILENAME_LEN);
				filename_temp[FILENAME_LEN] = '\0';
				printf(
					COLOR_YELLOW "TRANSACTIONADD WARNING: " COLOR_RESET
					"%s:%u: Address range [0x%lx, 0x%lx) overlaps with previously TransactionAdded addresses.\n",
					filename_temp,
					cur->line_num,
					addrinterval.lower(),
					addrinterval.upper());
			}
		#endif // PMTEST_WARN
			TransactionAddInfo += addrinterval;
		}
	}
}

inline void VeriProc_Exclude(Metadata *cur, interval_set_addr &ExcludeInfo)
{
	if (cur->size > 0) {
		size_t startaddr = (size_t)(cur->addr);
		size_t endaddr = startaddr + cur->size;
		discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
		LOG(
			"%s %p %d %s %u\n",
			MetadataTypeStr[_EXCLUDE],
			cur->addr,
			cur->size,
			cur->file_name,
			cur->line_num);
		ExcludeInfo += addrinterval;
	}
}

inline void VeriProc_Include(Metadata *cur, interval_set_addr &ExcludeInfo)
{
	if (cur->size > 0) {
		size_t startaddr = (size_t)(cur->addr);
		size_t endaddr = startaddr + cur->size;
		discrete_interval<size_t> addrinterval = interval<size_t>::right_open(startaddr, endaddr);
		LOG(
			"%s %p %d %s %u\n",
			MetadataTypeStr[_INCLUDE],
			cur->addr,
			cur->size,
			cur->file_name,
			cur->line_num);
		ExcludeInfo -= addrinterval;
	}
}

void PMTest::VeriProc(FastVector<Metadata *> *veriptr)
{
	// usually sizeof(size_t) = 8 on 64-bit system
	interval_set_addr ExcludeInfo;
	interval_set_addr PersistInfo;
	interval_set_addr TransactionAddInfo;
	interval_map_addr_timestamp OrderInfo;
	FastVector<Metadata *> TransactionPersistInfo;
	int timestamp = 1; // because we want to use the sign of timestamp to store whether this is an exact timestamp, so start from 1
	int transactionCount = 0; // this also determines the matching of _TRANSACTIONBEGIN and _TRANSACTIONEND

	size_t startaddr, endaddr;
	

	int prev = 0;
	int cur = 0;

	for (; cur != veriptr->size(); cur++) {
		if (((*veriptr)[cur])->type == _FENCE) {
			// process all Metadata in frame [prev, cur) if (*cur->type == _FENCE)
			for (int i = prev; i != cur; i++) {
				switch(((*veriptr)[i])->type) {
				case _ASSIGN:
					VeriProc_Assign((*veriptr)[i], ExcludeInfo, PersistInfo, TransactionAddInfo, OrderInfo, TransactionPersistInfo, timestamp, transactionCount); break;
				case _FLUSH:
					VeriProc_Flush((*veriptr)[i], ExcludeInfo, PersistInfo, OrderInfo, timestamp); break;
				case _FENCE:
					VeriProc_Fence((*veriptr)[i], timestamp); break;
				case _PERSIST:
					VeriProc_Persist((*veriptr)[i], ExcludeInfo, PersistInfo); break;
				case _ORDER:
					VeriProc_Order((*veriptr)[i], ExcludeInfo, OrderInfo, timestamp); break;
				case _TRANSACTIONBEGIN:
					VeriProc_TransactionBegin((*veriptr)[i], TransactionPersistInfo, transactionCount); break;
				case _TRANSACTIONEND:
					VeriProc_TransactionEnd((*veriptr)[i], ExcludeInfo, PersistInfo, TransactionPersistInfo, transactionCount); break;
				case _TRANSACTIONADD:
					VeriProc_TransactionAdd((*veriptr)[i], TransactionAddInfo, transactionCount); break;
				case _EXCLUDE:
					VeriProc_Exclude((*veriptr)[i], ExcludeInfo); break;
				case _INCLUDE:
					VeriProc_Include((*veriptr)[i], ExcludeInfo); break;
				default:
					LOG("Unidentified or unprocessed type.\n");
				}
			}

			prev = cur;
		}
	}
	// processing tail value of [prev, cur):
	// prev point to last _FENCE, cur point to veriptr->end
	for (int i = prev; i != cur; i++) {
		switch(((*veriptr)[i])->type) {
		case _ASSIGN:
			VeriProc_Assign(((*veriptr)[i]), ExcludeInfo, PersistInfo, TransactionAddInfo, OrderInfo, TransactionPersistInfo, timestamp, transactionCount); break;
		case _FLUSH:
			/* do nothing */ break;
		case _FENCE:
			VeriProc_Fence((*veriptr)[i], timestamp); break;
		case _PERSIST:
			VeriProc_Persist(((*veriptr)[i]), ExcludeInfo, PersistInfo); break;
		case _ORDER:
			VeriProc_Order(((*veriptr)[i]), ExcludeInfo, OrderInfo, timestamp); break;
		case _TRANSACTIONBEGIN:
			VeriProc_TransactionBegin((*veriptr)[i], TransactionPersistInfo, transactionCount); break;
		case _TRANSACTIONEND:
			VeriProc_TransactionEnd((*veriptr)[i], ExcludeInfo, PersistInfo, TransactionPersistInfo, transactionCount); break;
		case _TRANSACTIONADD:
			VeriProc_TransactionAdd((*veriptr)[i], TransactionAddInfo, transactionCount); break;
		case _EXCLUDE:
			VeriProc_Exclude((*veriptr)[i], ExcludeInfo); break;
		case _INCLUDE:
			VeriProc_Include((*veriptr)[i], ExcludeInfo); break;
		default:
			LOG("Unidentified or unprocessed type.\n");
		}
	}

	if (transactionCount > 0) {
		std::cerr << COLOR_RED << "TRANSACTIONEND ERROR: " << COLOR_RESET
			<< "TransactionBegin and TransactionEnd does not match." << std::endl;
	}
}

/* C programming interface for PMTest */

void *C_createVeriInstance()
{
	PMTest *result = new PMTest();
	veriInstancePtr = result;
	return (void *)result;
}

void C_deleteVeriInstance(void *veriInstance)
{
	PMTest *in = (PMTest *)veriInstance;
	delete in;
}

void C_execVeri(void *veriInstance, void *metadata_vector)
{
	PMTest *in = (PMTest *)veriInstance;
	in->execVeri((FastVector<Metadata *> *)metadata_vector);
}

void C_getVeri(void *veriInstance, void *veriResult)
{
	PMTest *in = (PMTest *)veriInstance;
	FastVector<VeriResult> r;
	in->getVeri(r);
	// TODO: cast veriResult
}


void C_getVeriDefault(void *veriInstance)
{
	PMTest *in = (PMTest *)veriInstance;
	FastVector<VeriResult> r;
	in->getVeri(r);
	// TODO: cast veriResult
}

// TODO: this is not user friendly
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
	pmtest_cur_idx = 0;
}

void C_getNewMetadataPtr() {
	metadataPtr = metadataVectorPtr[pmtest_cur_idx];
	pmtest_cur_idx++;
}

void C_createMetadata_Assign(void *metadata_vector, void *addr, size_t size, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _ASSIGN;

		m->addr = addr;
		m->size = size;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata assign %p, %d, %s, %u\n", m->addr, m->size, m->file_name, m->line_num);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}

void C_createMetadata_Flush(void *metadata_vector, void *addr, size_t size, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _FLUSH;

		m->addr = addr;
		m->size = size;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		LOG_NOTE("create metadata flush %p, %d, %s, %u\n", m->addr, m->size, m->file_name, m->line_num);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}


void C_createMetadata_Commit(void *metadata_vector, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _COMMIT;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata commit\n");
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}


void C_createMetadata_Barrier(void *metadata_vector, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _BARRIER;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata barrier\n");
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}


void C_createMetadata_Fence(void *metadata_vector, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _FENCE;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata fence\n");
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}


void C_createMetadata_Persist(void *metadata_vector, void *addr, size_t size, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _PERSIST;
		m->addr = addr;
		m->size = size;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata persist %p, %d, %s, %u\n", m->addr, m->size, m->file_name, m->line_num);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}


void C_createMetadata_Order(void *metadata_vector, void *addr, size_t size, void *addr_late, size_t size_late, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _ORDER;
		m->addr = addr;
		m->size = size;
		m->addr_late = addr_late;
		m->size_late = size_late;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		LOG_NOTE("create metadata order %p, %d, %p, %d, %s, %u\n", m->addr, m->size, m->addr_late, m->size_late, m->file_name, m->line_num);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}

void C_createMetadata_TransactionBegin(void *metadata_vector, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _TRANSACTIONBEGIN;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata transactionbegin\n");
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}

void C_createMetadata_TransactionEnd(void *metadata_vector, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _TRANSACTIONEND;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata transactionend\n");
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}

void C_createMetadata_TransactionAdd(void *metadata_vector, void *addr, size_t size, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _TRANSACTIONADD;
		m->addr = addr;
		m->size = size;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		LOG_NOTE("create metadata transactionadd %p, %d, %s, %u\n", m->addr, m->size, m->file_name, m->line_num);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
}

void C_createMetadata_Exclude(void *metadata_vector, void *addr, size_t size, const char file_name[], unsigned int line_num)
{
#ifdef PMTEST_EXCLUDE
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _EXCLUDE;
		m->addr = addr;
		m->size = size;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata exclude %p, %d, %s, %u\n", m->addr, m->size, m->file_name, m->line_num);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
#endif // PMTEST_EXCLUDE
}

void C_createMetadata_Include(void *metadata_vector, void *addr, size_t size, const char file_name[], unsigned int line_num)
{
#ifdef PMTEST_EXCLUDE
	if (existVeriInstance) {
		Metadata *m = new Metadata;
		m->type = _INCLUDE;
		m->addr = addr;
		m->size = size;
		m->line_num = line_num;
		int file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			m->file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);
		LOG_NOTE("create metadata include %p, %d, %s, %u\n", m->addr, m->size, m->file_name, m->line_num);
		((FastVector<Metadata *> *)metadata_vector)->push_back(m);
	}
#endif // PMTEST_EXCLUDE
}

void C_registerVariable(char* name, void* addr, size_t size)
{
	string variableName(name);
	variableName += std::to_string(thread_id);
	// if exist, then do nothing
	if (((PMTest*)veriInstancePtr)->VariableNameAddressMap.find(variableName) == ((PMTest*)veriInstancePtr)->VariableNameAddressMap.end()) {
		printf("Register name=%s, addr=%p, size=%lu, ", name, addr, size);
		printf("variableName=%s\n", variableName.c_str());
		VariableInfo new_var;
		new_var.addr = addr;
		new_var.size = size;
		((PMTest*)veriInstancePtr)->VariableNameAddressMap[variableName] = new_var; //std::pair<void*, int>(addr, (int)size);
	}
}

void C_unregisterVariable(char* name)
{
	string variableName(name);
	variableName += std::to_string(thread_id);
	((PMTest*)veriInstancePtr)->VariableNameAddressMap.erase(variableName);
}

void* C_getVariable(char* name, size_t* size)
{
	string variableName(name);
	variableName += std::to_string(thread_id);
	// if not found, return NULL
	if (((PMTest*)veriInstancePtr)->VariableNameAddressMap.find(variableName) == ((PMTest*)veriInstancePtr)->VariableNameAddressMap.end()) {
		*size = 0;
		return NULL;
	} else {
		*size = ((PMTest*)veriInstancePtr)->VariableNameAddressMap[variableName].size;
		return ((PMTest*)veriInstancePtr)->VariableNameAddressMap[variableName].addr;
	}
}


void C_initVeri(void **veriInstance, int metadataVectorLen)
{
	*veriInstance = (void *)(new PMTest());
	metadataVectorPtr = (void **)(new FastVector<FastVector<Metadata *> *>);
	for (int i = 0; i < metadataVectorLen; i++) {
		((FastVector<FastVector<Metadata *> *> *)metadataVectorPtr)->push_back(new FastVector<Metadata *>);
	}
	pmtest_cur_idx = 0;
	auto p = (FastVector<FastVector<Metadata *> *> *)metadataVectorPtr;
	metadataPtr = (*p)[0];
	
}


void C_sendTrace(void *veriInstance)
{
	auto p = (FastVector<FastVector<Metadata *> *> *)metadataVectorPtr;
	if (pmtest_cur_idx >= p->size()) {
		printf(COLOR_RED "SENDTRACE ERROR: " COLOR_RESET "All metadata traces have been sent.\n");
	}
	else {
		PMTest *in = (PMTest *)veriInstance;
		in->execVeri((*p)[pmtest_cur_idx]);
		pmtest_cur_idx++;
		if (pmtest_cur_idx < p->size()) {
			metadataPtr = (*p)[pmtest_cur_idx];
		}
		else {
			metadataPtr = NULL;
		}
	}
}

void C_exitVeri(void *veriInstance)
{
	auto p = (FastVector<FastVector<Metadata *> *> *)metadataVectorPtr;
	int metadataVectorLen = p->size();
	for (int i = 0; i < metadataVectorLen; i++) {
		delete (*p)[i];
	}
	delete p;
	PMTest *in = (PMTest *)veriInstance;
	delete in;
}


#endif // !PMTEST_KERNEL_CODE
