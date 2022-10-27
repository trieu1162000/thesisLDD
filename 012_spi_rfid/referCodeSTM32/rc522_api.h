#ifndef __rc522_h__
#define __rc522_h__

#include <linux/workqueue.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/atomic.h>
#include <linux/mutex.h>
#include <linux/spi/spi.h>
#include <linux/cdev.h>
#include <linux/gpio.h> 

// SPI CS define
#define GPIO_5		  5

#define SET_SPI_CS  do{gpio_set_value(GPIO_5, 1);}while(0)
#define CLR_SPI_CS  do{gpio_set_value(GPIO_5, 0);}while(0)

// Status enumeration, Used with most functions
#define MI_OK											0
#define MI_NOTAGERR										(-1)
#define MI_ERR										    (-2)

// MFRC522 Commands
#define PCD_IDLE										0x00		// NO action; Cancel the current command
#define PCD_AUTHENT										0x0E  	// Authentication Key
#define PCD_RECEIVE										0x08   	// Receive Data
#define PCD_TRANSMIT									0x04   	// Transmit data
#define PCD_TRANSCEIVE								    0x0C   	// Transmit and receive data,
#define PCD_RESETPHASE								    0x0F   	// Reset
#define PCD_CALCCRC										0x03   	// CRC Calculate

// Mifare_One card command word
#define PICC_REQIDL										0x26   	// find the antenna area does not enter hibernation
#define PICC_REQALL										0x52   	// find all the cards antenna area
#define PICC_ANTICOLL									0x93   	// anti-collision
#define PICC_SELECTTAG								    0x93   	// election card
#define PICC_AUTHENT1A								    0x60   	// authentication key A
#define PICC_AUTHENT1B								    0x61   	// authentication key B
#define PICC_READ										0x30   	// Read Block
#define PICC_WRITE										0xA0   	// write block
#define PICC_DECREMENT								    0xC0   	// debit
#define PICC_INCREMENT								    0xC1   	// recharge
#define PICC_RESTORE									0xC2   	// transfer block data to the buffer
#define PICC_TRANSFER									0xB0   	// save the data in the buffer
#define PICC_HALT										0x50   	// Sleep

// MFRC522 Registers
// Page 0: Command and Status
#define MFRC522_REG_RESERVED00				            0x00
#define MFRC522_REG_COMMAND						        0x01
#define MFRC522_REG_COMM_IE_N					        0x02
#define MFRC522_REG_DIV1_EN						        0x03
#define MFRC522_REG_COMM_IRQ					        0x04
#define MFRC522_REG_DIV_IRQ						        0x05
#define MFRC522_REG_ERROR							    0x06
#define MFRC522_REG_STATUS1						        0x07
#define MFRC522_REG_STATUS2						        0x08
#define MFRC522_REG_FIFO_DATA					        0x09
#define MFRC522_REG_FIFO_LEVEL				            0x0A
#define MFRC522_REG_WATER_LEVEL				            0x0B
#define MFRC522_REG_CONTROL						        0x0C
#define MFRC522_REG_BIT_FRAMING				            0x0D
#define MFRC522_REG_COLL							    0x0E
#define MFRC522_REG_RESERVED01				            0x0F
// Page 1: Command
#define MFRC522_REG_RESERVED10				            0x10
#define MFRC522_REG_MODE							    0x11
#define MFRC522_REG_TX_MODE						        0x12
#define MFRC522_REG_RX_MODE						        0x13
#define MFRC522_REG_TX_CONTROL				            0x14
#define MFRC522_REG_TX_AUTO						        0x15
#define MFRC522_REG_TX_SELL						        0x16
#define MFRC522_REG_RX_SELL						        0x17
#define MFRC522_REG_RX_THRESHOLD			            0x18
#define MFRC522_REG_DEMOD							    0x19
#define MFRC522_REG_RESERVED11				            0x1A
#define MFRC522_REG_RESERVED12				            0x1B
#define MFRC522_REG_MIFARE						        0x1C
#define MFRC522_REG_RESERVED13				            0x1D
#define MFRC522_REG_RESERVED14				            0x1E
#define MFRC522_REG_SERIALSPEED				            0x1F
// Page 2: CFG
#define MFRC522_REG_RESERVED20				            0x20
#define MFRC522_REG_CRC_RESULT_M			            0x21
#define MFRC522_REG_CRC_RESULT_L			            0x22
#define MFRC522_REG_RESERVED21				            0x23
#define MFRC522_REG_MOD_WIDTH					        0x24
#define MFRC522_REG_RESERVED22				            0x25
#define MFRC522_REG_RF_CFG						        0x26
#define MFRC522_REG_GS_N							    0x27
#define MFRC522_REG_CWGS_PREG					        0x28
#define MFRC522_REG__MODGS_PREG				            0x29
#define MFRC522_REG_T_MODE						        0x2A
#define MFRC522_REG_T_PRESCALER				            0x2B
#define MFRC522_REG_T_RELOAD_H				            0x2C
#define MFRC522_REG_T_RELOAD_L				            0x2D
#define MFRC522_REG_T_COUNTER_VALUE_H	                0x2E
#define MFRC522_REG_T_COUNTER_VALUE_L	                0x2F
// Page 3:TestRegister
#define MFRC522_REG_RESERVED30				            0x30
#define MFRC522_REG_TEST_SEL1					        0x31
#define MFRC522_REG_TEST_SEL2					        0x32
#define MFRC522_REG_TEST_PIN_EN				            0x33
#define MFRC522_REG_TEST_PIN_VALUE		                0x34
#define MFRC522_REG_TEST_BUS					        0x35
#define MFRC522_REG_AUTO_TEST					        0x36
#define MFRC522_REG_VERSION						        0x37
#define MFRC522_REG_ANALOG_TEST				            0x38
#define MFRC522_REG_TEST_ADC1					        0x39
#define MFRC522_REG_TEST_ADC2					        0x3A
#define MFRC522_REG_TEST_ADC0					        0x3B
#define MFRC522_REG_RESERVED31				            0x3C
#define MFRC522_REG_RESERVED32				            0x3D
#define MFRC522_REG_RESERVED33				            0x3E
#define MFRC522_REG_RESERVED34				            0x3F

#define MFRC522_DUMMY									0x00		// Dummy byte
#define MFRC522_MAX_LEN								    16			// Buf len byte

// #define RFID_IOCTL_BASE     0xCE
// #define CHANGE_PASSWD       _IOW(RFID_IOCTL_BASE, 0x01, char *)
// #define CHANGE_BLOCK        _IOW(RFID_IOCTL_BASE, (0x02 << 1), int)
// #define READ_CARD           _IOR(RFID_IOCTL_BASE, (0x03 << 1), char *)
// #define WRITE_CARD          _IOW(RFID_IOCTL_BASE, (0x04 << 1), char *)
// #define CHANGE_KEY          _IO(RFID_IOCTL_BASE, (0x05 << 1))
// #define GET_ID              _IOR(RFID_IOCTL_BASE, (0x06 << 1), char *)
// #define BEEP                _IO(RFID_IOCTL_BASE, (0x07 << 1))

#define CLASS_NAME "RC522"
#define DEVICE_NAME "rfid_rc522_dev"

// typedef struct ioctl_description {
//   uint32_t ioctlcode;
//   char	name[24];
// } IOCTLDescription_t;

extern struct spi_device *rc522_spi;
extern void MFRC522Init(void);
extern uint8_t MFRC522Check(uint8_t* id);

#endif
