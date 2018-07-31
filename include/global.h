#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifndef NVMVERI_KERNEL_CODE

extern __thread void* metadataPtr;
extern __thread int existVeriInstance;
extern __thread int nvmveri_cur_idx;
extern __thread int thread_id;
//extern void *metadataManagerPtr;
extern __thread void **metadataVectorPtr;

extern void* veriInstancePtr;

/* Interface macro */
#define NVTest_START existVeriInstance = 1
#define NVTest_END existVeriInstance = 0
#define TX_CHECKER_START C_createMetadata_TransactionBegin(metadataPtr, __FILE__, __LINE__)
#define TX_CHECKER_END C_createMetadata_TransactionEnd(metadataPtr, __FILE__, __LINE__)
#define NVTest_transactionAdd(addr, size) C_createMetadata_TransactionAdd(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define NVTest_assign(addr, size) C_createMetadata_Assign(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define NVTest_flush(addr, size) C_createMetadata_Flush(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define NVTest_commit() C_createMetadata_Commit(metadataPtr, __FILE__, __LINE__)
#define NVTest_barrier() C_createMetadata_Barrier(metadataPtr, __FILE__, __LINE__)
#define NVTest_fence() C_createMetadata_Fence(metadataPtr, __FILE__, __LINE__)
#define NVTest_isPersistent(addr, size) C_createMetadata_Persist(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define NVTest_isPersistedBefore(addrA, sizeA, addrB, sizeB) C_createMetadata_Order(metadataPtr, (addrA), (sizeA), (addrB), (sizeB), __FILE__, __LINE__)


#endif // !NVMVERI_KERNEL_CODE

#endif // __GLOBAL_H__