#include <linux/module.h>
#include <linux/hrtimer.h>
#include<asm/uaccess.h>
#include<linux/proc_fs.h>
#include<linux/seq_file.h>

static struct hrtimer timer1, timer2;
static ktime_t delay1, delay2;

static struct proc_dir_entry *directory_entry_pointer, *file_entry_pointer;
static char *directory_name = "procfs_test", *file_name = "procfs_file";	// określanie nazw dla folderu i pliku w systemie plików procfs
static char file_buffer[PAGE_SIZE];		// bufor używany przez system plików procfs

static int procfsmod_show(struct seq_file *seq, void *data)
{
	char *notice = (char *)data;
	seq_putc(seq,*notice);
	return 0;
}

static void *procfsmod_seq_start(struct seq_file *s, loff_t *position)
{
	loff_t buffer_index = *position;
	return (buffer_index<PAGE_SIZE && file_buffer[buffer_index]!='\0')?(void *)&file_buffer[buffer_index]:NULL;
}

static void *procfsmod_seq_next(struct seq_file *s, void *data, loff_t *position)
{
	loff_t next_element_index = ++*position;
	return (next_element_index<PAGE_SIZE && file_buffer[next_element_index]!='\0')?(void *)&file_buffer[next_element_index]:NULL; 
}

static void procfsmod_seq_stop(struct seq_file *s, void *data)
{
}

static struct seq_operations procfsmod_seq_operations = {
	.start = procfsmod_seq_start,
	.next = procfsmod_seq_next,
	.stop = procfsmod_seq_stop,
	.show = procfsmod_show
};

static int procfsmod_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &procfsmod_seq_operations);
}

static ssize_t procfsmod_wirte(struct file *file, const char __user *buffer, size_t count, loff_t *position)
{
	int length = count;
	if(count>PAGE_SIZE)
		length=PAGE_SIZE-1;
	if(copy_from_user(file_buffer,buffer,length))
		return -EFAULT;
	file_buffer[length]='\0';
	return length;
}

static struct file_operations procfsmod_fops = {
	.owner = THIS_MODULE,
	.open = procfsmod_open,
	.read = seq_read,
	.write = procfsmod_wirte,
	.llseek = seq_lseek,
	.release = seq_release
};

// funkcja wykonywana w ramach obsługi timera 1
static enum hrtimer_restart hrtimer_function1(struct hrtimer *hrtimer)
{
	pr_info("Wykonano funkcję timera 1\n");

	return HRTIMER_NORESTART;
}

// funkcja wykonywana w ramach obsługi timera 2
static enum hrtimer_restart hrtimer_function2(struct hrtimer *hrtimer)
{
	s64 zmienna1, zmienna2;
	ktime_t struct2;
	int typLicznika;
	char temp1[20], temp2[20];

	// czyszczenie zawartości bufora
	memset(file_buffer, 0, sizeof(file_buffer));

	// pobieranie danych
	zmienna1 = hrtimer_get_expires_ns(&timer1);
	struct2 = hrtimer_expires_remaining(&timer1);
	zmienna2 = ktime_to_ns(struct2);
	typLicznika = hrtimer_is_hres_active(&timer1);

	// konwersja danych liczbowych na łańcuchy znaków
	sprintf(temp1, "%lld", zmienna1);
	sprintf(temp2, "%lld", zmienna2);

	// zapis danych w buforze
	strcat(file_buffer, "Opóźnienie, po którym funkcja zostanie uruchomiona: ");
	strcat(file_buffer, temp1);
	strcat(file_buffer, "\n");
	strcat(file_buffer, "Czas pozostały do uruchomienia licznika: ");
	strcat(file_buffer, temp2);
	strcat(file_buffer, "\n");
	strcat(file_buffer, "Tryb działania licznika: ");
	if(typLicznika == 1) {
		strcat(file_buffer, "wysokiej rodzielczości\n");
	} else if (typLicznika == 0) {
		strcat(file_buffer, "niskiej rodzielczości\n");
	}

	// ustawienie opóźnienia licznika dla kolejnego cyklu
	hrtimer_forward_now(hrtimer,delay2);

	return HRTIMER_RESTART;
}

static int __init hrtimer_module_init(void) {
	pr_info("Start\n");

	// tworzenie katalogu w systemie plików procfs
	directory_entry_pointer = proc_mkdir(directory_name, NULL);
	if(IS_ERR(directory_entry_pointer)) {
		pr_alert("Error creating procfs directory: %s. Error code: %ld\n",directory_name,PTR_ERR(directory_entry_pointer));
		return -1;
	}
	file_entry_pointer = proc_create_data(file_name,0666,directory_entry_pointer,&procfsmod_fops,(void *)file_buffer);
	if(IS_ERR(file_entry_pointer)) {
		pr_alert("Error creating procfs file: %s. Error code: %ld\n",file_name,PTR_ERR(file_entry_pointer));
		proc_remove(directory_entry_pointer);
		return -1;
	}

	// ustawianie parametrów timera 1
	delay1 = ktime_set(60,0);
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

	// usuwanie timerów
	if(!hrtimer_cancel(&timer1)) {
		pr_alert("Timer 1 nie był aktywny!\n");
	}

	if(!hrtimer_cancel(&timer2)) {
		pr_alert("Timer 2 nie był aktywny!\n");
	}

	// usuwanie pliku i folderu z systemu plików procfs
	if(file_entry_pointer)
		proc_remove(file_entry_pointer);
	if(directory_entry_pointer)
		proc_remove(directory_entry_pointer);
	
	pr_info("Koniec\n");
}

module_init(hrtimer_module_init);
module_exit(hrtimer_module_exit);

MODULE_LICENSE("GPL");