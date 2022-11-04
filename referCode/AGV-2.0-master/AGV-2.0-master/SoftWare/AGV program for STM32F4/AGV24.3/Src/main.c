/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "stdio.h"
#include "i2c-lcd.h"
#include "motor.h"

#define goal 5000      								// PID's goal
#define MAX_PWM 5000					 				// Set maximum speed
#define MIN_PWM 50						 				// Set minimum speed
#define Low_battery 100			 				// adc value at battery-info port when it is low at 3V
#define Full_battery  3000
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
volatile uint16_t adc1[9];
volatile bool line[9];
volatile uint32_t adc3=0;
volatile uint8_t start = 4;
volatile uint32_t sensor_time = 0;
volatile uint16_t local_time = 0;
volatile int16_t error = 0;
volatile uint16_t lastError = 0;
volatile uint16_t totalError = 0;
volatile uint32_t adjustment = 0;
volatile int16_t totalLine = 0;
volatile int numLine = 0;
volatile uint16_t lineValue = 0;
volatile int16_t max[9], min[9];
volatile uint16_t maxLine=0 , minLine=0;
char LCD[30];
volatile char receiver[50],result[10],lcd[30],getIP[20],getIPResult[20];
char resultcpy[10];
volatile uint16_t speed = 0,KD=0,KI=0,KP=0;
volatile uint8_t a=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC3_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_UART4_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
//----------------------------------1us delay & get sonar sensor value-------------------------------------//                
void delay ( uint32_t us){
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while( __HAL_TIM_GET_COUNTER(&htim2) < us);
}
uint32_t getSonar (void) {
	local_time = 0;
	HAL_GPIO_WritePin(Trig_sonar_GPIO_Port,Trig_sonar_Pin,0);
	delay(2);
	HAL_GPIO_WritePin(Trig_sonar_GPIO_Port,Trig_sonar_Pin,1);
	delay(10);
	HAL_GPIO_WritePin(Trig_sonar_GPIO_Port,Trig_sonar_Pin,0);
	while(!(HAL_GPIO_ReadPin(Echo_sonar_GPIO_Port, Echo_sonar_Pin)));
	while(HAL_GPIO_ReadPin(Echo_sonar_GPIO_Port, Echo_sonar_Pin)){
		local_time++;
		delay(1);
	}
return local_time;
}
//---------------------------------------------------------------------------------------------------------//

//---------------------------------------------Line Sensors------------------------------------------------//
void LineSensor(void){
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1,9);
	HAL_Delay(5);
	HAL_ADC_Stop_DMA(&hadc1);
}
//---------------------------------------------------------------------------------------------------------//
		
//------------------------------------------PID calculation------------------------------------------------//
uint32_t PID (void){
	LineSensor();
	for(int i=0;i<=8;i++){
		if(adc1[i] <= lineValue){
			line[i]=0;
		} else line[i]=1;
	}
 for(int i=0 ; i<=8 ; i++){
	if(adc1[i] >= lineValue){
		line[i] = 1;
	} else line[i] = 0;
 }
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

//--------------------------------------------ADCs Interruption--------------------------------------------//
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance==hadc3.Instance){
		adc3=HAL_ADC_GetValue(&hadc3);
			if(adc3 <= Low_battery){
				start =0;
			}
	}
	HAL_ADC_Start_IT(&hadc3);
}
//---------------------------------------------------------------------------------------------------------//

