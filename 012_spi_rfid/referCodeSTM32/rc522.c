#include "rc522_api.h"

static unsigned char cardID[5];

struct spi_device *rc522_spi;

dev_t dev;
static struct cdev rfid_cdev;
static struct class *dev_class;
static struct device *dev_struct;

static int rc522_open(struct inode *inode,struct file *filp)
{
	MFRC522Init();
	printk(KERN_DEBUG"RC522 Device File Opened...!!!\n");
	return 0;
}

static ssize_t rc522_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
		if (MFRC522Check(cardID) == MI_OK) 
    {
      printk(KERN_DEBUG"Card info:[%02x-%02x-%02x-%02x-%02x] \r\n", cardID[0], cardID[1], cardID[2], cardID[3], cardID[4]);
    }
    else 
      printk(KERN_DEBUG"Cannot read Card ID!\n");
    return sizeof(buf);
  
}

// static ssize_t rc522_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
// {
// 	if (blockAddr == 0) {
// 		printk(KERN_DEBUG"block[0] is reserveed, can't write\n");
// 		return 0;
// 	}
// 	if (blockAddr < 0 || blockAddr > 63) {
// 		printk(KERN_DEBUG"block[%d] unreachable, please set the write block first", blockAddr);
// 		return -0;
// 	} 
// 	if ((blockAddr % 4) == 3) {
// 		printk(KERN_DEBUG"block[%d] is key block, not data block\n", blockAddr);
// 		return -0;
// 	}
// 	memset(WriteData, 0, sizeof(WriteData));
// 	if (copy_from_user(WriteData, (char *)buf, count)) {
// 		printk(KERN_DEBUG"%s, [line %d] copy from user err.\n", __FILE__, __LINE__);
// 		return 0;
// 	}
// 	/*PcdReset();*/
// 	operationcard =  WRITE_CARD;
// 	if(rc522_loop_work(operationcard))
// 		return -EFAULT;
// 	return 0;
// }

static int rc522_release(struct inode *inode,struct file *filp)
{
	printk(KERN_DEBUG"%s\n", __func__);
	return 0;
}

// static long rc522_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
// {
// 	printk(KERN_DEBUG "CMD = 0x%x\n", cmd);
// 	switch(cmd) {
// 	case CHANGE_PASSWD:
// 		operationcard = CHANGE_PASSWD;
// 		if (copy_from_user(PassWd, (char *)arg, sizeof(PassWd))) {
// 			printk(KERN_DEBUG"%s:change pass word err", __func__);
// 			return -EFAULT;
// 		}
// 		break;
// 	case CHANGE_BLOCK:
// 		if (arg < 0 || arg > 63) {
// 			printk(KERN_DEBUG"block number err %lu\n", arg);
// 			return -EFAULT;
// 		}
// 		blockAddr = (int)arg;
// 		printk(KERN_INFO "block = %d", blockAddr);
// 		break;
// 	case READ_CARD:
// 		break;
// 	case WRITE_CARD:
// 		break;
// 	case CHANGE_KEY:
// 		operationcard = CHANGE_KEY;
// 		break;
// 	case GET_ID:
// 		operationcard =  GET_ID;
// 		if(!rc522_loop_work(operationcard)){
// 			if (copy_to_user((char *)arg, MLastSelectedSnr,4)) {
// 				printk(KERN_DEBUG"%s, [line %d] copy to user err.\n", __FILE__, __LINE__);
// 				return -EFAULT;
// 			}
// 		}
// 		else
// 			return -EFAULT;
// 		break;
// 	case BEEP:
// 		buzzerFunc();
// 		break;
// 	default:
// 		break;
// 	}
// 	return 0;
// }

static int rc522_remove(struct spi_device *spi)
{

	return 0;
}
static int rc522_probe(struct spi_device *spi)
{
	printk(KERN_DEBUG "SPI number = %d",spi->controller->bus_num);
	printk(KERN_DEBUG"%s\n", __func__);
	rc522_spi = spi;
	return 0;
};

static struct of_device_id spi_rfid_dt_ids[] = { // DTS compatible //
		{ .compatible = "shtl,rfid_rc522" },
		{},
};

