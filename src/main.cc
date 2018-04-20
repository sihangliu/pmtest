#include "nvmveri.hh"
#include "nvmveri.h"

const int LOOP_CNT = 3;

/*
int main()
{
	NVMVeri nvm_instance;

	vector<Metadata *> m;
	vector<VeriResult> r;
	Metadata *dat;
	dat = new Metadata;

	Timer timer;

	timer.startTimer();

	for (int i = 0; i < LOOP_CNT; i++) {
		//printf("%d\n", i);
		m.push_back(dat);
		nvm_instance.execVeri(&m);
	}
	std::cout << std::endl;

	nvm_instance.getVeri(r);

	delete dat;
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n", LOOP_CNT, timer.getTime());

	return 0;
}
*/
int main()
{
	char arr[100];

	void *metadataVectorPtr;
	Timer timer;
	void *p;

/****************************test1*****************************/
	timer.startTimer();

	p = C_createVeriInstance();
	metadataVectorPtr = C_createMetadataVector();
	metadataPtr = metadataVectorPtr;
	existVeriInstance = 1;

	C_createMetadata_Assign(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Fence(metadataPtr);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[4]), 4);
	C_createMetadata_Flush(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Persist(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Order(metadataPtr, (void *)(&arr[0]), 4, (void *)(&arr[0]), 4);


	C_execVeri(p, metadataPtr);

	existVeriInstance = 0;

	C_getVeri(p, (void *)(0));
	C_deleteMetadataVector(metadataVectorPtr);


	C_deleteVeriInstance(p);
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test1*****************************/


/****************************test2*****************************/
	timer.startTimer();

	p = C_createVeriInstance();
	metadataVectorPtr = C_createMetadataVector();
	metadataPtr = metadataVectorPtr;
	existVeriInstance = 1;

	C_createMetadata_Assign(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Fence(metadataPtr);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[4]), 4);
	C_createMetadata_Flush(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Fence(metadataPtr);
	C_createMetadata_Persist(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Order(metadataPtr, (void *)(&arr[0]), 4, (void *)(&arr[4]), 4);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[0]), 4);

	C_execVeri(p, metadataPtr);

	existVeriInstance = 0;

	C_getVeri(p, (void *)(0));
	C_deleteMetadataVector(metadataVectorPtr);


	C_deleteVeriInstance(p);
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test2*****************************/


/****************************test3*****************************/
	timer.startTimer();

	p = C_createVeriInstance();
	metadataVectorPtr = C_createMetadataVector();
	metadataPtr = metadataVectorPtr;
	existVeriInstance = 1;

	C_createMetadata_Assign(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Fence(metadataPtr);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[4]), 4);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Flush(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Fence(metadataPtr);
	C_createMetadata_Persist(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Order(metadataPtr, (void *)(&arr[0]), 4, (void *)(&arr[4]), 4);


	C_execVeri(p, metadataPtr);

	existVeriInstance = 0;

	C_getVeri(p, (void *)(0));
	C_deleteMetadataVector(metadataVectorPtr);


	C_deleteVeriInstance(p);
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test3*****************************/


/****************************test4*****************************/
	timer.startTimer();

	p = C_createVeriInstance();
	metadataVectorPtr = C_createMetadataVector();
	metadataPtr = metadataVectorPtr;
	existVeriInstance = 1;

	C_createMetadata_Assign(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[4]), 4);
	C_createMetadata_Fence(metadataPtr);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[4]), 4);
	C_createMetadata_Persist(metadataPtr, (void *)(&arr[4]), 4);
	C_createMetadata_Order(metadataPtr, (void *)(&arr[0]), 4, (void *)(&arr[4]), 4);


	C_execVeri(p, metadataPtr);

	existVeriInstance = 0;

	C_getVeri(p, (void *)(0));
	C_deleteMetadataVector(metadataVectorPtr);


	C_deleteVeriInstance(p);
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test4*****************************/

/****************************test5*****************************/
	timer.startTimer();

	p = C_createVeriInstance();
	metadataVectorPtr = C_createMetadataVector();
	metadataPtr = metadataVectorPtr;
	existVeriInstance = 1;

	C_createMetadata_Assign(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Fence(metadataPtr);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[4]), 4);
	C_createMetadata_Assign(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Persist(metadataPtr, (void *)(&arr[0]), 4);
	C_createMetadata_Order(metadataPtr, (void *)(&arr[4]), 4, (void *)(&arr[0]), 4);


	C_execVeri(p, metadataPtr);

	existVeriInstance = 0;

	C_getVeri(p, (void *)(0));
	C_deleteMetadataVector(metadataVectorPtr);


	C_deleteVeriInstance(p);
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test5*****************************/
	return 0;
}
