#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

static dev_t          nc_dev_t;
static struct class*  nc_class;
static struct cdev*   nc_cdev;
static struct device* nc_device;
