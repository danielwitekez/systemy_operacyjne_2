#include <linux/module.h>
#include <linux/timer.h>

static struct timer_list timer;
int delayValue = 1; // wartość startowa opóźnienia
int maxDelay = 10;	// wartość maksymalna opóźnienia

static void timer_handler(unsigned long int data) {
	
	pr_info("Timer nr %lu został aktywowany z opóźnieniem %d sekund!\n", data, delayValue);

	delayValue *= 2;
	timer.expires = jiffies + delayValue*HZ;

	if(delayValue < maxDelay) {
		add_timer(&timer);
	} else {
		pr_info("Przekroczono ustaloną wartość opóźnienia, więc nie będzie kolejnych aktywacji timera\n");
	}
}

static int __init timer_module_init(void) {

	init_timer(&timer);
	timer.expires = jiffies + delayValue*HZ;
	timer.data = 0;
	timer.function = timer_handler;	
	add_timer(&timer);
	return 0;
}

static void __exit timer_module_exit(void) {
	
	if (del_timer_sync(&timer)) {
		pr_notice("Timer nie został aktywowany!\n");
	}
}

module_init(timer_module_init);
module_exit(timer_module_exit);

MODULE_LICENSE("GPL");