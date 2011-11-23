#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "netchar.h"

#include <linux/delay.h>

#define _MODULE_NAME     "netchar"
#define _MAJOR           27
#define _DEV_FIRST_CTL   MKDEV(_MAJOR,0)
#define _DEV_FIRST_IMP   MKDEV(_MAJOR,NETCHAR_NUM_DEVS)
#define _DEV_FIRST       _DEV_FIRST_CTL

#define _PKE             KERN_ERR  _MODULE_NAME ": "
#define _PKI             KERN_INFO _MODULE_NAME ": "

static struct class*  netchar_class;
static struct cdev*   netchar_cdev_ctl;
static struct cdev*   netchar_cdev_imp;

/* the netchar_device ties together the control and import devices
 * so that we know where to redirect data in their complementary
 * fs handlers
 *
 * we also keep an array of all allocated structs so that they can
 * be freed on errors and exit
 *
 * */

struct netchar_device {
	int                 index;
	int                 is_in_use;
	struct device*      ctl;
	struct device*      imp;
	struct netchar_msg  msg;
};

static struct netchar_device* netchar_devices[NETCHAR_NUM_DEVS];

static struct netchar_device* netchar_get_device(struct inode* ind)
{
	int i = iminor(ind) % NETCHAR_NUM_DEVS;
	return netchar_devices[i];
}

/*
 * CONTROL DEVICES
 */

static int netchar_ctl_open(struct inode* inodp, struct file* fp)
{
	struct netchar_device* dev = netchar_get_device(inodp);

	if (dev->is_in_use)
		return -EBUSY;

	fp->private_data = dev;
	
	return 0;
}

static int netchar_ctl_release(struct inode* inodp, struct file* fp)
{
	struct netchar_device* dev = fp->private_data;
	
	if (dev == NULL) {
		printk(_PKE "ctl_release dev == NULL");
		return 0;
	}

	dev->is_in_use = 0;
	return 0;
}

static ssize_t netchar_ctl_read(struct file* fp, char *buffer,
                                size_t length, loff_t* offset)
{
	int ret;
	struct netchar_device* dev = fp->private_data;
	
	if (length < sizeof(dev->msg))
		return -EINVAL;

	ret = copy_to_user(buffer, &dev->msg, sizeof(dev->msg));

	if (ret < 0)
		return -EFAULT; /* from ldd3 */

	return sizeof(dev->msg);
}

static ssize_t netchar_ctl_write(struct file* fp, const char *buffer,
                                 size_t length, loff_t* offset)
{
	int ret;
	struct netchar_device* dev = fp->private_data;

	if (dev->msg.status == FOP_STAT_RET_DATA) {

		ret = copy_from_user(dev->msg.buffer, buffer, length);
		dev->msg.status = FOP_STAT_RET;
		return length;
	}
	
	if (length < sizeof(dev->msg))
		return -EINVAL;

	ret = copy_from_user(&dev->msg, buffer, sizeof(dev->msg));

	if (ret < 0)
		return -EFAULT; /* from ldd3 */
	
	return sizeof(dev->msg);
}

static struct file_operations netchar_fops_ctl = {
	.owner   = THIS_MODULE,
	.open    = netchar_ctl_open,
	.release = netchar_ctl_release,
	.read    = netchar_ctl_read,
	.write   = netchar_ctl_write,
};

/*
 * IMPORT DEVICES
 */

static int netchar_imp_open(struct inode* inodp, struct file* fp)
{
	struct netchar_device* dev = netchar_get_device(inodp);
	fp->private_data = dev;
	
	dev->msg.type   = FOP_OPEN;
	dev->msg.status = FOP_STAT_WAIT;

	while (dev->msg.status != FOP_STAT_RET) { mdelay(10); }
	
	dev->msg.type   = FOP_NONE;
	dev->msg.status = FOP_STAT_NONE;

	if (dev->msg.ret.open > 0)
		return 0;
	else
		return dev->msg.ret.open;
}

