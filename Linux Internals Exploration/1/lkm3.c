#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include<linux/sched/signal.h>
#include<linux/list.h>
#include <linux/mm_types.h>
#include <asm/pgtable.h>
#include<linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mitesh Khemani");
MODULE_DESCRIPTION("Kernel module that takes process ID and virtual address as inputs, and return their (pseudo) physical address");

int given_pid;
module_param(given_pid, int, 0);
unsigned long given_virtual_address;
module_param(given_virtual_address, ulong, 0);

unsigned long int page_table_walk(int given_pid, unsigned long given_virtual_address) {
    struct task_struct *task;
    // finding the task_struct of the process with given pid and then doing the page table walk
    for_each_process(task)
    {
        if(task->pid == given_pid)
        {
            // page_global_directory -> page_level_4_directory -> page_upper_directory -> page_middle_directly -> page_table_entry
            pgd_t *page_global_directory = pgd_offset(task->mm, given_virtual_address);
            if (pgd_none(*page_global_directory) || pgd_bad(*page_global_directory)) {
                printk(KERN_INFO "Page Global Directory entry not found..!!\n");
                return 0;
            }
            p4d_t *page_level_4_directory = p4d_offset(page_global_directory, given_virtual_address);
            if(p4d_none(*page_level_4_directory))
            {
                printk(KERN_INFO "Page Level 4 Directory entry not found..!!\n");
                return 0;
            }
            pud_t *page_upper_directory = pud_offset(page_level_4_directory, given_virtual_address);
            if(pud_none(*page_upper_directory))
            {
                printk(KERN_INFO "Page Upper Directory entry not found..!!\n");
                return 0;
            }
            pmd_t *page_middle_directory = pmd_offset(page_upper_directory, given_virtual_address);
            if(pmd_none(*page_middle_directory))
            {
                printk(KERN_INFO "Page Middle Directory entry not found..!!\n");
                return 0;
            }
            pte_t *page_table_entry = pte_offset_kernel(page_middle_directory, given_virtual_address);
            if(pte_none(*page_table_entry))
            {
                printk(KERN_INFO "Page Table entry not found..!!\n");
                return 0;
            }
            // corresponding physical address = page_frame_number (bitwise or) page_offset
            unsigned long int translated_physical_address = (int)(pte_val(*page_table_entry) & PAGE_MASK) | (int)((~PAGE_MASK) & given_virtual_address);
            return translated_physical_address;
        }
    }
    printk(KERN_INFO "Process with given PID not found..!!\n");
    return 0;
}

static int __init lkm3_init(void) {
    printk(KERN_INFO "lkm3 module loaded\n");
    rcu_read_lock();
    unsigned long int translated_physical_address = page_table_walk(given_pid, given_virtual_address);
    if(translated_physical_address) {
        printk(KERN_INFO "The corresponding physical address is: 0x%lx\n", translated_physical_address);
    }
    rcu_read_unlock();
    return 0;
}

static void __exit lkm3_exit(void) {
    printk(KERN_INFO "lkm3 module unloaded\n");
}

module_init(lkm3_init);
module_exit(lkm3_exit);