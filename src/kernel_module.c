#define PMTEST_KERNEL_CODE
#include "pmtest.hh"
#include "kernel_module.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/kmod.h>
#include <linux/sched.h>

int existVeriInstance;
static DECLARE_KFIFO_PTR(pmtest_dev, Metadata);
int asleep = 0;
static DECLARE_WAIT_QUEUE_HEAD(kfifo_wq);

struct file_operations PMTestDeviceOps = {
	.owner = 	THIS_MODULE,
	.read = 	PMTestDeviceRead,
	.llseek = 	noop_llseek
};

// the user interface to read metadata from kfifo
ssize_t PMTestDeviceRead(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	unsigned int copied = 0;
	// printk(KERN_INFO "@ PMTEST: start reading\n");

	ret = kfifo_to_user(&pmtest_dev, buf, count, &copied);
	if (asleep != 0 && kfifo_avail(&pmtest_dev) >= KFIFO_THRESHOLD_LEN) {
		wake_up_interruptible(&kfifo_wq);
		asleep = 0;
	}

	// printk(KERN_INFO "@ PMTEST: end reading\n");

	if (copied == 0)
		return 1;
	else if (ret != 0)
		return ret;
	else
		return 0;
}

// the kernel interface to write metadata to kfifo
void PMTestFifoWrite(Metadata *input)
{
	if (kfifo_avail(&pmtest_dev) > 0) {
		kfifo_put(&pmtest_dev, *input);
	}
	if (kfifo_avail(&pmtest_dev) == 0) {
		asleep = 1;
		wait_event_interruptible(
			kfifo_wq,
			kfifo_avail(&pmtest_dev) >= KFIFO_THRESHOLD_LEN);
	}
}

int kC_initPMTestDevice(void)
{
	int ret;

	ret = kfifo_alloc(&pmtest_dev, KFIFO_LEN, GFP_KERNEL);
	if (ret) {
		printk(KERN_ERR "@ PMTEST: error kfifo_alloc\n");
		return ret;
	}

	remove_proc_entry(PROC_NAME, NULL);
	if (proc_create(PROC_NAME, 0, NULL, &PMTestDeviceOps) == NULL) {
		kfifo_free(&pmtest_dev);
		return -ENOMEM;
	}

	asleep = 0;
	return 0;
}

int kC_exitPMTestDevice(void)
{
	remove_proc_entry(PROC_NAME, NULL);
	kfifo_free(&pmtest_dev);
	return 0;
}


void kC_createMetadata_Assign(void *addr, size_t size, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside assign %p %lu. \n", addr, size);
		input.type = _ASSIGN;

		//log("assign_aa\n");

		input.addr = addr;
		input.size = size;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		PMTestFifoWrite(&input);

		//printk(KERN_INFO "@ Complete assign %p %lu. \n", addr, size);
		
		// prevent overflow kernel FIFO
		//if (kfifo_size(&pmtest_dev) > 0.7 * KFIFO_LEN) {
		//	Metadata suspend_signal;
		//	input.type = _SUSPEND;
		//	kfifo_put(&pmtest_dev, suspend_signal);
		//}
	}
}


void kC_createMetadata_Flush(void *addr, size_t size, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside flush %p %lu. \n", addr, size);
		input.type = _FLUSH;

		//log("flush_aa\n");

		input.addr = addr;
		input.size = size;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		PMTestFifoWrite(&input);
	}
}


void kC_createMetadata_Commit(const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside commit. \n");
		input.type = _COMMIT;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("commit_aa\n");

		PMTestFifoWrite(&input);
	}
}


void kC_createMetadata_Barrier(const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside barrier. \n");
		input.type = _BARRIER;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("barrier_aa\n");

		PMTestFifoWrite(&input);
	}
}


void kC_createMetadata_Fence(const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside fence. \n");
		input.type = _FENCE;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("fence_aa\n");

		PMTestFifoWrite(&input);
	}
}


void kC_createMetadata_Persist(void *addr, size_t size, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside persist %p %lu. \n", addr, size);
		input.type = _PERSIST;
		
		//log("persist_aa\n");

		input.addr = addr;
		input.size = size;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		PMTestFifoWrite(&input);
	}
}


void kC_createMetadata_Order(void *addr, size_t size, void *addr_late, size_t size_late, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside order %p %lu %p %lu. \n", early_addr, early_size, late_addr, late_size);
		input.type = _ORDER;
		input.addr = addr;
		input.size = size;
		input.addr_late = addr_late;
		input.size_late = size_late;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("order_aa\n");

		PMTestFifoWrite(&input);
	}
}


void kC_createMetadata_TransactionDelim()
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside transactiondelim metadata. \n");
		input.type = _TRANSACTIONDELIM;

		//log("transactiondelim_aa\n");

		PMTestFifoWrite(&input);

		//printk(KERN_INFO "@ Complete transactiondelim metadata. \n");
	}
}

void kC_createMetadata_Ending()
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside ending metadata. \n");
		input.type = _ENDING;

		//log("ending_aa\n");

		PMTestFifoWrite(&input);

		//printk(KERN_INFO "@ Complete ending metadata. \n");
	}
}

void kC_createMetadata_TransactionBegin(const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside transactionbegin metadata. \n");
		input.type = _TRANSACTIONBEGIN;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("transactionbegin_aa\n");

		PMTestFifoWrite(&input);

		//printk(KERN_INFO "@ Complete transactionbegin metadata. \n");
	}
}

void kC_createMetadata_TransactionEnd(const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside transactionend metadata. \n");
		input.type = _TRANSACTIONEND;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("transactionend_aa\n");

		PMTestFifoWrite(&input);

		//printk(KERN_INFO "@ Complete transactionend metadata. \n");
	}
}

void kC_createMetadata_TransactionAdd(void *addr, size_t size, const char file_name[], unsigned int line_num)
{
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside transactionadd metadata. \n");
		input.type = _TRANSACTIONADD;
		input.addr = addr;
		input.size = size;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("transactionadd_aa\n");

		PMTestFifoWrite(&input);

		//printk(KERN_INFO "@ Complete transactionadd metadata. \n");
	}
}

void kC_createMetadata_Exclude(void *addr, size_t size, const char file_name[], unsigned int line_num)
{
#ifdef PMTEST_EXCLUDE
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside exclude metadata. \n");
		input.type = _EXCLUDE;
		input.addr = addr;
		input.size = size;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("exclude_aa\n");

		PMTestFifoWrite(&input);

		//printk(KERN_INFO "@ Complete transactionadd metadata. \n");
	}
#endif // PMTEST_EXCLUDE
}

void kC_createMetadata_Include(void *addr, size_t size, const char file_name[], unsigned int line_num)
{
#ifdef PMTEST_EXCLUDE
	if (existVeriInstance) {
		Metadata input;
		int file_offset;
		//printk(KERN_INFO "@ Inside include metadata. \n");
		input.type = _INCLUDE;
		input.addr = addr;
		input.size = size;
		input.line_num = line_num;
		file_offset = strlen(file_name) - FILENAME_LEN;
		strncpy(
			input.file_name,
			file_name + (file_offset>0 ? file_offset : 0),
			FILENAME_LEN);

		//log("include_aa\n");

		PMTestFifoWrite(&input);

		//printk(KERN_INFO "@ Complete include metadata. \n");
	}
#endif // PMTEST_EXCLUDE
}
