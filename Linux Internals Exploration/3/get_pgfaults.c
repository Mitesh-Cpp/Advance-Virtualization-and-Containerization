#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>

// cat /proc/get_pgfaults_procfs
#define PROCFS_NAME "get_pgfaults_procfs"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mitesh Khemani");
MODULE_DESCRIPTION("procfs Total Page Faults since Bootup..!!");

static int get_pgfaults_procfs_show(struct seq_file *m, void *v) {
    unsigned long num_total_page_faults;
    unsigned long all_events[NR_VM_EVENT_ITEMS];
    all_vm_events(all_events);
    num_total_page_faults = all_events[PGFAULT];
    seq_printf(m, "%lu\n", num_total_page_faults);
    return 0;
}

static int get_pgfaults_procfs_open(struct inode *inode, struct file *file) {
    return single_open(file, get_pgfaults_procfs_show, NULL);
}

// configuring proc file operations
static const struct proc_ops hello_procfs_proc_ops = {
    .proc_open    = get_pgfaults_procfs_open,
    .proc_read    = seq_read,
};

// creating proc file entry
static int __init get_pgfaults_procfs_init(void) {
    printk(KERN_INFO "pgfaults_procfs module loaded\n");
    proc_create(PROCFS_NAME, 0, NULL, &hello_procfs_proc_ops);
    return 0;
}

// creating proc file entry
static void __exit get_pgfaults_procfs_exit(void) {
    remove_proc_entry(PROCFS_NAME, NULL);
    printk(KERN_INFO "pgfaults_procfs module unloaded\n");
}

module_init(get_pgfaults_procfs_init);
module_exit(get_pgfaults_procfs_exit);