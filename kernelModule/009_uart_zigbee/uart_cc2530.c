/* 
Raspberry Pi / BCM2835 Software-based UART Linux device driver
Copyright (C) 2014 Leonardo Ciocari

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <asm/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Leonardo Ciocari");
MODULE_DESCRIPTION("Raspberry Pi / BCM2835 Soft-UART Driver");

static int BAUDRATE = 4800;
module_param(BAUDRATE, int, 0);
MODULE_PARM_DESC(BAUDRATE, " Baudrate value [default = 4800] (min=1200, max=19200)");

static int GPIO_TX = 4;
module_param(GPIO_TX, int, 0);
MODULE_PARM_DESC(GPIO_TX, " GPIO TX pin [default = 4]");

static int GPIO_RX = 2;
module_param(GPIO_RX, int, 0);
MODULE_PARM_DESC(GPIO_RX, " GPIO RX pin [default = 2]");

#define TX_BUFFER_SIZE	256
#define RX_BUFFER_SIZE	256

static struct hrtimer hrtimer_tx;
static struct hrtimer hrtimer_rx;
unsigned char TX_DATA=0;
unsigned char RX_DATA=0;
unsigned char TX_BUFFER[TX_BUFFER_SIZE+1];
unsigned char RX_BUFFER[RX_BUFFER_SIZE+1];
bool LOOPBACK=0;

//--------------------------------------------------------------------------------------
static enum hrtimer_restart FunctionTimerTX(struct hrtimer * unused)
{
	static int bit=-1;
	
	if(strlen(TX_BUFFER)>0)	//Data ready to send
	{
		if(bit==-1)	//Start bit
			gpio_set_value(GPIO_TX, (0 & bit++) );
		else if(bit>=0 && bit <=7)	//Data bits
		{
			gpio_set_value(GPIO_TX, ((TX_BUFFER[strlen(TX_BUFFER)-1] & (1 << bit)) >> bit) );
			bit++;
		}
		else	if(bit==8)
		{
			gpio_set_value(GPIO_TX, 1);	//Stop bit
			TX_BUFFER[strlen(TX_BUFFER)-1]='\0';
			bit=-1;
		}	
	}
	
	hrtimer_forward_now(&hrtimer_tx, ktime_set(0, (1000000/BAUDRATE)*1000 ));
	
	return HRTIMER_RESTART;
}

//--------------------------------------------------------------------------------------
static enum hrtimer_restart FunctionTimerRX(struct hrtimer * unused)
{
	static int bit=-1;
	
	if(gpio_get_value(GPIO_RX)==0 && bit==-1)	//Start bit received
		bit++;
	else	if(bit>=0 && bit<8)	//Data bits
	{
		if(gpio_get_value(GPIO_RX)==0)
			RX_DATA &= 0b01111111;
		else
			RX_DATA |= ~0b01111111;
		
		if(bit!=7)
			RX_DATA >>= 1;
			
		bit++;
	}
	else	if(bit==8)	//Stop bit
	{
		bit=-1;
		
		RX_BUFFER[strlen(RX_BUFFER)]=RX_DATA;
		
		if(strlen(RX_BUFFER)==RX_BUFFER_SIZE+1)
			memset(RX_BUFFER,'\0',RX_BUFFER_SIZE+1);
	}
	
	hrtimer_forward_now(&hrtimer_rx, ktime_set(0, (1000000/BAUDRATE)*1000 ));
	
	return HRTIMER_RESTART;
}

//--------------------------------------------------------------------------------------
static ssize_t set_gpio_tx_callback(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	long gpio;
	
	if (kstrtol(buf, 10, &gpio) < 0)
		return -EINVAL;
		
	if (gpio > 53 || gpio < 0)	//Check GPIO range
		return -EINVAL;

	gpio_set_value(GPIO_TX, 0);		//Restore old GPIO default value

	GPIO_TX = gpio;
	gpio_direction_output(GPIO_TX, 0);		//Set new GPIO as output

	return count;
}

//--------------------------------------------------------------------------------------
static ssize_t set_gpio_rx_callback(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	long gpio;
	
	if (kstrtol(buf, 10, &gpio) < 0)
		return -EINVAL;
		
	if (gpio > 53 || gpio < 0)	//Lock GPIO out of range
		return -EINVAL;

	GPIO_RX = gpio;
	gpio_direction_input(GPIO_RX);		//Set new GPIO as input

	return count;
}

//--------------------------------------------------------------------------------------
static ssize_t get_gpio_tx_callback(struct device* dev, struct device_attribute* attr,char* buf)
{
	return sprintf(buf,"%i\n",GPIO_TX);
}

//--------------------------------------------------------------------------------------
static ssize_t get_gpio_rx_callback(struct device* dev, struct device_attribute* attr,char* buf)
{
	return sprintf(buf,"%i\n",GPIO_RX);
}

//--------------------------------------------------------------------------------------
static ssize_t set_data_callback(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	int n;
	
	for(n=0;n<=strlen(buf);n++)
	{
	
		if(LOOPBACK)
		{
			RX_BUFFER[strlen(RX_BUFFER)]=buf[n];
			if(strlen(RX_BUFFER)==RX_BUFFER_SIZE+1)
				memset(RX_BUFFER,'\0',RX_BUFFER_SIZE+1);
		}
		else
		{
			TX_BUFFER[strlen(TX_BUFFER)]=buf[strlen(buf)-n];
			if(strlen(TX_BUFFER)==TX_BUFFER_SIZE+1)
				memset(TX_BUFFER,'\0',TX_BUFFER_SIZE+1);
		}
	
	}
	
	hrtimer_start(&hrtimer_tx,  ktime_set(0, 0), HRTIMER_MODE_REL);
	
	return count;
}

//--------------------------------------------------------------------------------------
static ssize_t get_data_callback(struct device* dev, struct device_attribute* attr,char* buf)
{
	unsigned char tmp[RX_BUFFER_SIZE+1];
	
	strcpy(tmp,RX_BUFFER);
	memset(RX_BUFFER,'\0',RX_BUFFER_SIZE+1);
	
	return sprintf(buf,"%s",tmp);
}

//--------------------------------------------------------------------------------------
static ssize_t set_loopback_callback(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	long loopback;
	
	if (kstrtol(buf, 10, &loopback) < 0)
		return -EINVAL;
		
	if (loopback != 0 && loopback != 1)
		return -EINVAL;
	
	LOOPBACK=loopback;

	return count;
}

//--------------------------------------------------------------------------------------
static ssize_t get_loopback_callback(struct device* dev, struct device_attribute* attr,char* buf)
{
	return sprintf(buf,"%i\n",LOOPBACK);
}

//--------------------------------------------------------------------------------------
static ssize_t set_baudrate_callback(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	long baudrate;
	
	if (kstrtol(buf, 10, &baudrate) < 0)
		return -EINVAL;
		
	if (baudrate < 1200 || baudrate > 19200)	//Lock utopia values ;)
		return -EINVAL;
	
	BAUDRATE=baudrate;

	return count;
}

//--------------------------------------------------------------------------------------
static ssize_t get_baudrate_callback(struct device* dev, struct device_attribute* attr,char* buf)
{
	return sprintf(buf,"%i\n",BAUDRATE);
}

//--------------------------------------------------------------------------------------
static DEVICE_ATTR(gpio_tx, 0644, get_gpio_tx_callback, set_gpio_tx_callback);
static DEVICE_ATTR(gpio_rx, 0644, get_gpio_rx_callback, set_gpio_rx_callback);
static DEVICE_ATTR(data, 0644, get_data_callback, set_data_callback);
static DEVICE_ATTR(loopback, 0644, get_loopback_callback, set_loopback_callback);
static DEVICE_ATTR(baudrate, 0644, get_baudrate_callback, set_baudrate_callback);

static struct class *pDEVICE_CLASS;
static struct device *pDEVICE;

//--------------------------------------------------------------------------------------
static int __init ModuleInit(void)
{
	int result;

	/* GPIO_TX init */
	if(gpio_request(GPIO_TX, "rpi-gpio-tx")) {
		printk("Can not allocate GPIO_TX\n");
		gpio_free(GPIO_TX);
	}

	/* Set GPIO_TX direction */
	if(gpio_direction_output(GPIO_TX, 0)) {
		printk("Can not set GPIO_TX to output!\n");
		gpio_free(GPIO_TX);
	}

	/* GPIO_RX init */
	if(gpio_request(GPIO_RX, "rpi-gpio-rx")) {
		printk("Can not allocate GPIO_RX\n");
        gpio_free(GPIO_RX);
	}

	/* Set GPIO_RX direction */
	if(gpio_direction_input(GPIO_RX)) {
		printk("Can not set GPIO_RX to input!\n");
		gpio_free(GPIO_RX);
	}
	
	pDEVICE_CLASS = class_create(THIS_MODULE, "softuart");
	BUG_ON(IS_ERR(pDEVICE_CLASS));

	hrtimer_init(&hrtimer_tx, CLOCK_REALTIME, HRTIMER_MODE_REL);
	hrtimer_tx.function = FunctionTimerTX;
	
	hrtimer_init(&hrtimer_rx, CLOCK_REALTIME, HRTIMER_MODE_REL);
	hrtimer_rx.function = FunctionTimerRX;
	hrtimer_start(&hrtimer_rx,  ktime_set(0, 0), HRTIMER_MODE_REL);
	
	pDEVICE = device_create(pDEVICE_CLASS, NULL, 0, NULL, "softuart");
	BUG_ON(IS_ERR(pDEVICE));
	result = device_create_file(pDEVICE, &dev_attr_gpio_tx);
	BUG_ON(result < 0);
	result = device_create_file(pDEVICE, &dev_attr_gpio_rx);
	BUG_ON(result < 0);
	result = device_create_file(pDEVICE, &dev_attr_data);
	BUG_ON(result < 0);
	result = device_create_file(pDEVICE, &dev_attr_loopback);
	BUG_ON(result < 0);
	result = device_create_file(pDEVICE, &dev_attr_baudrate);
	BUG_ON(result < 0);
	
	return 0;
}

//--------------------------------------------------------------------------------------
static void __exit ModuleExit(void)
{
	hrtimer_cancel(&hrtimer_tx);
	hrtimer_cancel(&hrtimer_rx);
	
	//Restore default GPIO function
	gpio_set_value(GPIO_RX, 0);
	gpio_free(GPIO_RX);
	gpio_free(GPIO_RX);
	device_remove_file(pDEVICE, &dev_attr_gpio_tx);
	device_remove_file(pDEVICE, &dev_attr_gpio_rx);
	device_remove_file(pDEVICE, &dev_attr_data);
	device_remove_file(pDEVICE, &dev_attr_loopback);
	device_remove_file(pDEVICE, &dev_attr_baudrate);
	device_destroy(pDEVICE_CLASS, 0);
	
	class_unregister(pDEVICE_CLASS);
	class_destroy(pDEVICE_CLASS);	
} 

//--------------------------------------------------------------------------------------
module_init(ModuleInit);
module_exit(ModuleExit);
