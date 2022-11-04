/*
 * Encoder.h
 *
 *  Created on: Jun 21, 2020
 *      Author: Nhan
 */

#ifndef ENCODER_ENCODER_H_
#define ENCODER_ENCODER_H_

#include "main.h"

#define ENCL_TIM               	TIM4
#define ENCL_AF                	GPIO_AF_TIM4

#define ENCLA_GPIO_PORT         GPIOD
#define ENCLA_PIN               GPIO_Pin_12
#define ENCLA_SOURCE            GPIO_PinSource12

#define ENCLB_GPIO_PORT         GPIOD
#define ENCLB_PIN               GPIO_Pin_13
#define ENCLB_SOURCE            GPIO_PinSource13

// Right Motor Channels
#define ENCR_TIM               	TIM5

#define ENCR_AF                	GPIO_AF_TIM5
#define ENCRA_GPIO_PORT         GPIOA
#define ENCRA_PIN               GPIO_Pin_0
#define ENCRA_SOURCE            GPIO_PinSource0
#define ENCRB_GPIO_PORT         GPIOA
#define ENCRB_PIN               GPIO_Pin_1
#define ENCRB_SOURCE            GPIO_PinSource1

// Lifting Motor Channels
#define ENCLIFT_TIM             TIM8
#define ENCLIFT_AF              GPIO_AF_TIM8

#define ENCLIFTA_GPIO_PORT      GPIOC
#define ENCLIFTA_PIN            GPIO_Pin_6
#define ENCLIFTA_SOURCE         GPIO_PinSource6

#define ENCLIFTB_GPIO_PORT      GPIOC
#define ENCLIFTB_PIN            GPIO_Pin_7
#define ENCLIFTB_SOURCE         GPIO_PinSource7



void ENC_Init(void);
void ENC_config(TIM_TypeDef* TIMx,GPIO_TypeDef* A_GPIOx,GPIO_TypeDef* B_GPIOx,uint16_t A_GPIO_Pin_x,uint16_t B_GPIO_Pin_x );
float ENC_Position(TIM_TypeDef* TIMx,uint16_t N);
float ENC_Velocity(TIM_TypeDef* TIMx,uint16_t N);

float Position_Lift(TIM_TypeDef* TIMx,uint16_t N);
float LowpassFilter(float V,int dir);
#endif /* ENCODER_ENCODER_H_ */
