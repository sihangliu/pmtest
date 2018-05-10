#include "nvmveri_kernel.h"

MetadataStatus metadataStatus;

mydevice nvmveri_dev;

int existVeriInstance;

int TRACE_LEN;

static int Major = 250;
static struct sock *nl_sk = NULL;
int cur_idx;
dev_t dev_no, dev;

struct cdev *kernel_cdev;

Vector *metadataVectorPtr;

void initVector(Vector* vec) {
    vec->cur_size = 0;
    vec->vector_max_size = 200;
    vec->arr_vector = (Metadata *)
            kmalloc(vec->vector_max_size * sizeof(Metadata *), GFP_KERNEL);
}

void pushVector(Vector* vec, Metadata input) {
    if (vec->cur_size >= vec->vector_max_size) {
        vec->vector_max_size *= 10;
        vec->arr_vector = (Metadata *) krealloc(vec->arr_vector,
                        vec->vector_max_size * sizeof(Metadata *), GFP_KERNEL);
    }
    vec->arr_vector[vec->cur_size] = input;
    ++(vec->cur_size);
}

void deleteVector(Vector* vec) {
    kfree(vec->arr_vector);
}

void kC_createMetadataVector(void)
{
    initVector(metadataVectorPtr);
	//vec->reserve(100);
}


void kC_deleteMetadataVector(void)
{
    deleteVector(metadataVectorPtr);
}


void kC_createMetadata_Assign(void *addr, size_t size)
{
	if (existVeriInstance) {
		Metadata input;
		input.type = _ASSIGN;

		//log("assign_aa\n");

		input.assign.addr = addr;
		input.assign.size = size;
		pushVector(metadataVectorPtr, input);
	}
	else {
		//log("assign\n");
	}
}


void send_to_user(void)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    // the message if the length of metadata
    //char msg[sizeof(int)];
    //int msg_size = (msg) + 1;
    int msg_size = sizeof(int);
    int res;
    TRACE_LEN = metadataVectorPtr->cur_size;

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
    res = nlmsg_multicast(nl_sk, skb, 0, MYMGRP, GFP_KERNEL);
    if (res < 0)
        pr_info("nlmsg_multicast() error: %d\n", res);
    //else
    //    pr_info("Success.\n");

}


ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
    unsigned long ret;
    int size;
    //printk(KERN_INFO "@ Inside read \n");
    if (cur_idx > TRACE_LEN)
        return 0;
    size = MIN(BUFFER_LEN, TRACE_LEN - cur_idx);
    printk(KERN_INFO "@ count=%lu\n", count);

    memcpy(nvmveri_dev.data, metadataVectorPtr->arr_vector + cur_idx, size * sizeof(Metadata));
    cur_idx += size;

    ret = copy_to_user(buff, nvmveri_dev.data, count);

	return ret;
}


int open(struct inode *inode, struct file *filp)
{
    //printk(KERN_INFO "@ Inside open \n");
    if(down_interruptible(&nvmveri_dev.sem)) { //lock
        //printk(KERN_INFO "@ could not hold semaphore");
        return -1;
    }
    return 0;
}

int release(struct inode *inode, struct file *filp)
{
  //printk(KERN_INFO "@ Inside close \n");
  //printk(KERN_INFO "@ Releasing semaphore");
  up(&nvmveri_dev.sem);  //unlock
  metadataStatus = _RELEASED;
  return 0;
}

struct file_operations fops = {
    read:  read,
    //write:  write,
    open:   open,
    release: release
};


int knvmveri_init(void)
{
    int ret;

    kernel_cdev = cdev_alloc();
    kernel_cdev->ops = &fops;
    kernel_cdev->owner = THIS_MODULE;
    printk ("@ Inside init module\n");
    ret = alloc_chrdev_region(&dev_no, 0, 1, "chr_arr_dev");
    if (ret < 0) {
        printk("@ Major number allocation is failed\n");
        return ret;
    }

    dev = MKDEV(Major,0);
    sema_init(&nvmveri_dev.sem,1);
    ret = cdev_add( kernel_cdev,dev,1);
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
    kfree(metadataVectorPtr->arr_vector);
    printk("@ Exiting hello module.\n");
	return 0;
}
