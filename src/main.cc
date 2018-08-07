#include "pmtest.hh"
#include "common.hh"
#include "pmtest.h"
#include "test.hh"

int main()
{
	transaction();
	// test_icl();
	tx_wrapper();
	// fastvector();
	Metadata m;
	std::cout << "Metadata" << sizeof(Metadata) << std::endl
		<< "type" << sizeof(m.type) << std::endl 
		<< "line_num" << sizeof(m.line_num) << std::endl 
		<< "size" << sizeof(m.size) << std::endl 
		<< "addr" << sizeof(m.addr) << std::endl 
		<< "size_late" << sizeof(m.size_late) << std::endl 
		<< "addr_late" << sizeof(m.addr_late) << std::endl 
		<< "file_name" << sizeof(m.file_name) << std::endl;

	return 0;
}
