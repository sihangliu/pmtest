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

	*send_buffer = 1;
	*((int*)(send_buffer + sizeof(char))) = num_metadata;
	*((struct Metadata*)(send_buffer + sizeof(int))) = metadata;

    pr_info("@ Creating skb.\n");
    skb = nlmsg_new(MAX_MSG_LENGTH, GFP_KERNEL);
    if (!skb) {
        pr_err("@ Allocation failure.\n");
        return;
    }

    nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, MAX_MSG_LENGTH, 0);
	NETLINK_CB(skb).dst_group = 0; /* not in mcast group */
    memcpy(nlmsg_data(nlh), &send_buffer, MAX_MSG_LENGTH);

    pr_info("@ Sending skb.\n");
    res = nlmsg_unicast(nl_sk, skb, 0);
    if (res < 0)
        pr_info("@ nlmsg_unicast() error: %d\n", res);
    else
        pr_info("@ Success.\n");
}

static int __init hello_init(void)
{
    pr_info("@ Inserting hello module.\n");

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, NULL);
    if (!nl_sk) {
        pr_err("@ Error creating socket.\n");
        return -10;
    }

    send_to_user();

    netlink_kernel_release(nl_sk);
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("@ Exiting hello module.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
