#ifndef __NVMVERI_H__
#define __NVMVERI_H__

#define MAX_THREAD_POOL_SIZE 5
#define MAX_OP_NAME_SIZE 50

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
	struct Metadata** arr_vector;
	int cur_size;
	int vector_max_size;
};

void initVector(struct Vector* vec) {
	vec->cur_size = 0;
	vec->vector_max_size = 200;
	vec->arr_vector = (struct Metadata**) 
			malloc(vec->vector_max_size * sizeof(struct Metadata**));
}

void pushVector(struct Vector* vec, struct Metadata* input) {
	if (vec->cur_size >= vec->vector_max_size) {
		vec->vector_max_size *= 10;
		vec->arr_vector = (struct Metadata**) realloc(vec->arr_vector, 
						vec->vector_max_size * sizeof(struct Metadata**));
	}
	vec->arr_vector[vec->cur_size] = input;
	++(vec->cur_size);
}

void deleteVector(struct Vector* vec) {
	free(vec->arr_vector);
}

void *C_createVeriInstance(void);
void C_deleteVeriInstance(void *);

void C_execVeri(void *, void *);
void C_getVeri(void *, void *);


void *C_createMetadataVector(void);
void C_deleteMetadataVector(void *);
void C_createMetadata_OpInfo(void *, char *, void *, size_t);
void C_createMetadata_Assign(void *, void *, size_t);
void C_createMetadata_Flush(void *, void *, size_t);
void C_createMetadata_Commit(void *);
void C_createMetadata_Barrier(void *);
void C_createMetadata_Fence(void *);
void C_createMetadata_Persist(void *, void *, size_t);
void C_createMetadata_Order(void *, void *, size_t, void *, size_t);

extern void* metadataPtr;
extern int existVeriInstance;

#endif
