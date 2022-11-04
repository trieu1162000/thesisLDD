#ifndef I2C_H
#define I2C_H

#include "stm32f4xx.h"

#define SDA_0 GPIO_ResetBits(GPIOB, GPIO_Pin_10)
#define SDA_1 GPIO_SetBits(GPIOB, GPIO_Pin_10)
#define SCL_0 GPIO_ResetBits(GPIOB, GPIO_Pin_11)
#define SCL_1 GPIO_SetBits(GPIOB, GPIO_Pin_11)
#define SDA_VAL (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10))


void My_I2C_Init(void);
uint8_t I2C_Write(uint8_t Address, uint8_t *pData, uint8_t length);
uint8_t I2C_Read(uint8_t Address, uint8_t *pData, uint8_t length);

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t u8Data);
uint8_t i2c_read(uint8_t u8Ack);

void delay_ms(uint32_t u32DelayInMs);
void delay_us(uint32_t delay);

#endif
