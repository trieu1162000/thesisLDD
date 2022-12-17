#include "rc522_api.h"

#define  N_1  1
#define  N_2  2
#define  N_3  3
#define  N_4  4
#define  N_NOP  255

#define  RELOAD_COUNT  0xfb

static IOCTLDescription_t ioctls[] = {
		{ .ioctlcode = CHANGE_PASSWD, .name = "CHANGE PASSWD", },
		{ .ioctlcode = CHANGE_BLOCK, .name = "CHANGE BLOCK", },
		{ .ioctlcode = READ_CARD, .name = "READ CARD" },
		{ .ioctlcode = WRITE_CARD, .name = "WRITE CARD" },
		{ .ioctlcode = CHANGE_KEY, .name = "CHANGE KEY" },
		{ .ioctlcode = GET_ID, .name = "GET ID" },
		{ .ioctlcode = BEEP, .name = "BEEP" },

};

typedef unsigned char uchar;
uchar NewKey[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x07,0x80,0x69,0x00,0x00,0x00,0x00,0x00,0x00};

static unsigned char Read_Data[16]={0x00};
static unsigned char read_data_buff[16];

static uchar PassWd[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static uchar WriteData[17];
static unsigned char RevBuffer[30];
static unsigned char MLastSelectedSnr[4];

uint blockAddr,operationcard;

struct spi_device *rc522_spi;

dev_t dev;
static struct cdev rfid_cdev;
static struct class *dev_class;
static struct device *dev_struct;

void delay_ms(uint tms)
{
	mdelay(tms);
}

void InitRc522(void)
{
	unsigned char a;
	PcdReset();
	a = ReadRawRC(TReloadRegL);
	/* For debug purpose */
	if(a != 30)
		pr_err("%s: RC522 not found. - %d\n", __func__, a);
	else
		pr_info("%s: RC522 exist.\n", __func__);
	PcdAntennaOff();  
	PcdAntennaOn();
	M500PcdConfigISOType( 'A' );
}

static char rc522_loop_work(uint opnd)
{
	char *pdata = read_data_buff;
	char status;

	PcdReset();
	status=PcdRequest(PICC_REQIDL,&RevBuffer[0]);
	if(status!=MI_OK)
	{
		pr_err("%s: search card: no card.\n", __func__);
		return -EFAULT;
	}
	status=PcdAnticoll(&RevBuffer[2]);
	if(status!=MI_OK)
	{
		pr_err("%s: get card nu: no number.\n", __func__);
		return -EFAULT;
	} 
	memcpy(MLastSelectedSnr,&RevBuffer[2],4);

	status=PcdSelect(MLastSelectedSnr);
	if(status!=MI_OK)
	{
		pr_err("%s: select card: no card.\n", __func__);
		return -EFAULT;            
	}
	if (opnd == GET_ID) {
		PcdHalt();	
		return 0;	
	} else if (opnd == READ_CARD) {
		status=PcdAuthState(PICC_AUTHENT1A,blockAddr,PassWd,MLastSelectedSnr);
		if(status!=MI_OK)
		{
			pr_err("%s: read authorize card err.\n", __func__);
			return -EFAULT;
		}
		status=PcdRead(blockAddr,Read_Data);
		if(status!=MI_OK)
		{
			pr_err("%s: read card err.\n", __func__);
			return -EFAULT;
		} else {
			int i;
			memcpy(pdata, Read_Data, sizeof(Read_Data));
			pr_info("%s: read block %d info:", __func__, blockAddr);
			for(i = 0; i < 16; i++) {
				pr_info("%2.2X",pdata[i]);
			}
			pr_info("\n");
		}
	} else if (opnd == CHANGE_KEY) {
		status=PcdAuthState(PICC_AUTHENT1A,blockAddr,PassWd,MLastSelectedSnr);
		if(status!=MI_OK)
		{
			pr_err("%s: card authorize err.", __func__);
			return -EFAULT;
		}
		status=PcdWrite(blockAddr,&NewKey[0]);
		if(status!=MI_OK)
		{
			pr_err("%s: change password err.", __func__);
			return -EFAULT;
		} else
			pr_info("%s: set password success.", __func__);
	} else if (opnd == WRITE_CARD) {
		status=PcdAuthState(PICC_AUTHENT1A,blockAddr,PassWd,MLastSelectedSnr);
		if(status!=MI_OK)
		{
			pr_err("%s: write authrioze err.\n", __func__);
			return -EFAULT;
		}
		status=PcdWrite(blockAddr,&WriteData[0]);
		if(status!=MI_OK)
		{
			pr_err("%s: write data err.\n", __func__);
			return -EFAULT;
		} else {
			pr_info("%s: write data to block %d sucess\n", __func__, blockAddr);
		}
	}
	PcdHalt();
	return 0;
}



static int rc522_open(struct inode *inode,struct file *filp)
{
	InitRc522();
	// pr_info("Open rfid_rc522 driver successfully.\n");
	return 0;
}

static ssize_t rc522_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	/*PcdReset();*/
	operationcard = READ_CARD;
	if(rc522_loop_work(operationcard))
		return 0;
	pr_info("%s: card info:%2.2X\n", __func__, Read_Data[0]);
	if (copy_to_user(buf, read_data_buff, sizeof(read_data_buff))) {
		pr_err("%s: copy card number to userspace err.\n", __func__);
		return 0;
	}
	return sizeof(read_data_buff);
}

