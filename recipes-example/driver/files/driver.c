#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "data.h"

#define SUCCESS 0
#define ID "virtual_sensor"
#define N 2048


static dev_t mymod_dev;
struct cdev mymod_cdev;
static char buffer[64];
static int Device_Open = 0;
static int index = 0;


/*operations allowed on this device*/

ssize_t mymod_read(struct file *, char *, size_t, loff_t *);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);

struct file_operations mymod_fops = {
	.owner = THIS_MODULE,
	.read = mymod_read,
	.open = device_open,
	.release = device_release
};


ssize_t mymod_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	int data=acquisition[(index++)%N];
	//we allow to read only one data at a time!
	if(count>1)
		count=1;
	//secure transaction between Kernel space to User space
	if(copy_to_user(buf,&data,count*sizeof(int)))
	{
		printk(KERN_ERR "Read operation %d on device %s failed badly!\n",index,ID);
		return -EFAULT;
	}
	return count;
}

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	if (Device_Open)
	{
		printk(KERN_WARNING "Device %s busy!\n",ID);
		return -EBUSY;
	}else
		printk(KERN_INFO "Device %s opened\n",ID);
	Device_Open++;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}


/* 
 * Called when a process closes the device file.
 * Decrement the usage count, or else once opened y'll never be able to get rid of this module!
 */
 
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */
	printk(KERN_INFO "Device '%s' is now free\n",ID);
	module_put(THIS_MODULE);
	return SUCCESS;
}

/*this is the init function for a kernel module, registered to be a character device*/

static int __init mymod_module_init(void)
{
	alloc_chrdev_region(&mymod_dev, 0, 1, ID);
	printk(KERN_INFO "Kernel module registered to be character device: %s %s\n",ID,format_dev_t(buffer, mymod_dev));
	cdev_init(&mymod_cdev, &mymod_fops);
	mymod_cdev.owner = THIS_MODULE;
	cdev_add(&mymod_cdev, mymod_dev, 1);
	return SUCCESS;
}

static void __exit mymod_module_cleanup(void)
{
	printk(KERN_INFO "Character device: %s %s removed\n",ID, format_dev_t(buffer, mymod_dev));
	cdev_del(&mymod_cdev);
	unregister_chrdev_region(mymod_dev, 1);
}

module_init(mymod_module_init);
module_exit(mymod_module_cleanup);

MODULE_AUTHOR("Luca Dalmasso");
MODULE_LICENSE("GPL");
