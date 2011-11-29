/* netchar.h - netchar client module
 * module header file
 * Matthew Monaco
 * Andy Sayler
 */

#ifndef _NETCHARMOD_H_
#define _NETCHARMOD_H_

/* Moduel Includes */

#include "netcharproto.h"

/* Module Constants */

#define _MODULE_NAME       "netchar"

#ifndef NCD_MAJOR
#define NCD_MAJOR 0   /* dynamic major by default */
#endif

#ifndef NCD_MINOR
#define NCD_MINOR 0   /* first minor by default */
#endif

#ifndef NUM_DEVS
#define NUM_DEVS 1   /* number of devices */
#endif

#ifndef NCD_SERVER_ADDR
#define NCD_SERVER_ADDR "127.0.0.1"   /* default to localhost */
#endif

/* Moduel Macros */

#define _PKE(fmt,args...)  printk(KERN_ERR  _MODULE_NAME ": " fmt , ## args)
#define _PKI(fmt,args...)  printk(KERN_INFO _MODULE_NAME ": " fmt , ## args)
#define _PKA(fmt,args...)  printk(KERN_ALERT _MODULE_NAME ": " fmt , ## args)

/* Module Global Vars */

static char*  server = NCD_SERVER_ADDR;
static u16    port   = NCD_PORT;
static int ncd_major = NCD_MAJOR;
static int ncd_minor = NCD_MINOR;
static int num_devs  = NUM_DEVS;
static dev_t            nc_dev_t;
static struct class*    nc_class;
static struct cdev*     nc_cdev;
static struct device*   nc_device;
static struct socket*   nc_socket;

/* Module Parameters */

module_param(server, charp,  S_IRUGO);
MODULE_PARM_DESC(server, "server address");
module_param(port,   ushort, S_IRUGO);
MODULE_PARM_DESC(port,   "port number");
module_param(ncd_major, int, S_IRUGO);
MODULE_PARM_DESC(ncd_major,   "major number (0 for dynamic)");
module_param(ncd_minor, int, S_IRUGO);
MODULE_PARM_DESC(ncd_minor,   "first minor number");
module_param(num_devs, int, S_IRUGO);
MODULE_PARM_DESC(num_devs,   "number of minor devices");

/* Module Metadata */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Monaco <matthew.monaco@0x01b.net>, "
	      "Andy Sayler <andy.sayler@gmail.com>, "
	      "Landon");
MODULE_DESCRIPTION("Network character device module (client)");

/* Module Prototypes */

static int sock_write(struct socket* sock, void* buffer, size_t len);
static int sock_read(struct socket* sock, void* buffer, size_t len);

static int netchar_open(struct inode* inodp, struct file* fp);
static int netchar_release(struct inode* inodp, struct file* fp);
static ssize_t netchar_read(struct file* fp, char *buffer,
			size_t length, loff_t* offset);
static ssize_t netchar_write(struct file* fp, const char *buffer,
			size_t length, loff_t* offset);

static int __init netchar_init(void);
static void __exit netchar_exit(void);


/* Module fops */

static struct file_operations nc_fops = {
	.owner   = THIS_MODULE,
	.open    = netchar_open,
	.release = netchar_release,
	.read    = netchar_read,
	.write   = netchar_write,
};

/* Module Functions */

module_init(netchar_init);
module_exit(netchar_exit);

#endif /* _NETCHARMOD_H_ */
