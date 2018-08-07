#ifndef __KERNEL_MODULE__
#define __KERNEL_MODULE__

// KFIFO_THRESHOLD_LEN is set to half of maximum size
#define KFIFO_LEN 1024
#define KFIFO_THRESHOLD_LEN (KFIFO_LEN / 2)

#define PROC_NAME "pmtest"


#define PMTEST_MAJOR 0

#if defined (PMTEST_KERNEL_CODE) && !defined(PMTEST_USER_CODE)

#include <linux/fs.h>
#include <linux/ioctl.h>
#include "pmtest.hh"

ssize_t PMTestDeviceRead(struct file *, char __user *, size_t, loff_t *);
void PMTestFifoWrite(Metadata *input);

int kC_initPMTestDevice(void);
int kC_exitPMTestDevice(void);

void kC_createMetadata_Assign(void *, size_t, const char[], unsigned int);
void kC_createMetadata_Flush(void *, size_t, const char[], unsigned int);
void kC_createMetadata_Commit(const char[], unsigned int);
void kC_createMetadata_Barrier(const char[], unsigned int);
void kC_createMetadata_Fence(const char[], unsigned int);
void kC_createMetadata_Persist(void *, size_t, const char[], unsigned int);
void kC_createMetadata_Order(void *, size_t, void *, size_t, const char[], unsigned int);

void kC_createMetadata_TransactionDelim(void);
void kC_createMetadata_Ending(void);

void kC_createMetadata_TransactionBegin(const char[], unsigned int);
void kC_createMetadata_TransactionEnd(const char[], unsigned int);
void kC_createMetadata_TransactionAdd(void *, size_t, const char[], unsigned int);
void kC_createMetadata_Exclude(void *, size_t, const char[], unsigned int);
void kC_createMetadata_Include(void *, size_t, const char[], unsigned int);

extern void* metadataPtr;
extern int existVeriInstance;

/* Interface macro */
#define PMTest_START existVeriInstance = 1
#define PMTest_END existVeriInstance = 0
#define PMTest_initDevice() kC_initPMTestDevice()
#define PMTest_exitDevice() kC_exitPMTestDevice()
#define PMTest_CHECKER_START kC_createMetadata_TransactionBegin(__FILE__, __LINE__)
#define PMTest_CHECKER_END kC_createMetadata_TransactionEnd(__FILE__, __LINE__)
#define PMTest_transactionAdd() kC_createMetadata_TransactionAdd((addr), (size), __FILE__, __LINE__)
#define PMTest_assign(addr, size) kC_createMetadata_Assign((addr), (size), __FILE__, __LINE__)
#define PMTest_flush(addr, size) kC_createMetadata_Flush((addr), (size), __FILE__, __LINE__)
#define PMTest_commit() kC_createMetadata_Commit(__FILE__, __LINE__)
#define PMTest_barrier() kC_createMetadata_Barrier(__FILE__, __LINE__)
#define PMTest_fence() kC_createMetadata_Fence(__FILE__, __LINE__)
#define PMTest_isPersistent(addr, size) kC_createMetadata_Persist((addr), (size), __FILE__, __LINE__)
#define PMTest_isPersistedBefore(addrA, sizeA, addrB, sizeB) kC_createMetadata_Order((addrA), (sizeA), (addrB), (sizeB), __FILE__, __LINE__)
#define PMTest_exclude(addr, size) kC_createMetadata_Exclude((addr), (size), __FILE__, __LINE__)
#define PMTest_include(addr, size) kC_createMetadata_Include((addr), (size), __FILE__, __LINE__)
#define PMTest_transactionDelim() kC_createMetadata_TransactionDelim()
#define PMTest_ending() kC_createMetadata_Ending()


#endif // PMTEST_KERNEL_CODE && !PMTEST_USER_CODE

#endif // __KERNEL_MODULE__
