 #include <linux/init.h>         
#include <linux/module.h>        
#include <linux/device.h>        
#include <linux/kernel.h>        
#include <linux/fs.h>            
#include <asm/uaccess.h>         
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#define  DEVICE_NAME "encoder"    ///< The device will appear at /dev/encoder using this value
#define  CLASS_NAME  "encoder_driver"        ///< The device class -- this is a character device driver

#define CLOCKWISE 		1	
#define ANTI_CLOCKWISE 	2
#define STOP			0

#define REQUEST_ENC0_ANGLE 1
#define REQUEST_ENC1_ANGLE 2
#define REQUEST_ENC0_SPEED 3

#define MAGIC_NUM 158

//IOCTL interface prototypes
#define IOCTL_SPEED_LEFT _IOWR(MAGIC_NUM, 0, int16_t *)
#define IOCTL_SPEED_RIGHT _IOWR(MAGIC_NUM, 1, int16_t *)
 
static unsigned int gpioEncoder1_A 	= 20;
static unsigned int gpioEncoder1_B	 = 21; 
// static unsigned int gpioEncoder1_IDX = 26;

static unsigned int gpioEncoder0_A 	= 0;
static unsigned int gpioEncoder0_B	 = 1; 
// static unsigned int gpioEncoder0_IDX = 16; 

static unsigned int enc1_A_val;
static unsigned int enc1_B_val;
static int enc1_count;
static unsigned int enc1_A_val_prev;
static unsigned int enc1_B_val_prev;

static unsigned int enc0_A_val;
static unsigned int enc0_B_val;
static int enc0_count;
static unsigned int enc0_A_val_prev;
static unsigned int enc0_B_val_prev;

int irq_enc0_A;
int irq_enc0_B;

int irq_enc1_A;
int irq_enc1_B;

static long time_us_prev0;
static long time_us_prev1;
static int vel_index0 = 0;
static int vel_index1 = 0;
static long velocity0;
static long velocity1;
static long velocity_buffer0[100];
static long velocity_buffer1[100];

static unsigned int enc1_angle;
static unsigned int enc0_angle;

static int    major_number;                    
    
static struct class*  this_class  = NULL;
static struct device* this_device = NULL;
 
// The prototype functions for the character driver -- must come before the struct definition
static int encoder_driver_open(struct inode *, struct file *);
static int encoder_driver_close(struct inode *, struct file *);
long encoder_driver_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);
 
/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
	.open = encoder_driver_open,
	.unlocked_ioctl = encoder_driver_ioctl,
	.release = encoder_driver_close,
};
 
static long get_speed(long *vel)
 {
	 long sum = 0;
	 int i = 0;
	 for(;i<100;i++)
		 sum += vel[i];
	 return (sum/100);
 }
 
  
 
int getDirection(int prevA,int prevB, int A, int B )
{
	if((prevA == A) && (prevB == B))
	{
		return STOP;
	}
	else if(prevA == A)
	{
		if(A == 1)
		{
			if((B == 1) && (prevB == 0))	//Rising edge of B
				return CLOCKWISE;
			
			if((B == 0) && (prevB == 1))	//Falling edge of B
				return ANTI_CLOCKWISE;
		}
		if(A == 0)
		{
			if((B == 1) && (prevB == 0))	//Rising edge of B
				return ANTI_CLOCKWISE;
			
			if((B == 0) && (prevB == 1))	//Falling edge of B
				return CLOCKWISE;
		}		
	}
	else if(prevB == B)
	{
		if(B == 1)
		{
			if((A == 1) && (prevA == 0))	//Rising edge of A
				return ANTI_CLOCKWISE;
			
			if((A == 0) && (prevA == 1))	//Falling edge of A
				return CLOCKWISE;
		}
		if(B == 0)
		{
			if((A == 1) && (prevA == 0))	//Rising edge of A
				return CLOCKWISE;
			
			if((A == 0) && (prevA == 1))	//Falling edge of A
				return ANTI_CLOCKWISE;
		}			
	}
	return STOP;
	
}
 
