#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>     //GPIO


dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
 
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

//LED is connected to this GPIO
#define GPIO_21 15
static int __init etx_driver_init(void)
{
  
  //Checking the GPIO is valid or not
  if(gpio_is_valid(GPIO_21) == false){
    pr_err("GPIO %d is not valid\n", GPIO_21);
    goto r_device;
  }
  
  //Requesting the GPIO
  if(gpio_request(GPIO_21,"GPIO_21") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_21);
    goto r_gpio;
  }
  
  //configure the GPIO as output
  gpio_direction_output(GPIO_21, 0);
  gpio_set_value(GPIO_21,1);
  /* Using this call the GPIO 21 will be visible in /sys/class/gpio/
  ** Now you can change the gpio values by using below commands also.
  ** echo 1 > /sys/class/gpio/gpio21/value  (turn ON the LED)
  ** echo 0 > /sys/class/gpio/gpio21/value  (turn OFF the LED)
  ** cat /sys/class/gpio/gpio21/value  (read the value LED)
  ** 
  ** the second argument prevents the direction from being changed.
  */
  gpio_export(GPIO_21, false);
  
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;
 
r_gpio:
  gpio_free(GPIO_21);
r_device:
  
  return -1;
}
/*
** Module exit function
*/ 
static void __exit etx_driver_exit(void)
{
  gpio_set_value(GPIO_21,0);
  gpio_unexport(GPIO_21);
  gpio_free(GPIO_21);
  pr_info("Device Driver Remove...Done!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
