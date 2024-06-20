#include <linux/module.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <net/sock.h> 
#include <linux/netlink.h>
#include <linux/skbuff.h> 

#define NETLINK_TEST 17

static int error_level;
static char content[128];
struct sock *sock = NULL;

static int netlink_show(struct seq_file *seq, void *v)
{
	seq_printf(seq,content);
	return 0;
}

static int netlink_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file,netlink_show,NULL);
}

static struct file_operations netlink_proc_fops = {
	.owner = THIS_MODULE,
	.open = netlink_seq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static void netlink_receive_skb(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello netlink from kernel";
	char temp[50];
    int res;

    msg_size = strlen(msg);

    nlh = (struct nlmsghdr *)skb->data;

	sprintf(temp, "%s", (char *)nlmsg_data(nlh));

	if(strlen(temp) > 1) {
		sprintf(content,"Received message: \"%s\", sequence number: %u\n",(char *)NLMSG_DATA(nlh),nlh->nlmsg_seq);
		printk(KERN_INFO "Odebrano wiadomość: %s\n", (char *)nlmsg_data(nlh));
	}
    pid = nlh->nlmsg_pid;

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Błąd tworzenia nowej wiadomości\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(sock, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Błąd wysyłania wiadomości\n");
}

static struct netlink_kernel_cfg netlink_configuration = {
	.groups = 0,
	.input = netlink_receive_skb,
	.cb_mutex = NULL,
	.bind = NULL,
	.unbind = NULL,
	.compare = NULL,
};

static int __init netlink_start(void)
{
	if(proc_create("netlink",0,NULL,&netlink_proc_fops)==NULL) {
		printk(KERN_ALERT "Error creating procfs entry!\n");
		error_level=1;
		return -1;
	}
    sock = netlink_kernel_create(&init_net, NETLINK_TEST, &netlink_configuration);
	if(!sock) {
		printk(KERN_ALERT "Error creating netlink socket!\n");
		error_level=2;
		return -2;
	}

    return 0;
}

static void __exit netlink_stop(void)
{
    if(error_level!=1)
		remove_proc_entry("netlink",NULL);
	if(error_level!=2)
		netlink_kernel_release(sock);
}

module_init(netlink_start);
module_exit(netlink_stop);

MODULE_LICENSE("GPL");