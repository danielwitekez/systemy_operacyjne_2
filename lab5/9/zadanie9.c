#include<linux/module.h>
#include<linux/kthread.h>
#include<linux/wait.h>
#include<linux/slab.h>
#include<linux/rcupdate.h>

enum thread_index {WAKING_THREAD, FIRST_WRITER_THREAD, SECOND_WRITER_THREAD, FIRST_READER_THREAD, SECOND_READER_THREAD};

static struct thread_structure 
{
	struct task_struct *thread[5];
} threads;

static wait_queue_head_t wait_queue;
static bool condition;
static int number = 0;
static int *number_pointer;
static const int first_reader_number = 1, second_thread_number = 2;
static const int first_writer_number = 1, second_writer_number = 2;
static DEFINE_MUTEX(thread_lock);

static int reader_thread(void *data) 
{
	int *local_number_pointer = NULL; 
	for(;;) {
		rcu_read_lock();
		local_number_pointer = rcu_dereference(number_pointer);
		if(local_number_pointer)
			pr_info("[reader_number: %d] Value of \"number\" variable: %d\n", *(int *)data, *local_number_pointer);
		rcu_read_unlock();
		if(kthread_should_stop()) 
			return 0;
		set_current_state(TASK_INTERRUPTIBLE);
		if(schedule_timeout(HZ>>2))
			pr_info("Signal received!\n");
	}
}

static int writer_thread(void *data) 
{
	int *local_number_pointer = NULL;
	DEFINE_WAIT(wait);
	for(;;) {
		void *old_pointer = NULL;
		
		local_number_pointer = kmalloc(sizeof(int), GFP_KERNEL);
		if(IS_ERR(local_number_pointer)) {
			pr_alert("Error allocating memory: %ld\n",PTR_ERR(local_number_pointer));
			return 0;
		}
		
		mutex_lock(&thread_lock);
		*local_number_pointer = number++;
		old_pointer = number_pointer;
		rcu_assign_pointer(number_pointer, local_number_pointer);	
		mutex_unlock(&thread_lock);
		synchronize_rcu();

		if(old_pointer)
			kfree(old_pointer);
		add_wait_queue(&wait_queue,&wait);
		while(!condition) {
			prepare_to_wait(&wait_queue,&wait,TASK_INTERRUPTIBLE); 
			if(kthread_should_stop()) 
				return 0;
			pr_info("[writer_number: %d]: awake\n", *(int *)data);
			schedule();
		}
		condition=false;
		finish_wait(&wait_queue,&wait); 

	}
}

static int waking_thread(void *data)
{
	for(;;) {
		if(kthread_should_stop())
			return 0;
		set_current_state(TASK_INTERRUPTIBLE);
		if(schedule_timeout(HZ))
			pr_info("Signal received!\n");
		condition=true;
		wake_up(&wait_queue);
	}
}

static int __init threads_init(void)
{
	init_waitqueue_head(&wait_queue);
	threads.thread[FIRST_WRITER_THREAD] = kthread_run(writer_thread,(void *)&first_writer_number,"first_writer_thread");
	threads.thread[SECOND_WRITER_THREAD] = kthread_run(writer_thread,(void *)&second_writer_number,"second_writer_thread");
	threads.thread[WAKING_THREAD] = kthread_run(waking_thread,NULL,"waking_thread");
	threads.thread[FIRST_READER_THREAD] = kthread_run(reader_thread,(void *)&first_reader_number,"first_reader_thread");
	threads.thread[SECOND_READER_THREAD] = kthread_run(reader_thread,(void *)&second_thread_number,"second_reader_thread");
	return 0;
}

static void __exit threads_exit(void)
{
	kthread_stop(threads.thread[WAKING_THREAD]);
	kthread_stop(threads.thread[FIRST_WRITER_THREAD]);
	kthread_stop(threads.thread[SECOND_WRITER_THREAD]);
	kthread_stop(threads.thread[FIRST_READER_THREAD]);
	kthread_stop(threads.thread[SECOND_READER_THREAD]);
}

module_init(threads_init);
module_exit(threads_exit);

MODULE_LICENSE("GPL");

// Nie jestem pewien czy to git, bo wydaje się że po dwóch uruchomieniach pisarzy wartość powinna być inkrementowana o dwa, a nie o jeden