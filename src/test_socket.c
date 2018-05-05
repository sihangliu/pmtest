#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

#include "../include/knvmveri.h"


static struct sock *nl_sk = NULL;

static void send_to_user(void)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
	int res;
    //char *msg = "@ Hello from kernel";
    //int msg_size = strlen(msg) + 1;
	struct Metadata metadata;
	char* send_buffer;
	int num_metadata = 1;

	metadata.type = _ASSIGN;
	metadata.assign.addr = (void*) 0x100;
	metadata.assign.size = 4;

	send_buffer = (char*) kmalloc (MAX_MSG_LENGTH, GFP_KERNEL);
	memset(send_buffer, 0, MAX_MSG_LENGTH);

	*((int*)send_buffer) = 1;
	*((int*)(send_buffer + sizeof(int))) = num_metadata;
	*((struct Metadata*)(send_buffer + 2 * sizeof(int))) = metadata;

    printk("@ Creating skb.\n");
    skb = nlmsg_new(MAX_MSG_LENGTH, GFP_KERNEL);
    if (!skb) {
        printk("@ Allocation failure.\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 1, NLMSG_DONE, MAX_MSG_LENGTH, 0);
    memcpy(nlmsg_data(nlh), &send_buffer, MAX_MSG_LENGTH);

    printk("@ Sending skb.\n");
    res = nlmsg_multicast(nl_sk, skb, 0, MYMGRP, GFP_KERNEL);
    if (res < 0)
        printk("@ nlmsg_multicast() error: %d\n", res);
    else
        printk("@ Success.\n");
}

static int __init hello_init(void)
{
    printk("@ Inserting hello module.\n");

    nl_sk = netlink_kernel_create(&init_net, MYPROTO, NULL);
    if (!nl_sk) {
        printk("@ Error creating socket.\n");
        return -10;
    }

    send_to_user();

    netlink_kernel_release(nl_sk);
    return 0;
}

static void __exit hello_exit(void)
{
    printk("@ Exiting hello module.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
