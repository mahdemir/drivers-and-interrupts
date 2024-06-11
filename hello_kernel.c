#include <linux/module.h>	// all_modules
#include <linux/kernel.h>	// KERN_INFO
#include <linux/init.h>		// macro's

MODULE_LICENSE("GPL");		// GNU General Public License
MODULE_AUTHOR("mademir");
MODULE_DESCRIPTION("My first loadable kernel module (LKM)");

static int	__init ModuleInit(void)
{
	printk(KERN_INFO "Hello Kernel !\n");
	return (0);
}

static void	__exit ModuleExit(void)
{
	printk(KERN_INFO "Bye Kernel !\n");
	return ;
}

module_init(ModuleInit);
module_exit(ModuleExit);
