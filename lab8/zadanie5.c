#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<asm/uaccess.h>

#define NAME "clipboard"
static char clipboard[1024];
bool wasReaded = false;		// zmienna zapobiegająca zapętlaniu się odczytu
int length = 0;

static ssize_t clipboard_read(struct file *f, char __user *u, size_t size, loff_t* pos)
{
	if(!wasReaded) {
		if(copy_to_user(u,clipboard,length)) {
			return -EIO;
		}
		wasReaded = true;
    
		return length;
	}
	wasReaded = false;
    
	return 0;
}

static ssize_t clipboard_write(struct file *f, const char __user *u, size_t size, loff_t* pos)
{
	length = size;

	if(copy_from_user(clipboard,u,length)) {
		return -EFAULT;
	}

	clipboard[length]='\0';
	wasReaded = false;
	return length;
}

static int clipboard_open(struct inode *ind, struct file *f)
{
        return 0;
}

static int clipboard_release(struct inode *ind, struct file *f)
{
        return 0;
}

static struct file_operations clipboard_op =
{
        .owner = THIS_MODULE,
        .open = clipboard_open,
        .release = clipboard_release,
        .read = clipboard_read,
        .write = clipboard_write,
};

static dev_t number = 0;
static struct cdev clipboard_cdev;
static struct class *clipboard_class;
static struct device *clipboard_device;

static int __init clipboard_init(void)
{
    if(alloc_chrdev_region(&number,0,1,NAME)<0) {
        printk(KERN_ALERT"[clipboard]: Region allocation error!\n");
        return-1;
    }
	clipboard_class = class_create(THIS_MODULE,NAME);
    if(IS_ERR(clipboard_class)) {
        printk(KERN_ALERT"[clipboard]: Error creating class: %ld!\n",PTR_ERR(clipboard_class));
        unregister_chrdev_region(number,1);
        return -1;
    }

	cdev_init(&clipboard_cdev,&clipboard_op);
    clipboard_cdev.owner = THIS_MODULE;
    
	if(cdev_add(&clipboard_cdev,number,1)) {
    	printk(KERN_ALERT"[clipboard]: Error adding cdev!\n");
        class_destroy(clipboard_class);
        unregister_chrdev_region(number,1);
        return -1;
    }

    clipboard_device = device_create(clipboard_class, NULL, number, NULL, NAME);
    if(IS_ERR(clipboard_device)) {
        printk(KERN_ALERT"[clipboard]: Error creating device: %ld!\n",PTR_ERR(clipboard_device));
        cdev_del(&clipboard_cdev);
        class_destroy(clipboard_class);
        unregister_chrdev_region(number,1);
        return -1;
	}          
        
    return 0;
}

static void __exit clipboard_exit(void)
{
    if(clipboard_device)
        device_destroy(clipboard_class,number);
    cdev_del(&clipboard_cdev);
    if(clipboard_class)
        class_destroy(clipboard_class);
    if(number>=0)
        unregister_chrdev_region(number,1);
}

module_init(clipboard_init);
module_exit(clipboard_exit);
MODULE_LICENSE("GPL");