static ssize_t rc522_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	if (blockAddr == 0) {
		pr_info("%s: block[0] is reserveed, can not write.\n", __func__);
		return 0;
	}
	if (blockAddr < 0 || blockAddr > 63) {
		pr_info("%s: block[%d] unreachable, please set the write block first", __func__, blockAddr);
		return -0;
	} 
	if ((blockAddr % 4) == 3) {
		pr_info("%s: block[%d] is key block, not data block\n", __func__, blockAddr);
		return -0;
	}
	memset(WriteData, 0, sizeof(WriteData));
	if (copy_from_user(WriteData, (char *)buf, count)) {
		pr_err("%s, [line %d] copy from user err.\n", __FILE__, __LINE__);
		return 0;
	}
	/*PcdReset();*/
	operationcard =  WRITE_CARD;
	if(rc522_loop_work(operationcard))
		return -EFAULT;
	return 0;
}

static int rc522_release(struct inode *inode,struct file *filp)
{
	// pr_info("Close rfid_rc522 driver successfully.\n");
	return 0;
}

static long rc522_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info( "%s: CMD = 0x%x\n", __func__, cmd);
	switch(cmd) {
	case CHANGE_PASSWD:
		operationcard = CHANGE_PASSWD;
		if (copy_from_user(PassWd, (char *)arg, sizeof(PassWd))) {
			pr_err("%s: change pass word err", __func__);
			return -EFAULT;
		}
		break;
	case CHANGE_BLOCK:
		if (arg < 0 || arg > 63) {
			pr_err("%s: block number err %lu\n", __func__, arg);
			return -EFAULT;
		}
		blockAddr = (int)arg;
		pr_info( "%s: block = %d", __func__, blockAddr);
		break;
	case READ_CARD:
		break;
	case WRITE_CARD:
		break;
	case CHANGE_KEY:
		operationcard = CHANGE_KEY;
		break;
	case GET_ID:
		operationcard =  GET_ID;
		if(!rc522_loop_work(operationcard)){
			if (copy_to_user((char *)arg, MLastSelectedSnr,4)) {
				pr_err("%s, [line %d] copy to user err.\n", __FILE__, __LINE__);
				return -EFAULT;
			}
		}
		else
			return -EFAULT;
		break;
	default:
		break;
	}
	return 0;
}


static int rc522_remove(struct spi_device *spi)
{

	return 0;
}
static int rc522_probe(struct spi_device *spi)
{
	pr_info( "%s: SPI number = %d", __func__, spi->controller->bus_num);
	blockAddr = 1;
	rc522_spi = spi;
	return 0;
};

static struct of_device_id spi_rfid_dt_ids[] = { // DTS compatible //
		{ .compatible = "rfid_rc522" },
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
		.unlocked_ioctl = rc522_ioctl,
};

static int rfid_rc522_init(void)
{
	int i;
	
	pr_info("RFID_RC522 module init.\n");

	if((alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) < 0) {
		pr_err("%s: Error in allocation of device file number\n", __func__);
		return -1;
	}
	pr_info("%s: Device number with major: %d, minor: %d was registered.\n", __func__, MAJOR(dev), MINOR(dev));

	cdev_init(&rfid_cdev,&rc522_fops);

	if((cdev_add(&rfid_cdev, dev, 1)) < 0) {
		pr_err("%s: Cdev add failed.\n", __func__);
		goto rem_cdev;
	}

	if((dev_class = class_create(THIS_MODULE,CLASS_NAME)) == NULL) {
		pr_err("%s: Device class cannot be created\n", __func__);
		goto rem_class;
	}

	if((dev_struct = device_create(dev_class,NULL,dev,NULL,DEVICE_NAME)) == NULL) {
		pr_err("%s: Can not create device.\n", __func__);
		goto rem_device;
	}

	if(spi_register_driver(&rc522_driver) < 0) {
		pr_err("%s: SPI driver register failed.\n", __func__);
		goto rem_device;
	}
	for(i=0; i < (sizeof(ioctls) / sizeof(IOCTLDescription_t)); i++)
		pr_info("IOCTL Codes:\t%s=0x%02X\n", ioctls[i].name, ioctls[i].ioctlcode);
	
	return 0;

rem_device:
	device_destroy(dev_class,dev);
rem_class:
	class_destroy(dev_class);
rem_cdev:
	cdev_del(&rfid_cdev);
	unregister_chrdev_region(dev,1);
	return -1;
}

static void rfid_rc522_exit(void)
{
	spi_unregister_driver(&rc522_driver);
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&rfid_cdev);
	unregister_chrdev_region(dev,1);
	pr_info("rfid_rc522 driver module is unloaded.\n");
}

/* Meta information for alllowing correct loading of modules through distros*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("Driver for RFID RC522 <ThesisLDD Project>");


module_init(rfid_rc522_init);
module_exit(rfid_rc522_exit);

