#include "ADC_Voltage.h"
 

void ADC_Config(void)
{
  GPIO_InitTypeDef            GPIO_InitStructure;
    ADC_InitTypeDef             ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);

    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin     =   GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    ADC_InitStructure.ADC_Resolution            = ADC_Resolution_8b;
    ADC_Init(ADC1,&ADC_InitStructure);

    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
    ADC_CommonInitStructure.ADC_Mode            = ADC_Mode_Independent;
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_Cmd(ADC1,ENABLE);
}


uint8_t Read_ADC(void)
{
    //A0 Bacagindaki ADC verisini okur

    ADC_RegularChannelConfig(ADC1,ADC_Channel_3,1,ADC_SampleTime_56Cycles);
    ADC_SoftwareStartConv(ADC1);
    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);
    return ADC_GetConversionValue(ADC1);
}
