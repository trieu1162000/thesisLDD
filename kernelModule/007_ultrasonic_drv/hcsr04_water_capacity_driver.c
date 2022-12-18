
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  /* copy_to/from_user() */
#include <linux/gpio.h>     /* GPIO */
#include <linux/moduleparam.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/interrupt.h> 
 
/* Change these two lines to use differents GPIOs */
#define HCSR04_ECHO			7
#define HCSR04_TRIGGER		12	
#define IRQF_DISABLED 		0
#define DEV_MEM_SIZE 		512
#define DRIVER_NAME         "hcsr04_w_capacity"
#define DRIVER_CLASS        "hcsr04_w_capacity_class"

char device_buffer[DEV_MEM_SIZE];
static dev_t hcsr04_dev;
static struct class *hcsr04_class;
static struct cdev hcsr04_cdev;

static int gpio_irq = -1;
static int valid_value = 0;

static ktime_t echo_start;
static ktime_t echo_end;
 
static int __init hcsr04_module_init(void);
static void __exit hcsr04_module_exit(void);
 
 
/*************** Driver functions **********************/
static int hcsr04_driver_open(struct inode *inode, struct file *file);
static int hcsr04_driver_close(struct inode *inode, struct file *file);
static ssize_t hcsr04_driver_read(struct file *filp, 
                char __user *buf, size_t len,loff_t * off);
/******************************************************/

//File operation structure 
static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = hcsr04_driver_read,
	.open           = hcsr04_driver_open,
	.release        = hcsr04_driver_close,
};

/*
** This function will be called when we open the Device file
*/ 
static int hcsr04_driver_open(struct inode *inode, struct file *file)
{
//   pr_info("Open hcsr04_w_capacity driver successfully.\n");
  return 0;
}

/*
** This function will be called when we close the Device file
*/
static int hcsr04_driver_close(struct inode *inode, struct file *file)
{
//   pr_info("Close hcsr04_w_capacity driver successfully.\n");
  return 0;
}

/*
** This function will be called when we read the Device file
*/ 
static ssize_t hcsr04_driver_read(struct file *filp, 
                char __user *buf, size_t len, loff_t *off)
{
	int counter;
	int distance;

	// Send a 10us impulse to the TRIGGER line
	gpio_set_value(HCSR04_TRIGGER,1);
	udelay(10);
	gpio_set_value(HCSR04_TRIGGER,0);
	valid_value=0;
	counter=0;

	/* 
	** When we trigger the HCSR04_TRIGGER pin for 10us, it will call IRQ handler
	** and then we will wait for the handler set valid_value to 1.
	*/
	while (valid_value==0) {
		// Out of range
		if (++counter>23200) {
			return sprintf(buf, "%d\n", -1);
		}
		udelay(1);
	}

	distance = (int)(ktime_to_us(ktime_sub(echo_end,echo_start)))/58;

	sprintf(device_buffer, "%d", distance);
	len = strlen(device_buffer);

	/* copy_to_user(void *dst, const void *src, unsigned int size) */
	if( copy_to_user(buf, device_buffer, len) > 0)
	{
		pr_err("%s: Not all the bytes have been copied to user\n", __func__);
	}

	return 0;
}

/*
** Interrupt handler function on ECHO signal 
** Return value is IRQ_HANDLED, it indicates that the processing is completed successfully, 
** But if the return value is IRQ_NONE, the processing fails.
*/
static irqreturn_t gpio_isr(int irq, void *data)
{
	ktime_t ktime_dummy;

	if (valid_value == 0) {
		ktime_dummy = ktime_get();
		if (gpio_get_value(HCSR04_ECHO) == 1) {
			echo_start = ktime_dummy;
		} else {
			echo_end = ktime_dummy;
			valid_value = 1;
		}
	}
	
	return IRQ_HANDLED;
}

