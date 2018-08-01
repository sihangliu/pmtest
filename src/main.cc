#include "nvmveri.hh"
#include "common.hh"
#include "nvmveri.h"
#include "test.hh"


int main()
{
	// transaction();
	// test_icl();
	// tx_wrapper();
	// fastvector();
	ConditionalFilter f(std::cout);
	f.enable();
	f << "wow" << std::hex << 25 << std::endl;
	f.unsetf(std::ios_base::hex);
	f << "wow" << 25 << std::endl;

	return 0;
}
