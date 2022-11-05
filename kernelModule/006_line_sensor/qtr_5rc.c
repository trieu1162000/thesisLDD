#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/string.h>  
#include <linux/delay.h>

/* Variables for device and device class */
static dev_t qtr_5rc_dev;
static struct class *qtr_5rc_class;
static struct cdev qtr_5rc_cdev;

#define DRIVER_NAME         "qtr_5rc"
#define DRIVER_CLASS        "qtr_5rc_class"
#define GPIO_BASE_NUM       22


/**
 * @brief Initializing the GPIO
 */
static int qtr_5rc_init_gpio(uint gpio_num){
	char label[100];

	//Checking the GPIO is valid or not 
	if(!gpio_is_valid(gpio_num)){
		pr_err("ERROR: GPIO %d is not valid\n", gpio_num);
		return -1;
	}
	//Requesting the GPIO
	sprintf(label,"gpio-%d", gpio_num);
	if(gpio_request(gpio_num, label) < 0){
		pr_err("ERROR: GPIO %d request\n", gpio_num);
		return -1;
	}

	return 0;
}

/**
 * @brief set pin of gpios to the input mode
 */
static void qtr_5rc_gpio_input_mode(uint gpio_num)
{
	if(gpio_direction_input(gpio_num)) {
		pr_err("ERROR:Can not set GPIO %d to input!\n", gpio_num);
	}
}

/**
 * @brief set pin of gpios to the output mode and init high value
 */
static void qtr_5rc_gpio_output_high(uint gpio_num)
{
	if(gpio_direction_output(gpio_num, 1)) {
		pr_err("ERRORCan not set GPIO %d to output high!\n", gpio_num);
	}
}

/**
 * @brief set pin of gpios to the output mode and init low value
 */
// static void qtr_5rc_gpio_output_low(uint gpio_num)
// {
// 	if(gpio_direction_output(gpio_num, 0)) {
// 		pr_err("ERRORCan not set GPIO %d to output low!\n", gpio_num);
// 	}
// }

/**
 * @brief Read value from pin of sensor
 */
static void qtr_5rc_read_raw(uint16_t *qtr_5rc_value)
{
	int i;
	ktime_t time_to_low = 0;
	ktime_t first_time_captured = 0;

	/* Make sensor pin to output and init high value */
    for(i=0; i<5; i++){
		qtr_5rc_gpio_output_high(GPIO_BASE_NUM+i);
    }
	
	/* Wait for 10us */
	udelay(10);

    for(i=0; i<5; i++){
		/* Make sensor pin to input */
		qtr_5rc_gpio_input_mode(GPIO_BASE_NUM+i);
		
		/* Count time to wait for pin to be low */
		time_to_low = 0;
		first_time_captured = ktime_get();
		while(1){
			if(gpio_get_value(GPIO_BASE_NUM+i)==0)
			{
				time_to_low = ktime_get() - first_time_captured;
				qtr_5rc_value[i] = (uint16_t) time_to_low;
				break;
			}
			else 
			{
				qtr_5rc_value[i] = 3000;
				time_to_low = ktime_get() - first_time_captured;
			}
		}
    }

}

/**
 * @brief Read data out of the buffer
 */
static ssize_t qtr_5rc_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta, i, length;
	char buffer_for_read[100] = "Value of sensor: ";
	uint16_t data_read_raw[5];

	/* Read value of sensor */
	qtr_5rc_read_raw(data_read_raw);
	for(i=0; i<5; i++)
		sprintf(buffer_for_read,"%s %d", buffer_for_read, data_read_raw[i]);
	printk("%s\n", buffer_for_read);
	
	/* Get amount of data to copy */
	length = strlen(buffer_for_read);
	to_copy = min(count, (size_t) length);

	/* Copy data to user */
	sprintf(buffer_for_read,"%s\r\n", buffer_for_read);
	not_copied = copy_to_user(user_buffer, buffer_for_read, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

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

