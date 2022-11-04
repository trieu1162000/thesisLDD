//Line Follower Project
//Using 3 Pololu QTR-3RC reflectance sensor arrays
//Sequence for reading a sensor is:
//Set the I / O line to an output and drive it high.
//Wait several microseconds to give the 2.2 nF capacitor node time to reach 5 V.
//Make the I / O line an input(high impedance).
//Measure the time for the capacitor node voltage to decay by waiting for the I / O line to go low.
//Engines are steered with PWM using Pololu TB6612FNG Dual Motor Driver Carrier (with a 20kHz Timer)
//


#define true 1
#define false 0
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"

//PortE used for handling all the sensors
GPIO_InitTypeDef PortE;

//Capacitor charging Flag
int charging = 1;
//Counter used for measuring sensor discharge time
int counter = 0;
uint8_t deb1, deb2, deb3, deb4, deb5, deb6, deb7, deb8, deb9;
//Currently selected sensor
uint8_t selectedSensor = 0;
//Array holding sensor values (0 = white, 1 = black)
uint8_t sensorArray[9] = { 0 };

enum EngineSteering { Forward, Backward, Stop, REST };
//Enum used for engine choice
enum { Left = 4, Right = 5 };
//Current direction
int currentDirection = REST;


//**Init Section

//Measuring Timer, Set for 10 microseconds
void InitTimer2();

//Sampling Timer, Set for 50ms
void InitTimer3();

//Init Timer Interrupts 
void InitNVIC();

//Init GPIO Ports
void InitGPIO_Ports();

//Init PWM used for steering engines
void InitPWM(int period, int prescaler);

//Function calling all required init functions
void InitProject();


//**Steering Section

//Set Direction on both engines to forward
void SetForward();

//Manage direction based on sensor input
void ManageDirection();

//Set speed on the chosen engine (0-70)
void SetSpeed(int speed, int engine);

//Set direction for the chosen engine (enum EngineSteering)
void Steer(int direction, int engine);


//**Sensor Section

//Resetting counter and capacitor charging flag
//Setting bits to start charging capacitors
//Used to start new detection cycle for a given sensor
void InitSensors();

//Reads value from the current sensor (indicated by variable selectedSensor) 
//and converts it to binary, selects the next sensor to get input from
void SensorSelection();

//Boolean value - whether the detected surface is black
int isBlack(int x);




void main() {
	SystemInit();
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	InitProject();

	while (true);

}

//Initialisation
void InitTimer3() {
	TIM_TimeBaseInitTypeDef Timer3;
	Timer3.TIM_Period = 419;
	Timer3.TIM_Prescaler = 9999;
	Timer3.TIM_ClockDivision = TIM_CKD_DIV1;
	Timer3.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &Timer3);
	TIM_Cmd(TIM3, ENABLE);
}

void InitTimer2() {
	TIM_TimeBaseInitTypeDef Timer2;
	Timer2.TIM_Period = 41;
	Timer2.TIM_Prescaler = 19;
	Timer2.TIM_ClockDivision = TIM_CKD_DIV1;
	Timer2.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &Timer2);
	TIM_Cmd(TIM2, ENABLE);
}

void InitNVIC() {

	//Timer3 Interrupt
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);


	//Timer2 Interrupt
	NVIC_InitTypeDef NVIC_InitStructure2;
	NVIC_InitStructure2.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure2);

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

}

void InitGPIO_Ports() {

	//Diodes for debugging
	GPIO_InitTypeDef PortD;
	PortD.GPIO_Pin =
		GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	PortD.GPIO_Mode = GPIO_Mode_OUT;
	PortD.GPIO_OType = GPIO_OType_PP;
	PortD.GPIO_Speed = GPIO_Speed_100MHz;
	PortD.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &PortD);

	//Pins used for sensor handle
	PortE.GPIO_Pin =
		GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 |
		GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 |
		GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	PortE.GPIO_Mode = GPIO_Mode_OUT;
	PortE.GPIO_OType = GPIO_OType_PP;
	PortE.GPIO_Speed = GPIO_Speed_100MHz;
	PortE.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &PortE);
}