//---------------------------------------External Interrupt------------------------------------------------//
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	HAL_UART_AbortReceive_IT(&huart4);		//Turn off UART Interrupt
	if (GPIO_Pin == IT_READY_Pin){				//get values of line
		lineValue=0;
		maxLine=0;
		minLine=0;
		for(int i=0;i<=8;i++){
			max[i]=0;
			min[i]=0;
		}
		HAL_ADC_Stop_DMA(&hadc1);
		HAL_ADC_Stop_IT(&hadc3);
		start=2;
		sprintf(LCD,"  Getting Ready  ");
	}
	else if(GPIO_Pin == IT_START_Pin){ //start
		start = 1;
		HAL_ADC_Start_IT(&hadc3);
		lcd_goto_XY(1,0);
		lcd_send_string("    LamTran        ");		 
		lcd_goto_XY(2,0);
		lcd_send_string("    Running        ");
		HAL_UART_Receive_IT(&huart4,(uint8_t*)receiver,1);
	}
	else if(GPIO_Pin == IT_PAUSE_Pin) //stop
  {
		HAL_ADC_Stop_IT(&hadc3);
    start = 3;
		HAL_GPIO_WritePin(Running_light_GPIO_Port, Running_light_Pin, 1);
		HAL_GPIO_WritePin(stop_light_GPIO_Port, stop_light_Pin, 1);
	} 
	else if(GPIO_Pin == IT_CHARGE_INFO_Pin){   //Charging
		start = 6;
	}
	else if(GPIO_Pin == IT_BLOCK_Pin | GPIO_Pin == IT_RCWL_0516_Pin){    // Block
		start = 7;
	}
}
//----------------------------------------------------------------------------------------------//

