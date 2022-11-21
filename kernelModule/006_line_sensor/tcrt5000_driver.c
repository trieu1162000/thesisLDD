#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/string.h>  

#define DRIVER_NAME         "tcrt5000"
#define DRIVER_CLASS        "tcrt5000_class"
#define GPIO_BASE_NUM       22

/* Variables for device and device class */
static dev_t tcrt5000_dev;
static struct class *tcrt5000_class;
static struct cdev tcrt5000_cdev;

int tcrt5000_init_gpio(uint gpio_num)
{
	char label[100];

	//Checking the GPIO is valid or not 
	if(!gpio_is_valid(gpio_num)){
		pr_err("%s: GPIO %d is not valid.\n", __func__, gpio_num);
		return -1;
	}
	//Requesting the GPIO
	sprintf(label,"gpio-%d", gpio_num);
	if(gpio_request(gpio_num, label) < 0){
		pr_err("%s: Can not request GPIO %d.\n", __func__, gpio_num);
		return -1;
	}

	return 0;
}

void tcrt5000_gpio_input_mode(uint gpio_num)
{
	if(gpio_direction_input(gpio_num)) {
		pr_err("%s:Can not set GPIO %d to input.\n", __func__, gpio_num);
	}
}

/**
 * @brief Read data out of the buffer
 */
static ssize_t tcrt5000_read(struct file *file, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta, i, length;
	char buffer_for_read[100] = "Value of sensor: ";
	char data_read_raw[5] = {0};

	for(i = 0; i<5; i++){
		sprintf(&data_read_raw[i],"%d", gpio_get_value(GPIO_BASE_NUM+i));
	}
	sprintf(buffer_for_read,"%s %s\r\n", buffer_for_read, data_read_raw);
	pr_info("%s", buffer_for_read);

	/* Get amount of data to copy */
	length = strlen(data_read_raw);
	to_copy = min(count, (size_t) length);

	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, data_read_raw, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;
	pr_info("Read tcrt5000 driver successfully.\n");

	return delta;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int tcrt5000_open(struct inode *device_file, struct file *instance) {
	pr_info("Open tcrt5000 driver successfully.\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int tcrt5000_close(struct inode *device_file, struct file *instance) {
	pr_info("Close tcrt500 driver successfully.\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = tcrt5000_open,
	.release = tcrt5000_close,
	.read = tcrt5000_read,
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init tcrt5000_module_init(void) {

    int i;

	/*1. Dynamically allocate a device number */
	if( alloc_chrdev_region(&tcrt5000_dev, 0, 1, DRIVER_NAME) < 0) {
		pr_err("%s: Device number could not be allocated.\n", __func__);
		goto rem_unreg;
	}
	printk("Device number with najor: %d, minor: %d was registered.\n", MAJOR(tcrt5000_dev), MINOR(tcrt5000_dev));

	/*2. Initialize the cdev structure with fops*/
	cdev_init(&tcrt5000_cdev, &fops);

	/* 3. Register a device (cdev structure) with VFS */
	if(cdev_add(&tcrt5000_cdev, tcrt5000_dev, 1) == -1) {
		pr_err("%s: Registering of device to kernel failed.\n", __func__);
		goto rem_del;
	}

	/*4. create device class under /sys/class/ */
	if((tcrt5000_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		pr_err("%s: Device class can not be created.\n", __func__);
		goto rem_unreg;
	}

	/*5.  populate the sysfs with device information */
	if(device_create(tcrt5000_class, NULL, tcrt5000_dev, NULL, DRIVER_NAME) == NULL) {
		pr_err("%s: Can not create device file.\n", __func__);
		goto rem_class;
	}

	/* init gpio */
	for(i = 0; i<5; i++){
		if(tcrt5000_init_gpio(GPIO_BASE_NUM+i))
		{
			pr_err("%s: Can not init gpio.\n", __func__);
			goto rem_gpio;
		}
		tcrt5000_gpio_input_mode(GPIO_BASE_NUM+i);
	}

	pr_info("TCRT5000 driver module is loaded\n");

	return 0;

rem_gpio:
    for(i=0; i<5; i++){
	    gpio_free(GPIO_BASE_NUM + i);
    }
rem_device:
	device_destroy(tcrt5000_class, tcrt5000_dev);

rem_class:
	class_destroy(tcrt5000_class);

rem_del:								/* Delete cdev structure */	
	cdev_del(&tcrt5000_cdev);

rem_unreg:
	unregister_chrdev_region(tcrt5000_dev, 1);
	return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit tcrt5000_module_exit(void) {
    int i;
    for(i=0; i<5; i++){
	    gpio_free(GPIO_BASE_NUM + i);
    }
	cdev_del(&tcrt5000_cdev);
	device_destroy(tcrt5000_class, tcrt5000_dev);
	class_destroy(tcrt5000_class);
	unregister_chrdev_region(tcrt5000_dev, 1);
	pr_info("TCRT5000 driver module is unloaded.\n");
}

module_init(tcrt5000_module_init);
module_exit(tcrt5000_module_exit);

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Device Driver for TCRT5000 Sensor <ThesisLDD Project>");

