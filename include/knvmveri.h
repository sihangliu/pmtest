#ifndef __KNVMVERI_H__
#define __KNVMVERI_H__


#include <linux/module.h>
#include <linux/kernel.h>
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


/*
struct Vector {
	struct Metadata* arr_vector;
	int cur_size;
	int vector_max_size;
};

void initVector(struct Vector* vec) {
	vec->cur_size = 0;
	vec->vector_max_size = 200;
	vec->arr_vector = (struct Metadata*)
			kmalloc(vec->vector_max_size * sizeof(struct Metadata*), GFP_KERNEL);
}

void pushVector(struct Vector* vec, struct Metadata input) {
	if (vec->cur_size >= vec->vector_max_size) {
		vec->vector_max_size *= 10;
		vec->arr_vector = (struct Metadata*) krealloc(vec->arr_vector,
						vec->vector_max_size * sizeof(struct Metadata*), GFP_KERNEL);
	}
	vec->arr_vector[vec->cur_size] = input;
	++(vec->cur_size);
}

void deleteVector(struct Vector* vec) {
	kfree(vec->arr_vector);
}
*/

#endif
