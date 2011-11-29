/* netchar.h - netchar client module
 * kernel module file
 * Matthew Monaco
 * Andy Sayler
 */

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

#include "netcharproto.h"
#include "netcharmod.h"

/**
 * read/write wrappers
**/

static int sock_write(struct socket* sock, void* buffer, size_t len)
{
	int            ret;
	struct msghdr  msg;
	struct iovec   iov;
	mm_segment_t   oldfs;

	memset(&msg, 0, sizeof(msg));

	msg.msg_iov    = &iov;
	msg.msg_iovlen = 1; 

	iov.iov_base   = buffer;
	iov.iov_len    = len;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	ret = sock_sendmsg(sock, &msg, len);

	set_fs(oldfs);

	return ret;
}

static int sock_read(struct socket* sock, void* buffer, size_t len)
{
	int            ret;
	struct msghdr  msg;
	struct iovec   iov;
	mm_segment_t   oldfs;

	memset(&msg, 0, sizeof(msg));

	msg.msg_iov    = &iov;
	msg.msg_iovlen = 1;

	iov.iov_base   = buffer;
	iov.iov_len    = len;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	ret = sock_recvmsg(sock, &msg, len, msg.msg_flags);

	set_fs(oldfs);

	return ret;
}

/**
 * character device fops
**/

static int netchar_open(struct inode* inodp, struct file* fp)
{
	int                ret;
	struct fop_request req;
	struct fop_reply   rep;

	memset(&req, 0, sizeof(req));
	memset(&rep, 0, sizeof(rep));

	_PKI("open....");

	req.call  = FOP_OPEN;
	req.flags = fp->f_flags;
	req.mode  = fp->f_mode;

	ret = sock_write(nc_socket, &req, sizeof(req));

	_PKI("sendmsg: %i", ret);

	ret = sock_read(nc_socket, &rep, sizeof(rep));

	_PKI("recvmsg: %i", ret);
	_PKI("open returning %i", rep.open);

	return rep.open;
}

static int netchar_release(struct inode* inodp, struct file* fp)
{
	int                ret;
	struct fop_request req;
	struct fop_reply   rep;
	
	memset(&req, 0, sizeof(req));
	memset(&rep, 0, sizeof(rep));

	_PKI("release...");

	req.call  = FOP_RELEASE;

	ret = sock_write(nc_socket, &req, sizeof(req));

	_PKI("sendmsg: %i", ret);

	ret = sock_read(nc_socket, &rep, sizeof(rep));

	_PKI("recvmsg: %i", ret);
	_PKI("release returning %i", rep.close);

	return rep.close;
}

static ssize_t netchar_read(struct file* fp, char *buffer,
                            size_t length, loff_t* offset)
{
	int                ret;
	struct fop_request req;
	struct fop_reply   rep;
	void*              payload;

	memset(&req, 0, sizeof(req));
	memset(&rep, 0, sizeof(rep));

	_PKI("read...");

	req.call  = FOP_READ;
	req.count = length;

	ret = sock_write(nc_socket, &req, sizeof(req));

	_PKI("sendmsg: %i", ret);

	ret = sock_read(nc_socket, &rep, sizeof(rep));

	_PKI("recvmsg: %i", ret);
	_PKI("read returning %zi", rep.read);

	if (rep.read > 0) {

		payload = kmalloc(rep.read, GFP_KERNEL);

		ret = sock_read(nc_socket, payload, rep.read);

		_PKI("payload read returned: %i", ret);

		ret = copy_to_user(buffer, payload, rep.read);

		_PKI("payload copy returned: %i", ret);

		kfree(payload);
	}

	return rep.read;
}

static ssize_t netchar_write(struct file* fp, const char *buffer,
                             size_t length, loff_t* offset)
{
	int                ret;
	struct fop_request req;
	struct fop_reply   rep;
	void*        payload;

	memset(&req, 0, sizeof(req));
	memset(&rep, 0, sizeof(rep));

	_PKI("write...");

	req.call  = FOP_WRITE;
	req.count = length;

	ret = sock_write(nc_socket, &req, sizeof(req));

	_PKI("sendmsg: %i", ret);

	payload = kmalloc(length, GFP_KERNEL);

	ret = copy_from_user(payload, buffer, length);

	_PKI("payload copy returned: %i", ret);

	ret = sock_write(nc_socket, payload, length);

	kfree(payload);

	_PKI("write data: %i", ret);
	
	ret = sock_read(nc_socket, &rep, sizeof(rep));

	_PKI("recvmsg: %i", ret);
	_PKI("`write returning %zi", rep.write);

	return rep.read;
}

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
