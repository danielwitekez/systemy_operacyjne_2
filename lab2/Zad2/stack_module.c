#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#define STACK_SIZE 10

struct stack_node {
    int data;
    struct stack_node *next;
};

struct stack {
    struct stack_node *top;
};

static struct stack *my_stack;

static void stack_init(void) {
    my_stack = kmalloc(sizeof(struct stack), GFP_KERNEL);
    if (!my_stack) {
        printk(KERN_ALERT "Failed to allocate memory for stack\n");
        return;
    }
    my_stack->top = NULL;
}

static void stack_push(int data) {
    struct stack_node *new_node = kmalloc(sizeof(struct stack_node), GFP_KERNEL);
    if (!new_node) {
        printk(KERN_ALERT "Failed to allocate memory for stack node\n");
        return;
    }
    new_node->data = data;
    new_node->next = my_stack->top;
    my_stack->top = new_node;
}

static int stack_pop(void) {
    if (!my_stack->top) {
        printk(KERN_ALERT "Stack underflow\n");
        return -1;
    }
    int data = my_stack->top->data;
    struct stack_node *temp = my_stack->top;
    my_stack->top = my_stack->top->next;
    kfree(temp);
    return data;
}

static void stack_destroy(void) {
    printk(KERN_INFO "Stack contents:\n");
    while (my_stack->top) {
        printk(KERN_INFO "%d\n", stack_pop());
    }
    kfree(my_stack);
}

static int __init stack_module_init(void) {
    printk(KERN_INFO "Stack module initialized\n");
    stack_init();
    // Test push and pop operations
    stack_push(1);
    stack_push(2);
    stack_push(3);
    stack_push(4);
    return 0;
}

static void __exit stack_module_exit(void) {
    printk(KERN_INFO "Stack module exiting\n");
    stack_destroy();
}

module_init(stack_module_init);
module_exit(stack_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("maciek i daniel");
MODULE_DESCRIPTION(" asd");
