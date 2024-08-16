#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mitesh Khemani");
MODULE_DESCRIPTION("Kernel module that takes process ID as input, and for each of its child processes, print their pid and process state");

int given_pid = -1;
module_param(given_pid, int, 0);

static int __init lkm2_init(void) {
    printk(KERN_INFO "lkm2 module loaded\n");
    rcu_read_lock();
    struct task_struct *task;
    printk(KERN_INFO "Listing child processes' pid and process state:\n");
    // iterating through all the tasks and checking their parents id
    for_each_process(task) {
        if (task && task->real_parent && task->real_parent->pid == given_pid) {
            printk(KERN_INFO "PID: %d, State: %ld\n", task->pid, task->__state);
        }
    }
    rcu_read_unlock();
    return 0;
}

static void __exit lkm2_exit(void) {
    printk(KERN_INFO "lkm2 module unloaded\n");
}

module_init(lkm2_init);
module_exit(lkm2_exit);