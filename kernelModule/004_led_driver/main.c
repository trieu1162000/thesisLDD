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
 
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

//LED is connected to this GPIO
#define GPIO_5 5
static int __init etx_driver_init(void)
{
  
  //Checking the GPIO is valid or not 
  if(gpio_is_valid(GPIO_5) == false){
    pr_err("GPIO %d is not valid\n", GPIO_5);
    goto r_device;
  }
  
  //Requesting the GPIO
  if(gpio_request(GPIO_5,"GPIO_5") < 0){
    pr_err("ERROR: GPIO %d request\n", GPIO_5);
    goto r_gpio;
  }
  
  //configure the GPIO as output
  gpio_direction_output(GPIO_5, 0);
  gpio_set_value(GPIO_5,1);
  /* Using this call the GPIO 5 will be visible in /sys/class/gpio/
  ** Now you can change the gpio values by using below commands also.
  ** echo 1 > /sys/class/gpio/gpio5/value  (turn ON the LED)
  ** echo 0 > /sys/class/gpio/gpio5/value  (turn OFF the LED)
  ** cat /sys/class/gpio/gpio5/value  (read the value LED)
  ** 
  ** the second argument prevents the direction from being changed.
  */
  gpio_export(GPIO_5, false);
  
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;
 
r_gpio:
  gpio_free(GPIO_5);
r_device:
  
  return -1;
}
/*
** Module exit function
*/ 
static void __exit etx_driver_exit(void)
{
  gpio_set_value(GPIO_5,0);
  gpio_unexport(GPIO_5);
  gpio_free(GPIO_5);
  pr_info("Device Driver Remove...Done!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
