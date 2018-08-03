#include "nvmveri.hh"
#include "common.hh"
#include "nvmveri.h"
#include "test.hh"
#include <boost/version.hpp>
#define BOOST_ALL_DYN_LINK
#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>

namespace bs = boost::stacktrace;
int main()
{
	transaction();
	test_icl();
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
#if BOOST_VERSION >= 106500
	 bs::stacktrace b = bs::stacktrace();
	 std::cout << b.size() << sizeof(b) << std::endl;
#endif

	return 0;
}
