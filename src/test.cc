#include "nvmveri.hh"
#include "nvmveri.h"

#include <utility>

void transaction()
{
	char arr[100];

	void *metadataVectorPtr;
	Timer timer;
	void *p;

	const int LOOP_CNT = 3;

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
}

void fastvector()
{
	FastVector<std::pair<int, double>> fv;
	FastVector<std::pair<int, double>> fv2;
	for (int i = 0; i < 10; i++) {
		fv.push_back(std::make_pair(i, (double)i / 97.0));
	}
	for (int i = 0; i < 998; i++) {
		std::pair<int, double> tmp = std::make_pair(i, (double)i / 97.0);
		printf("%d %f\n", tmp.first, tmp.second);
		fv2.push_back(tmp);
	}

	for (int i = 0; i < fv.size(); i++) {
		printf("%d %f\n", fv[i].first, fv[i].second);
	}

	for (int i = 0; i < fv.size(); i++) {
		printf("%d %f\n", fv2[i].first, fv2[i].second);
	}

	fv.append(fv2);

	for (int i = 0; i < fv.size(); i++) {
		printf("%d %f\n", fv[i].first, fv[i].second);
	}

	for (int i = 0; i < 200; i++)
		fv.append(fv2);

}
