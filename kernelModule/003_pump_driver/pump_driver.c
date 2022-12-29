#include <linux/init.h>       
#include <linux/module.h>      
#include <linux/device.h>      
#include <linux/kernel.h>      
#include <linux/fs.h>          
#include <asm/uaccess.h>       
#include <linux/gpio.h>
#include <linux/cdev.h>

#define  DRIVER_NAME "pump" 
#define  DRIVER_CLASS  "pump_class" 

/* Variables for device and device class */
static dev_t pump_dev;
static struct class *pump_class;
static struct cdev pump_cdev;
 
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

//LED is connected to this GPIO
#define PUMP_GPIO 4

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the number_of_opens counter in this case.
 */
static int pump_driver_open(struct inode *inodep, struct file *filep){
   return 0;
}
 
/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 */
static int pump_driver_close(struct inode *inodep, struct file *filep){
   return 0;
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 */
static ssize_t pump_driver_write(struct file *filep, const char *user_buffer, size_t len, loff_t *offset)
{

   int to_copy, ret;
   char input[1];
   unsigned long long value = 0;

   to_copy = min(len, sizeof(input));

   ret = kstrtoull_from_user(user_buffer, to_copy, 10, &value);
	if (ret) {
		/* Negative error code. */
		pr_info("%s: Invalid input = %llu\n", __func__, value);
		return ret;
	} 
   *offset = len;

//    pr_info("%s: Valid input = %llu\n", __func__, direction_value);

   gpio_set_value(PUMP_GPIO, value);

   return len;
}

static struct file_operations fops =
{
   .open = pump_driver_open,
   .write = pump_driver_write,
   .release = pump_driver_close,
};

static int __init etx_driver_init(void)
{
  
	/*1. Dynamically allocate a device number */
	if( alloc_chrdev_region(&pump_dev, 0, 1, DRIVER_NAME) < 0) {
		pr_err("%s: Device number could not be allocated.\n", __func__);
		goto rem_unreg;
	}
	pr_info("%s: Device number with major: %d, minor: %d was registered.\n", __func__, MAJOR(pump_dev), MINOR(pump_dev));

	/*2. Initialize the cdev structure with fops*/
	cdev_init(&pump_cdev, &fops);

	/* 3. Register a device (cdev structure) with VFS */
	if(cdev_add(&pump_cdev, pump_dev, 1) == -1) {
		pr_err("%s: Registering of device to kernel failed.\n", __func__);
		goto rem_del;
	}

	/*4. create device class under /sys/class/ */
	if((pump_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		pr_err("%s: Device class can not be created.\n", __func__);
		goto rem_class;
	}

	/*5.  populate the sysfs with device information */
	if(device_create(pump_class, NULL, pump_dev, NULL, DRIVER_NAME) == NULL) {
		pr_err("%s: Can not create device file.\n", __func__);
		goto rem_device;
	}

	/* init gpio */

	//Checking the GPIO is valid or not 
	if(!gpio_is_valid(PUMP_GPIO )){
		pr_err("%s: GPIO is not valid.\n", __func__);
		return -1;
	}

	if(gpio_request(PUMP_GPIO, "pump") < 0){
		pr_err("%s: Can not request GPIO %d.\n", __func__, PUMP_GPIO);
		return -1;
	}

   gpio_direction_output(PUMP_GPIO, 0);

	pr_info("pump driver module is loaded.\n");
   
   return 0;

rem_device:
	device_destroy(pump_class, pump_dev);

rem_class:
	class_destroy(pump_class);

rem_del:								/* Delete cdev structure */	
	cdev_del(&pump_cdev);

rem_unreg:
	unregister_chrdev_region(pump_dev, 1);

	return -1;
}
/*
** Module exit function
*/ 
static void __exit etx_driver_exit(void)
{
  gpio_free(PUMP_GPIO);
	cdev_del(&pump_cdev);
	device_destroy(pump_class, pump_dev);
	class_destroy(pump_class);
	unregister_chrdev_region(pump_dev, 1);
	pr_info("pump driver module is unloaded.\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Driver for controlling ON/OFF pump motor <ThesisLDD Projec>t");
