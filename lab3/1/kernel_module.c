#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/fdtable.h>

#define BUF_SIZE 256

static int pid_of_interest = YOUR_PID;

static int __init my_module_init(void) {
    struct task_struct *task;
    struct files_struct *files;
    struct file *file;
    char buf[BUF_SIZE];
    int i = 0;

    printk(KERN_INFO "My Module: Init\n");

    for_each_process(task) {
        if (task->pid == pid_of_interest) {
            printk(KERN_INFO "My Module: Found process with PID %d\n", pid_of_interest);
            files = task->files;
            spin_lock(&files->file_lock);
            for (i = 0; i < files->max_fds; ++i) {
                file = files->fd[i];
                if (file != NULL) {
                    snprintf(buf, BUF_SIZE, "File descriptor: %d, File name: %s\n", i, file->f_path.dentry->d_name.name);
                    printk(KERN_INFO "%s", buf);
                }
            }
            spin_unlock(&files->file_lock);
            break;
        }
    }

    return 0;
}

static void __exit my_module_exit(void) {
    printk(KERN_INFO "My Module: Exit\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
