#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>

#include "ina219_ioctl.h"

#define DRIVER_NAME "ina219"
#define DRIVER_CLASS "ina219_class"

static struct i2c_adapter * ina219_i2c_adapter = NULL;
static struct i2c_client * ina219_i2c_client = NULL;

#define I2C_BUS_USED       1               /* The I2C Bus available on the raspberry */
#define INA219_SENSOR_NAME       "INA219"        /* Device and Driver Name */
#define INA219_SLAVE_ADDRESS    0x40            /* BMP280 I2C address */

#define CALIBRATION_VALUE 0x1064

static const struct i2c_device_id ina219_dev_id[]={
       { INA219_SENSOR_NAME, 0},
       { }
};

static struct i2c_driver ina219_driver = {
       .driver = {
              .name = INA219_SENSOR_NAME,
              .owner = THIS_MODULE
       }
};

static struct i2c_board_info ina219_i2c_board_info = {
       I2C_BOARD_INFO(INA219_SENSOR_NAME, INA219_SLAVE_ADDRESS)
};


static dev_t ina219_device_number;
static struct class *ina219_class;
static struct cdev ina219_cdev;

void reverse_byte(int16_t *data)
{
       char temp;
       char *ptr = (char*)data;
       temp=*ptr;
       *ptr=*(ptr+1);
       *(ptr+1)=temp;
}

static ssize_t ina219_driver_read(struct file *File, char __user *user_buffer, size_t count, loff_t *offs)
{
       int to_copy, not_copied, delta;
       char out_string[100];
       int16_t bus_voltage;
       int16_t shunt_voltage;
       int16_t power;
       int16_t measured_current;

       to_copy = min(sizeof(out_string), count);
       
       bus_voltage = i2c_smbus_read_word_data(ina219_i2c_client,0x02);
       reverse_byte(&bus_voltage);
       shunt_voltage = i2c_smbus_read_word_data(ina219_i2c_client,0x01);
       reverse_byte(&shunt_voltage);
       power = i2c_smbus_read_word_data(ina219_i2c_client,0x03);
       reverse_byte(&power);
       measured_current = i2c_smbus_read_word_data(ina219_i2c_client,0x04);
       reverse_byte(&measured_current);
       
       snprintf(out_string, sizeof(out_string), "Bus:%d,Shunt:%d,Power:%d,Current:%d\n",bus_voltage,shunt_voltage,power,measured_current);
       not_copied = copy_to_user(user_buffer, out_string, to_copy);

       delta = to_copy - not_copied;

       return 0;
}

static int16_t read_bus_voltage(void)
{
       int16_t temp;
       temp = i2c_smbus_read_word_data(ina219_i2c_client,0x02);
       reverse_byte(&temp);
       return temp;
}

static int16_t read_shunt_voltage(void)
{ 
       int16_t temp;
       temp = i2c_smbus_read_word_data(ina219_i2c_client,0x01);
       reverse_byte(&temp);
       return temp;
}

static int16_t read_measured_current(void)
{ 
       int16_t temp;
       temp = i2c_smbus_read_word_data(ina219_i2c_client,0x04);
       reverse_byte(&temp);
       return temp;
}

static int16_t read_power(void)
{ 
       int16_t temp;
       temp = i2c_smbus_read_word_data(ina219_i2c_client,0x03);
       reverse_byte(&temp);
       return temp;
}

long ina219_driver_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
 
       switch(ioctl_num)
       {
       case IOCTL_BUS_VOLTAGE:
              put_user(read_bus_voltage(),(int16_t*)ioctl_param);
              break;
       
       case IOCTL_SHUNT_VOLTAGE:
              put_user(read_shunt_voltage(),(int16_t*)ioctl_param);
              break;

       case IOCTL_MEASURED_CURRENT:
              put_user(read_measured_current(),(int16_t*)ioctl_param);
              break;

       case IOCTL_POWER:
              put_user(read_power(),(int16_t*)ioctl_param);
              break;
       }
       return 0;
}

static int ina219_driver_open(struct inode *deviceFile, struct file *instance)
{
	pr_info("Open ina219_i2c driver successfully.\n");
	return 0;
}

static int ina219_driver_close(struct inode *deviceFile, struct file *instance) 
{
	pr_info("Close ina219_i2c driver successfully.\n");
       return 0;
}

static struct file_operations fops  = {
       .owner = THIS_MODULE,
       .open = ina219_driver_open,
       .release = ina219_driver_close,
       .unlocked_ioctl = ina219_driver_ioctl,
       .read = ina219_driver_read,
};

static int __init ina219_module_init(void)
{
       int ret =-1;
       int16_t id;

       if ( alloc_chrdev_region(&ina219_device_number, 0, 1, DRIVER_NAME) < 0) 
       {
		pr_err("%s: Device number could not be allocated.\n", __func__);
		goto rem_unreg;
       }

	pr_info("%s: Device number with major: %d, minor: %d was registered.\n", __func__, MAJOR(ina219_device_number), MINOR(ina219_device_number));

	/*2. Initialize the cdev structure with fops*/
       cdev_init(&ina219_cdev, &fops);

	/* 3. Register a device (cdev structure) with VFS */
       if (cdev_add(&ina219_cdev, ina219_device_number, 1) == -1) 
       {
		pr_err("%s: Registering of device to kernel failed.\n", __func__);
		goto rem_del;
       }

              /* Create Device Class */
       if ((ina219_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) 
       {
		pr_err("%s: Device class can not be created.\n", __func__);
		goto rem_class;
       }

              /* Create Device file */
       if (device_create(ina219_class, NULL, ina219_device_number, NULL, DRIVER_NAME) == NULL) 
       {
		pr_err("%s: Can not create device file.\n", __func__);
		goto rem_device;
       }

       ina219_i2c_adapter = i2c_get_adapter(I2C_BUS_USED);

       if(ina219_i2c_adapter != NULL) 
       {
              ina219_i2c_client = i2c_new_client_device(ina219_i2c_adapter, &ina219_i2c_board_info);
       
              if(ina219_i2c_client != NULL) 
              {
                     if(i2c_add_driver(&ina219_driver) != -1) 
                     {
                     ret = 0;
                     }
                     else
                            printk("Can't add driver...\n");
              }
              i2c_put_adapter(ina219_i2c_adapter);
       }

	pr_info("ina219_i2c driver module is loaded.\n");
       // i2c_smbus_write_word_data(ina219_i2c_client, 0x00, 0x1df);
       id = i2c_smbus_read_word_data(ina219_i2c_client, 0x00);
       reverse_byte(&id);
       pr_info("Config Data: 0x%x\n", id);
       i2c_smbus_write_word_data(ina219_i2c_client, 0x05, CALIBRATION_VALUE);

       return ret;

rem_device:
       device_destroy(ina219_class, ina219_device_number);
rem_class:
       class_destroy(ina219_class);
rem_del:								/* Delete cdev structure */	
	cdev_del(&ina219_cdev);
rem_unreg:
       unregister_chrdev(ina219_device_number, DRIVER_NAME);
       return (-1);
}

static void __exit ina219_module_exit(void) 
{
       i2c_unregister_device(ina219_i2c_client);
       i2c_del_driver(&ina219_driver);
       cdev_del(&ina219_cdev);
       device_destroy(ina219_class, ina219_device_number);
       class_destroy(ina219_class);
       unregister_chrdev_region(ina219_device_number, 1);
       pr_info("ina219_i2c driver module is unloaded.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Driver for INA219 I2C <ThesisLDD Project>");

module_init(ina219_module_init);
module_exit(ina219_module_exit);
