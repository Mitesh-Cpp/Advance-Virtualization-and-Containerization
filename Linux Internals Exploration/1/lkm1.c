#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mitesh Khemani");
MODULE_DESCRIPTION("Kernel module to list processes in a running or runnable state");

static int __init lkm1_init(void) {
    printk(KERN_INFO "lkm1 module loaded\n");
    rcu_read_lock();
    struct task_struct *task;
    printk(KERN_INFO "Listing processes in a running or runnable state:\n");
    // __state variable in task struct stores the state of the process, comm stores the name
    for_each_process(task) {
        if (task->__state == TASK_RUNNING) {
            printk(KERN_INFO "PID: %d, Name: %s\n", task->pid, task->comm);
        }
    }
    rcu_read_unlock();
    return 0;
}

static void __exit lkm1_exit(void) {
    printk(KERN_INFO "lkm1 module unloaded\n");
}

module_init(lkm1_init);
module_exit(lkm1_exit);