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

#define Running_light_Pin GPIO_PIN_13
#define Running_light_GPIO_Port GPIOC
#define BatInfor_Pin GPIO_PIN_0
#define BatInfor_GPIO_Port GPIOC
#define LINE1_Pin GPIO_PIN_0
#define LINE1_GPIO_Port GPIOA
#define LINE2_Pin GPIO_PIN_1
#define LINE2_GPIO_Port GPIOA
#define LINE3_Pin GPIO_PIN_2
#define LINE3_GPIO_Port GPIOA
#define LINE4_Pin GPIO_PIN_3
#define LINE4_GPIO_Port GPIOA
#define LINE5_Pin GPIO_PIN_4
#define LINE5_GPIO_Port GPIOA
#define LINE6_Pin GPIO_PIN_5
#define LINE6_GPIO_Port GPIOA
#define LINE7_Pin GPIO_PIN_6
#define LINE7_GPIO_Port GPIOA
#define LINE8_Pin GPIO_PIN_7
#define LINE8_GPIO_Port GPIOA
#define LINE9_Pin GPIO_PIN_0
#define LINE9_GPIO_Port GPIOB
#define CHARGE_CTRL_Pin GPIO_PIN_1
#define CHARGE_CTRL_GPIO_Port GPIOB
#define LCD_SCL_Pin GPIO_PIN_10
#define LCD_SCL_GPIO_Port GPIOB
#define LCD_SDA_Pin GPIO_PIN_11
#define LCD_SDA_GPIO_Port GPIOB
#define Buzzer_Pin GPIO_PIN_12
#define Buzzer_GPIO_Port GPIOB
#define GetIP_Pin GPIO_PIN_14
#define GetIP_GPIO_Port GPIOB
#define PWM_LF_Pin GPIO_PIN_6
#define PWM_LF_GPIO_Port GPIOC
#define PWM_LB_Pin GPIO_PIN_7
#define PWM_LB_GPIO_Port GPIOC
#define PWM_RF_Pin GPIO_PIN_8
#define PWM_RF_GPIO_Port GPIOC
#define PWM_RB_Pin GPIO_PIN_9
#define PWM_RB_GPIO_Port GPIOC
#define Torch_Pin GPIO_PIN_9
#define Torch_GPIO_Port GPIOA
#define Trig_sonar_Pin GPIO_PIN_11
#define Trig_sonar_GPIO_Port GPIOA
#define Echo_sonar_Pin GPIO_PIN_15
#define Echo_sonar_GPIO_Port GPIOA
#define ESP_TX_Pin GPIO_PIN_10
#define ESP_TX_GPIO_Port GPIOC
#define ESP_RX_Pin GPIO_PIN_11
#define ESP_RX_GPIO_Port GPIOC
#define led_Pin GPIO_PIN_12
#define led_GPIO_Port GPIOC
#define IT_START_Pin GPIO_PIN_3
#define IT_START_GPIO_Port GPIOB
#define IT_START_EXTI_IRQn EXTI3_IRQn
#define IT_PAUSE_Pin GPIO_PIN_4
#define IT_PAUSE_GPIO_Port GPIOB
#define IT_PAUSE_EXTI_IRQn EXTI4_IRQn
#define IT_READY_Pin GPIO_PIN_5
#define IT_READY_GPIO_Port GPIOB
#define IT_READY_EXTI_IRQn EXTI9_5_IRQn
#define stop_light_Pin GPIO_PIN_6
#define stop_light_GPIO_Port GPIOB
#define RCWL_Pin GPIO_PIN_7
#define RCWL_GPIO_Port GPIOB
#define IT_BLOCK_Pin GPIO_PIN_8
#define IT_BLOCK_GPIO_Port GPIOB
#define IT_BLOCK_EXTI_IRQn EXTI9_5_IRQn
#define IT_CHARGE_INFO_Pin GPIO_PIN_9
#define IT_CHARGE_INFO_GPIO_Port GPIOB
#define IT_CHARGE_INFO_EXTI_IRQn EXTI9_5_IRQn

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
