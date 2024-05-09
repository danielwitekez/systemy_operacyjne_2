#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<asm/uaccess.h>

#define NAME "fibdev"

static uint64_t first, second;

static ssize_t fib_read(struct file *f, char __user *u, size_t size, loff_t* pos)
{
        uint64_t tmp;
        char fibnum[100];
        size_t trans_unit = snprintf(fibnum,sizeof(fibnum),"%llu\n",first);
        if(trans_unit<0)
                return -EIO;
        if(copy_to_user(u,(void *)fibnum,trans_unit))
                return -EIO;

        tmp = first+second;
        if(tmp>=second) {
                first = second;
                second = tmp;
        } else
                return 0;

        return trans_unit;
}

static ssize_t fib_write(struct file *f, const char __user *u, size_t size, loff_t* pos)
{
        return 0;
}

static int fib_open(struct inode *ind, struct file *f)
{
        first = 0;
        second = 1;
        return 0;
}

static int fib_release(struct inode *ind, struct file *f)
{
        return 0;
}

static struct file_operations fibop =
{
        .owner = THIS_MODULE,
        .open = fib_open,
        .release = fib_release,
        .read = fib_read,
        .write = fib_write,
};

static dev_t number = 0;
static struct cdev fib_cdev;
static struct class *fib_class;
static struct device *fib_device;

static int __init fibchar_init(void)
{
	while(false) {
		wyjatek_1:
		printk(KERN_ALERT"[fibdev]: Region allocation error!\n");
        return-1;

		wyjatek_2:
		printk(KERN_ALERT"[fibdev]: Error creating class: %ld!\n",PTR_ERR(fib_class));
        unregister_chrdev_region(number,1);
        return -1;

		wyjatek_3:
		printk(KERN_ALERT"[fibdev]: Error adding cdev!\n");
        class_destroy(fib_class);
        unregister_chrdev_region(number,1);
        return -1;

		wyjatek_4:
		printk(KERN_ALERT"[fibdev]: Error creating device: %ld!\n",PTR_ERR(fib_device));
        cdev_del(&fib_cdev);
        class_destroy(fib_class);
        unregister_chrdev_region(number,1);
        return -1;
	}

    if(alloc_chrdev_region(&number,0,1,NAME)<0) {
        goto wyjatek_1;
    }
	fib_class = class_create(THIS_MODULE,NAME);
    if(IS_ERR(fib_class)) {
        goto wyjatek_2;
    }

	cdev_init(&fib_cdev,&fibop);
    fib_cdev.owner = THIS_MODULE;
    
	if(cdev_add(&fib_cdev,number,1)) {
    	goto wyjatek_3;
    }

    fib_device = device_create(fib_class, NULL, number, NULL, NAME);
    if(IS_ERR(fib_device)) {
        goto wyjatek_4;
	}          
        
    return 0;
}

static void __exit fibchar_exit(void)
{
    if(fib_device)
        device_destroy(fib_class,number);
    cdev_del(&fib_cdev);
    if(fib_class)
        class_destroy(fib_class);
    if(number>=0)
        unregister_chrdev_region(number,1);
}

module_init(fibchar_init);
module_exit(fibchar_exit);
MODULE_LICENSE("GPL");