void InitPWM(int period, int prescaler) {

	//Using 20kHz timer to steer the engines with PWM
	//Port B responsible for PWM and steering bits 
	//for EngineA (Pin5 and Pin7) and EngineB (Pin4 and Pin9)
	TIM_TimeBaseInitTypeDef Timer4;
	Timer4.TIM_Period = period;
	Timer4.TIM_Prescaler = prescaler;
	Timer4.TIM_ClockDivision = TIM_CKD_DIV1;
	Timer4.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &Timer4);
	TIM_Cmd(TIM4, ENABLE);
	GPIO_InitTypeDef PortB;
	PortB.GPIO_Pin =
		//EngineA	//PWMA		 //EngineA	  //PWMB	   //EngineB    //EngineB
		GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_4;
	PortB.GPIO_Mode = GPIO_Mode_OUT;
	PortB.GPIO_OType = GPIO_OType_PP;
	PortB.GPIO_Speed = GPIO_Speed_100MHz;
	PortB.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &PortB);

	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;


	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	//Setting CCR1 to Pin6 (PWMA)
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
	PortB.GPIO_Pin = GPIO_Pin_6;
	PortB.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOB, &PortB);


	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

	//Setting CCR3 to Pin8 (PWMB)
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM4);
	PortB.GPIO_Pin = GPIO_Pin_8;
	PortB.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOB, &PortB);
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_7 | GPIO_Pin_5);
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);
	GPIO_SetBits(GPIOB, GPIO_Pin_4);
	currentDirection = REST;
}

void InitProject() {
	//Port Initialisation
	InitGPIO_Ports();
	//Init Timer for sensor sampling
	InitTimer3();
	//Init Timer for measuring capacitor discharge
	InitTimer2();
	//Init interrupts timer interrupts
	InitNVIC();
	//Init PWM used for steering the engine 20kHz
	InitPWM(99, 41);
}


//STEERING
void SetForward() 
{ 
	Steer(Forward, Left); 
	Steer(Forward, Right); 
}

void ManageDirection() {
	if (sensorArray[4] && !(sensorArray[3] || sensorArray[5])) {
		Steer(Forward, Left);
		Steer(Forward, Right);
		SetSpeed(15, Left);
		SetSpeed(15, Right);
	}
	else if (sensorArray[3] && !sensorArray[8]) { SetForward(); SetSpeed(10, Right); SetSpeed(20, Left); }
	else if (sensorArray[5] && !sensorArray[0]) { SetForward(); SetSpeed(10, Left); SetSpeed(20, Right); }

	else if (sensorArray[2] && !sensorArray[8]) { SetForward(); SetSpeed(2, Right); SetSpeed(25, Left); }
	else if (sensorArray[6] && !sensorArray[0]) { SetForward(); SetSpeed(2, Left); SetSpeed(25, Right); }

	else if (sensorArray[0]) { Steer(Backward, Left); SetSpeed(30, Right); SetSpeed(20, Left); }
	else if (sensorArray[8]) { Steer(Backward, Right); SetSpeed(30, Left); SetSpeed(20, Right); }



}

void SetSpeed(int speed, int engine) {
	switch (engine) {
	case Left:
		//Setting duty cycle to the left engine
		TIM4->CCR1 = (speed + 25 > 95 ? 70 : speed + 25);
		break;
	case Right:
		//Setting duty cycle to the right engine
		TIM4->CCR3 = (speed + 25 > 95 ? 70 : speed + 25);
		break;
	}
}

void Steer(int direction, int engine) {
	switch (engine) {
	case Left:
		switch (direction) {
		case Forward:
			if (currentDirection == Forward)return;
			else {
				GPIO_ResetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_7);
				GPIO_SetBits(GPIOB, GPIO_Pin_7);
			}
			break;
		case Backward:
			if (currentDirection == Backward)return;
			else {
				GPIO_ResetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_7);
				GPIO_SetBits(GPIOB, GPIO_Pin_5);
			}
			break;
		case Stop:
			GPIO_ResetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_7);
			break;
		}
		break;

	case Right:
		switch (direction) {
		case Forward:
			if (currentDirection == Forward)return;
			else {
				GPIO_ResetBits(GPIOB, GPIO_Pin_4 | GPIO_Pin_9);
				GPIO_SetBits(GPIOB, GPIO_Pin_4);
			}
			break;
		case Backward:
			if (currentDirection == Backward)return;
			else {
				GPIO_ResetBits(GPIOB, GPIO_Pin_4 | GPIO_Pin_9);
				GPIO_SetBits(GPIOB, GPIO_Pin_9);
			}
			break;
		case Stop:
			GPIO_ResetBits(GPIOB, GPIO_Pin_4 | GPIO_Pin_9);
			break;
		}
		break;

	}


}


