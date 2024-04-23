#include<linux/module.h>
#include<linux/kthread.h>
#include<linux/wait.h>
#include<linux/completion.h>
#include<linux/slab.h>

enum thread_index {WAKING_THREAD, WRITER_THREAD, READER_THREAD};

static struct thread_structure 
{
	struct task_struct *thread[3];
} threads;

static LIST_HEAD(head);

static wait_queue_head_t wait_queue;
static bool condition;
// makro deklarujące i inicjalizujące zmienną sygnałową
static DECLARE_COMPLETION(number_completion);
// dziesięcioelementowa tablica
static int tablica[10];
// iterator dla tablicy
static int iterator = -1;
// bufor do przechowywania kolejnej wartości dla producenta
static int number;

// makro deklarujące i inicjalizujące muteks
static DEFINE_MUTEX(number_lock);

int returned_value;
unsigned int returned_size;

static int writer_thread(void *data) 
{
	DEFINE_WAIT(wait);
	for(;;) {
		number++;
		iterator++;
		// tworzenie nowego elementu w tablicy i przydzielanie wartości równej aktualnej wartości bufora
		tablica[iterator] = number;

		pr_info("[Producent] Zapisano do tablicy wartość: %d\n", tablica[iterator]);
		
		// masz panie areczku zarówno zmienną sygnałową, jak i muteks
		complete(&number_completion);
		mutex_unlock(&number_lock);

		add_wait_queue(&wait_queue,&wait);
		while(!condition) {
			prepare_to_wait(&wait_queue,&wait,TASK_INTERRUPTIBLE); 
			if(kthread_should_stop()) 
				return 0;
			schedule();
		}
		condition=false;
		finish_wait(&wait_queue,&wait); 
	}
}

static int reader_thread(void *data) 
{
	DEFINE_WAIT(wait);
	for(;;) {
		// masz panie areczku zarówno zmienną sygnałową, jak i muteks
		wait_for_completion(&number_completion);
		mutex_lock(&number_lock);

		// odczytanie i usuwanie bieżącego elementu tablicy
		pr_info("[Konsument]: Odczytano z tablicy wartość: %d\n", tablica[iterator]);
		iterator--;
		
		if(kthread_should_stop())
			return 0;
		schedule();
	}
}

static int waking_thread(void *data)
{
	for(;;) {
		if(kthread_should_stop())
			return 0;
		set_current_state(TASK_INTERRUPTIBLE);
		if(schedule_timeout(1*HZ))
			printk(KERN_INFO "Signal received!\n");
		condition=true;
		wake_up(&wait_queue);
	}

}

static int __init threads_init(void)
{
	mutex_lock(&number_lock);

	init_waitqueue_head(&wait_queue);
	threads.thread[READER_THREAD] = kthread_run(reader_thread,NULL,"reader_thread");
	threads.thread[WRITER_THREAD] = kthread_run(writer_thread,NULL,"writer_thread");
	threads.thread[WAKING_THREAD] = kthread_run(waking_thread,NULL,"waking_thread");
	return 0;
}

static void __exit threads_exit(void)
{
	int i;

	kthread_stop(threads.thread[READER_THREAD]);
	kthread_stop(threads.thread[WAKING_THREAD]);
	kthread_stop(threads.thread[WRITER_THREAD]);

	for(i = 0; i <= iterator; i++) {
		pr_notice("W tablicy pozostał element o wartości: %d\n",tablica[i]);
	}
}

module_init(threads_init);
module_exit(threads_exit);

MODULE_LICENSE("GPL");