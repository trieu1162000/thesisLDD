#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED and reading a button");

/* Variables for device and device class */
static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;
static struct hrtimer qtr_5rc_hrtimer;
static uint count = 0;
static uint sensor_value = 0;
#define DRIVER_NAME "my_gpio_driver"
#define DRIVER_CLASS "MyModuleClass"
#define TIMEOUT_SEC         0
#define TIMEOUT_NSEC        1000

// static void set_up(int counter){
	
// }

enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
     /* do your timer stuff here */
    //pr_info("Timer Callback function Called [%d]\n",count);

	if(count == 0)
	{
		gpio_direction_output(22, 0);
		gpio_set_value(22, 1);
	}

    if(count == 20)
    {
		gpio_direction_input(22);
	}

	count++;

	if(count > 20)
	{
		if(gpio_get_value(22) == 0)
		{
			// if(count > 15)
			// {
			// 	sensor_value = 1;
			// }
			// else
			// {
			// 	sensor_value = 0;
			// }
    		pr_info("Count = %d\n",count);
			count = 0;
			return HRTIMER_NORESTART;
		}
	}
	// printk("Value of sensor: %d\n", sensor_value);
	// 	gpio_direction_output(22, 0);
	// 	gpio_set_value(22, 1);
	//  delay 10us
	// 	gpio_direction_input(22);



    hrtimer_forward_now(timer,ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC));
    return HRTIMER_RESTART;
}


/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	char buffer_for_read[100] = "Value of sensor before delay is:";
	uint value, counter = 0;
	u64 start_t, now_t;
 	printk("Start to read\n");


	if(gpio_direction_output(22, 0)) 
	{
		printk("Can not set GPIO 22 to output!\n");
	}
	gpio_set_value(22, 1);
	udelay(12);
	if(gpio_direction_input(22)) 
	{
		printk("Can not set GPIO 22 to input!\n");
	}

	value = gpio_get_value(22);

	udelay(60);
	// white is need more time than black down to 0
	// 70 is max, 50 is min
	// start_t = jiffies;
	//
	// while(gpio_get_value(22) != 0)
	// {
	// 	ndelay(1);
	// }
	// if(gpio_get_value(22) == 0)
	// 	now_t = jiffies;
	// sprintf(buffer_for_read, "%s %lld\r\n", buffer_for_read, now_t - start_t);
	// printk("now = %lld - start = %lld == %lld\n", now_t, start_t, now_t - start_t);
	sprintf(buffer_for_read, "%s %d and after delay is: %d\r\n", buffer_for_read, value, gpio_get_value(22));
	printk("%s", buffer_for_read);

	printk("Stop to read\n");

	/* Get amount of bytes to copy */
	to_copy = min(strlen(buffer_for_read), count);

	/* Copy Data to user */
	not_copied = copy_to_user(user_buffer, buffer_for_read, to_copy);

	/* Calculate delta */
	delta = to_copy - not_copied;

	return delta;
}


/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance) {
	printk("dev_nr - open was called!\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_close(struct inode *device_file, struct file *instance) {
	printk("dev_nr - close was called!\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
	printk("Hello, Kernel!\n");

	/* Allocate a device nr */
	if( alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
		printk("Device Nr. could not be allocated!\n");
		return -1;
	}
	printk("read_write - Device Nr. Major: %d, Minor: %d was registered!\n", my_device_nr >> 20, my_device_nr && 0xfffff);

	/* Create device class */
	if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		printk("Device class can not be created!\n");
		goto ClassError;
	}

	/* create device file */
	if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
		printk("Can not create device file!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&my_device, &fops);

	/* Regisering device to kernel */
	if(cdev_add(&my_device, my_device_nr, 1) == -1) {
		printk("Registering of device to kernel failed!\n");
		goto AddError;
	}

	/* GPIO 4 init */
	if(gpio_request(22, "rpi-gpio-22")) {
		printk("Can not allocate GPIO 22\n");
		goto AddError;
	}

	// /* Set GPIO 4 direction */
	// if(gpio_direction_output(22, 1)) {
	// 	printk("Can not set GPIO 4 to output!\n");
	// 	goto Gpio22Error;
	// }
    
    hrtimer_init(&qtr_5rc_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    qtr_5rc_hrtimer.function = &timer_callback;
    hrtimer_start(&qtr_5rc_hrtimer, ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC), HRTIMER_MODE_REL);

	return 0;
// Gpio22Error:
// 	gpio_free(22);
AddError:
	device_destroy(my_class, my_device_nr);
FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev_region(my_device_nr, 1);
	return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
	gpio_set_value(4, 0);
	gpio_free(22);
	cdev_del(&my_device);
	device_destroy(my_class, my_device_nr);
	class_destroy(my_class);
	unregister_chrdev_region(my_device_nr, 1);
    hrtimer_cancel(&qtr_5rc_hrtimer);
	printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);


