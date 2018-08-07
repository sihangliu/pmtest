#include "pmtest.hh"
#include "pmtest.h"

#include <utility>

void transaction()
{
	char arr[100];

	Timer timer;
	void *p = NULL;

	const int LOOP_CNT = 3;


/****************************test1*****************************/
	timer.startTimer();

	PMTest_init(p, 2);
	PMTest_START;

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_isPersistent((void *)(&arr[0]), 4);
	PMTest_isPersistent((void *)(&arr[0]), 0);
	PMTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[0]), 4);


	PMTest_sendTrace(p);

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_isPersistent((void *)(&arr[0]), 4);
	PMTest_isPersistent((void *)(&arr[0]), 0);
	PMTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[0]), 4);


	PMTest_sendTrace(p);
	PMTest_sendTrace(p);

	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);

	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test1*****************************/


/****************************test2*****************************/
	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_isPersistent((void *)(&arr[0]), 4);
	PMTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[4]), 4);
	PMTest_assign((void *)(&arr[0]), 4);

	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);

	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test2*****************************/


/****************************test3*****************************/
	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_isPersistent((void *)(&arr[0]), 4);
	PMTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[4]), 4);


	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);

	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test3*****************************/


/****************************test4*****************************/
	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_isPersistent((void *)(&arr[4]), 4);
	PMTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[4]), 4);


	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test4*****************************/

/****************************test5*****************************/
	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_isPersistent((void *)(&arr[0]), 4);
	PMTest_isPersistedBefore((void *)(&arr[4]), 4, (void *)(&arr[0]), 4);


	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test5*****************************/

/****************************test6*****************************/
	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_isPersistedBefore((void *)(&arr[0]), 4, (void *)(&arr[4]), 4);


	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test6*****************************/

/****************************test7*****************************/
	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_isPersistent((void *)(&arr[0]), 4);

	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test7*****************************/

/****************************test8*****************************/
	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	PMTest_assign((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_assign((void *)(&arr[4]), 4);
	PMTest_flush((void *)(&arr[0]), 4);
	PMTest_fence();
	PMTest_isPersistedBefore((void *)(&arr[4]), 4, (void *)(&arr[0]), 4);


	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
/****************************test8*****************************/

/****************************test9*****************************/
	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	PMTest_assign((void *)(128), 8);
	PMTest_assign((void *)(256), 8);
	PMTest_flush((void *)(128), 8);
	PMTest_fence();
	PMTest_isPersistent((void *)(128), 8);
	PMTest_isPersistent((void *)(256), 8);

	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);
	
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

	Timer timer;
	void *p;

	const int LOOP_CNT = 3;

	timer.startTimer();

	PMTest_init(p, 1);
	PMTest_START;

	struct fuc{
		int a;
		int b;
		int c;
	} fc;


	for (int i = 0; i < 1; i++) {
		PMTest_CHECKER_START;
		//PMTest_exclude(&fc, sizeof(fc));
		PMTest_assign(&fc.a, sizeof(fc.a));
		PMTest_assign(&fc.b, sizeof(fc.b));
		PMTest_CHECKER_END;
	}
	

	PMTest_sendTrace(p);
	PMTest_END;
	PMTest_getResult(p);
	PMTest_exit(p);
	
	timer.endTimer();
	printf("Total time for %d tasks = %llu(us)\n\n", LOOP_CNT, timer.getTime());
}
