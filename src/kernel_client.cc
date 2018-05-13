#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
//#include <sys/socket.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "nvmveri.hh"
// #include "nvmveri_kernel.h"
// TODO: FIX THIS HEADER WHEN WRITING KERNEL MODULE
#define BUFFER_LEN 20

FastVector<Metadata *> allocated;
Metadata buf[BUFFER_LEN];
int start_idx;

// obtain the semaphore and read a single block
int read_datablock()
{
	int fd = open("/dev/nvmveri_dev", O_RDONLY);
	if (fd == -1) {
		assert(0 && "Cannot open NVMVeri file device");
	}
	read(fd, buf, sizeof(Metadata) * BUFFER_LEN);
	close(fd);
}

// assemble a whole transaction and return
int read_transaction(FastVector<Metadata *> *tx)
{
	
	while (true) {
		for (size_t i = start_idx; i < BUFFER_LEN; i++) {
			if (buf[i].type == _TRANSACTIONDELIM) {
				Metadata *start_ptr = new Metadata[i - start_idx - 1];
				allocated.push_back(start_ptr);
				// insert all Metadata before index i
				for (int j = start_idx; j < i; j++) {
					start_ptr[j - start_idx] = buf[j];
					tx->push_back(&start_ptr[j - start_idx]);
				}
				start_idx = i + 1;
				return -2;
			}
			if (buf[i].type == _ENDING) {
				Metadata *start_ptr = new Metadata[i - start_idx - 1];
				allocated.push_back(start_ptr);
				// insert all metadata before index i
				for (int j = start_idx; j < i; j++) {
					start_ptr[j - start_idx] = buf[j];
					tx->push_back(&start_ptr[j - start_idx]);
				}
				return -3;
			}
		}
		Metadata *start_ptr = new Metadata[BUFFER_LEN];
		allocated.push_back(start_ptr);
		for (int j = start_idx; j < BUFFER_LEN; j++) {
			start_ptr[j - start_idx] = buf[j];
			tx->push_back(&start_ptr[j - start_idx]);
		}
		read_datablock();
		start_idx = 0;
	}

}

int main(int argc, char *argv[])
{
	NVMVeri veriInstance;
	start_idx = 0;
	FastVector<FastVector<Metadata *> *> tx_vector;
	while (true) {
		FastVector<Metadata *> *tx = new FastVector<Metadata *>;
		int flag = read_transaction(tx);
		veriInstance.execVeri(tx);
		tx_vector.push_back(tx);
		if (flag == -3) break;
	}
	FastVector<VeriResult> result;
	veriInstance.getVeri(result);
	for (int i = 0; i < tx_vector.size(); i++)
		delete tx_vector[i];

	return 0;
}
