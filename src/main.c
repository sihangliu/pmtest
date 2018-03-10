#include "nvmveri.h"
#include <stdio.h>

int main()
{
	void *p = C_createVeriInstance();
	C_execVeri(p, (void **)(0));
	C_getVeri(p, (void **)(0));
	C_deleteVeriInstance(p);
	return 0;
}
