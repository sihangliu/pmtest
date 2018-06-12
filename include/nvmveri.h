#ifndef __NVMVERI_H__
#define __NVMVERI_H__

//#define METADATA_PACKET_LENGTH 10

#ifndef NVMVERI_KERNEL_CODE
void *C_createVeriInstance(void);
void C_deleteVeriInstance(void *);

void C_execVeri(void *, void *);
void C_getVeri(void *, void *);

/* Metadata manager */
void *C_createMetadataManager(int);
void C_deleteMetadataManager(void *);
void C_setMetadataPtrInfoArray(void *, int, void **);
int C_getMetadataVectorCurIndex(void *);
void** C_getMetadataVectorArrayPtr(void *);
void* C_getMetadataVectorCurPtr(void *);
void C_registerThread(void *);
void C_setExistVeriInstance(void *);
void C_unsetExistVeriInstance(void *);
void C_incrMetadataVectorCurIndex(void *);
void C_resetMetadataVectorCurIndex(void *);
int C_getCurThreadIndex(void *);

/* Nvmveri */
void *C_createMetadataVector(void);
void C_deleteMetadataVector(void *);
void C_createMetadata_OpInfo(void *, char *, void *, size_t);
void C_createMetadata_Assign(void *, void *, size_t);
void C_createMetadata_Flush(void *, void *, size_t);
void C_createMetadata_Commit(void *);
void C_createMetadata_Barrier(void *);
void C_createMetadata_Fence(void *);
void C_createMetadata_Persist(void *, void *, size_t);
void C_createMetadata_Order(void *, void *, size_t, void *, size_t);
/* Multithread supported version */
void C_createMetadata_Assign_MultiThread(void *, void *, size_t);
void C_createMetadata_Flush_MultiThread(void *, void *, size_t);
void C_createMetadata_Commit_MultiThread(void *);
void C_createMetadata_Barrier_MultiThread(void *);
void C_createMetadata_Fence_MultiThread(void *);
void C_createMetadata_Persist_MultiThread(void *, void *, size_t);
void C_createMetadata_Order_MultiThread(void *, void *, size_t, void *, size_t);


extern void* metadataPtr;
extern int existVeriInstance;

extern void *metadataManagerPtr;

#endif // !NVMVERI_KERNEL_CODE

#endif // __NVMVERI_H__
