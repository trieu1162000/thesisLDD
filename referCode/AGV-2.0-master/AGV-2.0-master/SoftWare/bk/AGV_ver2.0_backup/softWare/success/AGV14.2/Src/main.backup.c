//-----------------------------------------------------BKHN------------------------------------------------//
//---------------------------------------------------AGV_Ver_2---------------------------------------------//
//---------------------------------------------------Lam_Tran----------------------------------------------//
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stdbool.h"
#include "stdio.h"
#include "i2c-lcd.h"
#include "motor.h"

#define Safety_distance 2000	 				// adc value 
#define Low_battery 1000			 				// adc value at battery-info port when it is low at 3V
#define Charging_info 2000     				// adc value at charging port
#define Full_battery  2000		 				// adc value at battery-info port when it is full at 4.2V
#define goal 5000							 				// PID's goal
#define KP 0.1								 				// PID's KP
#define KD 0.1								 				// PID's KD
#define KI 0.1								 				// PID's KI
#define MAX_PWM 5000					 				// Set maximum speed
#define MIN_PWM 50						 				// Set minimum speed
#define leftEncoderPerRevolution 34 	//number of Leftendcoder(timer3) pulses per revolution
#define rightEncoderPerRevolution 34  //number of Rightendcoder(timer4) pulses per revolution

//-----------Global	Variables-----------//
volatile uint16_t adc1[12];
volatile bool line[9];
bool start = 0;
volatile uint32_t sensor_time_1 = 0;
volatile uint32_t local_time_1 = 0;
volatile uint32_t sensor_time_2 = 0;
volatile uint32_t local_time_2 = 0;
volatile int16_t error = 0;
volatile uint16_t lastError = 0;
volatile uint16_t totalError = 0;
volatile uint32_t adjustment = 0;
volatile int16_t totalLine = 0;
volatile int numLine = 0;
volatile uint32_t count_1 = 0;
volatile uint32_t count_2 = 0;
volatile uint16_t lineValue = 0;
volatile int16_t max[9], min[9];
volatile uint16_t maxLine=0 , minLine=0;
//--------------------------------------//

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim3_ch4_up;
DMA_HandleTypeDef hdma_tim4_up;

UART_HandleTypeDef huart4;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_uart4_tx;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_I2C1_Init(void);
static void MX_UART4_Init(void);
static void MX_TIM4_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

//----------------------------------1us delay & get sonar sensor value--------------------------------------//                
void delay ( uint32_t us){
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	while( __HAL_TIM_GET_COUNTER(&htim3) < us);
}
uint32_t getSonar_1 (void) {
	local_time_1 = 0;
	HAL_GPIO_WritePin(Sonar_Trig_1_GPIO_Port,Sonar_Trig_1_Pin,GPIO_PIN_RESET);
	delay(2);
	HAL_GPIO_WritePin(Sonar_Trig_1_GPIO_Port,Sonar_Trig_1_Pin,GPIO_PIN_SET);
	delay(10);
	HAL_GPIO_WritePin(Sonar_Trig_1_GPIO_Port,Sonar_Trig_1_Pin,GPIO_PIN_RESET);
	while(!(HAL_GPIO_ReadPin(Sonar_Echo_1_GPIO_Port, Sonar_Echo_1_Pin)));
	while(HAL_GPIO_ReadPin(Sonar_Echo_1_GPIO_Port, Sonar_Echo_1_Pin)){
	local_time_1++;
	delay(1);
}
return local_time_1;
}

