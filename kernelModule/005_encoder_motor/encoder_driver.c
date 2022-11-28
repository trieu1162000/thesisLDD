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

 
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("Rishabh Bhatnagar and Krishanu Ganguly");    
MODULE_DESCRIPTION("A Encoder Driver to read rotary encoder values");
MODULE_VERSION("1.0");
 
static unsigned int gpioEncoder1_A 	= 17;
static unsigned int gpioEncoder1_B	 = 23; 
static unsigned int gpioEncoder1_IDX = 26;

static unsigned int gpioEncoder0_A 	= 6;
static unsigned int gpioEncoder0_B	 = 7; 
static unsigned int gpioEncoder0_IDX = 16; 

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

static long time_us_prev;
static int vel_index;
static long velocity;
static long velocity_buffer[100];

static unsigned int enc1_angle;
static unsigned int enc0_angle;



static int    major_number;              
static char   message[256] = {0};        
static short  size_of_message;           
static int    number_of_opens = 0;       
static struct class*  this_class  = NULL;
static struct device* this_device = NULL;
static char   *message_header;
 
// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
 
/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};
 
static long getVel(long *vel)
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

	enc0_A_val = gpio_get_value(gpioEncoder0_A);
	enc0_B_val = gpio_get_value(gpioEncoder0_B);
	int dir = getDirection(enc0_A_val_prev,enc0_B_val_prev,enc0_A_val,enc0_B_val);
	
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
		printk("STOPPED \t");
	}	

	vel_index = (vel_index++)%100;
	long time_us = (long)ktime_to_us(currtime);
	long time_interval = time_us - time_us_prev;
	velocity_buffer[vel_index] = 870000/time_interval;
	
	time_us_prev = time_us;	

	enc0_angle = (enc0_count)*360/1200;
	
/*	if(enc0_angle < 0)
		enc0_angle += 360;
	*/
	printk("Velocity = %ld \n",getVel(velocity_buffer) );
	return (irq_handler_t) IRQ_HANDLED;
} 
 
static irq_handler_t enc1_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	enc1_A_val = gpio_get_value(gpioEncoder1_A);
	enc1_B_val = gpio_get_value(gpioEncoder1_B);
	int dir = getDirection(enc1_A_val_prev,enc1_B_val_prev,enc1_A_val,enc1_B_val);
	
	enc1_A_val_prev = enc1_A_val;
	enc1_B_val_prev = enc1_B_val;
	enc1_count = enc1_count % 96;
	
	if(dir == CLOCKWISE)
	{
		printk("Rotated CLOCKWISE \t");
		enc1_count++;
	}
	else if(dir == ANTI_CLOCKWISE)
	{
		printk("Rotated ANTI-CLOCKWISE \t");
		enc1_count--;
	}	
	else
	{
		printk("STOPPED \t");
	}		

	enc1_angle = (enc1_count)*360/96;
/*	if(enc1_angle < 0)
		enc1_angle += 360;
	*/
	printk("Enc1 Angle = %d \n", enc1_angle);
	
	
	return (irq_handler_t) IRQ_HANDLED;
}
 
 
int strtoInt(const char *str, int len)
{
	int x = 0, ret = 0;
	int i = 0;
	for(;i<len;i++)
	{
		x = str[i] - 48;
		ret = (ret * 10) + x;
	}
	return ret;
}
 
