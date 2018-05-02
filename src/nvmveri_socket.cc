#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

#include "nvmveri.hh"

/* Protocol family, consistent in both kernel prog and user prog. */
#define NETLINK_USER 31

#define MAX_MSG_LENGTH 65536

// nvmveri
bool termSignal;
bool getResultSignal;

// netlink

sockaddr_nl src_addr, dest_addr;
nlmsghdr *nlh = NULL;
iovec iov;
int sock;
msghdr msg;

int open_netlink(void)
{
    sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock < 0) {
        printf("sock < 0.\n");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    if (bind(sock, (struct sockaddr *) &src_addr, sizeof(src_addr)) < 0) {
        printf("bind < 0.\n");
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    return sock;
}

/* new_tx_flag: new_tx - 0, old_tx - 1, tail_old_tx - 2 */


bool read_event(FastVector<Metadata*> *MetadataVectorPtr)
{
    char buffer[MAX_MSG_LENGTH];
    int msg_size;
    int pos;
    char* data;

    iov.iov_base = (void *) buffer;
    iov.iov_len = sizeof(buffer);
    msg.msg_name = (void *) &(nladdr);
    msg.msg_namelen = sizeof(nladdr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Ok, listening.\n");
    msg_size = recvmsg(sock, &msg, 0);

    if (msg_size < 0)
        exit(1);

    pos = 0;
    data = (char* )NLMSG_DATA((struct nlmsghdr *) &buffer);
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
        MetadataVectorPtr->push_back((Metadata*)(data + pos));
        pos += sizeof(Metadata);
    }

    /*
    if (ret < 0)
        printf("ret < 0.\n");
    else
        printf("Received message payload: %s\n", NLMSG_DATA((struct nlmsghdr *) &buffer));
    */
    return last_packet;
}

void send_ack(char* ack_msg) {

    nlh = (nlmsghdr *)malloc(NLMSG_SPACE(MAX_MSG_LENGTH));
    memset(nlh, 0, NLMSG_SPACE(MAX_MSG_LENGTH));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_MSG_LENGTH);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    strcpy((char*)NLMSG_DATA(nlh), ack_msg);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

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
        if (getResultSignal) send_ack("VERI_COMPLETE");

        if (last_packet) {
            execVeri(MetadataVectorPtr);
        }

    }

    return 0;
}
