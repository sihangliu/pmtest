#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "nvmveri.h"
#include "nvmveri_socket.h"


// nvmveri
//int termSignal;
//int getResultSignal;

// netlink
struct sockaddr_nl local_addr, remote_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock;
struct msghdr msg;
//char* buffer;

int open_netlink()
{
    int group = MYMGRP;

    sock = socket(PF_NETLINK, SOCK_RAW, MYPROTO);
    if (sock < 0) {
        printf("sock < 0.\n");
        return -1;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.nl_family = AF_NETLINK;
    local_addr.nl_pid = getpid();
    local_addr.nl_groups = 0;

    if (bind(sock, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0) {
        printf("bind < 0.\n");
        return -1;
    }

    if (setsockopt(sock, 270, NETLINK_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
        printf("setsockopt < 0\n");
        return -1;
    }

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.nl_family = AF_NETLINK;
    remote_addr.nl_pid = 0; /* For Linux Kernel */
    remote_addr.nl_groups = 0;

    return sock;
}

void close_netlink() {
	close(sock);
	free(nlh);
}

int read_event(int sock)
{
    struct sockaddr_nl nladdr;
    struct msghdr msg;
    struct iovec iov;
    char buffer[MAX_MSG_LENGTH];
    int ret;
    int metadata_len;

    iov.iov_base = (void *) buffer;
    iov.iov_len = sizeof(buffer);
    msg.msg_name = (void *) &(nladdr);
    msg.msg_namelen = sizeof(nladdr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Ok, listening.\n");
    ret = recvmsg(sock, &msg, 0);
    if (ret < 0) {
        printf("ret < 0.\n");
        return ret;
    } else {
        metadata_len = *(int*)(NLMSG_DATA((struct nlmsghdr *) &buffer));
        printf("New metadata length = %d\n", metadata_len);
    }
    return metadata_len;
}

void read_data(struct Vector* MetadataVectorPtr, int metadata_len)
{
    int fd;
    int i = 0;
    int j = 0;
    char* read_buf;

    fd = open("/dev/nvmveri_dev", O_RDWR);
	printf("fd=%d\n", fd);

    while (i < metadata_len) {
        // for each metadata packet, allocate a new read buffer
        read_buf = (char*) malloc (sizeof(struct Metadata) *
                                        MIN(MAX_BUFFER_LEN, metadata_len - i));
        // read one metadata_packet each time
        read(fd, read_buf,
                sizeof(struct Metadata) * MAX_BUFFER_LEN);
        // read the entire buffer, unless we are reading the last one
        for (j = 0; j < MIN(MAX_BUFFER_LEN, metadata_len - i); ++j) {
            // put the pointer to each metadata in read buffer to metadata vector
            pushVector(MetadataVectorPtr, (struct Metadata*)(read_buf) + j);
            printf("%p\n", ((struct Metadata*)(read_buf) + j)->assign.addr);
        }
        // number of metadata read previously
        i += MIN(MAX_BUFFER_LEN, metadata_len - i);
    }
    close(fd);
}


int main(int argc, char *argv[])
{
    int nls;
    int metadata_len;
    struct Vector* MetadataVectorPtr;
    struct Vector MetadataVector_array;
    int i, j;
    void* VeriInstancePtr;

    initVector(&MetadataVector_array);
    // Open connection
    nls = open_netlink();
    if (nls < 0)
        return nls;

    VeriInstancePtr = C_createVeriInstance();

    while (1) {
        printf("New iteration\n");

        metadata_len = read_event(nls);
        if (metadata_len > 0) {
            // create new metadata vector
            MetadataVectorPtr = (struct Vector*)malloc(sizeof(struct Vector));
            initVector(MetadataVectorPtr);
            // push the new metadata vector to an array
            pushVector(&MetadataVector_array, MetadataVectorPtr);
            // read data from file io
            read_data(MetadataVectorPtr, metadata_len);
        } else {
            printf("Exit\n");
            // metadata_len < 0 - exit signal
            break;
        }

        C_execVeri(VeriInstancePtr, MetadataVectorPtr);
    }

    C_getVeri(VeriInstancePtr, (void *)(0));

    for (i = 0; i < MetadataVector_array.cur_size; ++i) {
        C_deleteMetadataVector(MetadataVector_array.arr_vector[i]);
    }

    C_deleteVeriInstance(MetadataVectorPtr);

	close_netlink();

    return 0;
}
