#include<linux/module.h>
#include<linux/sysfs.h>
#include<linux/kobject.h>
#include<linux/random.h>

static struct kobject *kernel_object, *kernel_object_parent;
static int number;
static int file[7] = { 0 };
bool wasUsed = false;

static ssize_t
number_show1(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%d\n",file[0]);
}
static ssize_t
number_store1(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer,
size_t count)
{
	sscanf(buffer,"%d",&file[0]);
	return count;
}

static ssize_t
number_show2(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%d\n",file[1]);
}
static ssize_t
number_store2(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer,
size_t count)
{
	sscanf(buffer,"%d",&file[1]);
	return count;
}

static ssize_t
number_show3(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%d\n",file[2]);
}
static ssize_t
number_store3(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer,
size_t count)
{
	sscanf(buffer,"%d",&file[2]);
	return count;
}

static ssize_t
number_show4(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%d\n",file[3]);
}
static ssize_t
number_store4(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer,
size_t count)
{
	sscanf(buffer,"%d",&file[3]);
	return count;
}

static ssize_t
number_show5(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%d\n",file[4]);
}
static ssize_t
number_store5(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer,
size_t count)
{
	sscanf(buffer,"%d",&file[4]);
	return count;
}

static ssize_t
number_show6(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%d\n",file[5]);
}
static ssize_t
number_store6(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer,
size_t count)
{
	sscanf(buffer,"%d",&file[5]);
	return count;
}

static ssize_t
number_show7(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%d\n",file[6]);
}
static ssize_t
number_store7(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer,
size_t count)
{
	sscanf(buffer,"%d",&file[6]);
	return count;
}

static struct kobj_attribute number_kattribute1 = __ATTR(file1,0664,number_show1,number_store1);
static struct kobj_attribute number_kattribute2 = __ATTR(file2,0664,number_show2,number_store2);
static struct kobj_attribute number_kattribute3 = __ATTR(file3,0664,number_show3,number_store3);
static struct kobj_attribute number_kattribute4 = __ATTR(file4,0664,number_show4,number_store4);
static struct kobj_attribute number_kattribute5 = __ATTR(file5,0664,number_show5,number_store5);
static struct kobj_attribute number_kattribute6 = __ATTR(file6,0664,number_show6,number_store6);
static struct kobj_attribute number_kattribute7 = __ATTR(file7,0664,number_show7,number_store7);

static struct attribute *number_attribute1[7] = { &number_kattribute1.attr, &number_kattribute2.attr, &number_kattribute3.attr, &number_kattribute4.attr, &number_kattribute5.attr, &number_kattribute6.attr, &number_kattribute7.attr };

static ssize_t
number_show(struct kobject *kernel_object, struct kobj_attribute *attribute, char *buffer)
{
	return sprintf(buffer,"%d\n",number);
}

static ssize_t
number_store(struct kobject *kernel_object, struct kobj_attribute *attribute, const char *buffer,
size_t count)
{
	int a, i;

	if(!wasUsed) {
		unsigned char temp;
		sscanf(buffer,"%d",&a);

		if(a >= 1 && a <= 7)
		{
			wasUsed = true;
			sscanf(buffer,"%d",&number);

			for(i = 0; i < a; i++)
			{
				if(sysfs_create_file(kernel_object,number_attribute1[i]))
				{
					kobject_del(kernel_object);
					printk(KERN_ALERT "nie udalo sie stworzyc pliku!\n");
					return -ENOMEM;
				}

				get_random_bytes(&temp, 1);
				file[i] = temp;
			}
		}
	} else {
		printk(KERN_INFO "zakonczono\n");
	}


	return count;
}

static struct kobj_attribute number_kattribute =
__ATTR(number,0664,number_show,number_store);

static struct attribute *number_attribute = &number_kattribute.attr;

static int __init sysfs_test_init(void)
{
	kernel_object_parent = kobject_create_and_add("zadanie6", NULL);
	if(!kernel_object_parent)
		goto err;

	kernel_object = kobject_create_and_add("subdir", kernel_object_parent);
	if(!kernel_object)
		goto err;

	if(sysfs_create_file(kernel_object,number_attribute))
		goto err1;

	return 0;
err:
	printk(KERN_ALERT "nie stworzono obiektu\n");
	return -ENOMEM;
err1:
	kobject_del(kernel_object);
	printk(KERN_ALERT "nie stworzono sys \n");
	return -ENOMEM;
}

static void __exit sysfs_test_exit(void)
{
	sysfs_remove_file(kernel_object,number_attribute);
	if(kernel_object)
		kobject_del(kernel_object);

	if(kernel_object_parent)
		kobject_del(kernel_object_parent);
}

module_init(sysfs_test_init);
module_exit(sysfs_test_exit);
MODULE_LICENSE("GPL");
