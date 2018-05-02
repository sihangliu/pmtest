#ifndef __NVMVERI_H__
#define __NVMVERI_H__


enum MetadataType {_OPINFO, _ASSIGN, _FLUSH, _COMMIT,
                        _BARRIER, _FENCE, _PERSIST, _ORDER};
const char MetadataTypeStr[][20] = {"_OPINFO", "_ASSIGN", "_FLUSH", "_COMMIT",
                                    "_BARRIER", "_FENCE", "_PERSIST", "_ORDER"};

struct Metadata_OpInfo {
	enum State {NONE, WORK, COMMIT, ABORT, FINAL};
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
		Metadata_OpInfo op;
		Metadata_Assign assign;
		Metadata_Flush flush;
		Metadata_Commit commit;
		Metadata_Barrier barrier;
		Metadata_Fence fence;
		Metadata_Persist persist;
		Metadata_Order order;
	};
};

class FastVector {
public:
	FastVector();
	~FastVector();
	void push_back(metadata*);
	int size();
	//void insert(int pos, FastVector<T> &inputFV, int begin, int end);
	//void append(FastVector &input);
	const (metadata*)& operator[](int idx) const { return arr_vector[idx]; }
private:
	metadata** arr_vector;
	int cur_size;
	int vector_max_size;
};


FastVector::FastVector()
{
	cur_size = 0;
	vector_max_size = 200;
	arr_vector = (metadata**) malloc(vector_max_size * sizeof(metadata*));
}

FastVector::~FastVector()
{
	free(arr_vector);
}

void FastVector::push_back(metadata* input)
{
	if (cur_size >= vector_max_size) {
		vector_max_size *= 10;
		arr_vector = (metadata**) realloc(arr_vector, vector_max_size * sizeof(metadata*));
	}
	arr_vector[cur_size] = input;
	++cur_size;
}

void FastVector::append(FastVector &input)
{
	if (input.size() + cur_size >= vector_max_size) {
		vector_max_size = (cur_size + input.size()) * 10;
		arr_vector = (metadata**) realloc(arr_vector, vector_max_size * sizeof(metadata*));
	}

	memcpy(arr_vector + cur_size, input.arr_vector, input.size() * sizeof(metadata*));
	cur_size += input.size();
}

int FastVector::size()
{
	return cur_size;
}

//char size
//enum TxTypeFlag {NEW_TX, OLD_TX, TAIL_OLD_TX};


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
