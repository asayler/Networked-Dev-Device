#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/input.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Monaco <matthew.monaco@0x01b.net>");
MODULE_DESCRIPTION("Virtual mouse driver");
MODULE_SUPPORTED_DEVICE(DEV_NAME); /* /dev/vmouse */

static struct input_dev* vmouse_dev;
static struct class*     vmouse_class;
static struct device*    vmouse_device;

static const unsigned int MAJOR = 27;

static ssize_t vmouse_read(struct file* fp,
                           char* buf,
                           size_t length,
                           loff_t* offset)
{
	static int   state = 0;
	unsigned int code;
	int          val;

	if (state < 10) {
		code = REL_X;
		val  = 5;

	} else if (state < 20) {
		code = REL_Y;
		val  = 5;
	} else if (state < 30) {
		code = REL_X;
		val  = -5;
	} else {
		code = REL_Y;
		val  = -5;
		if (state == 39)
			state = -1;
	}

	state++;
	input_report_rel(vmouse_dev, code, val);
	input_sync(vmouse_dev);

	return 0;
}

static struct file_operations fops = {
	.read = vmouse_read
};

static int __init vmouse_init(void)
{
	int ret = 0;

	printk(KERN_INFO "vmouse: init\n");

	/* talk to the input subsystem */

	vmouse_dev = input_allocate_device();

	if (!vmouse_dev) {
		printk(KERN_ERR "vmouse: not enough mem to allocate device\n");
		return -ENOMEM;
	}

	vmouse_dev->name = "matt's vmouse";

	vmouse_dev->id.vendor  = 0x0001;
	vmouse_dev->id.product = 0x0003;
	vmouse_dev->id.version = 0x0100;
	vmouse_dev->id.bustype = BUS_VIRTUAL;

	set_bit(EV_REL,   vmouse_dev->evbit);
	set_bit(REL_X,    vmouse_dev->relbit);
	set_bit(REL_Y,    vmouse_dev->relbit);

	set_bit(EV_KEY,   vmouse_dev->evbit);
	set_bit(BTN_LEFT, vmouse_dev->keybit);

	ret = input_register_device(vmouse_dev);

	if (ret) {
		printk(KERN_ERR "vmouse: failed to register device\n");
		goto err_free_dev;
	}

	/* talk to the fs subsystem */

	ret = register_chrdev(MAJOR, "vmouse", &fops);

	if (ret < 0) {
		printk(KERN_ERR "vmouse: failed to register major number\n");
		goto err_unreg_dev;
	}

	/* create a device node, normally udev would do this! */

	vmouse_class = class_create(THIS_MODULE, "vmouse");

	if (vmouse_class <= 0) {
		printk(KERN_ERR "vmouse: failed to create class\n");
		ret = -1;
		goto err_unreg_chrdev;
	}

	vmouse_device = device_create(vmouse_class, NULL, MKDEV(MAJOR,0), NULL, "vmouse");

	if (vmouse_device <= 0) {
		printk(KERN_ERR "vmouse: failed to make device node \n");
		ret = -1;
		goto err_destroy_class;
	}

	return 0;

err_destroy_class:
	class_destroy(vmouse_class);
err_unreg_chrdev:
	unregister_chrdev(MAJOR, "vmouse");
err_unreg_dev:
	input_unregister_device(vmouse_dev);
err_free_dev:
	input_free_device(vmouse_dev);
	return ret;
}

static void __exit vmouse_exit(void)
{
	device_destroy(vmouse_class, MKDEV(MAJOR,0));
	class_destroy(vmouse_class);
	unregister_chrdev(MAJOR, "vmouse");
	input_unregister_device(vmouse_dev);
	printk(KERN_INFO "vmouse: exit\n");
}

module_init(vmouse_init);
module_exit(vmouse_exit);