/*
** Module Init function
*/ 
static int __init hcsr04_module_init(void)
{
	int rtc;
	
	/* Allocating Major number */
	if((alloc_chrdev_region(&hcsr04_dev, 0, 1, DRIVER_NAME)) <0){
		pr_err("%s: Device number could not be allocated.\n", __func__);
		goto rem_unreg;
	}
	pr_info("%s: Device number with major: %d, minor: %d was registered.\n", __func__, MAJOR(hcsr04_dev), MINOR(hcsr04_dev));
	
	/* Creating cdev structure */
	cdev_init(&hcsr04_cdev,&fops);
	
	/* Adding character device to the system */
	if((cdev_add(&hcsr04_cdev, hcsr04_dev, 1)) < 0){
		pr_err("%s: Registering of device to kernel failed.\n", __func__);
		goto rem_del;
	}
	
	/* Creating struct class */
	if((hcsr04_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
		pr_err("%s: Device class can not be created.\n", __func__);
		goto rem_class;
	}
	
	/* Creating device */
	if((device_create(hcsr04_class,NULL, hcsr04_dev, NULL, DRIVER_NAME)) == NULL){
		pr_err("%s: Can not create device file.\n", __func__);
		goto rem_device;
	}
	
	/* Checking the GPIO is valid or not */
	if(gpio_is_valid(HCSR04_ECHO) == false || gpio_is_valid(HCSR04_TRIGGER) == false)
	{
		pr_err("%s: GPIO %d is not valid\n", __func__, HCSR04_ECHO? HCSR04_ECHO:HCSR04_TRIGGER);
		goto rem_gpio;
	}
	
	/* Requesting the GPIO */
	if(gpio_request(HCSR04_ECHO,"ECHO") < 0){
		pr_err("%s: GPIO %d request\n", __func__, HCSR04_ECHO);
		goto rem_gpio;
	}
	if(gpio_request(HCSR04_TRIGGER,"TRIGGER") < 0){
		pr_err("%s: GPIO %d request\n", __func__, HCSR04_TRIGGER);
		goto rem_gpio;
	}
	
	/* Configuring the GPIO as input/output */
	gpio_direction_output(HCSR04_TRIGGER,0);
	gpio_direction_input(HCSR04_ECHO);

	/* 
	** Export GPIO to /sys/class/gpio and the second argument prevents the direction from being changed.
	*/
	gpio_export(HCSR04_ECHO, false);
	gpio_export(HCSR04_TRIGGER, false);
	
	/* Request IRQ number from GPIO pin */
	rtc = gpio_to_irq(HCSR04_ECHO);
	if (rtc<0) {
		pr_err("%s: Can not request irq number from gpio echo pin.\n", __func__);
		goto fail;
	}
	else 
	{
		gpio_irq=rtc;
	}
	
	/* Request IRQ line */
	rtc = request_irq(gpio_irq, gpio_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED , "hcsr04_obstacle", NULL);
	gpio_to_irq(HCSR04_ECHO);
	
	if(rtc) {
		pr_err("%s: Unable to request IR line for gpio_irq: %d\n", __func__, rtc);
		goto fail;
	}
	
	pr_info("hcsr04_w_capacity driver module is loaded.\n");

	return 0;
 
rem_gpio:								/* Free GPIO */
	gpio_free(HCSR04_ECHO);
	gpio_free(HCSR04_TRIGGER);
rem_device:								/* Delete device file */
	device_destroy(hcsr04_class, hcsr04_dev);		
rem_class:								/* Delete class of device file */
	class_destroy(hcsr04_class);
rem_del:								/* Delete cdev structure */	
	cdev_del(&hcsr04_cdev);
rem_unreg:								/* Free allocated region */
	unregister_chrdev_region(hcsr04_dev, 1);
fail:
	return -1;
}

/*
** Module exit function
*/ 
static void __exit hcsr04_module_exit(void)
{
	/*
	** All this functions below are explained similar as label rem_* in the init function 
	*/ 
	gpio_unexport(HCSR04_ECHO);
	gpio_unexport(HCSR04_TRIGGER);
	free_irq(gpio_irq, NULL);
	gpio_free(HCSR04_ECHO);
	gpio_free(HCSR04_TRIGGER);
	device_destroy(hcsr04_class, hcsr04_dev);
	class_destroy(hcsr04_class);
	cdev_del(&hcsr04_cdev);
	unregister_chrdev_region(hcsr04_dev, 1);
	pr_info("hcsr04_w_capacity driver module is unloaded.\n");
}
 
module_init(hcsr04_module_init);
module_exit(hcsr04_module_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Device Driver for HCSR04 Sensor to caculate water capacity. <ThesisLDD Project>");
