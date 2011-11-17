/* Module source file hellomodule.c */
/* Sourced from: 
 * https://www.linux.com/learn/linux-career-center/
 * 23685-the-kernel-newbie-corner-your-first-loadable-kernel-module
 */

#include <linux/module.h>	// For all modules
#include <linux/init.h>		// For entry/exit macros
#include <linux/kernel.h>	// For printk priority macros
#include <asm/current.h>	// Process information, just for fun
#include <linux/sched.h>	// For "struct task_struct"

static int hellomodule(void)
{
	printk(KERN_INFO "hellomodule module being loaded.\n");
	printk(KERN_INFO "User space process is '%s'\n", current->comm);
	printk(KERN_INFO "User space PID is %i\n", current->pid);
	return 0;
}

static void byemodule(void)
{
	printk(KERN_INFO "helloemodule module being unloaded.\n");
}

module_init(hellomodule);
module_exit(byemodule);

MODULE_AUTHOR("A Sayler and The Interwebs");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("You have to start somewhere.");
