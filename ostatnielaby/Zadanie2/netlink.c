#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/seq_file.h>
#include<linux/proc_fs.h>
#include<linux/netlink.h>
#include<net/netlink.h>
#include<net/sock.h>

#define NETLINK_TEST 17

static int error_level;
static char content[128];
static struct sock *sock;

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
	uint8_t data;

	nlh = nlmsg_hdr(skb);
	if (nlh->nlmsg_type != 0x11) {
		printk(KERN_ALERT "nie ten typ!\n");
		return;
	}
	data = *((uint8_t *) NLMSG_DATA(nlh));

	printk(KERN_INFO "Received message with data: %u\n", data);
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
	sock = netlink_kernel_create(&init_net,NETLINK_TEST, &netlink_configuration);
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
MODULE_DESCRIPTION("A user space to kernel space simple communication module");
MODULE_VERSION("1.0");
