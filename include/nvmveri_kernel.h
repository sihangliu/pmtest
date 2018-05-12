#ifndef __KNVMVERI_H__
#define __KNVMVERI_H__

// nvmveri
#define MAX_OP_NAME_SIZE 50

#define MYPROTO NETLINK_USERSOCK
#define MYMGRP 21
#define MAX_MSG_LENGTH 1024

// file io
#define BUFFER_LEN 20

#define MIN(a,b) (((a)<(b))?(a):(b))

#include <linux/fs.h> // required for various structures related to files liked fops.

typedef unsigned long long addr_t;

typedef enum {_INIT,
              _WRITTEN,
              _READING,
              _RELEASED} MetadataStatus;

extern MetadataStatus metadataStatus;

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

typedef struct Metadata_OpInfo {
    //enum State {NONE, WORK, COMMIT, ABORT, FINAL};
    char opName[MAX_OP_NAME_SIZE]; // function name
    addr_t address;		    	   // address of object being operated
    int size;					   // size of object
} Metadata_OpInfo;

typedef struct Metadata_Assign {
    void *addr;
    size_t size;
} Metadata_Assign;

typedef struct Metadata_Flush {
    void *addr;
    size_t size;
} Metadata_Flush;

typedef struct Metadata_Commit {

} Metadata_Commit;

typedef struct Metadata_Barrier {

} Metadata_Barrier;

typedef struct Metadata_Fence {

} Metadata_Fence;

typedef struct Metadata_Persist {
    void *addr;
    size_t size;
}  Metadata_Persist;

typedef struct Metadata_Order {
    void *early_addr;
    size_t early_size;
    void *late_addr;
    size_t late_size;
} Metadata_Order;

typedef struct Metadata {
    MetadataType type;
    union {
        Metadata_OpInfo op;
        Metadata_Assign assign;
        Metadata_Flush flush;
        Metadata_Commit commit;
        Metadata_Barrier barrier;
        Metadata_Fence fence;
        Metadata_Persist persist;
        Metadata_Order order;
    };
} Metadata;

#ifndef CLIENT_CODE // if it is a kernel module
    #include <linux/module.h>
    #include <linux/kernel.h>

    #include <asm/uaccess.h> // required for copy_from and copy_to user functions
    #include <linux/semaphore.h>
    #include <linux/cdev.h>
    #include <linux/slab.h>
    #include <linux/netlink.h>
    #include <net/netlink.h>
    #include <net/net_namespace.h>
    //#include <asm/current.h>

    typedef struct NVMVeriDevice
    {
        Metadata data[BUFFER_LEN];
        struct semaphore sem;
    } NVMVeriDevice;

    extern NVMVeriDevice nvmveri_dev;

    typedef struct Vector {
    	struct Metadata* arr_vector;
    	int cur_size;
    	int vector_max_size;
    } Vector;

    void initVector(Vector *);
    void pushVector(Vector *, Metadata);
    void deleteVector(Vector *);

    extern int existVeriInstance;

    extern int TRACE_LEN;

    extern int cur_idx;
    extern dev_t dev_no, dev;

    extern struct cdev *kernel_cdev;

    extern Vector *metadataVectorPtr;

    void kC_createMetadataVector(void);
    void kC_deleteMetadataVector(void);
    void kC_createMetadata_Assign(void *, size_t);

    void send_to_user(int);
    void wait_until_read(void);
    int knvmveri_init(void);
    int knvmveri_exit(void);


#endif


#endif
