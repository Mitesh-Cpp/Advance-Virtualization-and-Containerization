#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mitesh Khemani");
MODULE_DESCRIPTION("Driver for virtual-to-physical address translation and memory write using physical address");

#include "my_ioctl_header.h"

long int page_table_walk(int given_pid, unsigned long given_virtual_address) {
    struct task_struct *task;
    for_each_process(task)
    {
        if(task->pid == given_pid)
        {
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
            long int translated_physical_address = (int)(pte_val(*page_table_entry) & PAGE_MASK) | (int)((~PAGE_MASK) & given_virtual_address);
            return translated_physical_address;
        }
    }
    printk(KERN_INFO "Process with given PID not found..!!\n");
    return 0;
}

static long my_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_GET_PHYSICAL_ADDR: {
            struct task_struct *task = current;
            struct GET_P_ADDR gpa;
            // copy_from_user() for copying the data from user space to kernel space
            if (copy_from_user(&gpa, (struct GET_P_ADDR *)arg, sizeof(gpa))) {
                printk(KERN_INFO "copy_from_user() failed..!!\n");
                break;
            }
            // same page_table_walk as in lkm3
            gpa.physical_addr = page_table_walk(task->pid, gpa.virtual_addr);
            // copy_to_user() for copying the data from kernel to user space to kernel space
            if (copy_to_user((struct GET_P_ADDR *)arg, &gpa, sizeof(gpa))) {
                printk(KERN_INFO "copy_to_user() failed..!!\n");
                break;
            }
            printk(KERN_INFO "Physical address: 0x%lx\n", gpa.physical_addr);
            break;
        }
        case IOCTL_WRITE_PHYSICAL_MEM: {
            struct WRITE_P_MEM wpm;
            if (copy_from_user(&wpm, (struct WRITE_P_MEM *)arg, sizeof(wpm))) {
                printk(KERN_INFO "copy_from_user() failed..!!\n");
                break;
            }
            unsigned long *virt_addr = phys_to_virt(wpm.physical_addr);
            if (!virt_addr) {
                printk(KERN_INFO "phys_to_virt() failed..!!");
                break;
            }
            *virt_addr = wpm.value;
            break;
        }
    }
    return 0;
}

// specifying the required file operations
static const struct file_operations my_fops = {
  .owner = THIS_MODULE,
  .unlocked_ioctl = my_ioctl,
};

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;

static int __init my_init(void)
{
    int ret;
    struct device *dev_ret;
    // setting up the character device file
    if ((ret = alloc_chrdev_region(&dev, 0, 1, "my_char")) < 0)
    {
        return ret;
    }
    cdev_init(&c_dev, &my_fops);
    if ((ret = cdev_add(&c_dev, dev, 1)) < 0)
    {
        return ret;
    }
    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "my_char_device")))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(dev_ret);
    }
 
    return 0;
}
 
static void __exit my_exit(void)
{
    // for cleanup purpose
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, 1);
}

module_init(my_init);
module_exit(my_exit);