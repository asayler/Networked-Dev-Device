/*
 *  start.c - Illustration of multi filed modules
 */

/* With help and code from:
 * http://www.tldp.org/LDP/lkmpg/2.6/html/x351.html
 */

#include <linux/kernel.h>     /* We're doing kernel work */
#include <linux/module.h>     /* Specifically, a module */

#include <linux/init.h>     /* Needed for the macros */

MODULE_LICENSE("GPL");                                 /* Module license */
MODULE_AUTHOR("A Sayler <andy.sayler@gmail.com>");     /* Who wrote this module? */
MODULE_DESCRIPTION("A sample kernel module");          /* What does this module do? */
MODULE_SUPPORTED_DEVICE("testdevice";)                 /* What type of device? */

static int __init start_module(void);

static int __init start_module(void)
{
    printk(KERN_INFO "Hello, world - this is the kernel speaking\n");
    return 0;
}

module_init(start_module);
