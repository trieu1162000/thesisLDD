#include "I2C.h"
#include "LCD_2004.h"

static uint8_t u8LCD_Buff[8];//bo nho dem luu lai toan bo
static uint8_t u8LcdTmp;

#define	MODE_4_BIT		0x28
#define	CLR_SCR			0x01
#define	DISP_ON			0x0C
#define	CURSOR_ON		0x0E
#define	CURSOR_HOME		0x80

static void LCD_Write_4bit(uint8_t u8Data);
static void LCD_FlushVal(void);
static void LCD_WriteCmd(uint8_t u8Cmd);

void LCD_FlushVal(void)
{
	uint8_t i;
	
	for (i = 0; i < 8; ++i) {
		u8LcdTmp >>= 1;
		if(u8LCD_Buff[i]) {
			u8LcdTmp |= 0x80;
		}
	}
	I2C_Write(I2C_LCD_ADDR, &u8LcdTmp, 1);
}

void LCD_Init(void)
{
	TIM_TimeBaseInitTypeDef timInit;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);
	timInit.TIM_Prescaler = 84 - 1;
	timInit.TIM_Period = 0xFFFF;
	TIM_TimeBaseInit(TIM12, &timInit);
	TIM_Cmd(TIM12, ENABLE);
	
	uint8_t i;
	
	LCD_Delay_Ms(50);
	
	My_I2C_Init();
	
	for (i = 0; i < 8; ++i) {
		u8LCD_Buff[i] = 0;
	}
	
	LCD_FlushVal();
	
	u8LCD_Buff[LCD_RS] = 0;
	LCD_FlushVal();
	
	u8LCD_Buff[LCD_RW] = 0;
	LCD_FlushVal();
	
	LCD_Write_4bit(0x03);
	LCD_Delay_Ms(5);
	
	LCD_Write_4bit(0x03);
	LCD_Delay_Ms(1);
	
	LCD_Write_4bit(0x03);
	LCD_Delay_Ms(1);
	
	LCD_Write_4bit(MODE_4_BIT >> 4);
	LCD_Delay_Ms(1);
	
	LCD_WriteCmd(MODE_4_BIT);
	LCD_WriteCmd(DISP_ON);
	LCD_WriteCmd(CURSOR_ON);
	LCD_WriteCmd(CLR_SCR);
}

void LCD_Write_4bit(uint8_t u8Data)
{
	//4 bit can ghi chinh la 4 5 6 7
	//dau tien gan LCD_E=1
	//ghi du lieu
	//sau do gan LCD_E=0

	if(u8Data & 0x08) {
		u8LCD_Buff[LCD_D7] = 1;
	} else {
		u8LCD_Buff[LCD_D7] = 0;
	}
	if(u8Data & 0x04) {
		u8LCD_Buff[LCD_D6] = 1;
	} else {
		u8LCD_Buff[LCD_D6] = 0;
	}
	if(u8Data & 0x02) {
		u8LCD_Buff[LCD_D5] = 1;
	} else {
		u8LCD_Buff[LCD_D5] = 0;
	}
	if(u8Data & 0x01) {
		u8LCD_Buff[LCD_D4] = 1;
	} else {
		u8LCD_Buff[LCD_D4] = 0;
	}
	
	u8LCD_Buff[LCD_EN] = 1;
	LCD_FlushVal();	
	
	u8LCD_Buff[LCD_EN] = 0;
	LCD_FlushVal();
	
}

void LCD_WaitBusy(void)
{
	char temp;
	
	//dau tien ghi tat ca 4 bit thap bang 1
	u8LCD_Buff[LCD_D4] = 1;
	u8LCD_Buff[LCD_D5] = 1;
	u8LCD_Buff[LCD_D6] = 1;
	u8LCD_Buff[LCD_D7] = 1;
	LCD_FlushVal();
	
	u8LCD_Buff[LCD_RS] = 0;
	LCD_FlushVal();
	
	u8LCD_Buff[LCD_RW] = 1;
	LCD_FlushVal();
	
	do {
		u8LCD_Buff[LCD_EN] = 1;
		LCD_FlushVal();
		I2C_Read(I2C_LCD_ADDR + 1, &temp, 1);
		
		u8LCD_Buff[LCD_EN] = 0;
		LCD_FlushVal();
		u8LCD_Buff[LCD_EN] = 1;
		LCD_FlushVal();
		u8LCD_Buff[LCD_EN] = 0;
		LCD_FlushVal();
	} while (temp & 0x08);
}

void LCD_WriteCmd(uint8_t u8Cmd)
{
	
	LCD_WaitBusy();

	u8LCD_Buff[LCD_RS] = 0;
	LCD_FlushVal();
	
	u8LCD_Buff[LCD_RW] = 0;
	LCD_FlushVal();
	
	LCD_Write_4bit(u8Cmd >> 4);
	LCD_Write_4bit(u8Cmd);
}

void LCD_Write_Chr(char chr)
{
	
	LCD_WaitBusy();
	u8LCD_Buff[LCD_RS] = 1;
	LCD_FlushVal();
	u8LCD_Buff[LCD_RW] = 0;
	LCD_FlushVal();
	LCD_Write_4bit(chr >> 4);
	LCD_Write_4bit(chr);
	
}

void LCD_Puts(char *sz)
{
	
	while (1) {
		if (*sz) {
			LCD_Write_Chr(*sz++);
		} else {
			break;
		}
	}
}

void LCD_Clear(void)
{
	
	LCD_WriteCmd(CLR_SCR);
}

void LCD_NewLine(uint8_t line)
{
	if(line == 1)
	  LCD_WriteCmd(0x80 | 0x00);
	else if(line == 2)
	  LCD_WriteCmd(0x80 | 0x40);
	else if(line == 3)
	  LCD_WriteCmd(0x80 | 0x14);
	else 
		LCD_WriteCmd(0x80 | 0x54);
}

void LCD_BackLight(uint8_t u8BackLight)
{
	
	if(u8BackLight) {
		u8LCD_Buff[LCD_BL] = 1;
	} else {
		u8LCD_Buff[LCD_BL] = 0;
	}
	LCD_FlushVal();
}
