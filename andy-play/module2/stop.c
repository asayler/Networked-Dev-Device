/*
 *  stop.c - Illustration of multi filed modules
 */

/* With help and code from:
 * http://www.tldp.org/LDP/lkmpg/2.6/html/x351.html
 */

#include <linux/kernel.h>/* We're doing kernel work */
#include <linux/module.h>/* Specifically, a module  */

void __exit stop_module()
{
    printk(KERN_INFO "Short is the life of a kernel module\n");
}

module_exit(stop_module);
