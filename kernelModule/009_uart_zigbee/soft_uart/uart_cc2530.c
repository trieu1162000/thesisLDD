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
#include <linux/cdev.h>
#include <linux/uaccess.h>

static int BAUDRATE = 9600;
static int GPIO_TX = 22;
static int GPIO_RX = 23;

#define TX_BUFFER_SIZE	256
#define RX_BUFFER_SIZE	256

static struct hrtimer hrtimer_tx;
static struct hrtimer hrtimer_rx;
unsigned char TX_DATA=0;
unsigned char RX_DATA=0;
unsigned char TX_BUFFER[TX_BUFFER_SIZE+1];
unsigned char RX_BUFFER[RX_BUFFER_SIZE+1];
static struct class *uart_cc2530_class;
static dev_t uart_cc2530_no;
static struct cdev uart_cc2530_cdev;

/* Get/Receive func */
static ssize_t uart_cc2530_read(struct file *file, char *user_buffer, size_t count, loff_t *offs)
{
	int to_copy, not_copied, delta;
	
	to_copy = min(count, strlen(RX_BUFFER));
	not_copied = copy_to_user(user_buffer, &RX_BUFFER, to_copy);
	delta = to_copy - not_copied;
	memset(RX_BUFFER,'\0',RX_BUFFER_SIZE+1);

	return delta;
}

/* Set/Send func */
static ssize_t uart_cc2530_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs)
{
	int to_copy, not_copied, delta;
	int n;
	
	/* Get amount of data to copy */
	to_copy = min(count, sizeof(TX_BUFFER));
	pr_info("user bufer after: %s\n", user_buffer);
	/* Copy data to user */
	not_copied = copy_from_user(&TX_BUFFER, user_buffer, to_copy);

	for(n=0;n<=strlen(user_buffer);n++)
	{
	
		// TX_BUFFER[strlen(TX_BUFFER)]=user_buffer[strlen(user_buffer)-n];
		TX_BUFFER[n] = user_buffer[n];
		if(strlen(TX_BUFFER)==TX_BUFFER_SIZE+1)
			memset(TX_BUFFER,'\0',TX_BUFFER_SIZE+1);
	
	}
	pr_info("user bufer after: %s\n", TX_BUFFER);
	memset(user_buffer, '\0', strlen(user_buffer));
	hrtimer_start(&hrtimer_tx,  ktime_set(0, 0), HRTIMER_MODE_REL);
	
	delta = to_copy - not_copied;

	return delta;
}

/* Call back func for transmitting data */
static enum hrtimer_restart FunctionTimerTX(struct hrtimer * unused)
{
	static int bit=-1;
	int n;
	
	if(strlen(TX_BUFFER)>0)	//Data ready to send
	{
		if(bit==-1)	//Start bit
			gpio_set_value(GPIO_TX, (0 & bit++) );
		else if(bit>=0 && bit <=7)	//Data bits
		{
			gpio_set_value(GPIO_TX, ((TX_BUFFER[0] & (1 << bit)) >> bit) );
			// gpio_set_value(GPIO_TX, ((TX_BUFFER[strlen(TX_BUFFER)-1] & (1 << bit)) >> bit) );
			bit++;
		}
		else	if(bit==8)
		{
			gpio_set_value(GPIO_TX, 1);	//Stop bit
			for(n=0; n < (strlen(TX_BUFFER)-1); n++){
				TX_BUFFER[n] = TX_BUFFER[n+1];
			}
			TX_BUFFER[strlen(TX_BUFFER)-1]='\0';
			bit=-1;
		}	
	}
	else if(strlen(TX_BUFFER) == 0)
	{
		return HRTIMER_NORESTART;
	}
	
	hrtimer_forward_now(&hrtimer_tx, ktime_set(0, (1000000/BAUDRATE)*1000 ));
	
	return HRTIMER_RESTART;
}

/* Call back func for receiving data */
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

/**
 * @brief This function is called, when the device file is opened
 */
