/***************************************************************************//**
*  \file       driver.c
*
*  \details    Simple GPIO driver explanation
*
*  \author     Trieu Huynh
*
*  \Tested with Linux raspberrypi 5.4.51-v7l+
*
*******************************************************************************/
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
#define HCSR04_ECHO			49	// P9-23
#define HCSR04_TRIGGER		15	// P9-24
#define IRQF_DISABLED 		0
#define DEV_MEM_SIZE 		512

char device_buffer[DEV_MEM_SIZE];
dev_t dev = 0;
static struct class *dev_class;
static struct cdev hcsr04_cdev;

static int gpio_irq = -1;
static int valid_value = 0;

static ktime_t echo_start;
static ktime_t echo_end;
 
static int __init hcsr04_driver_init(void);
static void __exit hcsr04_driver_exit(void);
 
 
/*************** Driver functions **********************/
static int hcsr04_open(struct inode *inode, struct file *file);
static int hcsr04_release(struct inode *inode, struct file *file);
static ssize_t hcsr04_read(struct file *filp, 
                char __user *buf, size_t len,loff_t * off);
/******************************************************/


//File operation structure 
static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = hcsr04_read,
	.open           = hcsr04_open,
	.release        = hcsr04_release,
};

/*
** This function will be called when we open the Device file
*/ 
static int hcsr04_open(struct inode *inode, struct file *file)
{
  pr_info("Device file for HCSR04 driver opened.\n");
  return 0;
}

/*
** This function will be called when we close the Device file
*/
static int hcsr04_release(struct inode *inode, struct file *file)
{
  pr_info("Device file for HCSR04 driver closed.\n");
  return 0;
}

/*
** This function will be called when we read the Device file
*/ 
static ssize_t hcsr04_read(struct file *filp, 
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
			return sprintf(buf, "%d\n", -1);;
		}
		udelay(1);
	}

	distance = (int)(ktime_to_us(ktime_sub(echo_end,echo_start)))/58;

	sprintf(device_buffer, "%d\n", distance);
	len = strlen(device_buffer);

	/* copy_to_user(void *dst, const void *src, unsigned int size) */
	if( copy_to_user(buf, device_buffer, len) > 0)
	{
		pr_err("ERROR: Not all the bytes have been copied to user\n");
	}

	pr_info("Read function : Distance = %d\n",distance);
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
static int __init hcsr04_driver_init(void)
{
	int rtc;
	
	/* Allocating Major number */
	if((alloc_chrdev_region(&dev, 0, 1, "hcsr04_dev")) <0){
		pr_err("Cannot allocate major number\n");
		goto rem_unreg;
	}
	pr_info("Major = %d, Minor = %d \n",MAJOR(dev), MINOR(dev));
	
	/* Creating cdev structure */
	cdev_init(&hcsr04_cdev,&fops);
	
	/* Adding character device to the system */
	if((cdev_add(&hcsr04_cdev,dev,1)) < 0){
		pr_err("Cannot add the device to the system\n");
		goto rem_del;
	}
	
	/* Creating struct class */
	if((dev_class = class_create(THIS_MODULE,"hcsr04_class")) == NULL){
		pr_err("Cannot create the struct class\n");
		goto rem_class;
	}
	
	/* Creating device */
	if((device_create(dev_class,NULL,dev,NULL,"hcsr04_dev")) == NULL){
		pr_err( "Cannot create the Device \n");
		goto rem_device;
	}
	
	/* Checking the GPIO is valid or not */
	if(gpio_is_valid(HCSR04_ECHO) == false || gpio_is_valid(HCSR04_TRIGGER) == false)
	{
		pr_err("GPIO %d is not valid\n", HCSR04_ECHO? HCSR04_ECHO:HCSR04_TRIGGER);
		goto rem_device;
	}
	
	/* Requesting the GPIO */
	if(gpio_request(HCSR04_ECHO,"ECHO") < 0){
		pr_err("ERROR: GPIO %d request\n", HCSR04_ECHO);
		goto rem_gpio;
	}
	if(gpio_request(HCSR04_TRIGGER,"TRIGGER") < 0){
		pr_err("ERROR: GPIO %d request\n", HCSR04_TRIGGER);
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
		printk(KERN_INFO "Error %d\n",__LINE__);
		goto fail;
	}
	else 
	{
		gpio_irq=rtc;
	}
	
	/* Request IRQ line */
	rtc = request_irq(gpio_irq, gpio_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED , "hcsr04_device", NULL);
	gpio_to_irq(HCSR04_ECHO);
	
	if(rtc) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", rtc);
		goto fail;
	}
	
	pr_info("HCSR04 driver module is loaded into the kernel successfully.\n");
	return 0;
 
rem_gpio:								/* Free GPIO */
	gpio_free(HCSR04_ECHO);
	gpio_free(HCSR04_TRIGGER);
rem_device:								/* Delete device file */
	device_destroy(dev_class,dev);		
rem_class:								/* Delete class of device file */
	class_destroy(dev_class);
rem_del:								/* Delete cdev structure */	
	cdev_del(&hcsr04_cdev);
rem_unreg:								/* Free allocated region */
	unregister_chrdev_region(dev,1);
fail:
	return -1;
}

/*
** Module exit function
*/ 
static void __exit hcsr04_driver_exit(void)
{
	/*
	** All this functions below are explained similar as label rem_* in the init function 
	*/ 
	gpio_unexport(HCSR04_ECHO);
	gpio_unexport(HCSR04_TRIGGER);
	gpio_free(HCSR04_ECHO);
	gpio_free(HCSR04_TRIGGER);
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&hcsr04_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("HCSr04 driver remove out of the kernel successfully.\n");
}
 
module_init(hcsr04_driver_init);
module_exit(hcsr04_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Device Driver for HCSR04 Sensor <ThesisLDD Project>");
