#include "nvmveri.hh"

int main()
{

	NVMVeri nvm_instance;

	vector<Metadata> m;
	Metadata dat;
	dat.teststr = "test1";
	m.push_back(dat);
	dat.teststr = "test2";
	m.push_back(dat);
	nvm_instance.execVeri(&m);


	m.clear();
	dat.teststr = "test3";
	m.push_back(dat);
	dat.teststr = "test4";
	m.push_back(dat);
	nvm_instance.execVeri(&m);

	VeriResult r;
	nvm_instance.getVeri(&m, &r);

	return 0;
}
