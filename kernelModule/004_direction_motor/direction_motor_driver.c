#include <linux/init.h>       
#include <linux/module.h>      
#include <linux/device.h>      
#include <linux/kernel.h>      
#include <linux/fs.h>          
#include <asm/uaccess.h>       
#include <linux/gpio.h>
#include <linux/cdev.h>

#define  DRIVER_NAME "direction_motor_driver" 
#define  DRIVER_CLASS  "direction_motor_driver_class" 

enum direction{
   STOP,
   FORWARD,
   BACKWARD, 
   TURN_LEFT,
   TURN_RIGHT,
};
 
static unsigned int gpio_motor_in1 = 5; // tim 
static unsigned int gpio_motor_in2 = 6; // 
static unsigned int gpio_motor_in3 = 13; 
static unsigned int gpio_motor_in4 = 16; 

/* Variables for device and device class */
static dev_t direction_motor_dev;
static struct class *direction_motor_class;
static struct cdev direction_motor_cdev;

void set_direction(int direction)
{
	switch (direction)
	{

      	case FORWARD:
			// MOTOR LEFT
			gpio_set_value(gpio_motor_in1, 1);
			gpio_set_value(gpio_motor_in2, 0);
			// MOTOR RIGHT
			gpio_set_value(gpio_motor_in3, 0);
			gpio_set_value(gpio_motor_in4, 1);
			pr_info("%s: FORWARD.\n", __func__);	
			break;	

      	case BACKWARD:
			// MOTOR LEFT
			gpio_set_value(gpio_motor_in1, 0);
			gpio_set_value(gpio_motor_in2, 1);
			// MOTOR RIGHT
			gpio_set_value(gpio_motor_in3, 1);
			gpio_set_value(gpio_motor_in4, 0);
			pr_info("%s: BACKWARD.\n", __func__);
			break;

      	case TURN_LEFT:
			// MOTOR LEFT
			gpio_set_value(gpio_motor_in1, 0);
			gpio_set_value(gpio_motor_in2, 1);
			// MOTOR RIGHT
			gpio_set_value(gpio_motor_in3, 0);
			gpio_set_value(gpio_motor_in4, 1);
	      	pr_info("%s: TURN_LEFT.\n", __func__);
         	break;

      	case TURN_RIGHT:
			// MOTOR LEFT
			gpio_set_value(gpio_motor_in1, 1);
			gpio_set_value(gpio_motor_in2, 0);
			// MOTOR RIGHT
			gpio_set_value(gpio_motor_in3, 1);
			gpio_set_value(gpio_motor_in4, 0);
	      	pr_info("%s: TURN_RIGHT.\n", __func__);
         	break;
      	case STOP:
			gpio_set_value(gpio_motor_in1, 0);
			gpio_set_value(gpio_motor_in2, 0);
			gpio_set_value(gpio_motor_in3, 0);
			gpio_set_value(gpio_motor_in4, 0);
			pr_info("%s: STOP.\n", __func__);
			break;
      
      	default:
			gpio_set_value(gpio_motor_in1, 0);
			gpio_set_value(gpio_motor_in2, 0);
			gpio_set_value(gpio_motor_in3, 0);
			gpio_set_value(gpio_motor_in4, 0);
			pr_info("%s Invalid Command.\n", __func__);	   
			break;
   }
}
 

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the number_of_opens counter in this case.
 */
static int direction_motor_open(struct inode *inodep, struct file *filep){
   pr_info("Open direction_motor driver successfully.\n");
   return 0;
}
 
 
/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 */
static ssize_t direction_motor_write(struct file *filep, const char *user_buffer, size_t len, loff_t *offset)
{

   int to_copy, ret;
   char input_direction[1];
   unsigned long long direction_value = 0;

   to_copy = min(len, sizeof(input_direction));

   ret = kstrtoull_from_user(user_buffer, to_copy, 10, &direction_value);
	if (ret) {
		/* Negative error code. */
		pr_info("%s: Invalid input = %llu\n", __func__, direction_value);
		return ret;
	} 
   *offset = len;

   pr_info("%s: Valid input = %llu\n", __func__, direction_value);

   set_direction(direction_value);

   return len;
}
 
