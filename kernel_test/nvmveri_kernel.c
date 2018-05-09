#include <linux/module.h>
#include <linux/kernel.h>

/********** START HEADER **********/
#include <linux/fs.h> // required for various structures related to files liked fops.
#include <asm/uaccess.h> // required for copy_from and copy_to user functions
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>


// nvmveri
#define MAX_OP_NAME_SIZE 50

#define MYPROTO NETLINK_USERSOCK
#define MYMGRP 21
#define MAX_MSG_LENGTH 1024

// file io
#define BUFFER_LEN 20


#define MIN(a,b) (((a)<(b))?(a):(b))

bool existVeriInstance;

typedef unsigned long long addr_t;

typedef enum {_OPINFO,
              _ASSIGN,
              _FLUSH,
              _COMMIT,
              _BARRIER,
              _FENCE,
              _PERSIST,
              _ORDER} MetadataType;

//const char MetadataTypeStr[][20] = {"_OPINFO", "_ASSIGN", "_FLUSH", "_COMMIT",
//                                   "_BARRIER", "_FENCE", "_PERSIST", "_ORDER"};

struct Metadata_OpInfo {
    //enum State {NONE, WORK, COMMIT, ABORT, FINAL};
    char opName[MAX_OP_NAME_SIZE]; // function name
    addr_t address;		    	   // address of object being operated
    int size;					   // size of object
};

struct Metadata_Assign {
    void *addr;
    size_t size;
};

struct Metadata_Flush {
    void *addr;
    size_t size;
};

struct Metadata_Commit {

};

struct Metadata_Barrier {

};

struct Metadata_Fence {

};

struct Metadata_Persist {
    void *addr;
    size_t size;
};

struct Metadata_Order {
    void *early_addr;
    size_t early_size;
    void *late_addr;
    size_t late_size;
};

struct Metadata {
    MetadataType type;
    union {
        struct Metadata_OpInfo op;
        struct Metadata_Assign assign;
        struct Metadata_Flush flush;
        struct Metadata_Commit commit;
        struct Metadata_Barrier barrier;
        struct Metadata_Fence fence;
        struct Metadata_Persist persist;
        struct Metadata_Order order;
    };
};


struct mydevice
{
    struct Metadata data[BUFFER_LEN];
    struct semaphore sem;
} nvmveri_dev;
/********** END HEADER **********/

// only for testing
#define TRACE_LEN 105

static int Major = 250;
static struct sock *nl_sk = NULL;
struct Metadata *trace;
int cur_idx;
dev_t dev_no,dev;

struct cdev *kernel_cdev;


static void send_to_user(int metadata_len)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    // the message if the length of metadata
    //char msg[sizeof(int)];
    //int msg_size = (msg) + 1;
    int msg_size = sizeof(int);
    int res;

    //pr_info("Creating skb.\n");
    skb = nlmsg_new(NLMSG_ALIGN(msg_size), GFP_KERNEL);
    if (!skb) {
        pr_err("Allocation failure.\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 1, NLMSG_DONE, msg_size + 1, 0);
    //strcpy(nlmsg_data(nlh), msg);
    memcpy(nlmsg_data(nlh), &metadata_len, sizeof(int));

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

    memcpy(nvmveri_dev.data, trace + cur_idx, size * sizeof(struct Metadata));
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
    int i, ret;

    kernel_cdev = cdev_alloc();
    kernel_cdev->ops = &fops;
    kernel_cdev->owner = THIS_MODULE;
    printk ("@ Inside init module\n");
    ret = alloc_chrdev_region( &dev_no , 0, 1, "chr_arr_dev");
    if (ret < 0) {
        printk("@ Major number allocation is failed\n");
        return ret;
    }

    dev = MKDEV(Major,0);
    sema_init(&nvmveri_dev.sem,1);
    ret = cdev_add( kernel_cdev,dev,1);
    if(ret < 0 ) {
        printk(KERN_INFO "@ Unable to allocate cdev");
        return ret;
    }

    nl_sk = netlink_kernel_create(&init_net, MYPROTO, NULL);
    if (!nl_sk) {
        printk("@ Error creating socket.\n");
        return -10;
    }

	existVeriInstance = 0;

    return 0;
}

int knvmveri_exit(void)
{
    netlink_kernel_release(nl_sk);
    kfree(trace);
    printk("@ Exiting hello module.\n");
	return 0;
}

module_init(knvmveri_init);
module_exit(knvmveri_exit);

MODULE_LICENSE("GPL");
