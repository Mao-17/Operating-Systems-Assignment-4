#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

#define len 8

static DEFINE_SEMAPHORE(sfull);
static DEFINE_SEMAPHORE(semp);
static DEFINE_MUTEX(mutv);
static int max;

module_param(max, int, 0);

static int rear = -1;
static int front = -1;
static char **q_arr;

static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,

};
static struct miscdevice mmd = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "kernel",
    .fops = &my_fops};

static int __init load_init(void)
{
    int ret;
    int i = 0;
    ret = misc_register(&mmd);
    if (ret)
        printk(KERN_ERR "Unable to register \"Fifo Queue\" misc device\n");
    else
    {
        printk("Successful\n");
        sema_init(&sfull, 0);
        sema_init(&semp, max);
    }
    printk("MAX->%d\n", max);
    q_arr = kmalloc(sizeof(char *) * len, GFP_KERNEL);
    if (q_arr == NULL)
    {
        printk("Memory Allocation Error\n");
    }
    for (i = 0; i < max; i++)
    {
        q_arr[i] = kmalloc(sizeof(char) * max, GFP_KERNEL);
        if (q_arr[i] == NULL)
        {
            printk("Memory Allocation Error\n");
        }
    }
    return ret;
}

module_init(load_init);
static void __exit unload_exit(void)
{

    int i;
    for (i = 0; i < max; i++)
    {
        kfree(q_arr[i]);
        q_arr[i] = NULL;
    }
    kfree(q_arr);
    misc_deregister(&mmd);
}
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{

    int err_coun = 0;
    int ret;
    int mutret;
    int value;
    ret = down_interruptible(&sfull);
    if (ret != 0)
    {
        printk("Process wait on semaphore full\n");
        return ret;
    }

    mutret = mutex_lock_interruptible(&mutv);
    if (mutret != 0)
    {
        printk("Process wait on mutex read\n");
        up(&sfull);
        return mutret;
    }
    value = strlen(q_arr[front]);
    err_coun = copy_to_user(buffer, q_arr[front], strlen(q_arr[front]) + 1);

    if (err_coun == 0)
    {

        if (front == rear)
        {
            front = -1;
            rear = -1;
        }
        else
        {
            if (front == max - 1) //
                front = 0;
            else
                front = front + 1;
        }
    }

    else
    {
        printk(KERN_INFO " Failed to send %d characters to the user\n", err_coun);
        mutex_unlock(&mutv);
        up(&semp);
        return -EFAULT;
    }

    mutex_unlock(&mutv);
    up(&semp);

    return value;
}
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{

    int ret;
    int mutret;
    static char message[256] = {0};
    static short size_of_message;
    int err_coun = 0;

    err_coun = copy_from_user(message, buffer, len);
    if (err_coun != 0)
    {

        printk(KERN_INFO " Failed to send %d characters to the user\n", err_coun);
        return -EFAULT;
    }

    size_of_message = strlen(message);

    ret = down_interruptible(&semp);
    if (ret != 0)
    {
        printk("Process Wait on semaphore Empty\n");
        return ret;
    }

    mutret = mutex_lock_interruptible(&mutv);
    if (mutret != 0)
    {
        printk("Process Wait on mutex write\n");
        up(&sfull);
        return mutret;
    }
    if (front == -1 && rear == -1)
        front = rear = 0;

    else if (rear == max - 1) //
        rear = 0;
    else
        rear = rear + 1;

    strcpy(q_arr[rear], message);
    printk("QUEUE element %s", q_arr[rear]);
    mutex_unlock(&mutv);
    up(&sfull);
    printk(KERN_INFO "Received %zu characters from the user\n", len);
    return len;
}

module_exit(unload_exit);