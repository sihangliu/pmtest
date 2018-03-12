#include "nvmveri.h"
#include <stdio.h>

int main()
{
	void *p = C_createVeriInstance();
	void *vec = C_createMetadataVector();
	C_createMetadata(vec, "aaa", (void *)0, 4);

	C_execVeri(p, vec);
	C_getVeri(p, (void *)0);
	C_deleteMetadataVector(vec);
	C_deleteVeriInstance(p);
	return 0;
}
