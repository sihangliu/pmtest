#ifdef __MEMORY_HH__
#define __MEMORY_HH__

#include <cstdlib>
#include <unordered_set>
using std::unordered_set;
#include <unordered_map>
using std::unordered_map;


class Memory {
public:
	memory();
	~memory();

	bool upadteMemory(OpInfo);
	void resetMemory();
private:
	unordered_set<addr_t> incompleteRequestSet;
};




#endif
