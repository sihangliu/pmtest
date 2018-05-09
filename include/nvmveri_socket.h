#ifndef __NVMVERI_SOCKET_H__
#define __NVMVERI_SOCKET_H__

// nvmveri
#define MAX_OP_NAME_SIZE 50

// netlink
#define MYPROTO NETLINK_USERSOCK
#define MYMGRP 21
#define MAX_MSG_LENGTH 1024

// file io
#define MAX_BUFFER_LEN 20

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


struct Vector {
	void** arr_vector;
	int cur_size;
	int vector_max_size;
};

void initVector(struct Vector* vec) {
	vec->cur_size = 0;
	vec->vector_max_size = 200;
	vec->arr_vector = (void**) malloc(vec->vector_max_size * sizeof(void*));
}

void pushVector(struct Vector* vec, void* input) {
	if (vec->cur_size >= vec->vector_max_size) {
        printf("max_size=%d, cur_size=%d\n", vec->vector_max_size, vec->cur_size);
		vec->vector_max_size *= 10;
		vec->arr_vector = (void**) realloc(vec->arr_vector,
						vec->vector_max_size * sizeof(void*));
	}
	vec->arr_vector[vec->cur_size] = input;
	++(vec->cur_size);
}

void deleteVector(struct Vector* vec) {
	free(vec->arr_vector);
}


#endif
