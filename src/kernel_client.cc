#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
//#include <sys/socket.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "nvmveri.hh"

#define NVMVERI_USER_CODE
#include "kernel_module.h"

// FastVector<Metadata *> allocated;
// Metadata buf[BUFFER_LEN];
// int start_idx;

// obtain the semaphore and read a single block
/*
void read_datablock()
{
	int fd = open("/dev/nvmveri_dev", O_RDONLY);
	if (fd == -1) {
		assert(0 && "Cannot open NVMVeri file device");
	}
	read(fd, buf, sizeof(Metadata) * BUFFER_LEN);
	close(fd);
}*/

// void read_datablock()
// {
// 	int fd = open("/proc/nvmveri", O_RDONLY);
// 	if (fd == -1) {
// 		assert(0 && "Cannot open NVMVeri file device");
// 	}
// 	for (int i = 0; i < BUFFER_LEN; i++) {
// 		read(fd, &buf[i], sizeof(Metadata));
// 	}
// 	close(fd);
// }

// int timer = 0;
// Metadata ass, delim, end;
//
// void read_datablock()
// {
// 	if (timer == 0) {
// 		int i;
// 		ass.assign.size = 0;
// 		for (i = 0; i < 15; i++) buf[i] = ass;
// 		buf[i] = delim; i++;
// 		ass.assign.size = 1;
// 		for (; i < BUFFER_LEN; i++) buf[i] = ass;
// 	}
// 	else if (timer == 1) {
// 		int i;
// 		for (i = 0; i < 3; i++){
// 			buf[i] = ass;
// 		}
// 		buf[i] = delim; i++;
// 		ass.assign.size = 2;
// 		for (; i < 7; i++) buf[i] = ass;
// 		buf[i] = delim; i++;
// 		ass.assign.size = 3;
// 		for (; i < 12; i++) buf[i] = ass;
// 		buf[i] = delim; i++;
// 		ass.assign.size = 4;
// 		for (; i < BUFFER_LEN; i++) buf[i] = ass;
//
// 	}
// 	else if (timer == 2) {
// 		int i;
// 		for (i = 0; i < 9; i++) buf[i] = ass;
// 		buf[i] = delim; i++;
// 		ass.assign.size = 5;
// 		for (; i < BUFFER_LEN - 1; i++) buf[i] = ass;
// 		buf[i] = delim; i++;
// 		ass.assign.size = 6;
// 		printf("wow ");
// 		Metadata_print(&buf[i - 2]);
//
// 	}
// 	else {
// 		int i;
// 		for (i = 0; i < 3; i++) buf[i] = ass;
// 		buf[i] = delim; i++;
// 		buf[i] = end; i++;
// 		ass.assign.size = 7;
// 		for (; i < 6; i++) buf[i] = ass;
// 		buf[i] = end; i++;
//
// 	}
// 	timer++;
// }

// assemble a whole transaction and return
// int read_transaction(FastVector<Metadata *> *tx)
// {
// 	while (true) {
// 		if (start_idx >= BUFFER_LEN) {
// 			read_datablock();
// 			start_idx = 0;
// 		}
// 		for (size_t i = start_idx; i < BUFFER_LEN; i++) {
// 			if (buf[i].type == _TRANSACTIONDELIM) {
// 				Metadata *start_ptr = new Metadata[i - start_idx];
// 				allocated.push_back(start_ptr);
// 				// insert all Metadata before index i
// 				for (int j = start_idx; j < i; j++) {
// 					start_ptr[j - start_idx] = buf[j];
// 					printf("Transaction 1 %d %d %p %p\n",j,start_idx,start_ptr, &start_ptr[j - start_idx]);
// 					tx->push_back(&start_ptr[j - start_idx]);
// 				}
// 				start_idx = i + 1;
// 				return 0;
// 			}
// 			if (buf[i].type == _ENDING) {
// 				assert(i == start_idx && "Transaction not wrapped by delimiter");
// 				return -1;
// 			}
// 		}
// 		Metadata *start_ptr = new Metadata[BUFFER_LEN - start_idx];
// 		allocated.push_back(start_ptr);
// 		for (int j = start_idx; j < BUFFER_LEN; j++) {
// 			start_ptr[j - start_idx] = buf[j];
// 			printf("Transaction 2 %d %d %p, %p\n",j,start_idx, start_ptr, &start_ptr[j - start_idx]);
// 			tx->push_back(&start_ptr[j - start_idx]);
// 		}
// 		start_idx = BUFFER_LEN;
// 	}
//
// }
//
// int main(int argc, char *argv[])
// {
// 	NVMVeri veriInstance;
// 	start_idx = BUFFER_LEN;
// 	FastVector<FastVector<Metadata *> *> tx_vector;
// 	while (true) {
// 		FastVector<Metadata *> *tx = new FastVector<Metadata *>;
// 		printf("transaction head %p\n", tx);
// 		int flag = read_transaction(tx);
// 		tx_vector.push_back(tx);
// 		for (int i = 0; i < tx->size(); i++) {
// 			printf("local ");
// 			Metadata_print((*tx)[i]);
// 		}
// 		if (flag == -1) break;
// 		veriInstance.execVeri(tx);
// 	}
// 	FastVector<VeriResult> result;
// 	veriInstance.getVeri(result);
// 	for (int i = 0; i < tx_vector.size(); i++)
// 		delete [] allocated[i];
// 	for (int i = 0; i < tx_vector.size(); i++)
// 		delete tx_vector[i];
//
// 	return 0;
// }

FastVector<Metadata *> allocated;

int read_transaction(FastVector<Metadata *> *tx)
{
	// open the virtual file with default name "/proc/nvmveri"
	int fd = open((std::string("/proc/") + PROC_NAME).c_str(), O_RDONLY);
	printf("start reading transaction %p\n", tx);
	if (fd == -1) {
		printf("Cannot open NVMVeri file device, %d", errno);
		assert(0);
	}

	int sleeptime = 1, sleeptime_max = 512;
	while(true) {
		Metadata *buf = new Metadata;
		allocated.push_back(buf);
		ssize_t result = read(fd, buf, sizeof(Metadata));
		if (result < 0) {
			printf("NVMVeri read data error %ld\n", result);
			assert(0);
		}
		if (result == 1) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
			if (sleeptime < sleeptime_max)
			sleeptime *= 2;
			continue;
		}
		if (buf->type == _TRANSACTIONDELIM) return 0;
		if (buf->type == _ENDING) return -1;
		tx->push_back(buf);
	}
	close(fd);
}


int main(int argc, char *argv[])
{
	NVMVeri veriInstance;
	FastVector<FastVector<Metadata *> *> tx_vector;
	while (true) {
		FastVector<Metadata *> *tx = new FastVector<Metadata *>;
		printf("transaction head %p\n", tx);
		int flag = read_transaction(tx);
		tx_vector.push_back(tx);
		for (int i = 0; i < tx->size(); i++) {
			printf("local ");
			Metadata_print((*tx)[i]);
		}
		if (flag == -1) break;
		veriInstance.execVeri(tx);
	}
	FastVector<VeriResult> result;
	veriInstance.getVeri(result);
	for (int i = 0; i < tx_vector.size(); i++)
		delete allocated[i];
	for (int i = 0; i < tx_vector.size(); i++)
		delete tx_vector[i];

	return 0;
}
