#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/signal.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mitesh Khemani");
MODULE_DESCRIPTION("Driver for modifying the task structure of the current process to change its parent process to the process with the given pid");

#include "my_ioctl_header.h"

static long my_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
      case IOCTL_SEND_SIGCHLD_SIGNAL: {
          rcu_read_lock();
          struct SIGCHLD_SIGNAL ss;
          if (copy_from_user(&ss, (struct SIGCHLD_SIGNAL *)arg, sizeof(ss))) {
              printk(KERN_INFO "copy_from_user() failed..!!");
              break;
          }
          struct task_struct *control_station = pid_task(find_get_pid(ss.new_parent_process_id), PIDTYPE_PID);

          // Find and delete the current process from its current parent's children list
          struct list_head *pos, *q;
          list_for_each_safe(pos, q, &current->parent->children) {
              if (list_entry(pos, struct task_struct, sibling) == current) {
                  list_del(pos);
                  break;  // Exit the loop after removing
              }
          }

          // Update parent and sibling links
          current->real_parent = control_station;
          current->parent = control_station;
          // Add to the new parent's children list
          list_add_tail(&current->sibling, &control_station->children);
          rcu_read_unlock();
      }
  }

    return 0;
}

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