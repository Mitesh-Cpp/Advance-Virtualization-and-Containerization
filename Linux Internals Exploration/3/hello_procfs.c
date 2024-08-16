#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>

// cat /proc/hello_procfs
#define PROCFS_NAME "hello_procfs"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mitesh Khemani");
MODULE_DESCRIPTION("procfs Hello World..!!");

static int hello_world_show(struct seq_file *m, void *v) {
    seq_printf(m, "Hello World!\n");
    return 0;
}

static int hello_world_procfs_open(struct inode *inode, struct file *file) {
    return single_open(file, hello_world_show, NULL);
}

// configuring proc file operations
static const struct proc_ops hello_procfs_proc_ops = {
    .proc_open    = hello_world_procfs_open,
    .proc_read    = seq_read,
};

// creating proc file entry
static int __init hello_procfs_init(void) {
    printk(KERN_INFO "hello_procfs module loaded\n");
    proc_create(PROCFS_NAME, 0, NULL, &hello_procfs_proc_ops);
    return 0;
}

// removing proc file entry
static void __exit hello_procfs_exit(void) {
    remove_proc_entry(PROCFS_NAME, NULL);
    printk(KERN_INFO "hello_procfs module unloaded\n");
}

module_init(hello_procfs_init);
module_exit(hello_procfs_exit);