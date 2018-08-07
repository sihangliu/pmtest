#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
//#include <sys/socket.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "pmtest.hh"

#define PMTEST_USER_CODE
#include "kernel_module.h"


FastVector<Metadata *> allocated;

int read_transaction(FastVector<Metadata *> *tx, int fd)
{
	// open the virtual file with default name "/proc/pmtest"

	int sleeptime = 1, sleeptime_max = 512;
	while(true) {
		Metadata *buf = new Metadata;
		allocated.push_back(buf);
		ssize_t result = read(fd, buf, sizeof(Metadata));
		if (result < 0) {
			printf("PMTest read data error %ld\n", result);
			assert(0);
		}
		if (result == 1) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
			if (sleeptime < sleeptime_max)
			sleeptime *= 2;
			continue;
		}

		switch(buf->type) {
		/* Metadata that shall not be pushed to tx */
		case _TRANSACTIONDELIM:
			return 0;
		case _ENDING:
			return -1;
		/* Metadata that shall be pushed to tx */
		default:
			tx->push_back(buf);
		}
	}
}


int main(int argc, char *argv[])
{
	PMTest veriInstance;
	FastVector<FastVector<Metadata *> *> tx_vector;

	int fd = open((std::string("/proc/") + PROC_NAME).c_str(), O_RDONLY);
	if (fd == -1) {
		printf("PMTest: %d %s\n", errno, strerror(errno));
		close(fd);
		assert(0);
	}

	while (true) {
		FastVector<Metadata *> *tx = new FastVector<Metadata *>;
		int flag = read_transaction(tx, fd);
		tx_vector.push_back(tx);
		if (flag == -1) break;
		veriInstance.execVeri(tx);
	}
	FastVector<VeriResult> result;
	veriInstance.getVeri(result);
	for (int i = 0; i < tx_vector.size(); i++)
		delete allocated[i];
	for (int i = 0; i < tx_vector.size(); i++)
		delete tx_vector[i];
	
	close(fd);

	return 0;
}
