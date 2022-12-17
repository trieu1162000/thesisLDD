#include <linux/module.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>

#include "lcd_i2c_api.h"

#define CLASS_NAME	"hd44780"
#define DRIVER_NAME	"hd44780"
#define NUM_DEVICES	1

static struct class *hd44780_class;
static dev_t dev_no;

static LIST_HEAD(hd44780_list);
static DEFINE_SPINLOCK(hd44780_list_lock);

static int __init hd44780_driver_init(void);
static void __exit hd44780_driver_exit(void);

void hd44780_write(struct hd44780 *, const char *, size_t);
void hd44780_init_lcd(struct hd44780 *);
void hd44780_print(struct hd44780 *, const char *);
void hd44780_flush(struct hd44780 *);
void hd44780_set_geometry(struct hd44780 *, struct hd44780_geometry *);
void hd44780_set_backlight(struct hd44780 *, bool);
void hd44780_set_cursor_blink(struct hd44780 *, bool);
void hd44780_set_cursor_display(struct hd44780 *, bool);

/* Device attributes */

static ssize_t geometry_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct hd44780 *lcd;
	struct hd44780_geometry *geo;
	
	lcd = dev_get_drvdata(dev);
	geo = lcd->geometry;

	return scnprintf(buf, PAGE_SIZE, "%dx%d\n", geo->cols, geo->rows);
}

static ssize_t geometry_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd44780 *lcd;
	struct hd44780_geometry *geo;
	int cols = 0, rows = 0, i;

	sscanf(buf, "%dx%d", &cols, &rows);

	for (i = 0; hd44780_geometries[i] != NULL; i++) {
		geo = hd44780_geometries[i];

		if (geo->cols == cols && geo->rows == rows) {
			lcd = dev_get_drvdata(dev);

			mutex_lock(&lcd->lock);
			hd44780_set_geometry(lcd, geo);
			mutex_unlock(&lcd->lock);

			break;
		}
	}

	return count;
}
static DEVICE_ATTR_RW(geometry);

static ssize_t backlight_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", lcd->backlight);
}

static ssize_t backlight_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	mutex_lock(&lcd->lock);
	hd44780_set_backlight(lcd, buf[0] == '1');
	mutex_unlock(&lcd->lock);

	return count;
}
static DEVICE_ATTR_RW(backlight);

static ssize_t cursor_blink_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", lcd->cursor_blink);
}

static ssize_t cursor_blink_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	mutex_lock(&lcd->lock);
	hd44780_set_cursor_blink(lcd, buf[0] == '1');
	mutex_unlock(&lcd->lock);

	return count;
}
static DEVICE_ATTR_RW(cursor_blink);

static ssize_t cursor_display_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", lcd->cursor_display);
}

static ssize_t cursor_display_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	mutex_lock(&lcd->lock);
	hd44780_set_cursor_display(lcd, buf[0] == '1');
	mutex_unlock(&lcd->lock);

	return count;
}
static DEVICE_ATTR_RW(cursor_display);

static struct attribute *hd44780_device_attrs[] = {
	&dev_attr_geometry.attr,
	&dev_attr_backlight.attr,
	&dev_attr_cursor_blink.attr,
	&dev_attr_cursor_display.attr,
	NULL
};
ATTRIBUTE_GROUPS(hd44780_device);

/* File operations */

static int hd44780_file_open(struct inode *inode, struct file *filp)
{
	filp->private_data = container_of(inode->i_cdev, struct hd44780, cdev);
	// pr_info("Open lcd_i2c driver successfully.\n");

	return 0;
}

static int hd44780_file_release(struct inode *inode, struct file *filp)
{
	struct hd44780 *lcd = filp->private_data;
	hd44780_flush(lcd);
	// pr_info("Close lcd_i2c driver successfully.\n");

	return 0;
}

static ssize_t hd44780_file_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	struct hd44780 *lcd;
	size_t to_copy;

	lcd = filp->private_data;
	to_copy = min(count, (size_t)BUF_SIZE);

	// TODO: Consider using an interruptible lock
	mutex_lock(&lcd->lock);

	// TODO: Support partial writes during errors?
	if (copy_from_user(lcd->buf, buf, to_copy)) {
		mutex_unlock(&lcd->lock);
		return -EFAULT;
	}

	hd44780_write(lcd, lcd->buf, to_copy);
 	// pr_info("Write %zu bytes receive from user space to lcd.\n",to_copy);	
	
	mutex_unlock(&lcd->lock);

	return to_copy;		// return not_copied is more suitable
}

static void hd44780_init(struct hd44780 *lcd, struct hd44780_geometry *geometry,
		struct i2c_client *i2c_client)
{
	lcd->geometry = geometry;
	lcd->i2c_client = i2c_client;
	lcd->pos.row = 0;
	lcd->pos.col = 0;
	memset(lcd->esc_seq_buf.buf, 0, ESC_SEQ_BUF_SIZE);
	lcd->esc_seq_buf.length = 0;
	lcd->is_in_esc_seq = false;
	lcd->backlight = true;
	lcd->cursor_blink = false;
	lcd->cursor_display = false;
	mutex_init(&lcd->lock);
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = hd44780_file_open,
	.release = hd44780_file_release,
	.write = hd44780_file_write,
};

