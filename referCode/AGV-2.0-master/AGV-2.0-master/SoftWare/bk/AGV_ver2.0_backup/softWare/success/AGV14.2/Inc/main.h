/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define Batt_Pin GPIO_PIN_0
#define Batt_GPIO_Port GPIOC
#define ChargeInfo_Pin GPIO_PIN_1
#define ChargeInfo_GPIO_Port GPIOC
#define ChargeControl_Pin GPIO_PIN_2
#define ChargeControl_GPIO_Port GPIOC
#define Line_1_Pin GPIO_PIN_0
#define Line_1_GPIO_Port GPIOA
#define Line_2_Pin GPIO_PIN_1
#define Line_2_GPIO_Port GPIOA
#define Line_3_Pin GPIO_PIN_2
#define Line_3_GPIO_Port GPIOA
#define Line_4_Pin GPIO_PIN_3
#define Line_4_GPIO_Port GPIOA
#define Line_5_Pin GPIO_PIN_4
#define Line_5_GPIO_Port GPIOA
#define Line_6_Pin GPIO_PIN_5
#define Line_6_GPIO_Port GPIOA
#define Line_7_Pin GPIO_PIN_6
#define Line_7_GPIO_Port GPIOA
#define Line_8_Pin GPIO_PIN_7
#define Line_8_GPIO_Port GPIOA
#define Line_9_Pin GPIO_PIN_0
#define Line_9_GPIO_Port GPIOB
#define TiemCan_Pin GPIO_PIN_1
#define TiemCan_GPIO_Port GPIOB
#define PWM_3_Pin GPIO_PIN_10
#define PWM_3_GPIO_Port GPIOB
#define PWM_4_Pin GPIO_PIN_11
#define PWM_4_GPIO_Port GPIOB
#define Sonar_Trig_1_Pin GPIO_PIN_12
#define Sonar_Trig_1_GPIO_Port GPIOB
#define Sonar_Trig_2_Pin GPIO_PIN_13
#define Sonar_Trig_2_GPIO_Port GPIOB
#define Sonar_Echo_1_Pin GPIO_PIN_14
#define Sonar_Echo_1_GPIO_Port GPIOB
#define Sonar_Echo_2_Pin GPIO_PIN_15
#define Sonar_Echo_2_GPIO_Port GPIOB
#define Encoder_1_Pin GPIO_PIN_6
#define Encoder_1_GPIO_Port GPIOC
#define Encoder_2_Pin GPIO_PIN_7
#define Encoder_2_GPIO_Port GPIOC
#define RunningLight_Pin GPIO_PIN_8
#define RunningLight_GPIO_Port GPIOC
#define StoppingLight_Pin GPIO_PIN_9
#define StoppingLight_GPIO_Port GPIOC
#define StartButt_Pin GPIO_PIN_10
#define StartButt_GPIO_Port GPIOA
#define StopButt_Pin GPIO_PIN_11
#define StopButt_GPIO_Port GPIOA
#define GetReadyButt_Pin GPIO_PIN_12
#define GetReadyButt_GPIO_Port GPIOA
#define PWM_1_Pin GPIO_PIN_15
#define PWM_1_GPIO_Port GPIOA
#define Esp_Tx_Pin GPIO_PIN_10
#define Esp_Tx_GPIO_Port GPIOC
#define Esp_Rx_Pin GPIO_PIN_11
#define Esp_Rx_GPIO_Port GPIOC
#define PWM_2_Pin GPIO_PIN_3
#define PWM_2_GPIO_Port GPIOB
#define Encoder_3_Pin GPIO_PIN_6
#define Encoder_3_GPIO_Port GPIOB
#define Encoder_4_Pin GPIO_PIN_7
#define Encoder_4_GPIO_Port GPIOB
#define LCD_SCL_Pin GPIO_PIN_8
#define LCD_SCL_GPIO_Port GPIOB
#define LCD_SDA_Pin GPIO_PIN_9
#define LCD_SDA_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
