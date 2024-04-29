#include <linux/module.h>
#include <linux/hrtimer.h>

static struct hrtimer timer1, timer2;
static ktime_t delay1, delay2;

// zmienna współużytkowna przez dwie funkcje
int zmienna = 0;

// funkcja wywoływana przez timer 1
static void function1(void) {
	pr_info("[Funkcja 1] Wartość zmiennej współużytkowanej wynosi: %d\n", zmienna);
	zmienna += 3;
	
	return;
}

// funkcja wywoływana przez timer 2
static void function2(void) {
	pr_info("[Funkcja 2] Wartość zmiennej współużytkowanej wynosi: %d\n", zmienna);
	zmienna -= 2;

	return;
}

// funkcja wykonywana w ramach obsługi timera 1
static enum hrtimer_restart hrtimer_function1(struct hrtimer *hrtimer)
{
	function1();
	hrtimer_forward_now(hrtimer,delay1);

	return HRTIMER_RESTART;
}

// funkcja wykonywana w ramach obsługi timera 2
static enum hrtimer_restart hrtimer_function2(struct hrtimer *hrtimer)
{
	function2();
	hrtimer_forward_now(hrtimer,delay2);

	return HRTIMER_RESTART;
}

static int __init hrtimer_module_init(void) {

	// ustawianie parametrów timera 1
	delay1 = ktime_set(1,0);
	hrtimer_init(&timer1,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	timer1.function = hrtimer_function1;
	hrtimer_start(&timer1,delay1,HRTIMER_MODE_REL);

	// ustawianie parametrów timera 2
	delay2 = ktime_set(1,0);
	hrtimer_init(&timer2,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	timer2.function = hrtimer_function2;
	hrtimer_start(&timer2,delay2,HRTIMER_MODE_REL);

	return 0;
}

static void __exit hrtimer_module_exit(void) {

	if(!hrtimer_cancel(&timer1)) {
		pr_alert("Timer 1 nie był aktywowany!\n");
	}

	if(!hrtimer_cancel(&timer2)) {
		pr_alert("Timer 2 nie był aktywowany!\n");
	}
}

module_init(hrtimer_module_init);
module_exit(hrtimer_module_exit);

MODULE_LICENSE("GPL");