static int hd44780_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int major_number = MAJOR(dev_no);
	int minor_number;
	struct hd44780 *lcd = NULL;
	int ret;

	/* Dynamically allocate memory for the device private data  */
	lcd = kzalloc(sizeof(*lcd), GFP_KERNEL);
	if (!lcd) 
		goto lcd_aloccate_failed;	

	/* Init lcd parameter for client */
	hd44780_init(lcd, hd44780_geometries[0], client);

	minor_number = i2c_adapter_id(lcd->i2c_client->adapter);
	lcd->dev_number = MKDEV(major_number, minor_number);
	pr_info("%s: A lcd_i2c device is probed, major: %d, minor: %d\n", __func__, major_number, minor_number);
	lcd->device = device_create(hd44780_class, &client->dev, lcd->dev_number, lcd, "lcd%d", minor_number);
	if(lcd->device == NULL)
	{
			pr_info("%s: Can not create device.\n", __func__);
			goto rem_device;
	}

	spin_lock(&hd44780_list_lock);
	list_add(&lcd->list, &hd44780_list);
	spin_unlock(&hd44780_list_lock);

	cdev_init(&lcd->cdev, &fops);
	lcd->cdev.owner = THIS_MODULE;
	lcd->cdev.dev = lcd->dev_number;
	ret = cdev_add(&lcd->cdev, lcd->dev_number, 1);
	if (ret) {
		pr_err("%s: Cdev add failed.\n", __func__);
		goto rem_del;
	}

	dev_set_drvdata(&client->dev, lcd);
	hd44780_init_lcd(lcd);
	hd44780_set_cursor_blink(lcd, false);
	hd44780_set_cursor_display(lcd, false);
	hd44780_print(lcd, "/dev/");
	hd44780_print(lcd, lcd->device->kobj.name);
	lcd->dirty = true;
	pr_info("%s: Probe was called successfully.\n", __func__);

	return 0;

rem_del:
	cdev_del(&lcd->cdev);

	spin_lock(&hd44780_list_lock);
	list_del(&lcd->list);
	spin_unlock(&hd44780_list_lock);
rem_device:
	device_destroy(hd44780_class, lcd->dev_number);	
lcd_aloccate_failed:
	return -1;
}

static struct hd44780 * get_hd44780_by_i2c_client(struct i2c_client *i2c_client)
{
	struct hd44780 *lcd;

	spin_lock(&hd44780_list_lock);
	list_for_each_entry(lcd, &hd44780_list, list) {
		if (lcd->i2c_client->addr == i2c_client->addr) {
			spin_unlock(&hd44780_list_lock);
			return lcd;
		}
	}
	spin_unlock(&hd44780_list_lock);

	return NULL;
}


static int hd44780_remove(struct i2c_client *client)
{
	struct hd44780 *lcd;
	lcd = get_hd44780_by_i2c_client(client);

	spin_lock(&hd44780_list_lock);
	list_del(&lcd->list);
	spin_unlock(&hd44780_list_lock);
	device_destroy(hd44780_class, lcd->dev_number);
	cdev_del(&lcd->cdev);
	unregister_chrdev_region(lcd->dev_number, 1);
	
	pr_info("%s: Removed lcd_i2c%d device", __func__, lcd->i2c_client->adapter->nr); 
	
	return 0;
}

static const struct of_device_id hd44780_of_match[] = {
     {
        .compatible = "trieuhuynh, lcd_i2c0",     
     },
	 {},
};
MODULE_DEVICE_TABLE(of, hd44780_of_match);


static struct i2c_device_id hd44780_id_match[] = {
	{"lcd_i2c1", 0},
	{ },
};
MODULE_DEVICE_TABLE(i2c, hd44780_id_match);

static struct i2c_driver hd44780_driver = {
	.probe = hd44780_probe,
	.remove = hd44780_remove,
	.id_table = hd44780_id_match,
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(hd44780_of_match),
	},
};


static int __init hd44780_driver_init(void)
{
	int ret;

	ret = alloc_chrdev_region(&dev_no, 0, NUM_DEVICES, DRIVER_NAME);
	if (ret) {
		pr_err("%s: Error in allocation of device file number\n", __func__);
		goto ClassError;
	}

	// Device class creation 
	if((hd44780_class = class_create(THIS_MODULE, CLASS_NAME)) == NULL){
			pr_err("%s: Device class cannot be created\n", __func__);
			
			goto FileError;
	}

	ret = i2c_add_driver(&hd44780_driver);
	if (ret) {
		pr_err("Can't register I2C driver %s\n", hd44780_driver.driver.name);
		goto FileError;
	}

	pr_info("LCD_I2C driver module is loaded.\n");

	return 0;

FileError:
	class_destroy(hd44780_class);
ClassError:
	unregister_chrdev_region(dev_no, NUM_DEVICES);

	return -1;
}

static void __exit hd44780_driver_exit(void)
{
	i2c_del_driver(&hd44780_driver);
	class_destroy(hd44780_class);
	unregister_chrdev_region(dev_no, NUM_DEVICES);
	pr_info("LCD_I2C driver module is unloaded.\n");
}

module_init(hd44780_driver_init);
module_exit(hd44780_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Driver for LCD - I2C <ThesisLDD Project>");