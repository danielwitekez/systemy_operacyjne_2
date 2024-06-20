#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/seq_file.h>
#include<linux/proc_fs.h>
#include<linux/netlink.h>
#include<net/netlink.h>
#include<net/genetlink.h>
#include<net/sock.h>

#define TEST_GENETLINK_CMD 1
#define TEST_GENETLINK_A_MSG 1
#define TEST_GENETLINK_A_MAX 1

static int error_level;
static char content[128];

static struct genl_family test_genetlink_family = {
	.id = GENL_ID_GENERATE,
	.hdrsize = 0,
	.name = "GENETLINK_TEST",
	.version = 1,
	.maxattr = TEST_GENETLINK_A_MAX,
};

static struct nla_policy test_genetlink_policy[TEST_GENETLINK_A_MAX+1];

static int test_genetlink_doit(struct sk_buff *skb, struct genl_info *info)
{
	if(info && info->attrs)
		// %d zamiast %s i nla_get_u8() zamiast nla_data()
		sprintf(content,"Received message: \"%d\", sequence number: %u port ID: %d\n",nla_get_u8(info->attrs[TEST_GENETLINK_A_MSG]),info->snd_seq,info->snd_portid);

	return 0;
}

static struct genl_ops test_genetlink_ops[] =
{
	{
		.cmd = TEST_GENETLINK_CMD,
		.flags = 0,
		.policy = test_genetlink_policy,
		.doit = test_genetlink_doit,
		.dumpit = NULL,
	},
};

static int genetlink_show(struct seq_file *seq, void *v)
{
	seq_printf(seq,content);
	return 0;
}

static int genetlink_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file,genetlink_show,NULL);
}

static struct file_operations genetlink_proc_fops = {
	.owner = THIS_MODULE,
	.open = genetlink_seq_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init genetlink_start(void)
{
	// NLA_U8 zamiast NLA_STRING, przydzielanie rozmiaru zakomentowane i można wywalić
	// powód opisany na stronie 7 instrukcji
	test_genetlink_policy[TEST_GENETLINK_A_MSG].type = NLA_U8;
	//test_genetlink_policy[TEST_GENETLINK_A_MSG].len = 31;
	if(proc_create("genetlink",0,NULL,&genetlink_proc_fops)==NULL) {
		printk(KERN_ALERT "Error creating procfs entry!\n");
		error_level=1;
		return -1;
	}
	if(genl_register_family_with_ops(&test_genetlink_family,test_genetlink_ops)) {
		error_level=2;
		return -1;
	}
	return 0;
}

static void __exit genetlink_stop(void)
{
	if(error_level!=1)
		remove_proc_entry("genetlink",NULL);
	if(error_level!=2)
		if(genl_unregister_family(&test_genetlink_family))
			printk(KERN_ALERT "Error occured while unregistering genetlink family!\n");
}

module_init(genetlink_start);
module_exit(genetlink_stop);
MODULE_LICENSE("GPL");