uint32_t getSonar_2 (void) {
	local_time_2 = 0;
	HAL_GPIO_WritePin(Sonar_Trig_2_GPIO_Port,Sonar_Trig_2_Pin,GPIO_PIN_RESET);
	delay(2);
	HAL_GPIO_WritePin(Sonar_Trig_2_GPIO_Port,Sonar_Trig_2_Pin,GPIO_PIN_SET);
	delay(10);
	HAL_GPIO_WritePin(Sonar_Trig_2_GPIO_Port,Sonar_Trig_2_Pin,GPIO_PIN_RESET);
	while(!(HAL_GPIO_ReadPin(Sonar_Echo_2_GPIO_Port, Sonar_Echo_2_Pin)));
	while(HAL_GPIO_ReadPin(Sonar_Echo_2_GPIO_Port, Sonar_Echo_2_Pin)){
	local_time_2++;
	delay(1);
}
return local_time_2;
}
//---------------------------------------------------------------------------------------------------------//

//---------------------------------------------------adc1--------------------------------------------------//
void adc (void){
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1, 12);
	HAL_Delay(5);
	HAL_ADC_Stop_DMA(&hadc1);
}
//---------------------------------------------------------------------------------------------------------//

//---------------------------------------------Line Sensors------------------------------------------------//
void LineSensor(void){
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc1,sizeof(adc1));
HAL_Delay(5);
HAL_ADC_Stop_DMA(&hadc1);
}
//---------------------------------------------------------------------------------------------------------//

//---------------------------------------------Encoder-----------------------------------------------------//
void Endcoder(void) {
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_1 | TIM_CHANNEL_2);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1 | TIM_CHANNEL_2);
	count_1 = __HAL_TIM_GET_COUNTER(&htim3) / leftEncoderPerRevolution;
	count_2 = __HAL_TIM_GET_COUNTER(&htim4) / rightEncoderPerRevolution;
	HAL_Delay(5);
	HAL_TIM_Encoder_Stop(&htim3, TIM_CHANNEL_1 | TIM_CHANNEL_2);
	HAL_TIM_Encoder_Stop(&htim4, TIM_CHANNEL_1 | TIM_CHANNEL_2);
}
//---------------------------------------------------------------------------------------------------------//

//------------------------------------------PID calculation------------------------------------------------//
uint32_t PID (void){
 error = 0;
 totalLine = 0;
 numLine = 0;
	for (int i = 0; i<9; i++){
		if (line[i] == 1){
			totalLine += 1000*(i+1);
			numLine++;
		}
	}
	error = goal - (totalLine / numLine);
	lastError = error;
	totalError += error;
	adjustment = KP*error+KD*(error-lastError)+KI*totalError;
	return adjustment;
}
//---------------------------------------------------------------------------------------------------------//

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_UART4_Init();
  MX_TIM4_Init();
	HAL_ADC_Start_IT(&hadc1);

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 while(start){
		 
		 
		HAL_ADC_Start_IT(&hadc1);
		}
	}
}

