#include "nvmveri_kernel.h"

MetadataStatus metadataStatus;

NVMVeriDevice nvmveri_dev;

int existVeriInstance;

int TRACE_LEN;

static int Major;
// static int Major = 250;
static struct sock *nl_sk = NULL;
int cur_idx;
dev_t dev_no, dev;

struct cdev *kernel_cdev;

Vector *metadataVectorPtr;

void initVector(Vector* vec) {
    //(*vec) = (Vector *)kmalloc(sizeof(Vector), GFP_KERNEL);
    vec->cur_size = 0;
    vec->vector_max_size = 200;
    printk(KERN_INFO "@ Inside initVector.\n");
    vec->arr_vector = (Metadata *)
            kmalloc(vec->vector_max_size * sizeof(Metadata), GFP_KERNEL);
    printk(KERN_INFO "@ Complete initVector.\n");
}

void pushVector(Vector* vec, Metadata input) {
    printk(KERN_INFO "@ Inside pushVector, ptr=%p\n", vec);
    if (vec->cur_size >= vec->vector_max_size) {
        printk(KERN_INFO "@ Inside resize vector");
        vec->vector_max_size *= 10;
        vec->arr_vector = (Metadata *) krealloc(vec->arr_vector,
                        vec->vector_max_size * sizeof(Metadata), GFP_KERNEL);
        printk(KERN_INFO "@ Complete resize vector");
    }
    vec->arr_vector[vec->cur_size] = input;
    ++(vec->cur_size);
}

void deleteVector(Vector* vec) {
    printk(KERN_INFO "@ Inside delete vector. \n");
    kfree(vec->arr_vector);
    //kfree(vec);
    printk(KERN_INFO "@ Complete delete vector. \n");
}

void kC_createMetadataVector(void)
{
    printk(KERN_INFO "@ Inside create metadata. \n");
    metadataVectorPtr = (Vector*)kmalloc(sizeof(Vector), GFP_KERNEL);
    initVector(metadataVectorPtr);
    printk(KERN_INFO "@ Complete create metadata. \n");
}


void kC_deleteMetadataVector(void)
{
    printk(KERN_INFO "@ Inside delete metadata. \n");
    deleteVector(metadataVectorPtr);
    kfree(metadataVectorPtr);
    printk(KERN_INFO "@ Complete delete metadata. \n");
}


void kC_createMetadata_Assign(void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata input;
        printk(KERN_INFO "@ Inside assign metadata. \n");
		input.type = _ASSIGN;

		//log("assign_aa\n");

		input.assign.addr = addr;
		input.assign.size = size;
		pushVector(metadataVectorPtr, input);
        printk(KERN_INFO "@ Complete assign metadata. \n");
	}
	else {
		//log("assign\n");
	}
}


void send_to_user(int terminateSignal)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    // the message if the length of metadata
    //char msg[sizeof(int)];
    //int msg_size = (msg) + 1;
    int msg_size = sizeof(int);
    int res;

    if (terminateSignal == 0) {
        TRACE_LEN = metadataVectorPtr->cur_size;
    }
    else {
        TRACE_LEN = -1;
    }

    //pr_info("Creating skb.\n");
    skb = nlmsg_new(NLMSG_ALIGN(msg_size), GFP_KERNEL);
    if (!skb) {
        pr_err("Allocation failure.\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 1, NLMSG_DONE, msg_size + 1, 0);
    //strcpy(nlmsg_data(nlh), msg);
    memcpy(nlmsg_data(nlh), &TRACE_LEN, sizeof(int));

    // reset the index of output buffer before signaling the user
    cur_idx = 0;
    metadataStatus = _WRITTEN;

    //pr_info("Sending skb.\n");
    while (metadataStatus != _READING) {
        res = nlmsg_multicast(nl_sk, skb, 0, MYMGRP, GFP_KERNEL);
        //udelay(1);
    }
    if (res < 0)
        pr_info("nlmsg_multicast() error: %d\n", res);
    //else
    //    pr_info("Success.\n");


}


inline void wait_until_read(void)
{
    // a spinlock to ensure that all data are read
    while (metadataStatus != _RELEASED);
}


ssize_t NVMDeviceRead(struct file *filp, char *buff, size_t count, loff_t *offp)
{
    unsigned long ret;
    int size;
    //printk(KERN_INFO "@ Inside read \n");
    if (cur_idx > TRACE_LEN)
        return 0;
    size = MIN(BUFFER_LEN, TRACE_LEN - cur_idx);
    printk(KERN_INFO "@ count=%lu\n", count);
    metadataStatus = _READING;
    memcpy(nvmveri_dev.data, metadataVectorPtr->arr_vector + cur_idx, size * sizeof(Metadata));
    cur_idx += size;

    ret = copy_to_user(buff, nvmveri_dev.data, count);

	return ret;
}


int NVMDeviceOpen(struct inode *inode, struct file *filp)
{
    //printk(KERN_INFO "@ Inside open \n");
    if(down_interruptible(&nvmveri_dev.sem)) { //lock
        //printk(KERN_INFO "@ could not hold semaphore");
        return -1;
    }
    return 0;
}

int NVMDeviceRelease(struct inode *inode, struct file *filp)
{
  //printk(KERN_INFO "@ Inside close \n");
  //printk(KERN_INFO "@ Releasing semaphore");
  up(&nvmveri_dev.sem);  //unlock
  metadataStatus = _RELEASED;
  return 0;
}

struct file_operations fops = {
    read:  NVMDeviceRead,
    //write:  NVMDeviceWrite,
    open:   NVMDeviceOpen,
    release: NVMDeviceRelease
};


int knvmveri_init(void)
{
    int ret;

    kernel_cdev = cdev_alloc();
    kernel_cdev->ops = &fops;
    kernel_cdev->owner = THIS_MODULE;
    printk ("@ Inside knvmveri init module\n");
    ret = alloc_chrdev_region(&dev_no, 0, 1, "chr_arr_dev");
    if (ret < 0) {
        printk("@ Major number allocation is failed\n");
        return ret;
    }

    Major = MAJOR(dev_no);
    printk ("@ The major number for nvmveri_dev is %d\n", Major);
    dev = MKDEV(Major, 0);
    sema_init(&nvmveri_dev.sem, 1);
    ret = cdev_add( kernel_cdev,dev, 1);
    if(ret < 0) {
        printk(KERN_INFO "@ Unable to allocate cdev");
        return ret;
    }

    nl_sk = netlink_kernel_create(&init_net, MYPROTO, NULL);
    if (!nl_sk) {
        printk("@ Error creating socket.\n");
        return -10;
    }

	existVeriInstance = 0;
    metadataStatus = _INIT;

    return 0;
}

int knvmveri_exit(void)
{
    netlink_kernel_release(nl_sk);
	cdev_del(kernel_cdev);
    //kfree(metadataVectorPtr->arr_vector);
    printk("@ Exiting knvmveri module.\n");
	return 0;
}
