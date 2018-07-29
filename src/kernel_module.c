#define NVMVERI_KERNEL_CODE
#include "nvmveri.hh"
#include "kernel_module.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/kmod.h>
#include <linux/sched.h>

int existVeriInstance;
static DECLARE_KFIFO_PTR(nvmveri_dev, Metadata);
int asleep = 0;
static DECLARE_WAIT_QUEUE_HEAD(kfifo_wq);

struct file_operations NVMVeriDeviceOps = {
	.owner = 	THIS_MODULE,
	.read = 	NVMVeriDeviceRead,
	.llseek = 	noop_llseek
};

// the user interface to read metadata from kfifo
ssize_t NVMVeriDeviceRead(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	unsigned int copied = 0;
	// printk(KERN_INFO "@ NVMVERI: start reading\n");

	ret = kfifo_to_user(&nvmveri_dev, buf, count, &copied);
	if (asleep != 0 && kfifo_avail(&nvmveri_dev) >= KFIFO_THRESHOLD_LEN) {
		wake_up_interruptible(&kfifo_wq);
		asleep = 0;
	}

	// printk(KERN_INFO "@ NVMVERI: end reading\n");

	if (copied == 0)
		return 1;
	else if (ret != 0)
		return ret;
	else
		return 0;
}

// the kernel interface to write metadata to kfifo
void NVMVeriFifoWrite(Metadata *input)
{
	if (kfifo_avail(&nvmveri_dev) > 0) {
		kfifo_put(&nvmveri_dev, *input);
	}
	if (kfifo_avail(&nvmveri_dev) == 0) {
		asleep = 1;
		wait_event_interruptible(
			kfifo_wq,
			kfifo_avail(&nvmveri_dev) >= KFIFO_THRESHOLD_LEN);
	}
}

int kC_initNVMVeriDevice(void)
{
	int ret;

	ret = kfifo_alloc(&nvmveri_dev, KFIFO_LEN, GFP_KERNEL);
	if (ret) {
		printk(KERN_ERR "@ NVMVERI: error kfifo_alloc\n");
		return ret;
	}

	remove_proc_entry(PROC_NAME, NULL);
	if (proc_create(PROC_NAME, 0, NULL, &NVMVeriDeviceOps) == NULL) {
		kfifo_free(&nvmveri_dev);
		return -ENOMEM;
	}

	asleep = 0;
	return 0;
}

int kC_exitNVMVeriDevice(void)
{
	remove_proc_entry(PROC_NAME, NULL);
	kfifo_free(&nvmveri_dev);
	return 0;
}


void kC_createMetadata_Assign(void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside assign %p %lu. \n", addr, size);
		input.type = _ASSIGN;

		//log("assign_aa\n");

		input.assign.addr = addr;
		input.assign.size = size;

		NVMVeriFifoWrite(&input);

		//printk(KERN_INFO "@ Complete assign %p %lu. \n", addr, size);
		
		// prevent overflow kernel FIFO
		//if (kfifo_size(&nvmveri_dev) > 0.7 * KFIFO_LEN) {
		//	Metadata suspend_signal;
		//	input.type = _SUSPEND;
		//	kfifo_put(&nvmveri_dev, suspend_signal);
		//}
	}
	else {
		//log("assign\n");
	}
}


void kC_createMetadata_Flush(void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside flush %p %lu. \n", addr, size);
		input.type = _FLUSH;

		//log("flush_aa\n");

		input.flush.addr = addr;
		input.flush.size = size;

		NVMVeriFifoWrite(&input);
	}
	else {
		//log("flush\n");
	}
}


void kC_createMetadata_Commit(void)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside commit. \n");
		input.type = _COMMIT;

		//log("commit_aa\n");

		NVMVeriFifoWrite(&input);
	}
	else {
		//log("commit\n");
	}
}


void kC_createMetadata_Barrier(void)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside barrier. \n");
		input.type = _BARRIER;

		//log("barrier_aa\n");

		NVMVeriFifoWrite(&input);
	}
	else {
		//log("barrier\n");
	}
}


void kC_createMetadata_Fence(void)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside fence. \n");
		input.type = _FENCE;

		//log("fence_aa\n");

		NVMVeriFifoWrite(&input);
	}
	else {
		//log("fence\n");
	}
}


void kC_createMetadata_Persist(void *addr, size_t size, const char file_name[], unsigned short line_num)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside persist %p %lu. \n", addr, size);
		input.type = _PERSIST;

		//log("persist_aa\n");

		input.persist.addr = addr;
		input.persist.size = size;
		input.persist.line_num = line_num;
		strncpy(input.persist.file_name, (file_name+strlen(file_name)-FILENAME_LEN), FILENAME_LEN);

		NVMVeriFifoWrite(&input);
	}
	else {
		//log("persist\n");
	}
}


void kC_createMetadata_Order(void *early_addr, size_t early_size, void *late_addr, size_t late_size, const char file_name[], unsigned short line_num)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside order %p %lu %p %lu. \n", early_addr, early_size, late_addr, late_size);
		input.type = _ORDER;
		input.order.early_addr = early_addr;
		input.order.early_size = early_size;
		input.order.late_addr = late_addr;
		input.order.late_size = late_size;
		input.order.line_num = line_num;
		strncpy(input.order.file_name, (file_name+strlen(file_name)-FILENAME_LEN), FILENAME_LEN);

		//log("order_aa\n");

		NVMVeriFifoWrite(&input);
	}
	else {
		//log("order\n");
	}
}


void kC_createMetadata_TransactionDelim(void)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside transactiondelim metadata. \n");
		input.type = _TRANSACTIONDELIM;

		//log("transactiondelim_aa\n");

		NVMVeriFifoWrite(&input);

		//printk(KERN_INFO "@ Complete transactiondelim metadata. \n");
	}
	else {
		//log("transactiondelim\n");
	}
}

void kC_createMetadata_Ending(void)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside ending metadata. \n");
		input.type = _ENDING;

		//log("ending_aa\n");

		NVMVeriFifoWrite(&input);

		//printk(KERN_INFO "@ Complete ending metadata. \n");
	}
	else {
		//log("ending\n");
	}
}

void kC_createMetadata_TransactionBegin(void)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside transactionbegin metadata. \n");
		input.type = _TRANSACTIONBEGIN;

		//log("transactionbegin_aa\n");

		NVMVeriFifoWrite(&input);

		//printk(KERN_INFO "@ Complete transactionbegin metadata. \n");
	}
	else {
		//log("transactionbegin\n");
	}
}

void kC_createMetadata_TransactionEnd(void)
{
	if (existVeriInstance) {
		Metadata input;
		//printk(KERN_INFO "@ Inside transactionend metadata. \n");
		input.type = _TRANSACTIONEND;

		//log("transactionend_aa\n");

		NVMVeriFifoWrite(&input);

		//printk(KERN_INFO "@ Complete transactionend metadata. \n");
	}
	else {
		//log("transactionend\n");
	}
}
