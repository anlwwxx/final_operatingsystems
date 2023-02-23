#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mychardev"
#define BUF_LEN 80
MODULE_LICENSE("GPL");
MODULE_AUTHOR("AnelTMR, AnelTRF, ZhannaM");
MODULE_DESCRIPTION("Character device driver module");

static int major_num;
static char msg[BUF_LEN];
static char *msg_ptr;

static int device_open(struct inode *inode, struct file *file)
{
    if (msg_ptr) {
        return -EBUSY;
    }

    msg_ptr = msg;
    try_module_get(THIS_MODULE);

    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    msg_ptr = NULL;
    module_put(THIS_MODULE);

    return 0;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    ssize_t bytes_read = 0;

    while (length && *msg_ptr) {
        put_user(*(msg_ptr++), buffer++);
        length--;
        bytes_read++;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    ssize_t bytes_written = 0;

    while (length && (bytes_written < BUF_LEN)) {
        get_user(msg[bytes_written], buffer++);
        length--;
        bytes_written++;
    }

    msg_ptr = msg;

    return bytes_written;
}

static struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

static int __init mychardev_init(void)
{
    major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
    if (major_num < 0) {
        printk(KERN_ALERT "Failed to register a major number.\n");
        return major_num;
    }

    printk(KERN_INFO "Registered device with major number %d.\n", major_num);

    return 0;
}

static void __exit mychardev_exit(void)
{
    unregister_chrdev(major_num, DEVICE_NAME);

    printk(KERN_INFO "Unregistered device with major number %d.\n", major_num);
}

module_init(mychardev_init);
module_exit(mychardev_exit);