//----------------------------ADCs Interruption---------------------------//
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	
	if(hadc->Instance==hadc1.Instance){                
		if(adc1[8] >= Safety_distance){												// Block Interrupt
				start = 0;
				HAL_GPIO_WritePin(StoppingLight_GPIO_Port, StoppingLight_Pin, GPIO_PIN_SET);
				//HAL_GPIO_WritePin(RunningLight_GPIO_Port, RunningLight_Pin, GPIO_PIN_SET);
				//lcd code
		}
	
		else if(adc1[9] >= Low_battery){                			// Low battery Interrupt
				start = 0;
				HAL_GPIO_WritePin(StoppingLight_GPIO_Port, StoppingLight_Pin, GPIO_PIN_SET);
				//lcd code
		} 
	
		else if(adc1[10] >= Charging_info & adc1[10] < Full_battery){  // Charging Interrupt
				start = 0;
				HAL_GPIO_WritePin(StoppingLight_GPIO_Port, StoppingLight_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(ChargeControl_GPIO_Port, ChargeControl_Pin, GPIO_PIN_SET);
				//lcd code
		}
	
		else if(adc1[10] >= Charging_info & adc1[10] >= Full_battery){  // Full Charging Interrupt
				start = 0;
				HAL_GPIO_WritePin(StoppingLight_GPIO_Port, StoppingLight_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(StoppingLight_GPIO_Port, StoppingLight_Pin, GPIO_PIN_SET);
				//lcd code
		}
		else {
		start = 1;
		HAL_GPIO_WritePin(StoppingLight_GPIO_Port, StoppingLight_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(StoppingLight_GPIO_Port, StoppingLight_Pin, GPIO_PIN_SET);
		HAL_ADC_Stop_IT(&hadc1);
		}
	
}
}
//----------------------------------------------------------------------------------------------//

//---------------------------------External Interrupt-------------------------------------------//
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GetReadyButt_Pin){				//get values of line
		Endcoder();
		HAL_GPIO_WritePin(RunningLight_GPIO_Port,RunningLight_Pin,1);
		HAL_GPIO_WritePin(StoppingLight_GPIO_Port,StoppingLight_Pin,0);
		lineValue = 0;
		while(count_1 < 1){
			motorLeftBackward(4000);
		}
		count_1 = 0;
		while(count_1 < 2){
			motorLeftForward(4000);
			LineSensor();
			for(int i = 0; i <= 8;i++){
				LineSensor();
				if(adc1[i]>=max[i]){
					max[i] = adc1[i];
				}
			}
			for(int i = 0; i <= 8;i++){
				min[i]=5000;
			} 
			for(int i = 0; i <= 8;i++){
				LineSensor();
				if(adc1[i]<=min[i]){
					min[i] = adc1[i];
				}
			}
			for( int i = 0; i<= 8; i++){
				maxLine +=  max[i];
			}
			for( int i = 0; i<= 6; i++){
				minLine += min[i];
			}
			lineValue = ((maxLine/7)+(minLine/7))/2;
		}
		count_1 = 0;
		while(count_1 < 1){
			motorLeftBackward(4000);
		}
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
	}
	if(GPIO_Pin == StartButt_Pin){ //start
		start = 1;
	}
	if(GPIO_Pin == StopButt_Pin) //stop
  {
    start = 0;
		HAL_GPIO_WritePin(RunningLight_GPIO_Port, RunningLight_Pin, 1);
		HAL_GPIO_WritePin(StoppingLight_GPIO_Port, StoppingLight_Pin, 1);
	}
}


//----------------------------------------------------------------------------------------------//

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 12;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 9;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 10;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 11;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 12;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM2 init function */
//-----------------------timer for PWM--------------------------------//
//formular is UEV(update event)=Tim_CLK/((PSC+1)*(ARR+1)*(RCR+1)
//control motor drivers at 15kHz
//MAX_PWM speed is set at 5000
//Thus PSC is 1-1 when RCR is disable.
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xffff-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim2);

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_Encoder_InitTypeDef sConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 72-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 0xffff-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12; 											// select encoder 1 mode
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM4 init function */
static void MX_TIM4_Init(void)
{

  TIM_Encoder_InitTypeDef sConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 72-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 0xffff-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;											// select encoder 2 mode
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* UART4 init function */
static void MX_UART4_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
  /* DMA2_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);
  /* DMA2_Channel4_5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel4_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel4_5_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ChargeControl_Pin|RunningLight_Pin|StoppingLight_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Sonar_Trig_1_Pin|Sonar_Trig_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ChargeControl_Pin RunningLight_Pin StoppingLight_Pin */
  GPIO_InitStruct.Pin = ChargeControl_Pin|RunningLight_Pin|StoppingLight_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Sonar_Trig_1_Pin Sonar_Trig_2_Pin */
  GPIO_InitStruct.Pin = Sonar_Trig_1_Pin|Sonar_Trig_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Sonar_Echo_1_Pin Sonar_Echo_2_Pin */
  GPIO_InitStruct.Pin = Sonar_Echo_1_Pin|Sonar_Echo_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : StartButt_Pin StopButt_Pin GetReadyButt_Pin */
  GPIO_InitStruct.Pin = StartButt_Pin|StopButt_Pin|GetReadyButt_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

