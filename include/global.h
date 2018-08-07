#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifndef PMTEST_KERNEL_CODE

extern __thread void* metadataPtr; // point to the currently using metadata storage
extern __thread int existVeriInstance; // indicate if metadata needs to be recorded to the metadata storage
extern __thread int pmtest_cur_idx; // indicate the used space in the current metadata storage
extern __thread int thread_id;
//extern void *metadataManagerPtr;
extern __thread void **metadataVectorPtr; // point to the entire storage vector

extern void* veriInstancePtr;

/* Interface macro */
#define PMTest_START existVeriInstance = 1
#define PMTest_END existVeriInstance = 0
#define PMTest_CHECKER_START C_createMetadata_TransactionBegin(metadataPtr, __FILE__, __LINE__)
#define PMTest_CHECKER_END C_createMetadata_TransactionEnd(metadataPtr, __FILE__, __LINE__)
#define PMTest_transactionAdd(addr, size) C_createMetadata_TransactionAdd(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define PMTest_assign(addr, size) C_createMetadata_Assign(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define PMTest_flush(addr, size) C_createMetadata_Flush(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define PMTest_commit() C_createMetadata_Commit(metadataPtr, __FILE__, __LINE__)
#define PMTest_barrier() C_createMetadata_Barrier(metadataPtr, __FILE__, __LINE__)
#define PMTest_fence() C_createMetadata_Fence(metadataPtr, __FILE__, __LINE__)
#define PMTest_isPersistent(addr, size) C_createMetadata_Persist(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define PMTest_isPersistedBefore(addrA, sizeA, addrB, sizeB) C_createMetadata_Order(metadataPtr, (addrA), (sizeA), (addrB), (sizeB), __FILE__, __LINE__)
#define PMTest_exclude(addr, size) C_createMetadata_Exclude(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define PMTest_include(addr, size) C_createMetadata_Include(metadataPtr, (addr), (size), __FILE__, __LINE__)

#define PMTest_init(instancePtr, vectorLen) C_initVeri(&(instancePtr), (vectorLen))
#define PMTest_sendTrace(instancePtr) C_sendTrace((instancePtr))
#define PMTest_getResult(instancePtr) C_getVeriDefault((instancePtr))
#define PMTest_exit(instancePtr) C_exitVeri((instancePtr))


#endif // !PMTEST_KERNEL_CODE

#endif // __GLOBAL_H__