static irq_handler_t enc0_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	const ktime_t currtime = ktime_get();
	int dir;
	long time_us, time_interval;

	enc0_A_val = gpio_get_value(gpioEncoder0_A);
	enc0_B_val = gpio_get_value(gpioEncoder0_B);
	dir = getDirection(enc0_A_val_prev,enc0_B_val_prev,enc0_A_val,enc0_B_val);
	
	enc0_A_val_prev = enc0_A_val;
	enc0_B_val_prev = enc0_B_val;
	enc0_count = enc0_count % 1200;
	
	if(dir == CLOCKWISE)
	{
	//	printk("Rotated CLOCKWISE \t");
		enc0_count++;
	}
	else if(dir == ANTI_CLOCKWISE)
	{
	//	printk("Rotated ANTI-CLOCKWISE \t");
		enc0_count--;
	}	
	else
	{
		// printk("STOPPED \t");
	}	

	vel_index0 = vel_index0 + 1;
	vel_index0 %=100;
	time_us = (long)ktime_to_us(currtime);
	time_interval = time_us - time_us_prev0;
	velocity_buffer0[vel_index0] = 73526/time_interval;
	
	time_us_prev0 = time_us;	
	
	// printk("Velocity 0 = %ld \n",get_speed(velocity_buffer0) );
	return (irq_handler_t) IRQ_HANDLED;
} 
 
static irq_handler_t enc1_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	const ktime_t currtime = ktime_get();
	int dir;
	long time_us, time_interval;
	
	enc1_A_val = gpio_get_value(gpioEncoder1_A);
	enc1_B_val = gpio_get_value(gpioEncoder1_B);
	dir = getDirection(enc1_A_val_prev,enc1_B_val_prev,enc1_A_val,enc1_B_val);
	
	enc1_A_val_prev = enc1_A_val;
	enc1_B_val_prev = enc1_B_val;
	enc1_count = enc1_count % 96;
	
	if(dir == CLOCKWISE)
	{
		// printk("Rotated CLOCKWISE \t");
		enc1_count++;
	}
	else if(dir == ANTI_CLOCKWISE)
	{
		// printk("Rotated ANTI-CLOCKWISE \t");
		enc1_count--;
	}	
	else
	{
		// printk("STOPPED \t");
	}		

	vel_index1 = vel_index1 + 1;
	vel_index1 %=100;
	time_us = (long)ktime_to_us(currtime);
	time_interval = time_us - time_us_prev1;
	velocity_buffer1[vel_index1] = 73526/time_interval;
	
	time_us_prev1 = time_us;	
	
	// printk("Velocity 1 = %ld \n",get_speed(velocity_buffer1) );
	return (irq_handler_t) IRQ_HANDLED;
}
 
