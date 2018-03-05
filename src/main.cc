#include "nvmveri.hh"

int main()
{
    vector<Metadata> m;
    NVMVeri nvm_instance;
    nvm_instance.execVeri(&m);
    VeriResult r;
    nvm_instance.getVeri(&m, &r);
    
	return 0;
}
