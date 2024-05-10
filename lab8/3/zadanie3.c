#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<asm/uaccess.h>
#include<linux/mutex.h>

#define NAME1 "device1"
#define NAME2 "device2"
static char buf1[64];
static char buf2[64];
static u64 bufor_device1;
static u64 bufor_device2;
bool wasReaded_device1 = false;		// zmienna zapobiegająca zapętlaniu się odczytu z urządzenia 1
bool wasReaded_device2 = false;		// zmienna zapobiegająca zapętlaniu się odczytu z urządzenia 2
int length_device1 = 0;
int length_device2 = 0;

static DEFINE_MUTEX(synchronization_device1);
static DEFINE_MUTEX(synchronization_device2);

static ssize_t device1_read(struct file *f, char __user *u, size_t size, loff_t* pos)
{
	if(!wasReaded_device1) {
        mutex_lock(&synchronization_device1);
        sprintf(buf1,"%llu\n",bufor_device1);

		if(copy_to_user(u,buf1,length_device1)) {
            mutex_unlock(&synchronization_device1);
			return -EIO;
		}
        mutex_unlock(&synchronization_device1);
		wasReaded_device1 = true;
    
		return length_device1;
	}
	wasReaded_device1 = false;
    
	return 0;
}

static ssize_t device1_write(struct file *f, const char __user *u, size_t size, loff_t* pos)
{
	length_device1 = size;

    mutex_lock(&synchronization_device1);
	if(copy_from_user(buf1,u,length_device1)) {
        mutex_unlock(&synchronization_device1);
		return -EFAULT;
	}

    sscanf(buf1, "%llu", &bufor_device1);
    mutex_unlock(&synchronization_device1);
    bufor_device1++;

	wasReaded_device1 = false;
	return length_device1;
}

static ssize_t device2_read(struct file *f, char __user *u, size_t size, loff_t* pos)
{
	if(!wasReaded_device2) {
        mutex_lock(&synchronization_device2);
        sprintf(buf2,"%llu\n",bufor_device2);

		if(copy_to_user(u,buf2,length_device2)) {
            mutex_unlock(&synchronization_device2);
			return -EIO;
		}
        mutex_unlock(&synchronization_device2);
		wasReaded_device2 = true;
    
		return length_device2;
	}
	wasReaded_device2 = false;
    
	return 0;
}

static ssize_t device2_write(struct file *f, const char __user *u, size_t size, loff_t* pos)
{
	length_device2 = size;

    mutex_lock(&synchronization_device2);
	if(copy_from_user(buf2,u,length_device2)) {
        mutex_unlock(&synchronization_device2);
		return -EFAULT;
	}

    sscanf(buf2, "%llu", &bufor_device2);
    mutex_unlock(&synchronization_device2);
    bufor_device2--;

	wasReaded_device2 = false;
	return length_device2;
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

static struct file_operations device2_op =
{
        .owner = THIS_MODULE,
        .open = device_open,
        .release = device_release,
        .read = device2_read,
        .write = device2_write,
};

static dev_t number1 = 0, number2 = 0;
static struct cdev device1_cdev, device2_cdev;
static struct class *device1_class, *device2_class;
static struct device *device1, *device2;

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

    // init device 2
    if(alloc_chrdev_region(&number2,0,1,NAME2)<0) {
        printk(KERN_ALERT"[device 2]: Region allocation error!\n");
        return-1;
    }
	device2_class = class_create(THIS_MODULE,NAME2);
    if(IS_ERR(device2_class)) {
        printk(KERN_ALERT"[device 2]: Error creating class: %ld!\n",PTR_ERR(device2_class));
        unregister_chrdev_region(number2,1);
        return -1;
    }

	cdev_init(&device2_cdev,&device2_op);
    device2_cdev.owner = THIS_MODULE;
    
	if(cdev_add(&device2_cdev,number2,1)) {
    	printk(KERN_ALERT"[device 2]: Error adding cdev!\n");
        class_destroy(device2_class);
        unregister_chrdev_region(number2,1);
        return -1;
    }

    device2 = device_create(device2_class, NULL, number2, NULL, NAME2);
    if(IS_ERR(device2)) {
        printk(KERN_ALERT"[device 2]: Error creating device: %ld!\n",PTR_ERR(device2));
        cdev_del(&device2_cdev);
        class_destroy(device2_class);
        unregister_chrdev_region(number2,1);
        return -1;
	}         
        
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

    // end device 2
    if(device2)
        device_destroy(device2_class,number2);
    cdev_del(&device2_cdev);
    if(device2_class)
        class_destroy(device2_class);
    if(number2>=0)
        unregister_chrdev_region(number2,1);
}

module_init(devices_init);
module_exit(devices_exit);
MODULE_LICENSE("GPL");