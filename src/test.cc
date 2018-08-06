#include "nvmveri.hh"
#include "nvmveri.h"

#include <utility>

void transaction()
{
	char arr[100];

	Timer timer;
	void *p = NULL;

	const int LOOP_CNT = 3;


/****************************test1*****************************/
	timer.startTimer();

	NVTest_init(p, 2);
	NVTest_START;

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_isPersistent((void *)(&arr[0]), 4);
	NVTest_isPersistent((void *)(&arr[0]), 0);
	NVTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[0]), 4);


	NVTest_sendTrace(p);

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_isPersistent((void *)(&arr[0]), 4);
	NVTest_isPersistent((void *)(&arr[0]), 0);
	NVTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[0]), 4);


	NVTest_sendTrace(p);
	NVTest_sendTrace(p);

	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);

	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test1*****************************/


/****************************test2*****************************/
	timer.startTimer();

	NVTest_init(p, 1);
	NVTest_START;

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_isPersistent((void *)(&arr[0]), 4);
	NVTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[4]), 4);
	NVTest_assign((void *)(&arr[0]), 4);

	NVTest_sendTrace(p);
	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);

	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test2*****************************/


/****************************test3*****************************/
	timer.startTimer();

	NVTest_init(p, 1);
	NVTest_START;

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_isPersistent((void *)(&arr[0]), 4);
	NVTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[4]), 4);


	NVTest_sendTrace(p);
	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);

	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test3*****************************/


/****************************test4*****************************/
	timer.startTimer();

	NVTest_init(p, 1);
	NVTest_START;

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_isPersistent((void *)(&arr[4]), 4);
	NVTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[4]), 4);


	NVTest_sendTrace(p);
	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test4*****************************/

/****************************test5*****************************/
	timer.startTimer();

	NVTest_init(p, 1);
	NVTest_START;

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_isPersistent((void *)(&arr[0]), 4);
	NVTest_isPersistedBefore((void *)(&arr[4]), 4, (void *)(&arr[0]), 4);


	NVTest_sendTrace(p);
	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test5*****************************/

/****************************test6*****************************/
	timer.startTimer();

	NVTest_init(p, 1);
	NVTest_START;

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[4]), 4);


	NVTest_sendTrace(p);
	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test6*****************************/

/****************************test7*****************************/
	timer.startTimer();

	NVTest_init(p, 1);
	NVTest_START;

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_isPersistent((void *)(&arr[0]), 4);

	NVTest_sendTrace(p);
	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test7*****************************/

/****************************test8*****************************/
	timer.startTimer();

	NVTest_init(p, 1);
	NVTest_START;

	NVTest_assign((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_assign((void *)(&arr[4]), 4);
	NVTest_flush((void *)(&arr[0]), 4);
	NVTest_fence();
	NVTest_isPersistedBefore((void *)(&arr[4]), 4, (void *)(&arr[0]), 4);


	NVTest_sendTrace(p);
	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test8*****************************/

/****************************test9*****************************/
	timer.startTimer();

	NVTest_init(p, 1);
	NVTest_START;

	NVTest_assign((void *)(128), 8);
	NVTest_assign((void *)(256), 8);
	NVTest_flush((void *)(128), 8);
	NVTest_fence();
	NVTest_isPersistent((void *)(128), 8);
	NVTest_isPersistent((void *)(256), 8);

	NVTest_sendTrace(p);
	NVTest_END;
	NVTest_getResult(p);
	NVTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test9*****************************/
}

void test_icl()
{
	interval_set<size_t> persist;
	interval_map<size_t, int, partial_enricher, std::less, inplace_assign> order;
	discrete_interval<size_t> addrinterval = interval<size_t>::right_open(10, 20);
	persist += addrinterval;
	addrinterval = interval<size_t>::right_open(30, 40);
	persist += addrinterval;
	addrinterval = interval<size_t>::right_open(50, 60);
	persist += addrinterval;
	addrinterval = interval<size_t>::right_open(45, 55);
	auto it = persist.find(addrinterval);
	
	if (!contains(*it, addrinterval)) {
		addrinterval = (lower_less(addrinterval, *it) ? right_subtract(addrinterval, *it) : left_subtract(addrinterval, *it)
		);
	}
	if (it == persist.end() || !contains(*it, addrinterval)) {
		std::cerr << addrinterval << " is not TransactionAdded before modified." << std::endl;
	}
	// addrinterval -= (*it);
	// std::cout << addrinterval << std::endl;


	// order += make_pair(addrinterval, 0);

	// for (auto it = order.begin(); it != order.end(); it++)
	// 	std::cout << it->first << " " << it->second << std::endl;
	// addrinterval = interval<size_t>::right_open(15, 30);
	// order += make_pair(addrinterval, -1);
	// for (auto it = order.begin(); it != order.end(); it++)
	// 	std::cout << it->first << " " << it->second << std::endl;
	// order += make_pair(addrinterval, 2);
	// for (auto it = order.begin(); it != order.end(); it++)
	// 	std::cout << it->first << " " << it->second << std::endl;
}


void fastvector()
{
	FastVector<std::pair<int, double>> fv;
	FastVector<std::pair<int, double>> fv2;
	for (int i = 0; i < 10; i++) {
		fv.push_back(std::pair<int, double>(i, (double)i / 97.0));
	}


	for (int i = 0; i < 300; i++) {
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

	printf("fv size = %d\n", fv.size());
	printf("fv last item is %d %f\n", fv[fv.size()-1].first, fv[fv.size()-1].second);


	for (int i = 0; i < fv.size(); i++) {
		printf("%d %f\n", fv[i].first, fv[i].second);
	}


	for (int i = 0; i < 200; i++)
		fv.append(fv2);

}

void tx_wrapper()
{
	char arr[100];

	void *metadataVectorPtr;
	Timer timer;
	void *p;

	const int LOOP_CNT = 3;

	timer.startTimer();

	p = C_createVeriInstance();
	metadataVectorPtr = C_createMetadataVector();
	metadataPtr = metadataVectorPtr;
	existVeriInstance = 1;

	struct fuc{
		int a;
		int b;
		int c;
	} fc;


	for (int i = 0; i < 1; i++) {
		TX_CHECKER_START;
		//NVTest_exclude(&fc, sizeof(fc));
		NVTest_assign(&fc.a, sizeof(fc.a));
		NVTest_assign(&fc.b, sizeof(fc.b));
		TX_CHECKER_END;
	}
	

	C_execVeri(p, metadataPtr);

	existVeriInstance = 0;

	C_getVeri(p, (void *)(0));
	C_deleteMetadataVector(metadataVectorPtr);


	C_deleteVeriInstance(p);
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
}
