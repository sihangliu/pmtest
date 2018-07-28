#ifndef __KERNEL_MODULE__
#define __KERNEL_MODULE__

// KFIFO_THRESHOLD_LEN is set to half of maximum size
#define KFIFO_LEN 1024
#define KFIFO_THRESHOLD_LEN (KFIFO_LEN / 2)

#define PROC_NAME "nvmveri"


#define NVMVERI_MAJOR 0

#if defined (NVMVERI_KERNEL_CODE) && !defined(NVMVERI_USER_CODE)

#include <linux/fs.h>
#include <linux/ioctl.h>
#include "nvmveri.hh"

ssize_t NVMVeriDeviceRead(struct file *, char __user *, size_t, loff_t *);
void NVMVeriFifoWrite(Metadata *input);


int kC_initNVMVeriDevice(void);
int kC_exitNVMVeriDevice(void);

void kC_createMetadataVector(void);
void kC_deleteMetadataVector(void);
void kC_createMetadata_OpInfo(char *, void *, size_t);
void kC_createMetadata_Assign(void *, size_t);
void kC_createMetadata_Flush(void *, size_t);
void kC_createMetadata_Commit(void);
void kC_createMetadata_Barrier(void);
void kC_createMetadata_Fence(void);
void kC_createMetadata_Persist(void *, size_t, const char[], unsigned short);
void kC_createMetadata_Order(void *, size_t, void *, size_t, const char[], unsigned short);

void kC_createMetadata_TransactionDelim(void);
void kC_createMetadata_Ending(void);

void kC_createMetadata_TransactionBegin(void);
void kC_createMetadata_TransactionEnd(void);

extern void* metadataPtr;
extern int existVeriInstance;

#define NVTest_START existVeriInstance = 1
#define NVTest_END existVeriInstance = 0
#define isPersistent(addr,size) kC_createMetadata_Persist((addr), (size), __FILE__, __LINE__)
#define isPersistedBefore(addrA, sizeA, addrB, sizeB) kC_createMetadata_Order((addrA), (sizeA), (addrB), (sizeB), __FILE__, __LINE__)

#endif // NVMVERI_KERNEL_CODE && !NVMVERI_USER_CODE

#endif // __KERNEL_MODULE__
