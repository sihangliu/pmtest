#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

#include "../include/knvmveri.h"


static struct sock *nl_sk = NULL;
char* send_buffer;

static void send_to_user(int flag)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
	int res;
	struct Metadata metadata;
	int num_metadata = 1;

	if (flag < 0) num_metadata = -1;

	metadata.type = _ASSIGN;
	metadata.assign.addr = (void*)(100 + flag * 8);
	metadata.assign.size = 8;

	memset(send_buffer, 0, MAX_MSG_LENGTH);

	*((int*)send_buffer) = (flag == 9);
	*((int*)send_buffer + 1) = num_metadata;
	*((struct Metadata*)(send_buffer + 2 * sizeof(int))) = metadata;

    printk("@ Creating skb.\n");
    skb = nlmsg_new(NLMSG_ALIGN(MAX_MSG_LENGTH), GFP_KERNEL);
    if (!skb) {
        printk("@ Allocation failure.\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 1, NLMSG_DONE, MAX_MSG_LENGTH, 0);
    memcpy(nlmsg_data(nlh), send_buffer, sizeof(int) * 2 + sizeof(struct Metadata));
	
    printk("@ Sending skb.\n");
    res = nlmsg_multicast(nl_sk, skb, 0, MYMGRP, GFP_KERNEL);
    if (res < 0)
        printk("@ nlmsg_multicast() error: %d\n", res);
    else
        printk("@ Success.\n");
}

static int __init hello_init(void)
{
	int i;
	printk("@ Metadata size = %lu\n", sizeof(struct Metadata));

	send_buffer = (char*) kmalloc (MAX_MSG_LENGTH, GFP_KERNEL);
    printk("@ Inserting hello module.\n");

    nl_sk = netlink_kernel_create(&init_net, MYPROTO, NULL);
    if (!nl_sk) {
        printk("@ Error creating socket.\n");
        return -10;
    }
	
	for (i = 0; i < 10; ++i)
		send_to_user(i);

	//send_to_user(-1);
	

    return 0;
}

static void __exit hello_exit(void)
{
    netlink_kernel_release(nl_sk);
	kfree(send_buffer);
    printk("@ Exiting hello module.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