/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init encoder_module_init(void)
{
	enc1_count  = 0;
	enc1_A_val = 0;
	enc1_B_val = 0;
	enc1_A_val_prev = 0;
	enc1_B_val_prev = 0;	

	// Trying to dynamically allocate a major number for the device
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number<0){
	  printk(KERN_ALERT "%s failed to register a major number\n",__func__);
	  return major_number;
	}
	printk(KERN_INFO "%s registered correctly with major number %d\n",__func__, major_number);
 
   // Trying to register the device class
   this_class = class_create(THIS_MODULE, CLASS_NAME);
   // Check for error and clean up if there is
   if (IS_ERR(this_class))
   {                
      unregister_chrdev(major_number, DEVICE_NAME);
      printk(KERN_ALERT "%s Failed to register device class\n", __func__);
      return PTR_ERR(this_class);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "%s device class registered correctly\n",__func__);
 
   // Register the device driver
   this_device = device_create(this_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
   if (IS_ERR(this_device)){               // Clean up if there is an error
      class_destroy(this_class); 
      unregister_chrdev(major_number, DEVICE_NAME);
      printk(KERN_ALERT "%s Failed to create the device\n", __func__);
      return PTR_ERR(this_device);
   }
   printk(KERN_INFO "%s: device class created correctly\n", __func__);

	if (gpio_request(gpioEncoder0_A, "gpioEncoder0_A"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder0_A\n", __func__);
	if (gpio_request(gpioEncoder0_B, "gpioEncoder0_B"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder0_B\n", __func__);	
	// if (gpio_request(gpioEncoder0_IDX, "gpioEncoder0_IDX"))
	// 	printk(KERN_ALERT "%s Failed to initialize gpioEncoder0_IDX\n", __func__);	    	
   	
	if (gpio_request(gpioEncoder1_A, "gpioEncoder1_A"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder1_A\n", __func__);
	if (gpio_request(gpioEncoder1_B, "gpioEncoder1_B"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder1_B\n", __func__);	
	// if (gpio_request(gpioEncoder1_IDX, "gpioEncoder1_IDX"))
	// 	printk(KERN_ALERT "%s Failed to initialize gpioEncoder1_IDX\n", __func__);
	
	if (gpio_direction_input(gpioEncoder0_A))
		printk(KERN_ALERT "%s Failed to enable gpioEncoder0_A\n", __func__);
	if (gpio_direction_input(gpioEncoder0_B))
		printk(KERN_ALERT "%s Failed to enable gpioEncoder0_B\n", __func__);		
	
	if (gpio_direction_input(gpioEncoder1_A))
		printk(KERN_ALERT "%s Failed to enable gpioEncoder1_A\n", __func__);
	if (gpio_direction_input(gpioEncoder1_B))
		printk(KERN_ALERT "%s Failed to enable gpioEncoder1_B\n", __func__);			
	
	irq_enc0_A = gpio_to_irq(gpioEncoder0_A);
	irq_enc0_B = gpio_to_irq(gpioEncoder0_B); 
	
	irq_enc1_A = gpio_to_irq(gpioEncoder1_A);
	irq_enc1_B = gpio_to_irq(gpioEncoder1_B);
	
	request_irq(irq_enc0_A, (irq_handler_t) enc0_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpioEncoder_handler",NULL);
	request_irq(irq_enc0_B, (irq_handler_t) enc0_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpioEncoder_handler",NULL);	
	
	request_irq(irq_enc1_A, (irq_handler_t) enc1_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpioEncoder_handler",NULL);
	request_irq(irq_enc1_B, (irq_handler_t) enc1_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpioEncoder_handler",NULL);

	enc0_A_val_prev = gpio_get_value(gpioEncoder0_A);
	enc0_B_val_prev = gpio_get_value(gpioEncoder0_B);
	
	enc1_A_val_prev = gpio_get_value(gpioEncoder1_A);
	enc1_B_val_prev = gpio_get_value(gpioEncoder1_B);
	
	time_us_prev0 = 0;
	time_us_prev0 = 1;
	vel_index0 =  0;
	vel_index0 =  1;
	
  
   
   return 0;
}
 
/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit encoder_module_exit(void){
	free_irq(irq_enc0_A, NULL);
	free_irq(irq_enc0_B, NULL);
	free_irq(irq_enc1_A, NULL);
	free_irq(irq_enc1_B, NULL);
	gpio_free(gpioEncoder0_A);
	gpio_free(gpioEncoder0_B);
	gpio_free(gpioEncoder1_A);
	gpio_free(gpioEncoder1_B);
	device_destroy(this_class, MKDEV(major_number, 0));     // remove the device
	class_unregister(this_class);                          // unregister the device class
	class_destroy(this_class);                             // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);             // unregister the major number
	pr_info("encoder driver module is unloaded.\n");
}
 
/** @brief The device open function that is called each time the device is opened
 *  This will only increment the number_of_opens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int encoder_driver_open(struct inode *inodep, struct file *filep){
	// pr_info("Open encoder driver successfully.\n");
	return 0;
}
 
long encoder_driver_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
 
       switch(ioctl_num)
       {
			case IOCTL_SPEED_LEFT:
					put_user(get_speed(velocity_buffer1),(int16_t*)ioctl_param);
					break;
			
			case IOCTL_SPEED_RIGHT:
					put_user(get_speed(velocity_buffer0),(int16_t*)ioctl_param);
					break;
       }
       return 0;
}
 
 
/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int encoder_driver_close(struct inode *inodep, struct file *filep)
{
	// pr_info("Close encoder driver successfully.\n");   
	return 0;
}
 
/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Driver for INA219 I2C <ThesisLDD Project>");

module_init(encoder_module_init);
module_exit(encoder_module_exit);
