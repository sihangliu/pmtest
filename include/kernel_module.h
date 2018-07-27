#ifndef __KERNEL_MODULE__
#define __KERNEL_MODULE__

#define DEVICE_STORAGE_LEN 8192
// #define BUFFER_LEN 20
#define PROC_NAME "nvmveri"

// #if BUFFER_LEN > DEVICE_STORAGE_LEN
// 	#define BUFFER_LEN DEVICE_STORAGE_LEN
// #endif

#define NVMVERI_MAJOR 0

#if defined (NVMVERI_KERNEL_CODE) && !defined(NVMVERI_USER_CODE)

#include <linux/fs.h>
#include <linux/ioctl.h>


// typedef struct NVMVeriDevice
// {
// 	Metadata buf[DEVICE_STORAGE_LEN];
//
// 	wait_queue_head_t inq, outq;
// 	char *buf_head, *buf_tail;
// 	char *read_ptr, *write_ptr;
//
// 	struct semaphore sem;
// 	struct cdev cdev;
// } NVMVeriDevice;

// typedef struct KernelVector {
// 	struct Metadata* arr_vector;
// 	int cur_size;
// 	int vector_max_size;
// } KernelVector;

// void initKernelVector(KernelVector *);
// void pushKernelVector(KernelVector *, Metadata);
// void deleteKernelVector(KernelVector *);

ssize_t NVMVeriDeviceRead(struct file *, char __user *, size_t, loff_t *);
// static ssize_t NVMVeriDeviceWrite(struct file *, const char __user *, size_t, loff_t *);

// int NVMVeriDeviceOpen(struct inode *inode, struct file *filp);
// int NVMVeriDeviceRelease(struct inode *inode, struct file *filp);



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

extern void* metadataPtr;
extern int existVeriInstance;

#define NVTest_START existVeriInstance = 1
#define NVTest_END existVeriInstance = 0
#define isPersistent(addr,size) kC_createMetadata_Persist((addr), (size), __FILE__, __LINE__)
#define isPersistedBefore(addrA, sizeA, addrB, sizeB) kC_createMetadata_Order((addrA), (sizeA), (addrB), (sizeB), __FILE__, __LINE__)

#endif // NVMVERI_KERNEL_CODE && !NVMVERI_USER_CODE

#endif // __KERNEL_MODULE__
