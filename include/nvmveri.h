#ifndef __NVMVERI_H__
#define __NVMVERI_H__

#define MAX_THREAD_POOL_SIZE 5
#define MAX_OP_NAME_SIZE 50

void *C_createVeriInstance(void);
void C_deleteVeriInstance(void *);

void C_execVeri(void *, void *);
void C_getVeri(void *, void *);


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

extern void* metadataPtr;
extern int existVeriInstance;

#endif
