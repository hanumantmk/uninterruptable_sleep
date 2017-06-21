/* Necessary includes for device drivers */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/errno.h> /* error codes */
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/fs.h>    /* everything... */
#include <linux/init.h>
#include <linux/kernel.h> /* printk() */
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>  /* kmalloc() */
#include <linux/types.h> /* size_t */

MODULE_LICENSE("Dual BSD/GPL");

static DECLARE_WAIT_QUEUE_HEAD(wq);
static volatile int flag = 0;

/* Declaration of memory.c functions */
int memory_open(struct inode* inode, struct file* filp);
int memory_release(struct inode* inode, struct file* filp);
ssize_t memory_read(struct file* filp, char* buf, size_t count, loff_t* f_pos);
ssize_t memory_write(struct file* filp, const char* buf, size_t count, loff_t* f_pos);
void memory_exit(void);
int memory_init(void);

/* Structure that declares the usual file */
struct file_operations memory_fops = {
    .read = memory_read, .write = memory_write, .open = memory_open, .release = memory_release};

/* Declaration of the init and exit functions */
module_init(memory_init);
module_exit(memory_exit);

/* Global variables of the driver */
/* Major number */
int memory_major = 60;
/* Buffer to store data */
char* memory_buffer;

int memory_init(void) {
    int result;

    /* Registering device */
    result = register_chrdev(memory_major, "memory", &memory_fops);
    if (result < 0) {
        printk("<1>memory: cannot obtain major number %d\n", memory_major);
        return result;
    }

    /* Allocating memory for the buffer */
    memory_buffer = kmalloc(1, GFP_KERNEL);
    if (!memory_buffer) {
        result = -ENOMEM;
        goto fail;
    }
    memset(memory_buffer, 0, 1);

    printk("<1>Inserting memory module\n");
    return 0;

fail:
    memory_exit();
    return result;
}

void memory_exit(void) {
    /* Freeing the major number */
    unregister_chrdev(memory_major, "memory");

    /* Freeing buffer memory */
    if (memory_buffer) {
        kfree(memory_buffer);
    }

    printk("<1>Removing memory module\n");
}

int memory_open(struct inode* inode, struct file* filp) {
    /* Success */
    return 0;
}

int memory_release(struct inode* inode, struct file* filp) {
    /* Success */
    return 0;
}

ssize_t memory_read(struct file* filp, char* buf, size_t count, loff_t* f_pos) {
    printk("<1>going to sleep\n");
    flag = 0;
    // wait_event_interruptible(wq, flag != 0);
    wait_event(wq, flag != 0);

    printk("<1>Reading from memory module\n");
    /* Transfering data to user space */
    copy_to_user(buf, memory_buffer, 1);

    /* Changing reading position as best suits */
    if (*f_pos == 0) {
        *f_pos += 1;
        return 1;
    } else {
        return 0;
    }
}

ssize_t memory_write(struct file* filp, const char* buf, size_t count, loff_t* f_pos) {
    char* tmp;

    printk("<1>wake someone up\n");
    flag = 1;
    // wake_up_interruptible(&wq);
    wake_up(&wq);

    printk("<1>Writting to memory module\n");
    tmp = buf + count - 1;
    copy_from_user(memory_buffer, tmp, 1);
    return 1;
}
