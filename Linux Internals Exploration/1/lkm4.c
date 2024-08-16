#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mitesh Khemani");
MODULE_DESCRIPTION("Kernel module that prints the size of the allocated virtual address space (sum of all vmas) and the mapped physical address space of a given process");

int given_pid = -1;
module_param(given_pid, int, 0);

static int get_vm_info(int given_pid) {
    struct task_struct *task;
    struct mm_struct *mm;

    // getting the task_struct of the process with given_pid
    task = pid_task(find_vpid(given_pid), PIDTYPE_PID);
    if (task == NULL) {
        printk(KERN_INFO "Given PID not found\n");
        return 0;
    }

    // getting the memory map of the process with given_pid using its task_struct
    mm = get_task_mm(task);
    if (mm == NULL) {
        printk(KERN_INFO "Memory map of the given process not found\n");
        return 0;
    }

    int mapped_physical_address_space = get_mm_counter(mm, MM_FILEPAGES) + get_mm_counter(mm, MM_ANONPAGES) + get_mm_counter(mm, MM_SHMEMPAGES) + get_mm_counter(mm, MM_SWAPENTS);

    // Multiplying by 4, as page size is 4 KB
    printk(KERN_INFO "Total virtual address space = %d KB\n", mm->total_vm * 4);
    printk(KERN_INFO "Total mapped physical address space = %d KB\n", mapped_physical_address_space * 4);

    return 0;
}

static int lkm4_init(void)
{
    printk(KERN_INFO "lkm4 module loaded\n");
    if (given_pid == -1) {
        return -EINVAL;
    }
    rcu_read_lock();
    get_vm_info(given_pid);
    rcu_read_unlock();
    return 0;
}

static void lkm4_exit(void)
{
    printk(KERN_INFO "lkm4 module unloaded\n");
}

module_init(lkm4_init);
module_exit(lkm4_exit);