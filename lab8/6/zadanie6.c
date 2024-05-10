#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<asm/uaccess.h>
#include<linux/sysfs.h>
#include<linux/kobject.h>

#define NAME1 "device1"
bool wasReaded_device1 = false;		// zmienna zapobiegająca zapętlaniu się odczytu z urządzenia 1

static struct kobject *kernel_object;
static char bufor_sysfs[64];

static ssize_t device1_read(struct file *f, char __user *u, size_t size, loff_t* pos)
{
	if(!wasReaded_device1) {
        int size_data = sizeof(bufor_sysfs);

		if(copy_to_user(u,bufor_sysfs,size_data)) {
			return -EIO;
		}
		wasReaded_device1 = true;
    
		return size_data;
	}
	wasReaded_device1 = false;
    
	return 0;
}

static ssize_t device1_write(struct file *f, const char __user *u, size_t size, loff_t* pos)
{
    return 0;
}

static int device_open(struct inode *ind, struct file *f)
{
        return 0;
}

static int device_release(struct inode *ind, struct file *f)
{
        return 0;
}

static struct file_operations device1_op =
{
        .owner = THIS_MODULE,
        .open = device_open,
        .release = device_release,
        .read = device1_read,
        .write = device1_write,
};

static dev_t number1 = 0;
static struct cdev device1_cdev;
static struct class *device1_class;
static struct device *device1;

static ssize_t text_show(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%s\n",bufor_sysfs);
}

static ssize_t text_store(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer, size_t count)
{
    sprintf(bufor_sysfs,"%s",buffer);
	return count;
}

static struct kobj_attribute text_kattribute = 
__ATTR(text,0664,text_show,text_store);

static struct attribute *text_attribute = &text_kattribute.attr;

static int __init devices_init(void)
{
    // init device 1
    if(alloc_chrdev_region(&number1,0,1,NAME1)<0) {
        printk(KERN_ALERT"[device 1]: Region allocation error!\n");
        return-1;
    }
	device1_class = class_create(THIS_MODULE,NAME1);
    if(IS_ERR(device1_class)) {
        printk(KERN_ALERT"[device 1]: Error creating class: %ld!\n",PTR_ERR(device1_class));
        unregister_chrdev_region(number1,1);
        return -1;
    }

	cdev_init(&device1_cdev,&device1_op);
    device1_cdev.owner = THIS_MODULE;
    
	if(cdev_add(&device1_cdev,number1,1)) {
    	printk(KERN_ALERT"[device 1]: Error adding cdev!\n");
        class_destroy(device1_class);
        unregister_chrdev_region(number1,1);
        return -1;
    }

    device1 = device_create(device1_class, NULL, number1, NULL, NAME1);
    if(IS_ERR(device1)) {
        printk(KERN_ALERT"[device 1]: Error creating device: %ld!\n",PTR_ERR(device1));
        cdev_del(&device1_cdev);
        class_destroy(device1_class);
        unregister_chrdev_region(number1,1);
        return -1;
	}

    // init sysfs file
    kernel_object = kobject_create_and_add("subdir",&THIS_MODULE->mkobj.kobj);
	if(!kernel_object) 
		goto err;

	if(sysfs_create_file(kernel_object,text_attribute))
		goto err1;

	return 0;
	err:
		printk(KERN_ALERT "Could not create a kobject!\n");
		return -ENOMEM;
	err1:
		kobject_del(kernel_object);
		printk(KERN_ALERT "Could not create a sysfs file!\n");
		return -ENOMEM;
        
    return 0;
}

static void __exit devices_exit(void)
{
    // end device 1
    if(device1)
        device_destroy(device1_class,number1);
    cdev_del(&device1_cdev);
    if(device1_class)
        class_destroy(device1_class);
    if(number1>=0)
        unregister_chrdev_region(number1,1);
    
    // end sysfs file
    sysfs_remove_file(kernel_object,text_attribute);
	if(kernel_object)
		kobject_del(kernel_object);
}

module_init(devices_init);
module_exit(devices_exit);
MODULE_LICENSE("GPL");