static int netchar_imp_release(struct inode* inodp, struct file* fp)
{
	struct netchar_device* dev = fp->private_data;

	dev->msg.type   = FOP_RELEASE;
	dev->msg.status = FOP_STAT_WAIT;

	while (dev->msg.status != FOP_STAT_RET) { mdelay(10); }
	
	dev->msg.type = FOP_NONE;
	dev->msg.type = FOP_STAT_NONE;

	return dev->msg.ret.release;
}

static ssize_t netchar_imp_read(struct file* fp, char *buffer,
                                size_t length, loff_t* offset)
{
	struct netchar_device* dev = fp->private_data;

	dev->msg.type   = FOP_READ;
	dev->msg.status = FOP_STAT_WAIT;

	dev->msg.buffer = buffer;
	dev->msg.bufsiz = length;

	while (dev->msg.status != FOP_STAT_RET) { mdelay(10); }

	dev->msg.type = FOP_NONE;
	dev->msg.type = FOP_STAT_NONE;

	return dev->msg.ret.read;
	
}

static ssize_t netchar_imp_write(struct file* fp, const char *buffer,
                                 size_t length, loff_t* offset)
{
	return length;
}

static struct file_operations netchar_fops_imp = {
	.owner   = THIS_MODULE,
	.open    = netchar_imp_open,
	.release = netchar_imp_release,
	.read    = netchar_imp_read,
	.write   = netchar_imp_write,
};

/*
 * BASICS
 */

static long netchar_device_create(int i)
{
	int error;
	dev_t  cnum, inum;
	struct device* ctl;
	struct device* imp;
	struct netchar_device* nd;

	cnum = MKDEV(_MAJOR, i);
	inum = MKDEV(_MAJOR, i+NETCHAR_NUM_DEVS);

	/* create "control" device node in userspace */

	ctl = device_create(netchar_class, NULL, cnum, NULL, "netchar/ctl%i", i);
	error = PTR_ERR(ctl);

	if (IS_ERR_VALUE(error)) {
		printk(_PKE "error creating control device (%i): %i", i, -error);
		goto devcr_err_ctl;
	}

	/* ditto for "import" device */
	
	imp = device_create(netchar_class, NULL, inum, NULL, "netchar/imp%i", i);
	error = PTR_ERR(imp);

	if (IS_ERR_VALUE(error)) {
		printk(_PKE "error creating import device (%i): %i", i, -error);
		goto devcr_err_imp;
	}

	/* tie the two together in a netchar_device struct */

	nd = kmalloc(sizeof(struct netchar_device), GFP_KERNEL);
	error = PTR_ERR(nd);
	
	if (IS_ERR_VALUE(error)) {
		printk(_PKE "error creating netchar_device (%i): %i", i, -error);
		goto devcr_err_nd;
	}

	nd->index = i;
	nd->ctl   = ctl;
	nd->imp   = imp;
	nd->is_in_use = 0;
	
	nd->msg.type   = FOP_NONE;
	nd->msg.status = FOP_STAT_NONE;

	netchar_devices[i] = nd;

	return 0;
	
	devcr_err_nd:

	device_destroy(netchar_class, inum);
	devcr_err_imp:
	
	device_destroy(netchar_class, cnum);
	devcr_err_ctl:

	return error;
}

static void netchar_device_destroy(int i)
{
	dev_t  cnum, inum;

	cnum = MKDEV(_MAJOR, i);
	inum = MKDEV(_MAJOR, i+NETCHAR_NUM_DEVS);
	
	device_destroy(netchar_class, cnum);
	device_destroy(netchar_class, inum);
	
	kfree(netchar_devices[i]);
}

