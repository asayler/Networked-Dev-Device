#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

static dev_t          nc_dev_t;
static struct class*  nc_class;
static struct cdev*   nc_cdev;
static struct device* nc_device;

static int netchar_open(struct inode* inp, struct file* fp);
static ssize_t netchar_read(struct file* fp, char* buffer,
size_t length, loff_t* offset);

static struct file_operations nc_fops = {
	/* ... */
	.open  = nc_open,
	.read  = nc_read,
}
