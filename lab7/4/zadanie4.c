#include <linux/module.h>
#include <linux/hrtimer.h>

static struct hrtimer timer;
static ktime_t delay;

static enum hrtimer_restart hrtimer_function(struct hrtimer *hrtimer)
{
	u64 overruns;
	pr_info("The timer is active!\n");
	overruns = hrtimer_forward_now(hrtimer,delay);
	pr_info("The overruns number since last activation: %llu.", overruns);
	return HRTIMER_NORESTART; // tu zmienione tylko
}

static int __init hrtimer_module_init(void) {

	delay = ktime_set(1,0);
	hrtimer_init(&timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	timer.function = hrtimer_function;
	hrtimer_start(&timer,delay,HRTIMER_MODE_REL);
	return 0;
}

static void __exit hrtimer_module_exit(void) {

	if(!hrtimer_cancel(&timer)) {
		pr_alert("The timer was not active!\n");
	}
}

module_init(hrtimer_module_init);
module_exit(hrtimer_module_exit);

MODULE_LICENSE("GPL");