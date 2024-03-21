#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rbtree.h>
#include <linux/slab.h>

#define BUF_SIZE 1024

struct my_data {
    unsigned int key;
    char letter;
    struct rb_node node;
};

static struct rb_root my_tree = RB_ROOT;
static char kernel_buffer[BUF_SIZE];
static int buffer_index = 0;

static void insert_data(struct my_data *data) {
    struct rb_node **new = &(my_tree.rb_node);
    struct rb_node *parent = NULL;

    while (*new) {
        struct my_data *this = container_of(*new, struct my_data, node);
        parent = *new;
        if (data->key < this->key)
            new = &((*new)->rb_left);
        else if (data->key > this->key)
            new = &((*new)->rb_right);
        else
            return; // Already exists
    }

    rb_link_node(&data->node, parent, new);
    rb_insert_color(&data->node, &my_tree);
}

static void in_order_traversal(struct rb_node *node) {
    if (node) {
        struct my_data *data = container_of(node, struct my_data, node);
        in_order_traversal(node->rb_left);
        buffer_index += snprintf(kernel_buffer + buffer_index, BUF_SIZE - buffer_index, "Key: %u, Letter: %c\n", data->key, data->letter);
        in_order_traversal(node->rb_right);
    }
}

static int __init my_module_init(void) {
    int i;
    struct my_data *data;

    printk(KERN_INFO "My Module: Init\n");

    // Insert data into the RB tree
    for (i = 0; i < 10; ++i) {
        data = kmalloc(sizeof(struct my_data), GFP_KERNEL);
        if (!data) {
            printk(KERN_ALERT "Failed to allocate memory for data\n");
            return -ENOMEM;
        }
        data->key = i;
        data->letter = 'a' + (i % 26); // Wylosowana mała litera
        RB_CLEAR_NODE(&data->node);
        insert_data(data);
    }

    // Traverse the RB tree and store values in kernel buffer
    in_order_traversal(my_tree.rb_node);
    printk(KERN_INFO "My Module: Data in RB tree:\n%s", kernel_buffer);

    return 0;
}

static void __exit my_module_exit(void) {
    struct my_data *data, *temp;

    printk(KERN_INFO "My Module: Exit\n");

    // Free RB tree nodes
    rbtree_postorder_for_each_entry_safe(data, temp, &my_tree, node) {
        rb_erase(&data->node, &my_tree);
        kfree(data);
    }
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
