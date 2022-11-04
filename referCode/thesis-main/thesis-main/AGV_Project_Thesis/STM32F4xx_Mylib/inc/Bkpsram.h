
#ifndef TM_BKPSRAM_H
#define TM_BKPSRAM_H 110

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_pwr.h"


#define TM_BKPSRAM_GetMemorySize()			(0x00001000)


void BKPSRAM_Init(void);


#define TM_BKPSRAM_Write8(address, value)	(*(__IO uint8_t *) (BKPSRAM_BASE + (address)) = (value))

#define TM_BKPSRAM_Read8(address)			(*(__IO uint8_t *) (BKPSRAM_BASE + address))

#define TM_BKPSRAM_Write16(address, value)	(*(__IO uint16_t *) (BKPSRAM_BASE + (address)) = (value))

#define TM_BKPSRAM_Read16(address)			(*(__IO uint16_t *) (BKPSRAM_BASE + address))

#define TM_BKPSRAM_Write32(address, value)	(*(__IO uint32_t *) (BKPSRAM_BASE + (address)) = (value))

#define TM_BKPSRAM_Read32(address)			(*(__IO uint32_t *) (BKPSRAM_BASE + address))

#define TM_BKPSRAM_WriteFloat(address, value)	(*(__IO float *) (BKPSRAM_BASE + (address)) = (value))

#define TM_BKPSRAM_ReadFloat(address)			(*(__IO float *) (BKPSRAM_BASE + address))
#endif

