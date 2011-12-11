#include <linux/kernel.h>
#include <linux/module.h>

static int __init nc_init(void)
{
	/* do stuff */
	return 0;
}

static int __exit nc_exit(void)
{
	/* undo stuff */
}

MODULE_LICENSE("GPL");

module_init(nc_init);
module_exit(nc_exit);
