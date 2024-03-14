#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

struct list_node {
    int data;
    struct list_node *prev;
    struct list_node *next;
};

static struct list_node *head;

static void list_init(void) {
    head = NULL;
}

static void list_add(int data) {
    struct list_node *new_node = kmalloc(sizeof(struct list_node), GFP_KERNEL);
    if (!new_node) {
        printk(KERN_ALERT "Failed to allocate memory for list node\n");
        return;
    }
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = head;
    if (head)
        head->prev = new_node;
    head = new_node;
}

static void list_destroy(void) {
    struct list_node *current = head;
    struct list_node *temp;
    
    printk(KERN_INFO "List contents (forward):\n");
    while (current) {
        printk(KERN_INFO "%d\n", current->data);
        temp = current;
        current = current->next;
        kfree(temp);
    }

    printk(KERN_INFO "List contents (backward):\n");
    current = temp;
    while (current) {
        printk(KERN_INFO "%d\n", current->data);
        temp = current;
        current = current->prev;
        kfree(temp);
    }
}

static int __init list_module_init(void) {
    printk(KERN_INFO "List module initialized\n");
    list_init();
    // Test add operation
    list_add(1);
    list_add(2);
    list_add(3);
    list_add(4);
    return 0;
}

static void __exit list_module_exit(void) {
    printk(KERN_INFO "List module exiting\n");
    list_destroy();
}

module_init(list_module_init);
module_exit(list_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("maciekidaniel");
MODULE_DESCRIPTION(" as");
