#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mydevice"
#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AnelTRF, AnelTMR, ZhannaM");
MODULE_DESCRIPTION("Device kernel module");

static int Major;
static char msg[BUFFER_SIZE];
static int msg_size;

static int device_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset) {
    if (*offset >= msg_size) {
        return 0;
    }
    size_t size = msg_size - *offset;
    if (length < size) {
        size = length;
    }
    if (copy_to_user(buffer, msg + *offset, size) != 0) {
        return -EFAULT;
    }
    *offset += size;
    return size;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t *offset) {
    if (length > BUFFER_SIZE) {
        return -EINVAL;
    }
    if (copy_from_user(msg, buffer, length) != 0) {
        return -EFAULT;
    }
    msg_size = length;
    return length;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

static int __init mydevice_init(void) {
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if (Major < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return Major;
    }
    printk(KERN_INFO "Registered character device with major number %d\n", Major);
    return 0;
}

static void __exit mydevice_exit(void) {
    unregister_chrdev(Major, DEVICE_NAME);
    printk(KERN_INFO "Unregistered character device\n");
}

module_init(mydevice_init);
module_exit(mydevice_exit);