/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 */
static int direction_motor_close(struct inode *inodep, struct file *filep){
   pr_info("Close direction_motor driver successfully.\n");
   return 0;
}
 
/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = direction_motor_open,
   .write = direction_motor_write,
   .release = direction_motor_close,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init direction_motor_module_init(void){

	/*1. Dynamically allocate a device number */
	if( alloc_chrdev_region(&direction_motor_dev, 0, 1, DRIVER_NAME) < 0) {
		pr_err("%s: Device number could not be allocated.\n", __func__);
		goto rem_unreg;
	}
	pr_info("%s: Device number with major: %d, minor: %d was registered.\n", __func__, MAJOR(direction_motor_dev), MINOR(direction_motor_dev));

	/*2. Initialize the cdev structure with fops*/
	cdev_init(&direction_motor_cdev, &fops);

	/* 3. Register a device (cdev structure) with VFS */
	if(cdev_add(&direction_motor_cdev, direction_motor_dev, 1) == -1) {
		pr_err("%s: Registering of device to kernel failed.\n", __func__);
		goto rem_del;
	}

	/*4. create device class under /sys/class/ */
	if((direction_motor_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		pr_err("%s: Device class can not be created.\n", __func__);
		goto rem_class;
	}

	/*5.  populate the sysfs with device information */
	if(device_create(direction_motor_class, NULL, direction_motor_dev, NULL, DRIVER_NAME) == NULL) {
		pr_err("%s: Can not create device file.\n", __func__);
		goto rem_device;
	}

	/* init gpio */

	//Checking the GPIO is valid or not 
	if(!gpio_is_valid(gpio_motor_in1) || !gpio_is_valid(gpio_motor_in2)\
    || !gpio_is_valid(gpio_motor_in3) || !gpio_is_valid(gpio_motor_in4) ){
		pr_err("%s: GPIO is not valid.\n", __func__);
		return -1;
	}

	if(gpio_request(gpio_motor_in1, "IN1") < 0){
		pr_err("%s: Can not request GPIO %d.\n", __func__, gpio_motor_in1);
		return -1;
	}

   if(gpio_request(gpio_motor_in2, "IN2") < 0){
		pr_err("%s: Can not request GPIO %d.\n", __func__, gpio_motor_in2);
		return -1;
	}

	if(gpio_request(gpio_motor_in3, "IN3") < 0){
		pr_err("%s: Can not request GPIO %d.\n", __func__, gpio_motor_in3);
		return -1;
	}

	if(gpio_request(gpio_motor_in4, "IN4") < 0){
		pr_err("%s: Can not request GPIO %d.\n", __func__, gpio_motor_in4);
		return -1;
	}

   gpio_direction_output(gpio_motor_in1, 0);
   gpio_direction_output(gpio_motor_in2, 0);
   gpio_direction_output(gpio_motor_in3, 0);
   gpio_direction_output(gpio_motor_in4, 0);

	pr_info("direction_motor driver module is loaded.\n");
   
   return 0;

rem_device:
	device_destroy(direction_motor_class, direction_motor_dev);

rem_class:
	class_destroy(direction_motor_class);

rem_del:								/* Delete cdev structure */	
	cdev_del(&direction_motor_cdev);

rem_unreg:
	unregister_chrdev_region(direction_motor_dev, 1);

	return -1;

}

 
/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit direction_motor_module_exit(void){
   gpio_free(gpio_motor_in1);
   gpio_free(gpio_motor_in2);
   gpio_free(gpio_motor_in3);
   gpio_free(gpio_motor_in4);
	cdev_del(&direction_motor_cdev);
	device_destroy(direction_motor_class, direction_motor_dev);
	class_destroy(direction_motor_class);
	unregister_chrdev_region(direction_motor_dev, 1);
	pr_info("direction_motor driver module is unloaded.\n");
}
 
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Driver for controlling direction of 2 motor <ThesisLDD Projec>t");

module_init(direction_motor_module_init);
module_exit(direction_motor_module_exit);

