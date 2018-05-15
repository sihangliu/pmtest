#ifndef __KERNEL_MODULE__
#define __KERNEL_MODULE__

#define DEVICE_STORAGE_LEN 200
#define BUFFER_LEN 20

#if defined (NVMVERI_KERNEL_CODE) && !defined(NVMVERI_USER_CODE)

#include <linux/fs.h>


typedef struct NVMVeriDevice
{
	Metadata data[DEVICE_STORAGE_LEN];
	struct semaphore sem;
	struct cdev cdev;
} NVMVeriDevice;

typedef struct KernelVector {
	struct Metadata* arr_vector;
	int cur_size;
	int vector_max_size;
} KernelVector;

void initKernelVector(KernelVector *);
void pushKernelVector(KernelVector *, Metadata);
void deleteKernelVector(KernelVector *);

int kC_createNVMVeriDevice(void);
int kC_exitNVMVeriDevice(void);

void kC_createMetadataVector(void);
void kC_deleteMetadataVector(void);
void kC_createMetadata_OpInfo(char *, void *, size_t);
void kC_createMetadata_Assign(void *, size_t);
void kC_createMetadata_Flush(void *, size_t);
void kC_createMetadata_Commit(void);
void kC_createMetadata_Barrier(void);
void kC_createMetadata_Fence(void);
void kC_createMetadata_Persist(void *, size_t);
void kC_createMetadata_Order(void *, size_t, void *, size_t);

ssize_t NVMVeriDeviceRead(struct file *, char *, size_t, loff_t *);
int NVMVeriDeviceOpen(struct inode *inode, struct file *filp);
int NVMVeriDeviceRelease(struct inode *inode, struct file *filp);

struct file_operations NVMVeriDeviceOps = {
	.owner = 	THIS_MODULE,
	.read = 	NVMDeviceRead,
	.open = 	NVMDeviceOpen,
	.release = 	NVMDeviceRelease
};

extern void* metadataPtr;
extern NVMVeriDevice nvmveri_dev;
extern int existVeriInstance;

#endif // NVMVERI_KERNEL_CODE && !NVMVERI_USER_CODE



#endif // __KERNEL_MODULE__
