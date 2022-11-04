/*
 * PWM.h
 *
 *  Created on: Jun 10, 2020
 *      Author: Nhan
 */

#ifndef PWM_PWM_H_
#define PWM_PWM_H_
#include "main.h"

// PWM Setup
#define PWM_GPIO_PORT         GPIOB
#define PWM_GPIO_CLK          RCC_AHB1Periph_GPIOB
#define PWM_AF                GPIO_AF_TIM3
// Left Motor Channels 
#define PWML_PIN              GPIO_Pin_4
#define PWML_SOURCE           GPIO_PinSource4
#define DIRL_PIN              GPIO_Pin_6
#define LEFT_MOTOR						0
// Right Motor Channels 
#define PWMR_PIN              GPIO_Pin_5
#define PWMR_SOURCE           GPIO_PinSource5
#define DIRR_PIN              GPIO_Pin_7
#define RIGHT_MOTOR						1
// Lifting Motor Channels 

#define PWMLIFT_PIN           GPIO_Pin_0
#define PWMLIFT_SOURCE        GPIO_PinSource0
#define DIRLIFT_PIN           GPIO_Pin_1
#define LIFT_MOTOR						2

// Constant
#define FOWARD								1
#define RESERVE								0

void PWM_Init(void);
void PWM_configOUT(TIM_TypeDef* TIMx,float freq);
void PWM_Pin(void);
void Run_Motor(uint8_t motor,float duty);
#endif /* PWM_PWM_H_ */
