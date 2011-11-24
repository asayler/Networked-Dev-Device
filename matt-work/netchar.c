#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include <linux/net.h>
#include <linux/in.h>

#include "netchar.h"

#include <linux/delay.h>

/**
 * module parameters
**/

static char*  server = "127.0.0.1";
static u16    port   = 2000;

module_param(server, charp,  S_IRUGO);
module_param(port,   ushort, S_IRUGO);

MODULE_PARM_DESC(server, "server address");
MODULE_PARM_DESC(port,   "port number");

/**
 * local header
**/

#define _MODULE_NAME       "netchar"
#define _MAJOR             27
#define _DEV_FIRST         MKDEV(_MAJOR,0)

#define _PKE(fmt,args...)  printk(KERN_ERR  _MODULE_NAME ": " fmt , ## args)
#define _PKI(fmt,args...)  printk(KERN_INFO _MODULE_NAME ": " fmt , ## args)

static dev_t            nc_dev_t;
static struct class*    nc_class;
static struct cdev*     nc_cdev;
static struct device*   nc_device;
static struct socket*   nc_socket;


/**
 * character device fops
**/

static int netchar_open(struct inode* inodp, struct file* fp)
{
	return 0;
}

static int netchar_release(struct inode* inodp, struct file* fp)
{
	return 0;
}

static ssize_t netchar_read(struct file* fp, char *buffer,
                            size_t length, loff_t* offset)
{
	return 0;
}

static ssize_t netchar_write(struct file* fp, const char *buffer,
                             size_t length, loff_t* offset)
{
	return length;
}

static struct file_operations nc_fops = {
	.owner   = THIS_MODULE,
	.open    = netchar_open,
	.release = netchar_release,
	.read    = netchar_read,
	.write   = netchar_write,
};

/**
 * module basics
**/

static int __init netchar_init(void)
{
	int                 error;
	struct sockaddr_in  server_addr;
	
	_PKI("initializing");

	/**
	 * create socket
	**/

	error = sock_create(AF_INET, SOCK_STREAM, IPPROTO_TCP, &nc_socket);

	if (error != 0) {
		_PKE("error creating socket: %i", error);
		goto init_err_sock_create;
		
	}

	/**
	 * setup server address
	**/

	memset(&server_addr, 0, sizeof(server_addr));
	
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = in_aton(server);
	server_addr.sin_port        = htons(port);

	/**
	 * connect to server
	**/

	error = nc_socket->ops->connect(nc_socket, (struct sockaddr*) &server_addr,
			                sizeof(server_addr), 0);

	if (error != 0) {
		_PKE("error connecting to server: %i", error);
		goto init_err_connect;
	}

	/**
	 * reserve a randomly allocated dev_t major/minor number
	**/

	error = alloc_chrdev_region(&nc_dev_t, 0, 1, _MODULE_NAME);

	if (error != 0) {
		_PKE("error registering major/minors: %i", error);
		goto init_err_region;
	}

	/**
	 * create device class (subsystem)
	 *
	 * this creates a class to which all of our devices will belong (both
	 * control devices and import devices). besides that it is required for
	 * a device to have a class, it gives us the SUBSYSTEM tag in udev so
	 * we can change the permissions on all /dev nones of interest
	**/

	nc_class = class_create(THIS_MODULE, "netchar");
	error = PTR_ERR(nc_class);

	if (IS_ERR_VALUE(error)) {
		_PKE("failed to create class: %i", -error);
		goto init_err_class;
	}

	/**
	 * setup cdev 
	 *
	 * the cdev ties a set of file_operations (handlers for fs system calls)
	 * to a range of major,minor pairs.
	**/

	nc_cdev = cdev_alloc();
	error = PTR_ERR(nc_cdev);

	if (IS_ERR_VALUE(error)) {
		_PKE("error allocating ctl cdev: %i", -error);
		goto init_err_cdev;
	}

	nc_cdev->owner = THIS_MODULE;
	nc_cdev->ops   = &nc_fops;

	/**
	 * add cdev
	 *
	 * once the cdev is initailized, we need to tell the kernel gods about
	 * it. it's important to do this after the bulk of the module is
	 * initialized because once the add happens, we're expected to be able
	 * to handle fs calls in full
	**/

	error = cdev_add(nc_cdev, nc_dev_t, 1);

	if (error != 0) {
		_PKE("error adding ctl cdev: %i", -error);
		goto init_err_cdev;
	}

	/**
	 * create device node in userspace
	**/

	nc_device = device_create(nc_class, NULL, nc_dev_t, NULL,
	                          _MODULE_NAME "/import");
	error = PTR_ERR(nc_device);

	if (IS_ERR_VALUE(error)) {
		_PKE("error creating device node: %i", error);
		goto init_err_device;
	}

	/**
	 * proper cleanup
	 *
	 * any memory the we've allocated and kobjects that we've created
	 * need to be cleaned up in more or less the reverse order
	 *
	**/

	return 0;

	init_err_device:

	cdev_del(nc_cdev);
	init_err_cdev:
	
	class_destroy(nc_class);
	init_err_class:
	
	unregister_chrdev_region(nc_dev_t, 1);
	init_err_region:
	
	nc_socket->ops->shutdown(nc_socket, 0);
	init_err_connect:
	
	sock_release(nc_socket);
	init_err_sock_create:

	return error;
}

static void __exit netchar_exit(void)
{
	device_destroy(nc_class, nc_dev_t);
	cdev_del(nc_cdev);
	class_destroy(nc_class);
	unregister_chrdev_region(nc_dev_t, 1);
	nc_socket->ops->shutdown(nc_socket, 0);
	sock_release(nc_socket);
	
	_PKI("exit");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Monaco <matthew.monaco@0x01b.net>, Andy, Landon");
MODULE_DESCRIPTION("Network character device (client)");

module_init(netchar_init);
module_exit(netchar_exit);

