#include <linux/module.h>
#include <linux/timer.h>

static struct timer_list timer;

static void timer_handler(unsigned long int data) {
	
	pr_info("Timer %lu is active!\n", data);
}

static int __init timer_module_init(void) {

	init_timer(&timer);
	timer.expires = jiffies + 15*HZ;
	timer.data = 0;
	timer.function = timer_handler;	
	add_timer(&timer);
	return 0;
}

static void __exit timer_module_exit(void) {
	
	if (del_timer_sync(&timer)) {
		pr_notice("The timer was not active!\n");
	}
}

module_init(timer_module_init);
module_exit(timer_module_exit);

MODULE_LICENSE("GPL");