//---------------------------------------UART Interrupt-----------------------------------------//
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
 if(huart->Instance == UART4){
	 HAL_ADC_Stop_IT(&hadc3);
	 HAL_ADC_Stop_DMA(&hadc1);
		for (int i = 0; i < 10; i++) {
			resultcpy[i] = 0;
			result[i] = 0;
		}
		HAL_UART_Receive(&huart4,(uint8_t*)receiver,50,300);
		HAL_ADC_Start_IT(&hadc3);
		start =5;
	}
}
//----------------------------------------------------------------------------------------------//

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

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
  MX_ADC3_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_UART4_Init();

  /* USER CODE BEGIN 2 */
	HAL_ADC_Start_IT(&hadc3);
	lcd_init();
	lcd_goto_XY(1,0);
	lcd_send_string("    Welcome   ");
	lcd_goto_XY(2,0);
	lcd_send_string("  AGV Project");
	HAL_Delay(3000);
	lcd_clear_display();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		/*Main processing loop-----------------------------------------------------------------------*/
	 while(start == 1){
		PID();
		HAL_GPIO_TogglePin(led_GPIO_Port,led_Pin);
	 }
	/*--------------------------------------------------------------------------------------------*/ 
	 
	/*Low battery alarm using ADC interrupt loop--------------------------------------------------*/
	 while(start == 0){
		HAL_GPIO_WritePin(stop_light_GPIO_Port,stop_light_Pin,1);
		HAL_GPIO_WritePin(led_GPIO_Port,led_Pin,0);
		lcd_goto_XY(1,0);
		lcd_send_string("  AGV_BKHN_Lam  ");
		lcd_goto_XY(2,0);
		lcd_send_string("    PIN YEU!   ");
	   HAL_Delay(500);
		 lcd_clear_display();
		 HAL_Delay(500);
	 }
	/*--------------------------------------------------------------------------------------------*/
	 
	/*Reading line value loop---------------------------------------------------------------------*/ 
	 while(start == 2){
		lcd_goto_XY(1,0);
		lcd_send_string("  AGV_BKHN_Lam  ");
		lcd_goto_XY(2,0);
		lcd_send_string(LCD);
		while(lineValue == 0){
			HAL_GPIO_WritePin(Running_light_GPIO_Port,Running_light_Pin,1);
			HAL_GPIO_WritePin(stop_light_GPIO_Port,stop_light_Pin,0);
			lineValue = 0;
			motorLeftBackward(4000);
			HAL_Delay(300);

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
			HAL_Delay(300);
			
			motorLeftBackward(4000);
			HAL_Delay(300);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
			sprintf(LCD,"    Ready :)     ");
			start=2;
			}	
			HAL_Delay(1000);
	}
	 /*--------------------------------------------------------------------------------------------*/
	
	/*Pause loop-----------------------------------------------------------------------------------*/
	 	while(start == 3){
		lcd_goto_XY(1,0);
		lcd_send_string("  AGV_BKHN_Lam  ");
		lcd_goto_XY(2,0);
		lcd_send_string("     Pause   ");
		 HAL_Delay(500);
		 lcd_clear_display();
		 HAL_Delay(500);
	 }
	/*--------------------------------------------------------------------------------------------*/
	 
	/*Start up and print out IP adress loop-------------------------------------------------------*/
		while(start==4){
			HAL_GPIO_WritePin(GetIP_GPIO_Port,GetIP_Pin,0);
			HAL_UART_Receive(&huart4,(uint8_t*)getIP,20,1000);
			for(int i=0;i<=20;i++){
				if(getIP[i] != 13){
					getIPResult[i]=getIP[i];
				}
			}
			lcd_goto_XY(1,0);
			lcd_send_string("  Press Ready    ");
			lcd_goto_XY(2,0);
			lcd_send_string(" ");
			lcd_goto_XY(2,1);
			lcd_send_string(getIPResult);
		}
		/*--------------------------------------------------------------------------------------------*/
		
		/*Changing factors receiving loop--------------------------------------------------------------*/
		while(start==5){
			HAL_GPIO_WritePin(led_GPIO_Port,led_Pin,0);
        if (receiver[49] != 0) {
            for (int i = 0; i < 50; i++) {
                receiver[i] = 0;
            }
        }
        for (int i = 0; i <= 50; i++) {
            if (receiver[i] == ':') {

                result[0] = receiver[i + 4];
                resultcpy[0] = receiver[i + 4];
                result[1] = receiver[i + 5];
                resultcpy[1] = receiver[i + 5];
                result[2] = receiver[i + 6];
                resultcpy[2] = receiver[i + 6];
                result[3] = receiver[i + 7];
                resultcpy[3] = receiver[i + 7];

                if (receiver[i - 1] == ' ') {
                    a = 1;
                }
                else if (receiver[i - 1] == 'D') {
                    a = 2;
                }
                else if (receiver[i - 1] == 'I') {
                    a = 3;
                }
                else if (receiver[i - 1] == 'P') {
                    a = 4;
                }
                else a = 0;
            }
        }
        if (a == 1) {
            if (atoi(resultcpy) != 0) {
                speed = atoi(resultcpy);
								lcd_init();
                sprintf(lcd, "  speed: %d       ", speed);
								lcd_goto_XY(1,0);
								lcd_send_string("    LamTran        ");							
                lcd_goto_XY(2, 0);
                lcd_send_string(lcd);
								HAL_Delay(1000);
								lcd_goto_XY(1,0);
								lcd_send_string("    LamTran         ");		 
								lcd_goto_XY(2,0);
								lcd_send_string("    Running         ");
            }
        }
        else if (a == 2) {
            if (atoi(resultcpy) != 0) {
                KD = atoi(resultcpy);
                sprintf(lcd, "    KD: %d           ", KD);
								lcd_goto_XY(1,0);
								lcd_send_string("    LamTran        ");							
								lcd_init();
                lcd_goto_XY(2, 0);
                lcd_send_string(lcd);
								HAL_Delay(1000);
								lcd_goto_XY(1,0);
								lcd_send_string("    LamTran        ");		 
								lcd_goto_XY(2,0);
								lcd_send_string("    Running        ");
            }
        }
        else if (a == 3) {
            if (atoi(resultcpy) != 0) {
                KI = atoi(resultcpy);
								lcd_init();
                sprintf(lcd, "    KI: %d           ", KI);
								lcd_goto_XY(1,0);
								lcd_send_string("    LamTran       ");							
                lcd_goto_XY(2, 0);
                lcd_send_string(lcd);
								HAL_Delay(1000);
								lcd_goto_XY(1,0);
								lcd_send_string("    LamTran       ");		 
								lcd_goto_XY(2,0);
								lcd_send_string("    Running       ");							
            }
        }
        else if (a == 4) {
            if (atoi(resultcpy) != 0) {
                KP = atoi(resultcpy);
								lcd_init();
                sprintf(lcd, "    KP: %d            ", KP);
								lcd_goto_XY(1,0);
								lcd_send_string("    LamTran   ");							
                lcd_goto_XY(2, 0);
                lcd_send_string(lcd);
								HAL_Delay(1000);
								lcd_goto_XY(1,0);
								lcd_send_string("    LamTran      ");		 
								lcd_goto_XY(2,0);
								lcd_send_string("    Running      ");							
            }
        }
				HAL_UART_Receive_IT(&huart4,(uint8_t*)receiver,1);
				HAL_ADC_Start_IT(&hadc3);
				start=1;
		}
		/*--------------------------------------------------------------------------------------------*/
		
		/*Charging loop-------------------------------------------------------------------------------*/
		while(start==6){
			HAL_ADC_Start_IT(&hadc3);
			adc3 = HAL_ADC_GetValue(&hadc3);
			if(adc3 <= Full_battery){
				HAL_GPIO_WritePin(CHARGE_CTRL_GPIO_Port,CHARGE_CTRL_Pin,1);
				lcd_goto_XY(2,0);
				lcd_send_string("  Charging...");
			}			else {
					HAL_GPIO_WritePin(CHARGE_CTRL_GPIO_Port,CHARGE_CTRL_Pin,0);
					lcd_goto_XY(2,0);
					lcd_send_string(" Fully charge!");
				}	
		 if(HAL_GPIO_ReadPin(IT_CHARGE_INFO_GPIO_Port,IT_CHARGE_INFO_Pin)==0){
				adc3 = HAL_ADC_GetValue(&hadc3);
				start=3;
				lcd_goto_XY(1,0);
				lcd_send_string("    LamTran       ");		 
				lcd_goto_XY(2,0);
				lcd_send_string("    Running       ");
			}
		}
	/*--------------------------------------------------------------------------------------------*/
	
	/*Block loop----------------------------------------------------------------------------------*/
		while(start==7){
			if(HAL_GPIO_ReadPin(IT_BLOCK_GPIO_Port,IT_BLOCK_Pin)==1 | HAL_GPIO_ReadPin(IT_RCWL_0516_GPIO_Port,IT_RCWL_0516_Pin)==1){
				lcd_goto_XY(1,0);
				lcd_send_string("    LamTran       ");	
				lcd_goto_XY(2,0);
				lcd_send_string("    Block!!!!    ");
			} else {
					start =1;
					lcd_goto_XY(1,0);
					lcd_send_string("    LamTran       ");		 
					lcd_goto_XY(2,0);
					lcd_send_string("    Running       ");				
				}
		}
	/*--------------------------------------------------------------------------------------------*/
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 9;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 9;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* ADC3 init function */
static void MX_ADC3_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
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
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xffff-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 72-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 0xffff-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);

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
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, CHARGE_CTRL_Pin|Trig_sonar_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, stop_light_Pin|Running_light_Pin|Torch_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GetIP_GPIO_Port, GetIP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : IT_START_Pin */
  GPIO_InitStruct.Pin = IT_START_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(IT_START_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IT_PAUSE_Pin IT_READY_Pin IT_RCWL_0516_Pin IT_CHARGE_INFO_Pin */
  GPIO_InitStruct.Pin = IT_PAUSE_Pin|IT_READY_Pin|IT_RCWL_0516_Pin|IT_CHARGE_INFO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : IT_BLOCK_Pin */
  GPIO_InitStruct.Pin = IT_BLOCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(IT_BLOCK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CHARGE_CTRL_Pin */
  GPIO_InitStruct.Pin = CHARGE_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CHARGE_CTRL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Trig_sonar_Pin */
  GPIO_InitStruct.Pin = Trig_sonar_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(Trig_sonar_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Echo_sonar_Pin */
  GPIO_InitStruct.Pin = Echo_sonar_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Echo_sonar_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : stop_light_Pin Running_light_Pin Torch_Pin */
  GPIO_InitStruct.Pin = stop_light_Pin|Running_light_Pin|Torch_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : led_Pin */
  GPIO_InitStruct.Pin = led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(led_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GetIP_Pin */
  GPIO_InitStruct.Pin = GetIP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GetIP_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
