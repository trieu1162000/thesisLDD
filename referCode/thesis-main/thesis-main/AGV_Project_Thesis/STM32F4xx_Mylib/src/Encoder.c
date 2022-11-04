/*
 * Encoder.c
 *
 *  Created on: Jun 21, 2020
 *      Author: Nhan
 */

#include "Encoder.h"

uint8_t Fs = 100;
// Left Motor Channels


void ENC_Init(void)
	{
	
		// Config ENC ENCLA: PD12 (CH1-TIM4) ENCLB: PD13 (CH2-TIM4)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	ENC_config(TIM4,ENCLA_GPIO_PORT,ENCLB_GPIO_PORT,ENCLA_PIN,ENCLB_PIN);
	GPIO_PinAFConfig(ENCLA_GPIO_PORT, ENCLA_SOURCE, ENCL_AF);  
	GPIO_PinAFConfig(ENCLB_GPIO_PORT, ENCLB_SOURCE, ENCL_AF);  
	TIM_SetCounter(ENCL_TIM ,30000);
	
		// Config ENC ENCRA: PA0 (CH1-TIM5) ENCRB: PA1 (CH2-TIM5)
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
	ENC_config(TIM5,ENCRA_GPIO_PORT,ENCRB_GPIO_PORT,ENCRA_PIN,ENCRB_PIN);
	GPIO_PinAFConfig(ENCRA_GPIO_PORT, ENCRA_SOURCE, ENCR_AF);  
	GPIO_PinAFConfig(ENCRB_GPIO_PORT, ENCRB_SOURCE, ENCR_AF);  
	TIM_SetCounter(ENCR_TIM ,30000);
		
		//// Config ENC ENCLIFTA: PA0 (CH1-TIM5) ENCLIFTB: PA1 (CH2-TIM5)
/*	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);
	ENC_config(TIM8,ENCLIFTA_GPIO_PORT,ENCLIFTB_GPIO_PORT,ENCLIFTA_PIN,ENCLIFTB_PIN);
	GPIO_PinAFConfig(ENCLIFTA_GPIO_PORT, ENCLIFTA_SOURCE, ENCLIFT_AF);  
	GPIO_PinAFConfig(ENCLIFTB_GPIO_PORT, ENCLIFTB_SOURCE, ENCLIFT_AF);  
	TIM_SetCounter(TIM8,0);
	*/
	
	GPIO_InitTypeDef GPIO_InitStructure;
	 /* TIM1 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	/* TIM1 channel1,2 configuration */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM8);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM8);

	/* Initialise encoder interface */
	TIM_EncoderInterfaceConfig(TIM8, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	
	/* TIM enable counter */
	TIM8->CNT = 0;
	TIM_Cmd(TIM8, ENABLE);
	
	
		
		
	}

void ENC_config(TIM_TypeDef* TIMx,GPIO_TypeDef* A_GPIOx,GPIO_TypeDef* B_GPIOx,uint16_t A_GPIO_Pin_x,uint16_t B_GPIO_Pin_x )
{
	GPIO_InitTypeDef enc;
	enc.GPIO_Pin = A_GPIO_Pin_x;
	enc.GPIO_Mode = GPIO_Mode_AF;
	enc.GPIO_PuPd = GPIO_PuPd_UP;
	enc.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(A_GPIOx, &enc);
	
	enc.GPIO_Pin = B_GPIO_Pin_x;
	GPIO_Init(B_GPIOx, &enc);
	TIM_TimeBaseInitTypeDef tim_enc;
	tim_enc.TIM_ClockDivision = TIM_CKD_DIV1;
	tim_enc.TIM_CounterMode = TIM_CounterMode_Up;
	tim_enc.TIM_Prescaler = 0;
	tim_enc.TIM_Period = 65535;

	TIM_TimeBaseInit(TIMx, &tim_enc);
	TIM_SetCounter(TIMx, 0);
	TIM_EncoderInterfaceConfig(TIMx, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	TIM_Cmd(TIMx, ENABLE);
}

float ENC_Velocity(TIM_TypeDef* TIMx,uint16_t N)
{

	int16_t xungENC ;
	float vel;
	xungENC = TIM_GetCounter(TIMx);
	vel = (Fs*(xungENC-30000)*8.5*3.14)/(N*4);
	TIM_SetCounter(TIMx,30000);
	return vel;
}

float ENC_Position(TIM_TypeDef* TIMx,uint16_t N)
{

	int16_t xung_ENC ;
	float current_pos;
	xung_ENC = TIM_GetCounter(TIMx);
	current_pos = (xung_ENC-30000)*8.5*3.14/(N*4);    
	TIM_SetCounter(TIMx,30000) ;
	return current_pos;
}

float Position_Lift(TIM_TypeDef* TIMx,uint16_t N)
{

	int16_t xung_ENC ;
	float current_pos;
	xung_ENC = TIM_GetCounter(TIMx);
	current_pos  = (float)(xung_ENC)*4/(N*4);     // 1 vong = 4mm
  TIM_SetCounter(TIMx,0);
	return current_pos;
}

//float V_1 = 0,tempV1 = 0;
//float to1 = 0.25,Ts = 0.01;
//float v_preFilter1R = 0, v_Filter1R,tempR = 0;



float V1_L = 0,tempV1_L = 0;
float V1_R = 0,tempV1_R = 0;
float to1 = 0.25,Ts = 0.01;
float v_preFilter1_R = 0, v_Filter1_R,temp_R = 0;
float v_preFilter1_L = 0, v_Filter1_L,temp_L = 0;
float LowpassFilter(float V,int dir) // ham lay mau
{	

	if(dir == LEFT_MOTOR)
	{
		V1_L = (1 - exp(-1/to1*Ts))*v_preFilter1_L + exp(-1/to1*Ts)*v_Filter1_L ;
		temp_L = v_Filter1_L;
		v_Filter1_L = V1_L;
		v_preFilter1_L = V;
		return V1_L;
	}
	else if (dir == RIGHT_MOTOR)
	{
		V1_R = (1 - exp(-1/to1*Ts))*v_preFilter1_R + exp(-1/to1*Ts)*v_Filter1_R ;
		temp_R = v_Filter1_R;
		v_Filter1_R = V1_R;
		v_preFilter1_R = V;
		return V1_R;
	}
}
