/*
 * Encoder.c
 *
 *  Created on: Jun 21, 2020
 *      Author: Nhan
 */

#include "QTR_5RC.h"

#define sensorCount 5
#define maxValue 1000
uint32_t tick = 0;
uint16_t sensorValues[sensorCount];
uint16_t sensorPins[sensorCount] = {GPIO_Pin_0,GPIO_Pin_1,GPIO_Pin_2,GPIO_Pin_3,GPIO_Pin_7};
char calibInitialized = '0'; // Calibration status
uint16_t minCalibValues[sensorCount]; // Lowest readings seen during calibration
uint16_t maxCalibValues[sensorCount]; // Highest readings seen during calibration
uint16_t position;

void TIM7_INT_Init(void)
{
    // Enable clock for TIM
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct; 
    TIM_TimeBaseInitStruct.TIM_Prescaler = 167 ;
    TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF ;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    // Nested vectored interrupt settings
    // TIM2 interrupt is most important (PreemptionPriority and 
    // SubPriority = 0)
    //NVIC_InitTypeDef NVIC_InitStruct;
    //NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
    //NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    //NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    //NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    //NVIC_Init(&NVIC_InitStruct);
	// TIM2 initialize
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);
    // Enable TIM2 interrupt
    //TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    // Start TIM2
		//TIM_Cmd(TIM7, ENABLE);
}	

void QTRPins_OUTPUT_Mode(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 PA4 */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
}
void QTRPins_INPUT_Mode(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 PA4 */
  GPIO_InitStruct.GPIO_Pin =GPIO_Pin_0| GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
}

//----------QTR Read Function-------------------------------
// Reads the raw values of the sensors
void QTRSensorsRead(uint16_t *_sensorValues)
{
	uint16_t __sensorValues[sensorCount];
	uint16_t __sensorValues1[sensorCount];
	for(int i = 0; i < sensorCount; i++)
	{
		__sensorValues[i] = maxValue; // maxValue = 1000
		__sensorValues1[i] = maxValue;
	// make sensor line an output
	}
	QTRPins_OUTPUT_Mode();
	// drive sensor line high
	GPIO_WriteBit(GPIOD, GPIO_Pin_0 |GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_7, Bit_SET);
	// charge lines for 10 us
	// HAL_Delay(1);
	// disable interrupts so we can switch all the pins as close to the same
  // time as possible
	//HAL_TIM_Base_Stop_IT(&htim5);
	// Enable TIM2 interrupt
	
    //TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
		//TIM_Cmd(TIM7, DISABLE);
	// record start time before the first sensor is switched to input
  // (similarly, time is checked before the first sensor is read in the
  // loop below)
	//uint32_t startTime = tick;
	uint16_t time = 0;
	// make sensor line an input (should also ensure pull-up is disabled)
	QTRPins_INPUT_Mode();
	TIM7->CNT = 0;
	// Enable TIM2 interrupt
    //TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
TIM_Cmd(TIM7, ENABLE);
	while(time < maxValue)
	{
		// disable interrupts so we can read all the pins as close to the same
    // time as possible &&  (GPIO_ReadInputDataBit(GPIOD, sensorPins[i]) == Bit_RESET)
		
		time = TIM7->CNT;
		
			for (int i = 0; i < sensorCount; i++)
    {
			if ((GPIO_ReadInputDataBit(GPIOD, sensorPins[i]) == Bit_RESET) && (time < __sensorValues[i]))
			{
				// record the first time the line reads low
				__sensorValues[i] = time;
				_sensorValues[i] = __sensorValues[i];
			}
    }
			
    
		
		
		
		
		
		//TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM7, ENABLE);
	}
}


//-----------QTR Read Calibrated Values Function------------
//Reads the sensors and provides calibrated values between 0 and 500.
uint16_t QTRSensorsReadCalibrated(uint16_t *_sensorValues)
{
	uint16_t __sensorValue[sensorCount];
	uint16_t calibmin=1000, calibmax=0;
	// if not calibrated, do nothing
	// read the needed values
	QTRSensorsRead(__sensorValue);
	for(int i = 0; i < sensorCount; i++)
	{
		if(__sensorValue[i]>calibmax) calibmax = __sensorValue[i];
		if(__sensorValue[i]<calibmin) calibmin = __sensorValue[i];
		
	}
	uint16_t delta = calibmax - calibmin;
	
	for(int i = 0; i < sensorCount; i++)
	{
		if(__sensorValue[i]> calibmax -delta/2 ) _sensorValues[i] = 1;
		else _sensorValues[i] = 0;
	}
		
	//return (_sensorValues[0] * 0 + _sensorValues[1] * 0 + _sensorValues[2] * 1000 + _sensorValues[3] * 2000 + _sensorValues[4] * 2000)/(_sensorValues[1] + _sensorValues[2] + _sensorValues[3] + _sensorValues[4]+ _sensorValues[0]);
		return (_sensorValues[0] * 2000 + _sensorValues[1] * 2000 + _sensorValues[2] * 1000 + _sensorValues[3] * 0 + _sensorValues[4] * 0)/(_sensorValues[1] + _sensorValues[2] + _sensorValues[3] + _sensorValues[4]+ _sensorValues[0]);
}
