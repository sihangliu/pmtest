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

	vector<VeriResult> r;
	nvm_instance.getVeri(r);

	printf("OUT\n");

	for(auto i = r.begin(); i < r.end(); i++)
		printf("%s\n", i->teststr.c_str());

	 m.clear();
	 dat.teststr = "test5";
	 m.push_back(dat);
	 dat.teststr = "test6";
	 m.push_back(dat);
	 nvm_instance.execVeri(&m);
	
	 r.clear();
	 nvm_instance.getVeri(r);

	return 0;
}
