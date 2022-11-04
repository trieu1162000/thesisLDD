
#ifndef UART_DMA_H_
#define UART_DMA_H_

#include "stm32f4xx.h"
#include "system_timetick.h"
#include "stm32f4xx_it.h"
#include "stdio.h"
#include "stdlib.h"
extern float kp,ki,kd;
void UART_Init(void);
void UART_Print( const char *pcString );
void UART_PrintNumber(const char *templateC,float a);
void UART_SendData( uint8_t *s_data, uint16_t size ) ;
uint8_t get_status(void);
int get_velF(void);
uint16_t  CRC_Cal(uint8_t messageArray[],uint8_t size);
#endif /* UART_DMA_H_ */
