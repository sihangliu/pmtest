#include "nvmveri.hh"
#include "common.hh"
#include "nvmveri.h"
#include "test.hh"


int main()
{
	//transaction();
	test_icl();
	tx_wrapper();
	// fastvector();
	printf("%d\n", sizeof(Metadata));

	return 0;
}
