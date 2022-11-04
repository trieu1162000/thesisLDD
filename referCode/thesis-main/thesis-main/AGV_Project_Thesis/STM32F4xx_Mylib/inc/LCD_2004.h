#ifndef I2C_LCD_H_
#define I2C_LCD_H_

#include "stm32f4xx.h"
#include "i2c.h"
#include <stdint.h>

#define I2C_LCD_ADDR 0x4E

extern void delay_ms(uint32_t u32DelayInMs);

#define LCD_Delay_Ms(u16DelayMs) delay_ms(u16DelayMs)

#define LCD_EN 2
#define LCD_RW 1
#define LCD_RS 0
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7
#define LCD_BL 3

void LCD_Init(void);
void LCD_Puts(char *szStr);
void LCD_Clear(void);
void LCD_NewLine(uint8_t line);
void LCD_BackLight(uint8_t u8BackLight);

#endif