//SENSORS AND TIMERS
void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
		ManageDirection();
		//Resetting counter and capacitor charging flag
		//Setting bits to start charging capacitors
		InitSensors();
		//Set the current sensor to first
		selectedSensor = 0;
		//Enable timer responsible for measuring time of capacitor discharge
		TIM_Cmd(TIM2, ENABLE);
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}

void TIM2_IRQHandler(void) {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		//If 20 microseconds passed the capacitors are fully charged
		//Changing Pins mode to input
		if (counter == 2) {
			PortE.GPIO_Mode = GPIO_Mode_IN;
			PortE.GPIO_OType = GPIO_OType_OD;
			PortE.GPIO_Speed = GPIO_Speed_100MHz;
			PortE.GPIO_PuPd = GPIO_PuPd_NOPULL;
			GPIO_Init(GPIOE, &PortE);
			charging = 0;
		}
		counter++;
		//When capacitor is charged, check when input bit on the current sensor switches to 0
		if (counter > 2)
			if (!charging)
				SensorSelection();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	}
}

//Left-To-Right
/*
 0-1-2-3			-5- 6 -7 -8
 8-7-9-6- MID>4<MID -5-10-12-11
 */
void SensorSelection() 
{
	//Managing the input from the current sensor
	//Converting the value of counter to binary (0 = white, 1 = black) 
	//for each sensor indicated by selectedSensor
	//Disables the timer after last value is read
	switch (selectedSensor) 
	{
	case 0:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0) {
			if (isBlack(counter))sensorArray[4] = 1;
			else sensorArray[4] = 0;
			deb5 = sensorArray[4];
			selectedSensor = 1;
			InitSensors();

		}
		break;
	case 1:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_6) == 0) {
			if (isBlack(counter))sensorArray[3] = 1;
			else sensorArray[3] = 0;
			deb4 = sensorArray[3];
			selectedSensor = 2;

			InitSensors();
		}
		break;
	case 2:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5) == 0) {
			if (isBlack(counter))sensorArray[5] = 1;
			else sensorArray[5] = 0;
			deb6 = sensorArray[5];
			selectedSensor = 3;
			InitSensors();
		}

		break;
	case 3:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_9) == 0) {
			if (isBlack(counter))sensorArray[2] = 1;
			else sensorArray[2] = 0;
			deb3 = sensorArray[2];
			selectedSensor = 4;
			InitSensors();
		}
		break;
	case 4:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_10) == 0) {
			if (isBlack(counter))sensorArray[6] = 1;
			else sensorArray[6] = 0;
			deb7 = sensorArray[6];
			selectedSensor = 7;
			InitSensors();
		}
		break;
	case 5:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_7) == 0) {
			if (isBlack(counter))sensorArray[1] = 1;
			else sensorArray[1] = 0;
			deb2 = counter;
			selectedSensor = 6;
			InitSensors();
		}
		break;
	case 6:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_12) == 0) {
			if (isBlack(counter))sensorArray[7] = 1;
			else sensorArray[7] = 0;
			deb8 = sensorArray[7];
			selectedSensor = 7;
			InitSensors();
		}
		break;
	case 7:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_8) == 0) {
			if (isBlack(counter))sensorArray[0] = 1;
			else sensorArray[0] = 0;
			deb1 = sensorArray[0];
			selectedSensor = 8;
			InitSensors();
		}
		break;
	case 8:
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11) == 0) {
			if (isBlack(counter))sensorArray[8] = 1;
			else sensorArray[8] = 0;
			deb9 = sensorArray[8];
			selectedSensor = 0;
			TIM_Cmd(TIM2, DISABLE);
		}
		break;
	default:
		TIM_Cmd(TIM2, DISABLE);
		break;
	}

}

void InitSensors() {
	//Set GPIO Mode to output
	PortE.GPIO_Mode = GPIO_Mode_OUT;
	PortE.GPIO_OType = GPIO_OType_PP;
	PortE.GPIO_Speed = GPIO_Speed_100MHz;
	PortE.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &PortE);
	//Reset capacitor charging flag
	charging = 1;
	//Reset counter
	counter = 0;
	//Set Bits to 1 and start charging the capacitors
	GPIO_SetBits(GPIOE,
		GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 |
		GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 |
		GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12
	);
}

int isBlack(x) 
{
	return x > 15; 
}


