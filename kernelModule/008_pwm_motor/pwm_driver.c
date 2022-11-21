
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/kernel.h>

// Name of the device 
#define DRIVER_NAME "pwm_driver" 
#define DRIVER_CLASS "pwm_driver_class"

// Period for pwm data (10 usecond <=> 100kHz)
#define PWM_PERIOD 10000

// Initial period for duty cycle (1%)
#define PWM_DUTY_INITIAL 100

// Struct that defines the pwm control abstraction
struct pwm_device *pwm_left = NULL;
struct pwm_device *pwm_right = NULL;

/* Device data struct */
struct pwm_motor{
        struct platform_device *pwmpdev;
        struct pwm_device *pwmdev;
	dev_t dev_number;
	struct cdev c_dev;
	struct device *device;
	struct list_head device_entry;
};

/* Linked list for each entry  (In here, I have 2 motor equivalent to 2 entry) */
static LIST_HEAD(device_list);

// Variables for interfacing with the devices and their class
static dev_t dev_num;
static struct class *pwm_driver_class;
static struct pwm_state pwm_state_setted;

// Opening function, lower layer abstraction to the kernel's
// open() function. This can be defined in another way, with
// permissions and options as well
static int pwm_driver_open(struct inode *device_file, struct file *instance)
{
        struct pwm_motor *motor = NULL;
        int status = -1;

        list_for_each_entry(motor, &device_list, device_entry){
                if(motor->dev_number == device_file->i_rdev){
                        status = 0;
                        break;
                }
        }

        if(status)
        {
                pr_info("%s: Cannot find the device in linked list.\n", __func__);
		return status;
        }
        pr_info("Open pwm driver successfully.\n");

        instance->private_data = motor;

        return 0;
}


// Same thing, but for close
static int pwm_driver_close(struct inode *device_file, struct file *instance)
{
        pr_info("Close pwm driver successfully.\n");

        return 0;
}

// Write data to driver, it's usual for linux's native write/read functions to return the amount 
// of *something* that it was able to write/read
static ssize_t pwm_driver_write(struct file *file, const char *user_buffer, size_t count, loff_t *offs){

        int to_copy, ret;
        char input_pwm_value[2];
        unsigned long long pwm_value = 0;
        struct pwm_motor *motor = file->private_data;

        to_copy = min(count, sizeof(input_pwm_value));

        ret = kstrtoull_from_user(user_buffer, to_copy, 10, &pwm_value);
	if (ret) {
		/* Negative error code. */
		pr_info("%s: Invalid input = %llu\n", __func__, pwm_value);
		return ret;
	} 
        *offs = count;

        // Set pwm frequency
        // Rules: as it's a character device, let's define that sending 
        // the 0 corresponds to a duty cycle of 0%, through 99 corresponding to one of 99%  
        if(pwm_value < 0 || pwm_value > 99)
        {
                pr_info("%s: Invalid input pwm value: %llu, try again\n", __func__, pwm_value);
        }
        else
        {
                pwm_state_setted.duty_cycle = pwm_value*PWM_DUTY_INITIAL;
                pwm_apply_state(motor->pwmdev, &pwm_state_setted);
        }

        // delta = to_copy - not_copied;

        return count;
}
 
// Assuming pwm function is, for now, a char dev
// We could instantiate even more operations here (like read)
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = pwm_driver_open,
    .release = pwm_driver_close,
    .write = pwm_driver_write
};

