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

#ifndef NCD_SERVER_ADDR
#define NCD_SERVER_ADDR "127.0.0.1"   /* localhost */
#endif

/* Moduel Macros */

#define _PKE(fmt,args...)  printk(KERN_ERR  _MODULE_NAME ": " fmt , ## args)
#define _PKI(fmt,args...)  printk(KERN_INFO _MODULE_NAME ": " fmt , ## args)

/* Module Global Vars */

static char*  server = NCD_SERVER_ADDR;
static u16    port   = NCD_PORT;

/* Module Parameters */

module_param(server, charp,  S_IRUGO);
module_param(port,   ushort, S_IRUGO);

/* Module Metadata */

MODULE_PARM_DESC(server, "server address");
MODULE_PARM_DESC(port,   "port number");

static dev_t            nc_dev_t;
static struct class*    nc_class;
static struct cdev*     nc_cdev;
static struct device*   nc_device;
static struct socket*   nc_socket;

#endif /* _NETCHARMOD_H_ */
