#ifndef __NVMVERI_H__
#define __NVMVERI_H__

//#define METADATA_PACKET_LENGTH 10

#ifndef NVMVERI_KERNEL_CODE
void *C_createVeriInstance(void);
void C_deleteVeriInstance(void *);

void C_execVeri(void *, void *);
void C_getVeri(void *, void *);
void C_getVeriDefault(void *);

/* Thread control */
void C_initThread();
void C_getNewMetadataPtr();

/* Nvmveri */
void *C_createMetadataVector(void);
void C_deleteMetadataVector(void *);
void C_createMetadata_OpInfo(void *, char *, void *, size_t);
void C_createMetadata_Assign(void *, void *, size_t, const char[], unsigned short);
void C_createMetadata_Flush(void *, void *, size_t);
void C_createMetadata_Commit(void *);
void C_createMetadata_Barrier(void *);
void C_createMetadata_Fence(void *);
void C_createMetadata_Persist(void *, void *, size_t, const char[], unsigned short);
void C_createMetadata_Order(void *, void *, size_t, void *, size_t, const char[], unsigned short);
void C_registerVariable(char*, void*, size_t);
void C_unregisterVariable(char*);
void* C_getVariable(char*, size_t*);
void C_transactionBegin(void *);
void C_transactionEnd(void *);

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
#define TX_CHECKER_START C_transactionBegin(metadataPtr)
#define TX_CHECKER_END C_transactionEnd(metadataPtr)

#define assign(addr, size) C_createMetadata_Assign(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define isPersistent(addr, size) C_createMetadata_Persist(metadataPtr, (addr), (size), __FILE__, __LINE__)
#define isPersistedBefore(addrA, sizeA, addrB, sizeB) C_createMetadata_Order(metadataPtr, (addrA), (sizeA), (addrB), (sizeB), __FILE__, __LINE__)


#endif // !NVMVERI_KERNEL_CODE

#endif // __NVMVERI_H__