static int pwm_driver_probe(struct platform_device *pdev)
{
        struct pwm_motor *motor = NULL;
        int major_number = MAJOR(dev_num);
        int minor_number;
        int ret;

	/* Dynamically allocate memory for the device private data  */
        motor = devm_kzalloc(&pdev->dev, sizeof(*motor),GFP_KERNEL);
	if (motor == NULL)
		goto motor_aloccate_failed;

        /*save the device private data pointer in platform device structure */
	dev_set_drvdata(&pdev->dev, motor);
        motor->pwmpdev = pdev;
        motor->pwmdev = pwm_get(&pdev->dev, NULL);

        minor_number = motor->pwmdev->pwm;
        motor->dev_number = MKDEV(major_number, minor_number);
        pr_info("%s: A motor device is probed, major: %d, minor: %d\n", __func__, major_number, minor_number);
        motor->device = device_create(pwm_driver_class, &pdev->dev, motor->dev_number, motor, "motor%d", minor_number);
        if(motor->device == NULL)
        {
                pr_info("%s: Can not create device.\n", __func__);
                goto rem_device;
        }

        cdev_init(&motor->c_dev, &fops);
        motor->c_dev.owner = THIS_MODULE;
        motor->c_dev.dev = motor->dev_number;
        ret = cdev_add(&motor->c_dev, motor->dev_number, 1);
        if(ret < 0)
        {
		pr_err("%s: Cdev add failed.\n", __func__);
		goto rem_del;
	}

        INIT_LIST_HEAD(&motor->device_entry);
        list_add(&motor->device_entry, &device_list);

        pwm_state_setted.period = PWM_PERIOD;
        pwm_state_setted.duty_cycle = 0;
        pwm_state_setted.polarity = PWM_POLARITY_NORMAL;
        pwm_state_setted.enabled = true;
        pwm_apply_state(motor->pwmdev, &pwm_state_setted);

        return 0;

rem_del:
        cdev_del(&motor->c_dev);
rem_device:
        device_destroy(pwm_driver_class, motor->dev_number);	
motor_aloccate_failed:	
        return -1;
}

static int pwm_driver_remove(struct platform_device *pdev)
{
        struct pwm_motor *motor = dev_get_drvdata(&pdev->dev);;

        if(motor == NULL)
        {
                pr_err("%s: Error to get drvdata.\n", __func__);
                return -1;
        }
        // Clean up pwm motor device and all the struct involving
        pwm_disable(motor->pwmdev);
        pwm_free(motor->pwmdev);
        list_del(&motor->device_entry);
        cdev_del(&motor->c_dev);
        device_destroy(pwm_driver_class, motor->dev_number);
        unregister_chrdev_region(motor->dev_number, 1);

        pr_info("%s: Removing motor %d device", __func__, motor->pwmdev->pwm); 
        return 0;
}


static struct of_device_id pwm_match_table[] = {
     {
        .compatible = "motor_left",     
     },
     {
        .compatible = "motor_right",  
     },
     {},
};
MODULE_DEVICE_TABLE(of, pwm_match_table);

static struct platform_driver pwm_platform_driver = {
    .probe = pwm_driver_probe,
    .remove = pwm_driver_remove,
    .driver = {
        .name = "motor",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(pwm_match_table),
    },
};

/* Function called when entering the Kernel */
static int __init pwm_driver_module_init(void)
{

        pr_info("PWM driver module is loaded.\n");

        // Instead of using a static device number, like in many tutorials, 
        // let's try to used a system allocated one by the means of interfacing 
        // with the alloc_chrdev_region function

        // Allocated new device number
        if(alloc_chrdev_region(&dev_num, 0, 2, DRIVER_NAME) < 0){
                pr_err("%s: Error in allocation of device file number\n", __func__);
                goto ClassError;
        }

        // Device class creation 
        if((pwm_driver_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
                pr_err("%s: Device class cannot be created\n", __func__);
                
                goto FileError;
        }

        platform_driver_register(&pwm_platform_driver);

        // Sucess
        return 0;

        // Fail Handling
        FileError:
                class_destroy(pwm_driver_class);

        ClassError:
                unregister_chrdev(dev_num, DRIVER_NAME);

                return -1;
}

/* Function called when exiting the Kernel */
static void __exit pwm_driver_module_exit(void)
{

	platform_driver_unregister(&pwm_platform_driver);
	class_destroy(pwm_driver_class);
	unregister_chrdev_region(dev_num, 2);

	pr_info("PWM driver module is unloaded.\n");
}

/* Meta information for alllowing correct loading of modules through distros*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Driver for PWM controlling 2 motor <ThesisLDD Projec>t");

// Linking the initialization and exit of the module to
// it's respective functions
module_init(pwm_driver_module_init);
module_exit(pwm_driver_module_exit);