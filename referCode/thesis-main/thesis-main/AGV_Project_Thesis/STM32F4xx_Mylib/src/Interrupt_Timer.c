
#include "Interrupt_Timer.h"

void TIM6_INT_Init(void)
{
	// Initialize the interrupt
	__NVIC_EnableIRQ(TIM6_DAC_IRQn);
 
	EXTI->IMR |= (1<<0);
	EXTI->EMR |= (1<<0);
	EXTI->FTSR &= ~(1<<0);
	EXTI->RTSR |= (1<<0);
	// Peripheral clock
	RCC->APB1ENR |= (1<<4);	// Enables APB1 bus to use Timer 6
	// 2. Init timer
	TIM6->CR1 |= (1<<0);	// CEN: Enables counter
	TIM6->CR1 &= ~(1<<1);	// UDIS: Update event enabled
	TIM6->CR1 |= (1<<2);	// URS: Update request source enabled for overflow/underflow only
	TIM6->CR1 &= ~(1<<3);	// OPM: One Pulse Mode. Counter continues counting.
	TIM6->CR1 &= ~(1<<7);	// Auto reload preload enabled
	TIM6->DIER |= (1<<0);	// UIE: Update interrupt enabled
	TIM6->EGR |= (1<<0);	// UG: Update generation. Re-initializes the counter and updates registers
	//TIM6->CNT = 399;		// Counter goes up to 250 to have 1s timer ???
	TIM6->PSC = 8399;		// Sets prescaler to 64000. Timer clock is now 84MHz/(4200*10000)= 2Hz
	TIM6->ARR = 99;		// Counter goes up to 250 to have 1s timer399
}


void TIM2_INT_Init()
{
    // Enable clock for TIM2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // TIM2 initialization overflow every 500ms
    // TIM2 by default has clock of 84MHz
    // Here, we must set value of prescaler and period,
    // so update event is 0.5Hz or 500ms
    // Update Event (Hz) = timer_clock / ((TIM_Prescaler + 1) * 
    // (TIM_Period + 1))
    // Update Event (Hz) = 84MHz / ((4199 + 1) * (9999 + 1)) = 2 Hz
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 8399;
    TIM_TimeBaseInitStruct.TIM_Period = 999;   //999 la 100ms             //9999 la 1s
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    // TIM2 initialize
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    // Enable TIM2 interrupt
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    // Start TIM2
    TIM_Cmd(TIM2, ENABLE);

    // Nested vectored interrupt settings
    // TIM2 interrupt is most important (PreemptionPriority and 
    // SubPriority = 0)
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}





