#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>

/* header */

static const char* MODNAME  = "vmouse";

static const char* DEV_PATH = "input/event99";

static const unsigned int MAJOR = 27;

/* doc */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Monaco <matthew.monaco@0x01b.net>");
MODULE_DESCRIPTION("Virtual mouse driver");
MODULE_SUPPORTED_DEVICE(DEV_NAME); /* /dev/vmouse */


/* globals */

static struct class*  class_vmouse;
static struct device* device_vmouse;

/* parms */

static int verbose = 0;
module_param(verbose, int, S_IRUSR);
MODULE_PARM_DESC(verbose, "control verbosity");

/* handlers */

static int vmouse_open(struct inode *inod, struct file *file)
{

	printk(KERN_INFO "vmouse: open\n");
	return 0;
}

static int vmouse_release(struct inode *inod, struct file *file)
{
	printk(KERN_INFO "vmouse: release\n");
	return 0;
}

static ssize_t vmouse_read(struct file* filep,
                           char* buffer,
                           size_t length,
                           loff_t* offset)
{
	printk(KERN_INFO "vmouse: read\n");
	return 0;
}

static ssize_t vmouse_write(struct file* filep,
                           const char* buffer,
                           size_t length,
                           loff_t* offset)
{
	printk(KERN_INFO "vmouse: write\n");
	return length;
}

static struct file_operations fops = {

	.open     = vmouse_open,
	.read     = vmouse_read,
	.write    = vmouse_write,
	.release  = vmouse_release
};

/* init/exit */

static void __exit vmouse_exit(void)
{
	device_destroy(class_vmouse, MKDEV(MAJOR,0));
	class_destroy(class_vmouse);
	unregister_chrdev(MAJOR, MODNAME);
	printk(KERN_INFO "vmouse: exit\n");
}

static int __init vmouse_init(void)
{
	int ret = 0;

	printk(KERN_INFO "vmouse: init\n");

	printk(KERN_INFO "vmouse: registering chrdev '%i'... ", MAJOR);

	ret = register_chrdev(MAJOR, MODNAME, &fops);

	if (ret < 0) {
		printk(KERN_ERR "failed!: %i\n", ret);
		return ret;
	} else {
		printk(KERN_INFO "ok\n");
	}

	printk(KERN_INFO "vmouse: creating class '%s'... ", MODNAME);

	class_vmouse  = class_create(THIS_MODULE, MODNAME);

	if (class_vmouse <= 0) {
		printk(KERN_ERR "failed!: %i\n", class_vmouse);
		return -1;
	} else {
		printk(KERN_INFO "ok\n");
	}

	printk(KERN_INFO "vmouse: creating device node /dev/%s... ", DEV_PATH);

	device_vmouse = device_create(class_vmouse, NULL, MKDEV(MAJOR, 0), NULL, DEV_PATH);

	if (device_vmouse <= 0) {
		printk(KERN_ERR "failed!: %i\n", device_vmouse);
		class_destroy(class_vmouse);
		return -1;
	} else {
		printk(KERN_INFO "ok\n");
	}

	return ret;
}

module_init(vmouse_init);
module_exit(vmouse_exit);