static int __init netchar_init(void)
{
	int error, i = 0;
	
	printk(_PKI "initializing");

	/* register device major/minors
	 *
	 * this just reserves a group of major/minor pairs for this module
	 *
	 * */

	error = register_chrdev_region(_DEV_FIRST, NETCHAR_NUM_DEVS*2, _MODULE_NAME);

	if (error != 0) {
		printk(_PKE "error registering major/minors: %i", -error);
		goto init_err_region;
	}
	
	/* create device class (subsystem)
	 *
	 * this creates a class to which all of our devices will belong (both
	 * control devices and import devices). besides that it is required for
	 * a device to have a class, it gives us the SUBSYSTEM tag in udev so
	 * we can change the permissions on all /dev nones of interest
	 *
	 * */

	netchar_class = class_create(THIS_MODULE, "netchar");
	error = PTR_ERR(netchar_class);

	if (IS_ERR_VALUE(error)) {
		printk(_PKE "failed to create class: %i", -error);
		goto init_err_class;
	}

	/* setup ctl cdev 
	 *
	 * the cdev ties a set of file_operations (handlers for fs system calls)
	 * to a range of major,minor pairs. this cdev is for our "control"
	 * devices
	 *
	 * */

	netchar_cdev_ctl = cdev_alloc();
	error = PTR_ERR(netchar_cdev_ctl);

	if (IS_ERR_VALUE(error)) {
		printk(_PKE "error allocating ctl cdev: %i", -error);
		goto init_err_cdev_ctl;
	}

	netchar_cdev_ctl->owner = THIS_MODULE;
	netchar_cdev_ctl->ops   = &netchar_fops_ctl;

	/* add ctl cdev
	 *
	 * once the cdev is initailized, we need to tell the kernel gods about
	 * it. it's important to do this after the bulk of the module is
	 * initialized because once the add happens, we're expected to be able
	 * to handle fs calls in full
	 *
	 * */

	error = cdev_add(netchar_cdev_ctl, _DEV_FIRST_CTL, NETCHAR_NUM_DEVS);

	if (error != 0) {
		printk(_PKE "error adding ctl cdev: %i", -error);
		goto init_err_cdev_ctl;
	}

	/* setup imp cdev 
	 *
	 * similar to the above, but for the "import" devices
	 *
	 * */

	netchar_cdev_imp = cdev_alloc();
	error = PTR_ERR(netchar_cdev_imp);

	if (IS_ERR_VALUE(error)) {
		printk(_PKE "error allocating imp cdev: %i", -error);
		goto init_err_cdev_imp;
	}

	netchar_cdev_imp->owner = THIS_MODULE;
	netchar_cdev_imp->ops   = &netchar_fops_imp;

	/* add imp cdev
	 *
	 * ditto
	 *
	 * */

	error = cdev_add(netchar_cdev_imp, _DEV_FIRST_IMP, NETCHAR_NUM_DEVS);

	if (error != 0) {
		printk(_PKE "error adding ctl cdev: %i", -error);
		goto init_err_cdev_imp;
	}

	/* create pairs of device nodes in userspace
	 *
	 * we need NETCHAR_NUM_DEVS pairs of devices nodes. in the future we'd
	 * like to do this dynamically, but in the name of progress we'll do
	 * some hard-coding
	 *
	 * */

	for (i = 0; i < NETCHAR_NUM_DEVS; i++) {

		error = netchar_device_create(i);

		if (error != 0)
			goto init_err_node;
	}

	/* proper cleanup
	 *
	 * any memory the we've allocated and kobjects that we've created
	 * need to be cleaned up in more or less the reverse order
	 *
	 * */

	return 0;

	init_err_node:
	for (i = i-1; i >= 0; i--) netchar_device_destroy(i);

	cdev_del(netchar_cdev_imp);
	init_err_cdev_imp:
	
	cdev_del(netchar_cdev_ctl);
	init_err_cdev_ctl:
	
	class_destroy(netchar_class);
	init_err_class:
	
	unregister_chrdev_region(_DEV_FIRST, NETCHAR_NUM_DEVS*2);
	init_err_region:
	
	return error;
}

static void __exit netchar_exit(void)
{
	int i;
	
	for (i = 0; i < NETCHAR_NUM_DEVS; i++) netchar_device_destroy(i);
	
	cdev_del(netchar_cdev_ctl);
	cdev_del(netchar_cdev_imp);
	class_destroy(netchar_class);
	unregister_chrdev_region(_DEV_FIRST, NETCHAR_NUM_DEVS*2);
	
	printk(_PKI "exit");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Monaco <matthew.monaco@0x01b.net>");
MODULE_DESCRIPTION("Network character device (client)");

module_init(netchar_init);
module_exit(netchar_exit);

