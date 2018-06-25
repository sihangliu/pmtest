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
void C_createMetadata_Assign(void *, void *, size_t);
void C_createMetadata_Flush(void *, void *, size_t);
void C_createMetadata_Commit(void *);
void C_createMetadata_Barrier(void *);
void C_createMetadata_Fence(void *);
void C_createMetadata_Persist(void *, void *, size_t);
void C_createMetadata_Order(void *, void *, size_t, void *, size_t);
void C_registerVariable(char*, void*, int);
void C_unregisterVariable(char*);
void* C_getVariable(char*, int*);


extern __thread void* metadataPtr;
extern __thread int existVeriInstance;
extern __thread int nvmveri_cur_idx;
extern __thread int thread_id;
//extern void *metadataManagerPtr;
extern __thread void **metadataVectorPtr;

extern void* veriInstancePtr;

#endif // !NVMVERI_KERNEL_CODE

#endif // __NVMVERI_H__