static int uart_cc2530_open(struct inode *device_file, struct file *instance) {
	pr_info("Open driver uart_cc2530 successfully.\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int uart_cc2530_close(struct inode *device_file, struct file *instance) {
	pr_info("Close driver uart_cc2530 successfully.\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = uart_cc2530_open,
	.release = uart_cc2530_close,
	.read = uart_cc2530_read,
	.write = uart_cc2530_write
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init uart_cc2530_module_init(void)
{
	/* GPIO_TX init */
	if(gpio_request(GPIO_TX, "rpi-gpio-tx")) {
		pr_info("%s: Can not allocate GPIO_TX.\n", __func__);
		gpio_free(GPIO_TX);
	}

	/* Set GPIO_TX direction */
	if(gpio_direction_output(GPIO_TX, 0)) {
		pr_info("%s: Can not set GPIO_TX to output.\n", __func__);
		gpio_free(GPIO_TX);
	}

	/* GPIO_RX init */
	if(gpio_request(GPIO_RX, "rpi-gpio-rx")) {
		pr_info("%s: Can not allocate GPIO_RX.\n", __func__);
        gpio_free(GPIO_RX);
	}

	/* Set GPIO_RX direction */
	if(gpio_direction_input(GPIO_RX)) {
		pr_info("%s: Can not set GPIO_RX to input.\n", __func__);
		gpio_free(GPIO_RX);
	}
	
	/*1. Dynamically allocate a device number */
	if( alloc_chrdev_region(&uart_cc2530_no, 0, 1, "uart_cc2530_device") < 0) {
		pr_err("%s: Device number could not be allocated.\n", __func__);
		goto rem_unreg;
	}
	pr_info("Device number with najor: %d, minor: %d was registered.\n",MAJOR(uart_cc2530_no), MINOR(uart_cc2530_no));

	/*2. Initialize the cdev structure with fops*/
	cdev_init(&uart_cc2530_cdev, &fops);

	/* 3. Register a device (cdev structure) with VFS */
	if((cdev_add(&uart_cc2530_cdev, uart_cc2530_no, 1)) < 0){
		pr_err("%s: Registering of device to kernel failed.\n", __func__);
		goto rem_del;
	}

	/*4. create device class under /sys/class/ */
	if((uart_cc2530_class = class_create(THIS_MODULE,"uart_cc2530_class")) == NULL){
		pr_err("%s: Device class can not be created.\n", __func__);
		goto rem_class;
	}

	/*5.  populate the sysfs with device information */
	if((device_create(uart_cc2530_class, NULL, uart_cc2530_no, NULL, "uart_cc2530_device")) == NULL){
		pr_err( "%s: Cannot create the device.\n", __func__);
		goto rem_device;
	}

	hrtimer_init(&hrtimer_tx, CLOCK_REALTIME, HRTIMER_MODE_REL);
	hrtimer_tx.function = FunctionTimerTX;
	
	hrtimer_init(&hrtimer_rx, CLOCK_REALTIME, HRTIMER_MODE_REL);
	hrtimer_rx.function = FunctionTimerRX;
	hrtimer_start(&hrtimer_rx,  ktime_set(0, 0), HRTIMER_MODE_REL);
	
	pr_info("UART Zigbee CC2530 driver module is loaded.\n");
	return 0;

rem_device:								/* Delete device file */
	device_destroy(uart_cc2530_class, uart_cc2530_no);		
rem_class:								/* Delete class of device file */
	class_destroy(uart_cc2530_class);
rem_del:								/* Delete cdev structure */	
	cdev_del(&uart_cc2530_cdev);
rem_unreg:								/* Free allocated region */
	unregister_chrdev_region(uart_cc2530_no, 1);
	return -1;

}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
 static void __exit uart_cc2530_module_exit(void)
{
	hrtimer_cancel(&hrtimer_tx);
	hrtimer_cancel(&hrtimer_rx);

	/* Restore default GPIO function and free them */
	gpio_set_value(GPIO_TX, 0);
	gpio_free(GPIO_RX);
	gpio_free(GPIO_TX);

	device_destroy(uart_cc2530_class, uart_cc2530_no);
	class_destroy(uart_cc2530_class);
	cdev_del(&uart_cc2530_cdev);
	unregister_chrdev_region(uart_cc2530_no, 1);
	pr_info("UART Zigbee CC2530 driver module is unloaded.\n");
} 

//--------------------------------------------------------------------------------------
module_init(uart_cc2530_module_init);
module_exit(uart_cc2530_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Device Driver for UART-ZIGBEE-CC2530 <ThesisLDD Project>");
module_param(BAUDRATE, int, 0);
MODULE_PARM_DESC(BAUDRATE, " Baudrate value [default = 4800] (min=1200, max=19200)");
module_param(GPIO_TX, int, 0);
MODULE_PARM_DESC(GPIO_TX, " GPIO TX pin [default = 4]");
module_param(GPIO_RX, int, 0);
MODULE_PARM_DESC(GPIO_RX, " GPIO RX pin [default = 2]");