MODULE_DEVICE_TABLE(of, spi_rfid_dt_ids); // Add our device to Devices Table - for "Hot Pluggin" //

static struct spi_driver rc522_driver = {
		.driver = {
				.name			= "rfid_rc522",
				.owner  		= THIS_MODULE,
				.of_match_table = of_match_ptr(spi_rfid_dt_ids),
		},
		.probe = rc522_probe,
		.remove = rc522_remove,
};

static struct file_operations rc522_fops = {
		.owner = THIS_MODULE,
		.open = rc522_open,
		.release = rc522_release,
		.read = rc522_read,
		.write = rc522_write,
		// .unlocked_ioctl = rc522_ioctl,
};

static int RC522_driver_init(void)
{	
	printk(KERN_DEBUG"RFID_RC522 module init.\n");

	if((alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) < 0) {
		printk(KERN_INFO "Error allocation Major number !\n");
		return -1;
	}
	printk(KERN_INFO "RFID_RC522 module has been inserted, Major num: %d | Minor num: %d\n", MAJOR(dev), MINOR(dev));

	cdev_init(&rfid_cdev,&rc522_fops);

	if((cdev_add(&rfid_cdev, dev, 1)) < 0) {
		printk(KERN_INFO "Cannot add rfid_cdev to the system\n");
		goto rem_cdev;
	}
	printk(KERN_INFO "Char device for RC522_RFID module has been added");

	if((dev_class = class_create(THIS_MODULE,CLASS_NAME)) == NULL) {
		printk(KERN_INFO "Could not create a class");
		goto rem_class;
	}

	if((dev_struct = device_create(dev_class,NULL,dev,NULL,DEVICE_NAME)) == NULL) {
		printk(KERN_INFO "Talos device cannot be created");
		goto rem_device;
	}

	if(spi_register_driver(&rc522_driver) < 0) {
		printk(KERN_DEBUG"SPI driver register failed\n");
		goto rem_device;
	}

	// for(i=0; i < (sizeof(ioctls) / sizeof(IOCTLDescription_t)); i++)
	// 	printk(KERN_INFO"IOCTL Codes:\t%s=0x%02X\n", ioctls[i].name, ioctls[i].ioctlcode);

	if(gpio_is_valid(GPIO_5) == false){
    		pr_err("GPIO %d is not valid\n", GPIO_5);
    		goto rem_device;
 	}
  
  	//Requesting the GPIO
  	if(gpio_request(GPIO_5,"GPIO_5") < 0){
    		pr_err("ERROR: GPIO %d request\n", GPIO_5);
    		goto rem_gpio;
 	}
  
  	//configure the GPIO as output
  	gpio_direction_output(GPIO_5, 0);
  	gpio_set_value(GPIO_5, 0);
  /* Using this call the GPIO 5 will be visible in /sys/class/gpio/
  ** Now you can change the gpio values by using below commands also.
  ** echo 1 > /sys/class/gpio/gpio21/value  (turn ON the LED)
  ** echo 0 > /sys/class/gpio/gpio21/value  (turn OFF the LED)
  ** cat /sys/class/gpio/gpio21/value  (read the value LED)
  ** 
  ** the second argument prevents the direction from being changed.
  */
  	gpio_export(GPIO_5, false);
  
  	pr_info("GPIO init Pass!!!\n");
  	return 0;
 
rem_gpio:
  	gpio_free(GPIO_5);
rem_device:
	device_destroy(dev_class,dev);
rem_class:
	class_destroy(dev_class);
rem_cdev:
	cdev_del(&rfid_cdev);
	unregister_chrdev_region(dev,1);
	return -1;
}

static void RC522_driver_exit(void)
{
	gpio_set_value(GPIO_5,0);
	gpio_unexport(GPIO_5);
	gpio_free(GPIO_5);
	spi_unregister_driver(&rc522_driver);
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&rfid_cdev);
	unregister_chrdev_region(dev,1);
	printk(KERN_DEBUG"RC522_RFID module has been removed\n");
}

module_init(RC522_driver_init);
module_exit(RC522_driver_exit);

MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Device Driver for RFID RC522 Module");
MODULE_LICENSE("GPL");	

