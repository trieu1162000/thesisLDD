#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/string.h>  

/* Variables for device and device class */
static dev_t qtr_5rc_dev;
static struct class *qtr_5rc_class;
static struct cdev qtr_5rc_cdev;

#define DRIVER_NAME         "qtr_5rc"
#define DRIVER_CLASS        "qtr_5rc_class"
#define GPIO_BASE_NUM       22
/**
 * @brief Read data out of the buffer
 */
static ssize_t qtr_5rc_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	char gpio_read_value[5] = {0};
    char value_of_sensor[100];
    char *gpio_num;
    char *pValue = value_of_sensor;
    int i;

	/* Read value of button */
    for(i=0; i<5; i++){
        gpio_read_value[i] = gpio_get_value(GPIO_BASE_NUM+i);
        sprintf(gpio_num,"%d", GPIO_BASE_NUM + i);
        pValue = strcat(pValue, "-");
        pValue = strcat(pValue, gpio_num);
    }
	printk("Value read from sensor: %s\n", value_of_sensor);

	/* Get amount of data to copy */
	to_copy = min(count, strlen(value_of_sensor));

	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, &value_of_sensor, to_copy);

	/* Calculate data */
	delta = count - not_copied;

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
    char *label;

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

    for(i=0; i<5; i++){
	    /* GPIO init */
        sprintf(label,"rpi-gpio%d", GPIO_BASE_NUM+i);
        if(gpio_request(GPIO_BASE_NUM + i, label, GPIO_BASE_NUM + i)) {
            printk("Can not allocate GPIO %d\n", GPIO_BASE_NUM +i);
            goto GpioError;
        }

        /* Set GPIO direction */
        if(gpio_direction_input(GPIO_BASE_NUM + i)) {
            printk("Can not set GPIO %d to input!\n", GPIO_BASE_NUM + i);
            goto GpioError;
        }
    }

	printk(KERN_DEBUG"Kernel module for QTR-5RC was loaded.\n");

	return 0;
GpioError:
    for(i=0; i<5; i++){
	    gpio_free(GPIO_BASE_NUM + i);
    }
AddError:
	device_destroy(qtr_5rc_class, qtr_5rc_dev);
FileError:
	class_destroy(qtr_5rc_class);
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

