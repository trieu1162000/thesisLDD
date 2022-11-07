#include "qtr_5rc_api.h"

/* Variables for device and device class */
static dev_t qtr_5rc_dev;
static struct class *qtr_5rc_class;
static struct cdev qtr_5rc_cdev;

static struct hrtimer qtr_5rc_hrtimer;

/**
 * @brief Read data out of the buffer
 */
static ssize_t qtr_5rc_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta, i, length;
	char buffer_for_read[100] = "Value of sensor: ";
	uint16_t data_read_raw[5];
	
	printk(KERN_DEBUG"Start to read.\n");

	/* Start hr timer */
	hrtimer_start(&qtr_5rc_hrtimer, ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC), HRTIMER_MODE_ABS);
	
	do 
	{

	} while (flag_stop_timer == 0);

	for(i = 0; i<5; i++){
		sprintf(buffer_for_read,"%s %d", buffer_for_read, sensor_array[i]);
	}
	printk(KERN_DEBUG"%s\n", buffer_for_read);

	/* Get amount of data to copy */
	length = strlen(buffer_for_read);
	to_copy = min(count, (size_t) length);

	/* Copy data to user */
	sprintf(buffer_for_read,"%s\r\n", buffer_for_read);
	not_copied = copy_to_user(user_buffer, buffer_for_read, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;
	printk(KERN_DEBUG"Stop to read.\n");


	return delta;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int qtr_5rc_open(struct inode *device_file, struct file *instance) {
	printk(KERN_DEBUG"Device driver for QTR-5RC was opened.\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int qtr_5rc_close(struct inode *device_file, struct file *instance) {
	printk(KERN_DEBUG"Device driver for QTR-5RC was closed.\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = qtr_5rc_open,
	.release = qtr_5rc_close,
	.read = qtr_5rc_read,
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init qtr_5rc_init(void) {

    int i;

	/* Allocate a device nr */
	if( alloc_chrdev_region(&qtr_5rc_dev, 0, 1, DRIVER_NAME) < 0) {
		printk("Device number could not be allocated!\n");
		return -1;
	}
	printk("Device number with najor: %d, minor: %d was registered!\n", MAJOR(qtr_5rc_dev), MINOR(qtr_5rc_dev));

	/* Create device class */
	if((qtr_5rc_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		printk("Device class can not be created!\n");
		goto ClassError;
	}

	/* create device file */
	if(device_create(qtr_5rc_class, NULL, qtr_5rc_dev, NULL, DRIVER_NAME) == NULL) {
		printk("Can not create device file!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&qtr_5rc_cdev, &fops);

	/* Regisering device to kernel */
	if(cdev_add(&qtr_5rc_cdev, qtr_5rc_dev, 1) == -1) {
		printk("Registering of device to kernel failed!\n");
		goto AddError;
	}

	for(i = 0; i<5; i++){
		if(qtr_5rc_init_gpio(GPIO_BASE_NUM+i))
			goto GpioError;
	}

	hrtimer_init(&qtr_5rc_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
	qtr_5rc_hrtimer.function = &hrtimer_handler;

	printk(KERN_DEBUG"Kernel module for QTR-5RC was loaded.\n");

	return 0;
GpioError:
    for(i=0; i<5; i++){
	    gpio_free(GPIO_BASE_NUM + i);
    }
	printk(KERN_DEBUG"GPIO ERROR.\n");
AddError:
	device_destroy(qtr_5rc_class, qtr_5rc_dev);
	printk(KERN_DEBUG"Device create ERROR.\n");

FileError:
	class_destroy(qtr_5rc_class);
	printk(KERN_DEBUG"Class create ERROR.\n");

ClassError:
	unregister_chrdev_region(qtr_5rc_dev, 1);
	return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit qtr_5rc_exit(void) {
    int i;
    for(i=0; i<5; i++){
	    gpio_free(GPIO_BASE_NUM + i);
    }
	cdev_del(&qtr_5rc_cdev);
	device_destroy(qtr_5rc_class, qtr_5rc_dev);
	class_destroy(qtr_5rc_class);
	unregister_chrdev_region(qtr_5rc_dev, 1);
	printk(KERN_DEBUG"Kernel module for QTR-5RC was removed.\n");
}

module_init(qtr_5rc_init);
module_exit(qtr_5rc_exit);

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Device Driver for LCD - I2C <ThesisLDD Project>");

