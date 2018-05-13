#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "nvmveri.h"

// read a whole transaction, return
void *read_data(void )
{
	void *p = C_createMetadataBuffer(METADATA_PACKET_LENGTH);
	for ()
}

int main(int argc, char *argv[])
{
	void *p = C_createVeriInstance();

	while (1) {
		existVeriInstance = 1;
		void *result = read_data();
		existVeriInstance = 0;
		if (result == NULL) break;
		C_execVeri(result, metadataPtr);
	}
	C_getVeri(p, (void *)(0));
	C_deleteVeriInstance(p);
	return 0;
}
