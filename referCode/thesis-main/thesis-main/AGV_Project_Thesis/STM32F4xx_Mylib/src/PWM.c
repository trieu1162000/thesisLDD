
#include "PWM.h"


void PWM_Init(void)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
		PWM_Pin();
		PWM_configOUT(TIM3,20000.0);
	}


void PWM_Pin(void)
{
	/*PWM1:PD14 (CH3-TIM4)	PWM2:PB0 (CH3-TIM3)*/
	GPIO_InitTypeDef pwm_Out; 
	pwm_Out.GPIO_Pin = PWML_PIN|PWMR_PIN|PWMLIFT_PIN;
	pwm_Out.GPIO_Mode = GPIO_Mode_AF;
	pwm_Out.GPIO_OType =GPIO_OType_PP;
	pwm_Out.GPIO_PuPd = GPIO_PuPd_UP;
	pwm_Out.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(PWM_GPIO_PORT, &pwm_Out);
	GPIO_PinAFConfig(PWM_GPIO_PORT, PWML_SOURCE, PWM_AF); 
	GPIO_PinAFConfig(PWM_GPIO_PORT, PWMR_SOURCE, PWM_AF); 
	GPIO_PinAFConfig(PWM_GPIO_PORT, PWMLIFT_SOURCE, PWM_AF); 
	/*DIR1:PD15 (OUTPUT) 	DIR2:PB1 (OUTPUT)*/
	GPIO_InitTypeDef pwm_Dir;
	//DIR1
	pwm_Dir.GPIO_Pin = DIRL_PIN|DIRR_PIN|DIRLIFT_PIN;
	pwm_Dir.GPIO_Mode = GPIO_Mode_OUT;
	pwm_Dir.GPIO_OType = GPIO_OType_PP;
	pwm_Dir.GPIO_PuPd = GPIO_PuPd_DOWN;
	pwm_Dir.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWM_GPIO_PORT,&pwm_Dir);
	//DIR2
}


void PWM_configOUT(TIM_TypeDef* TIMx,float freq)
{	
	uint32_t period = 0 ;
	uint32_t prescale = 0;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	prescale = RCC_Clocks.HCLK_Frequency/2/2000000 - 1;
	period = RCC_Clocks.HCLK_Frequency/2/((prescale + 1)*freq) - 1 ;
	//RCC_Clocks.HCLK_Frequency/2/((prescale + 1)*1000) - 1 ;

	TIM_TimeBaseInitTypeDef Timer_PWM;
	Timer_PWM.TIM_ClockDivision = 0;
	Timer_PWM.TIM_CounterMode = TIM_CounterMode_Up; 
	Timer_PWM.TIM_Prescaler = prescale;
	Timer_PWM.TIM_Period = period;
	TIM_TimeBaseInit(TIMx, &Timer_PWM);

	TIM_OCInitTypeDef PWM;
	PWM.TIM_OCMode = TIM_OCMode_PWM1;
	PWM.TIM_OCPolarity = TIM_OCPolarity_High;
	PWM.TIM_OutputState = TIM_OutputState_Enable;
	PWM.TIM_Pulse = 0;
	
	TIM_OC1Init(TIMx, &PWM); 
	TIM_OC2Init(TIMx, &PWM); 
	TIM_OC3Init(TIMx, &PWM);
	TIM_CtrlPWMOutputs(TIMx, ENABLE);
	TIM_Cmd(TIMx, ENABLE);

}

void Run_Motor(uint8_t motor,float duty)
{
	int dir;
	if (duty >= 100) duty = 100;
	else if (duty <= -100) duty = -100;
	if (duty >= 0 ) 
	{	
		dir = 0;
	}
	else
	{
	dir = 1;
	duty = 100 + duty;
	}
	if (motor == LEFT_MOTOR )
		{
			TIM_SetCompare1(TIM3,duty);
			GPIO_WriteBit(PWM_GPIO_PORT,DIRL_PIN,dir);	
		}
	else if (motor == RIGHT_MOTOR )
		{
			TIM_SetCompare2(TIM3,duty);
			GPIO_WriteBit(PWM_GPIO_PORT,DIRR_PIN,dir);
		}
	else if (motor == LIFT_MOTOR )
		{
			TIM_SetCompare3(TIM3,duty);
			GPIO_WriteBit(PWM_GPIO_PORT,DIRLIFT_PIN,dir);
		}	
	
}




