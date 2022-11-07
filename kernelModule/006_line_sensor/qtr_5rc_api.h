
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/string.h>  
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>

#define DRIVER_NAME         "qtr_5rc"
#define DRIVER_CLASS        "qtr_5rc_class"
#define GPIO_BASE_NUM       22
#define TIMEOUT_SEC         0
#define TIMEOUT_NSEC        10000
#define NUM_OF_SENSORS      5

extern uint flag_stop_timer;
extern int sensor_array[NUM_OF_SENSORS];
extern uint selected_qtr_5rc_pin;
extern int qtr_5rc_init_gpio(uint gpio_num);
void qtr_5rc_gpio_input_mode(uint gpio_num);
void qtr_5rc_gpio_output_high(uint gpio_num);
void init_sensor(void);
void sensor_selection(void);
int is_black(uint x);
extern enum hrtimer_restart hrtimer_handler(struct hrtimer *timer);