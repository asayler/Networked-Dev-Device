#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define _MODULE_NAME     "netchar"
#define _MAJOR           27
#define _DEV_CONTROL     MKDEV(_MAJOR,0)
#define _MAX_IMPORTS     16 /* very arbitrary */

/*
 * CONTROL DEVICE
 */

static struct cdev* cdev_control;

static ssize_t netchar_read(struct file* fp, char* bufer,
                           size_t length, loff_t* offset)
{
	printk(KERN_INFO _MODULE_NAME ": control read");
	return 0;
}

static ssize_t netchar_write(struct file* fp, const char* buffer,
                             size_t length, loff_t* offset)
{
	printk(KERN_INFO _MODULE_NAME ": control write");
	return length;
}

static struct file_operations fops_control = {
	.owner  = THIS_MODULE,
	.read   = netchar_read,
	.write  = netchar_write
};

static int netchar_init_control(void)
{
	int error;
	
	printk(KERN_INFO _MODULE_NAME ": init control dev\n");

	/* register major,minor of control device*/

	error = register_chrdev_region(_DEV_CONTROL, 1,_MODULE_NAME);

	if (error < 0) {
		printk(KERN_ERR _MODULE_NAME
		       ": failed to register major number %i: %i\n",
		       _MAJOR, -error);
		goto err;
	}

	/* setup control device */

	cdev_control = cdev_alloc();

	if (cdev_control <= 0) {
		printk(KERN_ERR _MODULE_NAME
		       ": failed to allocate control cdev\n");
		error = -ENOMEM;
		goto err_unreg_chrdev;
	}

	cdev_init(cdev_control, &fops_control);
	cdev_control->owner = THIS_MODULE;

	/* register control device */

	error = cdev_add(cdev_control, _DEV_CONTROL, 1);
	
	if (error < 0) {
		printk(KERN_ERR _MODULE_NAME
		       ": failed to add cdev: %i\n", -error);
		goto err_del_cdev;
	}

	return 0;


err_del_cdev:
	cdev_del(cdev_control);
err_unreg_chrdev:
	unregister_chrdev_region(_DEV_CONTROL, 1);
err:
	return error;
}

static void netchar_exit_control(void)
{
	cdev_del(cdev_control);
	unregister_chrdev_region(_DEV_CONTROL, 1);
}

/*
 * IMPORTED DEVICES
 */


/*
 * BASICS
 */

static int __init netchar_init(void)
{
	int error;

	printk(KERN_INFO _MODULE_NAME ": init\n");
	
	error = netchar_init_control();

	return error;
}

static void __exit netchar_exit(void)
{
	netchar_exit_control();
	printk(KERN_INFO _MODULE_NAME ": exit\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Monaco <matthew.monaco@0x01b.net>");
MODULE_DESCRIPTION("Network character device (client)");

module_init(netchar_init);
module_exit(netchar_exit);

