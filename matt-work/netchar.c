#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define _MODULE_NAME     "netchar"
#define _MAJOR           27
#define _DEV_CONTROL     MKDEV(_MAJOR,0)
#define _MAX_IMPORTS     16 /* very arbitrary */

#define _PKE             KERN_ERR  _MODULE_NAME ": "
#define _PKI             KERN_INFO _MODULE_NAME ": "

static struct device* netchar_device_ctrl;
static struct class*  netchar_class;
static struct cdev*   netchar_cdev;

/*
 * CONTROL DEVICE
 */

static ssize_t netchar_read(struct file* fp, char* bufer,
                            size_t length, loff_t* offset)
{
	printk(_PKI "control read");
	return 0;
}

static ssize_t netchar_write(struct file* fp, const char* buffer,
                             size_t length, loff_t* offset)
{
	printk(_PKE "control write");
	return length;
}

static struct file_operations fops_control = {
	.owner  = THIS_MODULE,
	.read   = netchar_read,
	.write  = netchar_write
};

/*
 * IMPORTED DEVICES
 */


/*
 * BASICS
 */

static int __init netchar_init(void)
{
	int error;
	
	printk(_PKI "initializing");

	/* register major,minor of control device*/

	error = register_chrdev_region(_DEV_CONTROL, 1,_MODULE_NAME);

	if (error != 0) {
		printk(_PKE "error registering control device: %i", -error);
		goto init_err;
	}
	
	/* setup cdev */

	netchar_cdev = cdev_alloc();
	error = PTR_ERR(netchar_cdev);

	if (IS_ERR_VALUE(error)) {
		printk(_PKE "error allocating cdev: %i", -error);
		goto init_err_region;
	}

	netchar_cdev->owner = THIS_MODULE;
	netchar_cdev->ops   = &fops_control;

	/* add cdev */

	error = cdev_add(netchar_cdev, _DEV_CONTROL, 1);

	if (error != 0) {
		printk(_PKE "error adding cdev: %i", -error);
		goto init_err_cdev;
	}

	/* create device class (subsystem) */

	netchar_class = class_create(THIS_MODULE, "netchar");
	error = PTR_ERR(netchar_class);

	if (IS_ERR_VALUE(error)) {
		printk(_PKE "failed to create class: %i", -error);
		goto init_err_cdev;
	}

	/* create device node in userspace */

	netchar_device_ctrl = device_create(netchar_class, NULL, _DEV_CONTROL,
	                                    NULL, "netchar_control");
	error = PTR_ERR(netchar_device_ctrl);

	if (IS_ERR_VALUE(error)) {
		printk(_PKI "error creating device: %i", -error);
		goto init_err_class;
	}

	return 0;

	init_err_class:
	class_destroy(netchar_class);
	init_err_cdev:
	cdev_del(netchar_cdev);
	init_err_region:
	unregister_chrdev_region(_DEV_CONTROL, 1);
	init_err:
	return error;
}

static void __exit netchar_exit(void)
{
	device_destroy(netchar_class, _DEV_CONTROL);
	class_destroy(netchar_class);
	cdev_del(netchar_cdev);
	unregister_chrdev_region(_DEV_CONTROL, 1);
	
	printk(_PKI "exit");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Monaco <matthew.monaco@0x01b.net>");
MODULE_DESCRIPTION("Network character device (client)");

module_init(netchar_init);
module_exit(netchar_exit);

