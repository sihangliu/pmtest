#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

#include "nvmveri.hh"

/* Protocol family, consistent in both kernel prog and user prog. */
#define MYPROTO NETLINK_USERSOCK
#define MYMGRP 21

#define MAX_MSG_LENGTH 65536

// nvmveri
bool termSignal;
bool getResultSignal;

// netlink
sockaddr_nl local_addr, remote_addr;
nlmsghdr *nlh = NULL;
iovec iov;
int sock;
msghdr msg;

int open_netlink()
{
    sock = socket(AF_NETLINK, SOCK_RAW, MYPROTO);
    if (sock < 0) {
        printf("sock < 0.\n");
        return -1;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.nl_family = AF_NETLINK;
    local_addr.nl_pid = getpid();

    if (bind(sock, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0) {
        printf("bind < 0.\n");
        return -1;
    }

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.nl_family = AF_NETLINK;
    remote_addr.nl_pid = 0; /* For Linux Kernel */
    remote_addr.nl_groups = 0; /* unicast */

	// initialize netlink buffer
	nlh = (nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_LENGTH));
	memset(nlh, 0, NLMSG_SPACE(MAX_MSG_LENGTH));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_MSG_LENGTH);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

	iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *) &(remote_addr);
    msg.msg_namelen = sizeof(remote_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    return sock;
}

int close_netlink() {
	close(sock);
	free(nlh);
}


bool read_event(FastVector<Metadata*> *MetadataVectorPtr)
{
    int msg_size;
    int pos = 0;
    char* data;
    
    printf("Ok, listening.\n");
	//memset(nlh, 0, NLMSG_SPACE(MAX_MSG_LENGTH));
    msg_size = recvmsg(sock, &msg, 0);

    if (msg_size < 0)
        exit(1);

    data = (char*)NLMSG_DATA(nlh);
    // Read tx flag
    bool last_packet = *(data + pos);
    pos += sizeof(char);
    // Read number of metadata packets
    // num_metadata < 0  -  terminate
    // num_metadata = 0  -  getVeri
    // otherwise, regular packets
    int num_metadata = *(int *)(data + pos);
    if (num_metadata < 0) {
        termSignal = true;
        return 0;
    } else if (num_metadata == 0) {
        getResultSignal = true;
        return 0;
    }
    pos += sizeof(int);

    //Read each packet and push to metadata vector
    for (int i = 0; i < num_metadata; ++i) {
		Metadata* metadata = new Metadata;
		memcpy(metadata, (Metadata*)(data + pos), sizeof(Metadata));
		printf("metadata type=%d, addr=%llu, size=%lu\n", int(metadata->type), (addr_t)(metadata->assign.addr), metadata->assign.size);
        MetadataVectorPtr->push_back(metadata);
        pos += sizeof(Metadata);
    }
	printf("num_metadata = %d\n", num_metadata);

	return last_packet;
}

void send_ack(char* ack_msg) {
	
	memset(nlh, 0, NLMSG_SPACE(MAX_MSG_LENGTH));
    strcpy((char*)NLMSG_DATA(nlh), ack_msg);

    sendmsg(sock, &msg, 0);
}



int main(int argc, char *argv[])
{
    int nls;
    termSignal = false;
    getResultSignal = false;
    bool last_packet = true;

    // Open connection
    nls = open_netlink();
    if (nls < 0) return nls;

    FastVector<Metadata*> *MetadataVectorPtr;
    while (true) {
        if (last_packet)
            MetadataVectorPtr = new FastVector<Metadata*>;

        last_packet = read_event(MetadataVectorPtr);

        // Check termination signal
        if (termSignal) break;
        // Check getVeri signal
		char signal[] = "VERI_COMPLETE";
        if (getResultSignal) send_ack(signal);
		// Start verification once all packets have been received
        if (last_packet) {
			printf("last one");
            //execVeri(MetadataVectorPtr);
        }
    }

	close_netlink();

    return 0;
}
