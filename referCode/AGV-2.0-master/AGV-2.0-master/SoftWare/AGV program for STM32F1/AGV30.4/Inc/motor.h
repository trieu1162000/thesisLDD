//-----------------------------------------------------BKHN------------------------------------------------//
//---------------------------------------------------AGV_Ver_2---------------------------------------------//
//---------------------------------------------------Lam_Tran----------------------------------------------//
#include "stm32f1xx_hal.h"
extern TIM_HandleTypeDef htim3;
void motorLeftForward(uint32_t speed);
void motorLeftBackward(uint32_t speed);
void motorRightForward(uint32_t speed);
void motorRightBackward(uint32_t speed);
void motorStop();