/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init encoder_init(void)
{

	enc1_count  = 0;
	enc1_A_val = 0;
	enc1_B_val = 0;
	enc1_A_val_prev = 0;
	enc1_B_val_prev = 0;	
 
	message_header = "Encoder Driver (encoder.ko):";
	printk(KERN_INFO "%s Initializing the LKM...\n",message_header );

	// Trying to dynamically allocate a major number for the device
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number<0){
	  printk(KERN_ALERT "%s failed to register a major number\n",message_header);
	  return major_number;
	}
	printk(KERN_INFO "%s registered correctly with major number %d\n",message_header, major_number);
 
   // Trying to register the device class
   this_class = class_create(THIS_MODULE, CLASS_NAME);
   // Check for error and clean up if there is
   if (IS_ERR(this_class))
   {                
      unregister_chrdev(major_number, DEVICE_NAME);
      printk(KERN_ALERT "%s Failed to register device class\n", message_header);
      return PTR_ERR(this_class);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "%s device class registered correctly\n",message_header);
 
   // Register the device driver
   this_device = device_create(this_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
   if (IS_ERR(this_device)){               // Clean up if there is an error
      class_destroy(this_class); 
      unregister_chrdev(major_number, DEVICE_NAME);
      printk(KERN_ALERT "%s Failed to create the device\n", message_header);
      return PTR_ERR(this_device);
   }
   printk(KERN_INFO "%s: device class created correctly\n", message_header);

	if (gpio_request(gpioEncoder0_A, "gpioEncoder0_A"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder0_A\n", message_header);
	if (gpio_request(gpioEncoder0_B, "gpioEncoder0_B"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder0_B\n", message_header);	
	if (gpio_request(gpioEncoder0_IDX, "gpioEncoder0_IDX"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder0_IDX\n", message_header);	    	
   	
	if (gpio_request(gpioEncoder1_A, "gpioEncoder1_A"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder1_A\n", message_header);
	if (gpio_request(gpioEncoder1_B, "gpioEncoder1_B"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder1_B\n", message_header);	
	if (gpio_request(gpioEncoder1_IDX, "gpioEncoder1_IDX"))
		printk(KERN_ALERT "%s Failed to initialize gpioEncoder1_IDX\n", message_header);
	
	if (gpio_direction_input(gpioEncoder0_A))
		printk(KERN_ALERT "%s Failed to enable gpioEncoder0_A\n", message_header);
	if (gpio_direction_input(gpioEncoder0_B))
		printk(KERN_ALERT "%s Failed to enable gpioEncoder0_B\n", message_header);		
	
	if (gpio_direction_input(gpioEncoder1_A))
		printk(KERN_ALERT "%s Failed to enable gpioEncoder1_A\n", message_header);
	if (gpio_direction_input(gpioEncoder1_B))
		printk(KERN_ALERT "%s Failed to enable gpioEncoder1_B\n", message_header);			
	
	int irq_enc0_A = gpio_to_irq(gpioEncoder0_A);
	int irq_enc0_B = gpio_to_irq(gpioEncoder0_B); 
	
	int irq_enc1_A = gpio_to_irq(gpioEncoder1_A);
	int irq_enc1_B = gpio_to_irq(gpioEncoder1_B);
	
	request_irq(irq_enc0_A, (irq_handler_t) enc0_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpioEncoder_handler",NULL);
	request_irq(irq_enc0_B, (irq_handler_t) enc0_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpioEncoder_handler",NULL);	
	
	request_irq(irq_enc1_A, (irq_handler_t) enc1_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpioEncoder_handler",NULL);
	request_irq(irq_enc1_B, (irq_handler_t) enc1_irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpioEncoder_handler",NULL);

	enc0_A_val_prev = gpio_get_value(gpioEncoder0_A);
	enc0_B_val_prev = gpio_get_value(gpioEncoder0_B);
	
	enc1_A_val_prev = gpio_get_value(gpioEncoder1_A);
	enc1_B_val_prev = gpio_get_value(gpioEncoder1_B);
	
	time_us_prev = 0;
	vel_index =  0;
	enc1_angle = 0;
	enc0_angle = 0;
	
	printk(KERN_ALERT "GPIO init successful!\n");	   
   
   return 0;
}
 
/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit encoder_exit(void){
   device_destroy(this_class, MKDEV(major_number, 0));     // remove the device
   class_unregister(this_class);                          // unregister the device class
   class_destroy(this_class);                             // remove the device class
   unregister_chrdev(major_number, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "encoder: Goodbye from the LKM!\n");
}
 
/** @brief The device open function that is called each time the device is opened
 *  This will only increment the number_of_opens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   number_of_opens++;
   printk(KERN_INFO "encoder: Device has been opened %d time(s)\n", number_of_opens);
   return 0;
}
 
/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message-1);
   
   printk("Message printed to user: %s \n", message);
   
   if (error_count==0){            // if true then have success
      printk(KERN_INFO "encoder: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "encoder: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}
 
/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   size_of_message = strlen(buffer);                 // store the length of the stored message
  
   switch(strtoInt(buffer,len-1))
   {
	   case REQUEST_ENC0_ANGLE:	   	sprintf(message,"%d ", enc0_angle);   
									size_of_message = strlen(message);
									printk("Read ENC0 Angle \n");
									break;
	   
	   case REQUEST_ENC1_ANGLE:	   	sprintf(message,"%d ", enc1_angle);   
									size_of_message = strlen(message);
									printk("Read ENC1 Angle \n");
									break;
									
	   case REQUEST_ENC0_SPEED:   	sprintf(message,"%d ", getVel(velocity_buffer));   
									size_of_message = strlen(message);
									printk("Read ENC0 Speed \n");
									break;
								
	    default:			   		sprintf(message,"Invalid! \n");   
									size_of_message = strlen(message);
									break;
   }
   return len;
}
 
/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "encoder: Device successfully closed\n");
   return 0;
}
 
/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(encoder_init);
module_exit(encoder_exit);
