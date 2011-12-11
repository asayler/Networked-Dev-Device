#include <linux/fs.h>

static int netchar_open(struct inode* inp, struct file* fp);
static ssize_t netchar_read(struct file* fp, char* buffer,
size_t length, loff_t* offset);

static struct file_operations nc_fops = {
	/* ... */
	.open  = nc_open,
	.read  = nc_read,
}
