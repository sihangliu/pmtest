#define NVMVERI_KERNEL_CODE
#include "nvmveri.hh"
#include "kernel_module.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>

// NVMVeriDevice *nvmveri_dev;
//
// int nvmveri_major = NVMVERI_MAJOR;
// int nvmveri_minor = 0;
//
// void initVector(Vector* vec) {
// 	//(*vec) = (Vector *)kmalloc(sizeof(Vector), GFP_KERNEL);
// 	vec->cur_size = 0;
// 	vec->vector_max_size = 200;
// 	printk(KERN_INFO "@ Inside initVector.\n");
// 	vec->arr_vector = (Metadata *) kmalloc(vec->vector_max_size * sizeof(Metadata), GFP_KERNEL);
// 	printk(KERN_INFO "@ Complete initVector.\n");
// }
//
// void pushVector(Vector* vec, Metadata input) {
// 	printk(KERN_INFO "@ Inside pushVector, ptr=%p\n", vec);
// 	if (vec->cur_size >= vec->vector_max_size) {
// 		printk(KERN_INFO "@ Inside resize vector");
// 		vec->vector_max_size *= 10;
// 		vec->arr_vector = (Metadata *) krealloc(vec->arr_vector, vec->vector_max_size * sizeof(Metadata), GFP_KERNEL);
// 		printk(KERN_INFO "@ Complete resize vector");
// 	}
// 	vec->arr_vector[vec->cur_size] = input;
// 	++(vec->cur_size);
// }
//
// void deleteVector(Vector* vec) {
// 	printk(KERN_INFO "@ Inside delete vector. \n");
// 	kfree(vec->arr_vector);
// 	//kfree(vec);
// 	printk(KERN_INFO "@ Complete delete vector. \n");
// }
//
// int kC_initNVMVeriDevice(void)
// {
// 	int result, i;
// 	dev_t dev = 0;
//
// 	// request a dynamic major number unless preset value exists
// 	if (nvmveri_major != 0) {
// 		dev = MKDEV(nvmveri_major, nvmveri_minor);
// 		result = register_chrdev_region(dev, 1, "nvmveri_device");
// 	}
// 	else {
// 		result = alloc_chrdev_region(&dev, nvmveri_minor, 1, "nvmveri_device");
// 		nvmveri_major = MAJOR(dev);
// 	}
// 	if (result < 0) {
// 		printk(KERN_WARNING "@ NVMVERI: can't get major %d\n", nvmveri_major);
// 		return result;
// 	}
//
// 	// allocate space for NVMVeriDevice
// 	nvmveri_dev = kmalloc(sizeof(NVMVeriDevice), GFP_KERNEL);
// 	if (nvmveri_dev == NULL) {
// 		kC_exitNVMVeriDevice();
// 		return -1;
// 	}
//
// 	// initialization
// 	init_waitqueue_head(&nvmveri_dev->inq);
// 	init_waitqueue_head(&nvmveri_dev->outq);
// 	sema_init(&nvmveri_dev->sem, 1);
//
// 	// setup cdev
// 	int err;
// 	dev = MKDEV(nvmveri_major, nvmveri_minor);
//
// 	cdev_init(&nvmveri_dev->cdev, &NVMVeriDeviceOps);
// 	nvmveri_dev->cdev.owner = THIS_MODULE;
// 	nvmveri_dev->cdev.ops = &NVMVeriDeviceOps;
// 	err = cdev_add(&nvmveri_dev->cdev, dev, 1);
// 	/* Fail gracefully if need be */
// 	if (err)
// 		printk(KERN_NOTICE "@ NVMVERI: Error %d adding cdev", err);
//
// 	return 0;
// }
//
// int kC_exitNVMVeriDevice(void)
// {
// 	dev_t dev = MKDEV(nvmveri_major, nvmveri_minor);
// 	cdev_del(&nvmveri_dev->cdev);
// 	if (nvmveri_dev != NULL) {
// 		kfree(nvmveri_dev);
// 	}
// 	unregister_chrdev_region(dev, 1);
// 	return 0;
// }

int existVeriInstance;

static DECLARE_KFIFO_PTR(nvmveri_dev, Metadata);

ssize_t NVMVeriDeviceRead(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	unsigned int copied = 0;
	printk(KERN_INFO "@ NVMVERI: start reading\n");

	ret = kfifo_to_user(&nvmveri_dev, buf, count, &copied);

	printk(KERN_INFO "@ NVMVERI: end reading\n");

	if (copied == 0)
		return 1;
	else if (ret != 0)
		return ret;
	else
		return 0;
}

// ssize_t NVMVeriDeviceWrite(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
// {
// 	int ret;
// 	unsigned int copied;
//
// 	if (mutex_lock_interruptible(&write_lock))
// 		return -ERESTARTSYS;
// 	ret = kfifo_from_user(&nvmveri_dev, buf, count, &copied);
// 	mutex_unlock(&write_lock);
// 	return ret ? ret : copied;
// }

struct file_operations NVMVeriDeviceOps = {
	.owner = 	THIS_MODULE,
	.read = 	NVMVeriDeviceRead,
	//.write = 	NVMVeriDeviceWrite,
	.llseek = 	noop_llseek
};


int kC_initNVMVeriDevice(void)
{
	int ret;

	ret = kfifo_alloc(&nvmveri_dev, DEVICE_STORAGE_LEN, GFP_KERNEL);
	if (ret) {
		printk(KERN_ERR "@ NVMVERI: error kfifo_alloc\n");
		return ret;
	}

	remove_proc_entry(PROC_NAME, NULL);
	if (proc_create(PROC_NAME, 0, NULL, &NVMVeriDeviceOps) == NULL) {
		kfifo_free(&nvmveri_dev);
		return -ENOMEM;
	}
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
		printk(KERN_INFO "@ Inside assign metadata %p %lu. \n", addr, size);
		input.type = _ASSIGN;

		//log("assign_aa\n");

		input.assign.addr = addr;
		input.assign.size = size;
		kfifo_put(&nvmveri_dev, input);
		printk(KERN_INFO "@ Complete assign metadata %p %lu. \n", addr, size);
	}
	else {
		//log("assign\n");
	}
}

void kC_createMetadata_TransactionDelim(void)
{
	if (existVeriInstance) {
		Metadata input;
		printk(KERN_INFO "@ Inside transactiondelim metadata. \n");
		input.type = _TRANSACTIONDELIM;

		//log("transactiondelim_aa\n");

		kfifo_put(&nvmveri_dev, input);
		printk(KERN_INFO "@ Complete transactiondelim metadata. \n");
	}
	else {
		//log("transactiondelim\n");
	}
}

void kC_createMetadata_Ending(void)
{
	if (existVeriInstance) {
		Metadata input;
		printk(KERN_INFO "@ Inside ending metadata. \n");
		input.type = _ENDING;

		//log("ending_aa\n");

		kfifo_put(&nvmveri_dev, input);
		printk(KERN_INFO "@ Complete ending metadata. \n");
	}
	else {
		//log("ending\n");
	}
}
