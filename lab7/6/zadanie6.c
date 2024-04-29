#include <linux/module.h>
#include <linux/hrtimer.h>
#include<linux/list.h>
#include<linux/slab.h>
#include<linux/random.h>

struct example_struct 
{
	int elementNumber;
	int random_value;
	struct list_head list_element;
};

static struct hrtimer timer1, timer2;
static ktime_t delay1, delay2;

struct example_struct *element;
struct list_head *entry;

static LIST_HEAD(head);

int maxElements = 100; // określa maksymalną ilość elementów możliwych do dodania do listy
int countElementsIterator = 0; // określa bieżącą ilość elementów w liście

// funkcja wykonywana w ramach obsługi timera 1 - tworzy i dodaje na koniec listy kolejne elementy
static enum hrtimer_restart hrtimer_function1(struct hrtimer *hrtimer)
{
	if(countElementsIterator < maxElements) {
		element = (struct example_struct *)kmalloc(sizeof(struct example_struct),GFP_KERNEL);
		if(!IS_ERR(element)) {
			get_random_bytes((int *)&element->random_value,sizeof(element->random_value));
			element->elementNumber = countElementsIterator;
			INIT_LIST_HEAD(&element->list_element);
			list_add_tail(&element->list_element,&head);
		}

		countElementsIterator++;
	}

	hrtimer_forward_now(hrtimer,delay1);

	return HRTIMER_RESTART;
}

// funkcja wykonywana w ramach obsługi timera 2 - odczytuje i wyświetla zawartość listy
static enum hrtimer_restart hrtimer_function2(struct hrtimer *hrtimer)
{
	pr_info("Zawartość listy:\n");

	list_for_each(entry, &head) {
		element = list_entry(entry, struct example_struct, list_element);
		pr_notice("Element nr %d - wartość: %u\n", element->elementNumber, element->random_value);
	}

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
	delay2 = ktime_set(3,0);
	hrtimer_init(&timer2,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	timer2.function = hrtimer_function2;
	hrtimer_start(&timer2,delay2,HRTIMER_MODE_REL);

	return 0;
}

static void __exit hrtimer_module_exit(void) {
	struct example_struct *element, *next;

	// usuwanie timerów
	if(!hrtimer_cancel(&timer1)) {
		pr_alert("Timer 1 nie był aktywowany!\n");
	}
	if(!hrtimer_cancel(&timer2)) {
		pr_alert("Timer 2 nie był aktywowany!\n");
	}

	// usuwanie listy
	list_for_each_entry_safe(element, next, &head, list_element) {
		 list_del(&element->list_element);
		 kfree(element);
	 }
}

module_init(hrtimer_module_init);
module_exit(hrtimer_module_exit);

MODULE_LICENSE("GPL");